/*
 * Copyright 2012 Vaibhav Bajpai <contact@vaibhavbajpai.com>
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

#include "branch.h"

void *
branch_start(void *arg) {

  struct branch* branch = (struct branch *)arg;










#ifdef FILTER

  /* -----------------------------------------------------------------------*/
  /*                                filter                                  */
  /* -----------------------------------------------------------------------*/

  /* records are filtered as soon as the trace is read */
  if (filter_enabled) {
    if (branch->filter_result == NULL)
      pthread_exit((void*)EXIT_FAILURE);
  }

  /* -----------------------------------------------------------------------*/

#endif










#ifdef GROUPER

  /* -----------------------------------------------------------------------*/
  /*                               grouper                                  */
  /* -----------------------------------------------------------------------*/

  if (grouper_enabled) {

    branch->grouper_result = grouper(
                                      branch->num_filter_clauses,
                                      branch->filter_clauseset,

                                      branch->num_grouper_clauses,
                                      branch->grouper_clauseset,

                                      branch->num_aggr_clause_terms,
                                      branch->aggr_clause_termset,

                                      branch->filter_result,
                                      branch->data->rec_size,

                                      branch->data,
                                      branch->branch_id
                                    );
    if (branch->grouper_result == NULL)
      pthread_exit((void*)EXIT_FAILURE);
    else {

      /* free grouper rules */
      for (int j = 0; j < branch->num_grouper_clauses; j++) {

        struct grouper_clause* gclause = branch->grouper_clauseset[j];

        for (int i = 0; i < gclause->num_terms; i++) {
          struct grouper_term* gterm = gclause->termset[i];
          free(gterm->op); gterm->op = NULL;
          free(gterm); gterm = NULL; gclause->termset[i] = NULL;
        }
        free(gclause->termset); gclause->termset = NULL;
        free(gclause); gclause = NULL; branch->grouper_clauseset[j] = NULL;
      }
      free(branch->grouper_clauseset); branch->grouper_clauseset = NULL;

      /* free grouper aggregation rules */
      if (groupaggregations_enabled) {
        for (int i = 0; i < branch->num_aggr_clause_terms; i++) {
          struct aggr_term* term = branch->aggr_clause_termset[i];
          free(term->op); term->op = NULL;
          free(term); term = NULL; branch->aggr_clause_termset[i] = NULL;
        }
        free(branch->aggr_clause_termset); branch->aggr_clause_termset = NULL;
      }
    }
  }

#ifdef FILTER
  /* free filter rules */
  if (filter_enabled) {
    for (int j = 0; j < branch->num_filter_clauses; j++) {

      struct filter_clause* fclause = branch->filter_clauseset[j];

      for (int i = 0; i < fclause->num_terms; i++) {
        struct filter_term* fterm = fclause->termset[i];
        free(fterm->op); fterm->op = NULL;
        free(fterm); fterm = NULL; fclause->termset[i] = NULL;
      }
      free(fclause->termset); fclause->termset = NULL;
      free(fclause); fclause = NULL; branch->filter_clauseset[j] = NULL;
    }
    free(branch->filter_clauseset); branch->filter_clauseset = NULL;
  }
#endif

  /* -----------------------------------------------------------------------*/

#endif










#ifdef GROUPFILTER

  /* -----------------------------------------------------------------------*/
  /*                            grouper-filter                              */
  /* -----------------------------------------------------------------------*/

  if (groupfilter_enabled) {

    branch->gfilter_result = groupfilter(
                                          branch->num_groupfilter_clauses,
                                          branch->groupfilter_clauseset,

                                          branch->grouper_result,

                                          branch->data,
                                          branch->branch_id
                                        );

    if (branch->gfilter_result == NULL)
      pthread_exit((void*)EXIT_FAILURE);
    else {

      /* free group filter rules */
      for (int j = 0; j < branch->num_groupfilter_clauses; j++) {

        struct groupfilter_clause* gfclause = branch->groupfilter_clauseset[j];

        for (int i = 0; i < gfclause->num_terms; i++) {
          struct groupfilter_term* term = gfclause->termset[i];
          free(term->op); term->op = NULL;
          free(term); term = NULL; gfclause->termset[i] = NULL;
        }
        free(gfclause->termset); gfclause->termset = NULL;
        free(gfclause); gfclause = NULL; branch->groupfilter_clauseset[j] = NULL;
      }
      free(branch->groupfilter_clauseset); branch->groupfilter_clauseset = NULL;
    }
  }

  /* -----------------------------------------------------------------------*/

#endif










  pthread_exit((void*)EXIT_SUCCESS);
}
