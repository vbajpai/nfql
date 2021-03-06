/*
 * Copyright 2012 Vaibhav Bajpai <contact@vaibhavbajpai.com>
 * Copyright 2011 Johannes 'josch' Schauer <j.schauer@email.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE

#include "merger.h"

#ifdef MERGER

struct merger_result*
merger(
       size_t num_merger_clauses,
       struct merger_clause** const merger_clauseset,

       size_t num_branches,
       struct branch** const branchset,

       struct io_handler_s* io,
       struct io_reader_s*  read_ctxt
      ) {

  /* free'd just before calling ungrouper(...) */
  struct merger_result* mresult = calloc(1, sizeof(struct merger_result));
  if (mresult == NULL)
    errExit("calloc");

  /* initialize the iterator */
  struct permut_iter* iter = iter_init(num_branches, branchset);
  if (iter == NULL)
    return mresult;

  /* assign merger func for each term */
  for (int k = 0; k < num_merger_clauses; k++) {

    struct merger_clause* mclause = merger_clauseset[k];

    for (int j = 0; j < mclause->num_terms; j++) {

      /* assign a uintX_t specific function depending on grule->op */
      struct merger_term* term = mclause->termset[j];
      assign_merger_func(term);
    }
  }

  /* initialze an output stream, if file write is requested */
  io_writer_t* writer_ctxt = NULL;
  if (verbose_v && file) {

    /* get a file descriptor */
    char* filename = (char*)0L;
    if (asprintf(&filename, "%s/merger-merged-groups.%s", dirpath,
                            io->io_get_format_suffix()) < 0)
      errExit("asprintf(): failed");
    int out_fd = get_wronly_fd(filename);
    if(out_fd == -1) errExit("get_wronly_fd(...) returned -1");
    else free(filename);

    uint32_t num_flows = (uint32_t) (mresult->num_group_tuples * num_branches);

    /* get the output stream */
    writer_ctxt = io->io_write_init(read_ctxt, out_fd, num_flows);
    exitOn(writer_ctxt == NULL);
  }

  /* initialze an output stream, if file write is requested */
  io_writer_t* writer_ctxt_tobe = NULL;
  if (verbose_vv && file) {

    /* get a file descriptor */
    char* filename = (char*)0L;
    if (asprintf(&filename, "%s/merger-to-be-merged-groups.%s",
                            dirpath, io->io_get_format_suffix()) < 0)
      errExit("asprintf(): failed");
    int out_fd = get_wronly_fd(filename);
    if(out_fd == -1) errExit("get_wronly_fd(...) returned -1");
    else free(filename);

    uint32_t num_flows =
    (uint32_t) (mresult->total_num_group_tuples * num_branches);

    /* get the output stream */
    writer_ctxt_tobe = io->io_write_init(read_ctxt, out_fd, num_flows);
    exitOn(writer_ctxt_tobe == NULL);
  }

  /* iterate over all permutations */
  unsigned int index = 0;
  while(iter_next(iter)) {
    bool clause = true;
    index += 1;

    /* write to-be matched tuple to file if requested */
    if (verbose_vv && file) {
      for (int j = 0; j < num_branches; j++) {
        /* write the record to the output stream */
        char* record = branchset[j]->gfilter_result->filtered_groupset
                          [
                           iter->filtered_group_tuple[j] - 1
                          ]->aggr_result->aggr_record->aggr_record;
        exitOn(io->io_write_record(writer_ctxt_tobe, record) < 0);
      }
    }

    /* process each merger clause (clauses are OR'd) */
    for (int k = 0; k < num_merger_clauses; k++) {

      struct merger_clause* const mclause = merger_clauseset[k];

      /* process each merger term (terms within a clause are AND'd) */
      for (int j = 0; j < mclause->num_terms; j++) {

        struct merger_term* const term = mclause->termset[j];
        size_t group1_id = iter->filtered_group_tuple[term->branch1->branch_id];
        size_t group2_id = iter->filtered_group_tuple[term->branch2->branch_id];

        if (!term->
            func(
                 term->branch1->gfilter_result->filtered_groupset[group1_id-1],
                 term->field1,
                 term->branch2->gfilter_result->filtered_groupset[group2_id-1],
                 term->field2,
                 0
                )
           ) {
          clause = false;
          break;
        }
      }

      /* if any term is not satisfied, move to the next clause */
      if (!clause)
        continue;
      /* else this clause is TRUE; so everything is TRUE; break out */
      else {
        break;
      }
    }

    /* add the groups to the group tuple, if one of the clause matched */
    if(clause){

      /* free'd just before calling ungrouper(...) */
      struct group **matched_tuple = (struct group **)
                                     calloc(num_branches,
                                            sizeof(struct group *));
      if (matched_tuple == NULL)
        errExit("calloc");

      /* save the groups in the matched tuple */
      for (int j = 0; j < num_branches; j++){
        size_t group_id = iter->filtered_group_tuple[j];
        matched_tuple[j] =
        branchset[j]->gfilter_result->filtered_groupset[group_id-1];

        /* write to the output stream */
        if (verbose_v && file) {
          exitOn(io->io_write_record(writer_ctxt, 
                              matched_tuple[j]->aggr_result->
                              aggr_record->aggr_record) < 0);
        }
      }

      /* free'd just before calling ungrouper(...) */
      mresult->num_group_tuples += 1;
      mresult->group_tuples = (struct group ***)
      realloc(mresult->group_tuples,
              mresult->num_group_tuples *sizeof(struct group**));
      if (mresult->group_tuples == NULL)
        errExit("realloc");

      mresult->group_tuples[mresult->num_group_tuples-1] = matched_tuple;
    }
  };

  /* close the output stream */
  if (verbose_vv && file) {
    exitOn(io->io_write_close(writer_ctxt_tobe) < 0);
  }

  /* close the output stream */
  if (verbose_v && file) {
    exitOn(io->io_write_close(writer_ctxt) < 0);
  }

  mresult->total_num_group_tuples = index;
  iter_destroy(iter);
  return mresult;
}

#endif
