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
       struct branch** const branchset
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

  /* iterate over all permutations */
  unsigned int index = 0;
  while(iter_next(iter)) {
    bool clause = true;
    index += 1;

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

    /* add the groups to the group tuple, if on of the clause matched */
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

  mresult->total_num_group_tuples = index;
  iter_destroy(iter);
  return mresult;
}

#endif
