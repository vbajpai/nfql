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
       struct ft_data* dataformat
      ) {

  /* free'd just before calling ungrouper(...) */
  struct merger_result* mresult = calloc(1, sizeof(struct merger_result));
  if (mresult == NULL)
    errExit("calloc");

  /* initialize the iterators */
  struct permut_iter* iter = iter_init(num_branches, branchset);
  if (iter == NULL) return mresult;
  struct permut_iter* iter_suc = iter_init(num_branches, branchset);
  if (iter_suc == NULL) return mresult;

  /* assign merger func for each term */
  for (int k = 0; k < num_merger_clauses; k++) {

    struct merger_clause* mclause = merger_clauseset[k];

    for (int j = 0; j < mclause->num_terms; j++) {

      /* assign a uintX_t specific function depending on grule->op */
      struct merger_term* term = mclause->termset[j];
      assign_merger_func(term);
    }
  }

  /* initialze an output stream for --verbose=1, if file write is requested */
  struct ftio* ftio_out = NULL; int n = -1;
  if (verbose_v && file) {

    /* get a file descriptor */
    char* filename = (char*)0L;
    asprintf(&filename, "%s/merger-merged-groups.ftz", dirpath);
    int out_fd = get_fd(filename);
    if(out_fd == -1) errExit("get_fd(...) returned -1");
    else free(filename);

    /* get the output stream */
    ftio_out = get_ftio(
                         dataformat,
                         out_fd,
                         mresult->num_matches
                       );

    /* write the header to the output stream */
    if ((n = ftio_write_header(ftio_out)) < 0)
      fterr_errx(1, "ftio_write_header(): failed");

  }

  /* initialze an output stream for --verbose=2, if file write is requested */
  struct ftio* ftio_tobe_out = NULL; int n_tobe = -1;
  if (verbose_vv && file) {

    /* get a file descriptor */
    char* filename = (char*)0L;
    asprintf(&filename, "%s/merger-to-be-merged-groups.ftz", dirpath);
    int out_fd = get_fd(filename);
    if(out_fd == -1) errExit("get_fd(...) returned -1");
    else free(filename);

    /* get the output stream */
    ftio_tobe_out = get_ftio(
                              dataformat,
                              out_fd,
                              mresult->num_match_tries
                            );

    /* write the header to the output stream */
    if ((n_tobe = ftio_write_header(ftio_tobe_out)) < 0)
      fterr_errx(1, "ftio_write_header(): failed");
  }

  /* hold a reference group from first branch for each merger match */
  struct group* ref_group = branchset[0]->gfilter_result->filtered_groupset[0];

  /* iterate over all reference records */
  while (ref_group != NULL) {
    
    /* TODO: when free'd? */
    struct merger_match* match = calloc(1, sizeof(struct merger_match));
    if (match == NULL)
      errExit("calloc");
    
    /* iterate over all iterator permutations */
    do {
      
      bool clause = true;
      bool continue_iter = false;
      
      struct group* cur_group = branchset[iter->num_branches - 1]->
                                 gfilter_result->filtered_groupset
                                 [iter->filtered_group_tuple
                                 [iter->num_branches - 1] - 1];
      
      /* write to-be matched tuple to file if requested */
      if (verbose_vv) {
        
        mresult->num_match_tries += 1;
        
        /* TODO: when free'd? */
        mresult->matchtryset =
        realloc (
                  mresult->matchtryset,
                  mresult->num_match_tries * sizeof(struct group**)
                );
        if (mresult->matchtryset == NULL)
          errExit("realloc");
        
        /* TODO: when free'd? */
        struct group** match_try = calloc(2, sizeof(struct group*));
        if (match_try == NULL)
          errExit("realloc");

        if (file) {
          if ((n_tobe = ftio_write(ftio_tobe_out,
                                   ref_group->aggr_result->aggr_record) < 0))
            fterr_errx(1, "ftio_write(): failed");
          
          if ((n_tobe = ftio_write(ftio_tobe_out,
                                   cur_group->aggr_result->aggr_record) < 0))
            fterr_errx(1, "ftio_write(): failed");
        }
        
        match_try[0] = ref_group;
        match_try[1] = cur_group;
        
        mresult->matchtryset[mresult->num_match_tries - 1] = match_try;
      }
      
      /* process each merger clause (clauses are OR'd) */
      for (int k = 0; k < num_merger_clauses; k++) {
        
        struct merger_clause* const mclause = merger_clauseset[k];
        
        /* process each merger term (terms within a clause are AND'd) */
        for (int j = 0; j < mclause->num_terms; j++) {
          
          struct merger_term* const term = mclause->termset[j];
          
          if (
              !term->
              func (
                     ref_group,
                     term->field1,
                     cur_group,
                     term->field2,
                     0
                   )
             ) {
            
            clause = false;
            if (
                 term->comp (
                              ref_group,
                              term->field1,
                              cur_group,
                              term->field2,
                              0
                            )
               ) {
              continue_iter = true;
            }
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
        
        /* save the groups in the matched tuple */
        size_t group_id = iter->filtered_group_tuple[iter->num_branches - 1];
        struct group* group = branchset[iter->num_branches - 1]->
                              gfilter_result->filtered_groupset[group_id-1];
        
        /* write to the output stream */
        if (verbose_v && file) {
          if ((n = ftio_write(ftio_out, group->
                              aggr_result->aggr_record) < 0))
            fterr_errx(1, "ftio_write(): failed");
          if (match->num_groups == 0)
            if ((n = ftio_write(ftio_out, ref_group->
                                aggr_result->aggr_record) < 0))
              fterr_errx(1, "ftio_write(): failed");
        }
        
        /* free'd just before calling ungrouper(...) */
        match->num_groups += 1;
        match->groupset = (struct group **)
        realloc(match->groupset,
                match->num_groups * sizeof(struct group*));
        if (match->groupset == NULL)
          errExit("realloc");
        
        match->groupset[match->num_groups-1] = group;
        
      }
      else {
        
        if (!continue_iter) {
          iter_suc->filtered_group_tuple[iter->num_branches - 1] =
          iter->filtered_group_tuple[iter->num_branches - 1];
          
          iter->num_branches -= 1;
          
          continue_iter = false;
        }
      }
      
    } while (iter_next(iter));
    
    if (match->num_groups != 0) {
      
      match->num_groups += 1;
      match->groupset = (struct group **)
      realloc(match->groupset,
              match->num_groups * sizeof(struct group*));
      if (match->groupset == NULL)
        errExit("realloc");
      match->groupset[match->num_groups-1] = ref_group;
      
      mresult->num_matches += 1;
      mresult->matchset = (struct merger_match **)
      realloc(mresult->matchset,
              mresult->num_matches * sizeof(struct merger_match*));
      if (mresult->matchset == NULL)
        errExit("realloc");
      
      mresult->matchset[mresult->num_matches-1] = match;
    }
    
    /* wrap around */
    if ( (iter->filtered_group_tuple[iter->num_branches] - 1) == 0)
      break;
    
    if ( (iter->num_branches) != 0)
      break;
    
    ref_group = branchset[0]->gfilter_result->filtered_groupset
                 [iter->filtered_group_tuple[iter->num_branches] - 1];
    
    for (int i = 0; i < iter_suc->num_branches; i++)
      iter->filtered_group_tuple = iter_suc->filtered_group_tuple;
    iter->num_branches = iter_suc->num_branches;
    
  }
  
  /* close the output stream */
  if (verbose_vv && file) {
    if ((n_tobe = ftio_close(ftio_tobe_out)) < 0)
      fterr_errx(1, "ftio_close(): failed");
    free(ftio_tobe_out);
  }

  /* close the output stream */
  if (verbose_v && file) {
    if ((n = ftio_close(ftio_out)) < 0)
      fterr_errx(1, "ftio_close(): failed");
    free(ftio_out);
  }

  iter_destroy(iter);
  return mresult;
}

#endif
