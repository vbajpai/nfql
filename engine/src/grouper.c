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

#include "grouper.h"


struct aggr_result*
grouper_aggregations(
                     size_t num_filter_clauses,
                     struct filter_clause** const filter_clauseset,

                     size_t num_grouper_clauses,
                     struct grouper_clause** const grouper_clauseset,

                     size_t num_aggr_clause_terms,
                     struct aggr_term** const aggr_clause_termset,

                     const struct group* const group,
                     int rec_size
                     ) {

  /* free'd just after returning from merger(...) */
  struct aggr_result* aresult = calloc(1, sizeof(struct aggr_result));
  if (aresult == NULL)
    errExit("calloc");

  struct aggr* (*aggr_function)(char **records,
                                char *group_aggregation,
                                size_t num_records,
                                size_t field_offset,
                                bool if_aggr_common) = NULL;

  /* free'd just after returning from merger(...) */
  struct aggr** aggrset = (struct aggr**)
  calloc(num_aggr_clause_terms,
         sizeof(struct aggr*));
  if (aggrset == NULL)
    errExit("calloc");
  aresult->aggrset = aggrset;

  /* free'd just after returning from merger(...) */
  char* aggr_record = (char *)calloc(1, rec_size);
  if (aggr_record == NULL)
    errExit("calloc");
  aresult->aggr_record = aggr_record;


  /* save common fields (coming from the filter rule) in aggregation record
   * currently assumes that the filter rule was `eq`, such that each record
   * member has the same value for that field in the group, still need
   * to investigate how it might affect other filter operations */
  for (int i = 0; i < num_filter_clauses; i++) {

    struct filter_clause* fclause = filter_clauseset[i];

    for (int j = 0; j < fclause->num_terms; j++) {

      size_t field_offset = fclause->termset[j]->field_offset;
      aggr_function = get_aggr_fptr(false,
                                    fclause->termset[j]->op->field_type);
      if(aggr_function == NULL)
        errExit("get_aggr_fptr(...) returned NULL");

      struct aggr* aggr = (*aggr_function)(group->members,
                                           aggr_record,
                                           group->num_members,
                                           field_offset,
                                           TRUE);
      if (aggr == NULL)
        errExit("aggr_function(...) returned NULL");
      else {
        free(aggr->values); aggr->values = NULL;
        free(aggr); aggr = NULL;
      }
    }
  }

  /* save common fields (coming from the grouper rule) in aggregation record
   * currently assumes that the grouper rule was `eq`, such that each record
   * member has the same value for that field in the group, still need
   * to investigate how it might affect other grouper operations */
  for (int i = 0; i < num_grouper_clauses; i++) {

    struct grouper_clause* gclause = grouper_clauseset[i];

    for (int j = 0; j < gclause->num_terms; j++) {

      struct grouper_term* term = gclause->termset[j];

      size_t goffset_1 = term->field_offset1;
      size_t goffset_2 = term->field_offset2;

      aggr_function = get_aggr_fptr(true,
                                    term->op->field1_type);

      if(aggr_function == NULL)
        errExit("get_aggr_fptr(...) returned NULL");

      if(goffset_1 != goffset_2) {

        struct aggr* aggr = (*aggr_function)(group->members,
                                             aggr_record,
                                             group->num_members,
                                             goffset_1,
                                             TRUE);
        if (aggr == NULL)
          errExit("aggr_function(...) returned NULL");
        else {
          free(aggr->values); aggr->values = NULL;
          free(aggr); aggr =  NULL;
        }
      }

      struct aggr* aggr = (*aggr_function)(group->members,
                                           aggr_record,
                                           group->num_members,
                                           goffset_2,
                                           TRUE);
      if (aggr == NULL)
        errExit("aggr_function(...) returned NULL");
      else {
        free(aggr->values); aggr->values = NULL;
        free(aggr); aggr = NULL;
      }
    }
  }

  /* aggregate the fields, but ignore fields used in grouper and filter
   * module. Again it assume that the operation is `eq`. Need to investigate
   * more on how it might affect for other type of operations */
  for (int j = 0; j < num_aggr_clause_terms; j++){

    bool if_ignore_aggr_term = false;
    struct aggr_term* term = aggr_clause_termset[j];
    size_t aggr_offset = term->field_offset;


    /* if aggr rule is same as any filter rule, just ignore it */
    for (int i = 0; i < num_filter_clauses; i++) {

      struct filter_clause* fclause = filter_clauseset[i];
      for (int k = 0; k < fclause->num_terms; k++) {

        size_t filter_offset = fclause->termset[k]->field_offset;
        if (aggr_offset == filter_offset){
          if_ignore_aggr_term = true;
          break;
        }
      }
    }

    /* if aggr rule is same as any grouper rule, just ignore it */
    for (int i = 0; i < num_grouper_clauses; i++) {

      struct grouper_clause* gclause = grouper_clauseset[i];

      for (int k = 0; k < gclause->num_terms; k++) {

        struct grouper_term* term = gclause->termset[k];

        size_t goffset_1 = term->field_offset1;
        size_t goffset_2 = term->field_offset2;

        if (aggr_offset == goffset_1 || aggr_offset == goffset_2){
          if_ignore_aggr_term = true;
          break;
        }
      }
    }

    /* assign a specific uintX_t function depending on aggrule->op */
    assign_aggr_func(term);

    /* free'd just after returning from merger(...) */
    aggrset[j] = term->func(
                             group->members,
                             aggr_record,
                             group->num_members,
                             aggr_offset,
                             if_ignore_aggr_term
                            );
  }

  aggrset = NULL; aggr_record = NULL;

  return aresult;
}


struct grouper_intermediate_result *
get_grouper_intermediates
                (
                  size_t num_filtered_records,
                  char** const filtered_recordset_copy,

                  size_t num_grouper_terms_in_first_clause,
                  struct grouper_term** const grouper_termset_of_first_clause,

                  struct grouper_result* const gresult,
                  const struct grouper_type* const gtype
                ) {

  /* last record in sorted_records is NULL
   * unlinked sorted_recordset_ref[i], and free'd sorted_recordset_ref
   * just before calling grouper_aggregations(...)
   */
  char*** sorted_recordset_ref = (char ***)
  calloc(num_filtered_records+1,
         sizeof(char **));
  if (sorted_recordset_ref == NULL)
    errExit("calloc");

  for (int i = 0; i < num_filtered_records; i++)
    sorted_recordset_ref[i] = &filtered_recordset_copy[i];

  /* sort the record references according to the right hand side
   * item in the statement of the first grouper term of the first clause
   * and save them in sorted_recordset_reference in place
   */

#if defined(__APPLE__) || defined(__FreeBSD__)
  qsort_r(
          sorted_recordset_ref,
          num_filtered_records,
          sizeof(char **),
          (void*)&grouper_termset_of_first_clause[0]->field_offset2,
          gtype->qsort_comp
         );
#elif defined(__linux)
  qsort_r(
          sorted_recordset_ref,
          num_filtered_records,
          sizeof(char **),
          gtype->qsort_comp,
          (void*)&grouper_termset_of_first_clause[0]->field_offset2
         );
#endif

  if(verbose_vv){

    /* free'd just before calling merger(...) ?*/
    gresult->sorted_recordset = (char**) calloc(num_filtered_records,
                                                sizeof(char*));
    if (gresult->sorted_recordset == NULL)
      errExit("calloc");

    for (int i = 0; i < num_filtered_records; i++)
      gresult->sorted_recordset[i] = *sorted_recordset_ref[i];
  }

  // mark the end of sorted records
  sorted_recordset_ref[num_filtered_records] = NULL;

  /* free'd just before calling grouper_aggregations(...) */
  struct grouper_intermediate_result*
  intermediate_result = (struct grouper_intermediate_result *)
  calloc(1 , sizeof(struct grouper_intermediate_result));
  if (intermediate_result == NULL)
    errExit("calloc");

  intermediate_result->sorted_recordset_reference = sorted_recordset_ref;

  struct uniq_recordset_result*
  uresult = gtype->alloc_uniqresult(
                                    num_filtered_records,
                                    grouper_termset_of_first_clause,
                                    sorted_recordset_ref
                                   );

  if (uresult == NULL)
    errExit("get_uniqrecordset(...) returned NULL");

  intermediate_result->uniq_result = uresult; uresult = NULL;
  sorted_recordset_ref = NULL;

  return intermediate_result;
}


struct grouper_result*
grouper(
        size_t num_filter_clauses,
        struct filter_clause** const filter_clauseset,

        size_t num_grouper_clauses,
        struct grouper_clause** const grouper_clauseset,

        size_t num_aggr_clause_terms,
        struct aggr_term** const aggr_clause_termset,

        const struct filter_result* const fresult,
        int rec_size
       ) {

  /* free'd just after returning from ungrouper(...) */
  struct grouper_result* gresult = calloc(1, sizeof(struct grouper_result));
  if (gresult == NULL)
    errExit("calloc");

  /* free'd just after returning from ungrouper(...) */
  struct group** groupset = (struct group **)calloc(1, sizeof(struct group *));
  if (groupset == NULL)
    errExit("calloc");
  else
    gresult->groupset = groupset;

  /* go ahead if there is something to group */
  if (fresult->num_filtered_records > 0) {
    /* club all filtered records into one group,
     * if no group modules are defined
     */
    if (num_grouper_clauses == 0) {

      /* groupset with space for 1 group */
      gresult->num_groups = 1;

      /* free'd just after returning from ungrouper(...) */
      struct group* group = (struct group *)calloc(1, sizeof(struct group));
      if (group == NULL)
        errExit("calloc");

      groupset[gresult->num_groups - 1] = group;
      group->num_members = fresult->num_filtered_records;

      /* free'd after returning from ungrouper(...) */
      group->members = (char **)calloc(group->num_members, sizeof(char *));
      if (group->members == NULL)
        errExit("calloc");

      for (int i = 0; i < fresult->num_filtered_records; i++)
        group->members[i] = fresult->filtered_recordset[i];
    }
    else {

      /* assign grouper func for each term */
      for (int k = 0; k < num_grouper_clauses; k++) {

        struct grouper_clause* gclause = grouper_clauseset[k];

        for (int j = 0; j < gclause->num_terms; j++) {

          /* assign a uintX_t specific function depending on grule->op */
          struct grouper_term* term = gclause->termset[j];
          assign_grouper_func(term);
        }
      }

      for (int l = 0; l < num_grouper_clauses; l++) {

        struct grouper_clause* gclause = grouper_clauseset[l];

        /* unlinked each item from original traces, ie
         * filtered_recordset_copy[i]; free'd filtered_recordset_copy just
         * before exiting from this function
         */
        char** filtered_recordset_copy = calloc(fresult->num_filtered_records,
                                                sizeof(char*));
        if (filtered_recordset_copy == NULL)
          errExit("calloc");

        for (int i = 0; i < fresult->num_filtered_records; i++)
          filtered_recordset_copy[i] = fresult->filtered_recordset[i];

        /* get uintX_t specific function pointers, given the type of the
         * RHS field of the first term of the first clause
         */
        struct grouper_type* gtype =
        get_gtype(gclause->termset[0]->op->field2_type);
        if (gtype == NULL)
          errExit("get_type(...) returned NULL");

        struct grouper_intermediate_result* intermediate_result =
        get_grouper_intermediates(
                                   fresult->num_filtered_records,
                                   filtered_recordset_copy,

                                   gclause->num_terms,
                                   gclause->termset,

                                   gresult,
                                   gtype
                                 );

        if (intermediate_result == NULL)
          errExit("get_grouper_intermediates(...) returned NULL");

        if(verbose_vv){

          /* free'd just before calling merger(...) */
          gresult->num_unique_records = intermediate_result->
          uniq_result->num_uniq_records;
          gresult->unique_recordset = (char**)
          calloc(gresult->num_unique_records,
                 sizeof(char*));
          if (gresult->unique_recordset == NULL)
            errExit("calloc");

          for (int i = 0; i < gresult->num_unique_records; i++) {
            gresult->unique_recordset[i] =
            gtype->get_uniq_record(intermediate_result->uniq_result,i);
            if (gresult->unique_recordset[i] == NULL)
              errExit("get_uniq_record(...) returned NULL");
          }
        }

        for (int i = 0; i < fresult->num_filtered_records; i++) {

          if (filtered_recordset_copy[i] == NULL)
            continue;

          gresult->num_groups += 1;

          /* free'd just after returning from ungrouper(...) */
          groupset = (struct group **)
          realloc(groupset, (gresult->num_groups)*sizeof(struct group*));
          if (groupset == NULL)
            errExit("realloc");
          else
            gresult->groupset = groupset;

          /* free'd just after returning from ungrouper(...) */
          struct group* group = (struct group *)calloc(1, sizeof(struct group));
          if (group == NULL)
            errExit("calloc");

          groupset[gresult->num_groups-1] = group;
          group->num_members = 1;

          /* free'd after returning from ungrouper(...) */
          group->members = (char **)calloc(1, sizeof(char *));
          if (group->members == NULL)
            errExit("calloc");
          group->members[0] = filtered_recordset_copy[i];

          // search for left hand side of comparison in records ordered by right
          // hand side of comparison
          char ***record_iter = gtype->bsearch(
                                               filtered_recordset_copy[i],
                                               gclause->termset,
                                               intermediate_result
                                               );
          if (record_iter != NULL) {
            // iterate until terminating NULL in sorted_records
            for (int k = 0; *record_iter != NULL; record_iter++) {

              // already processed record from filtered_records
              if (**record_iter == NULL)
                continue;

              // do not group with itself
              if (**record_iter == filtered_recordset_copy[i])
                continue;

              // check all terms for this grouper clause for those two records
              for (k = 0; k < gclause->num_terms; k++) {

                struct grouper_term* term = gclause->termset[k];

                if (
                    !term->func(
                                  group,
                                  term->field_offset1,
                                  **record_iter,
                                  term->field_offset2,
                                  term->delta
                                )
                    )
                  break;
              }

              // first term didnt match
              if (k == 0)
                break;

              // one of the other terms didnt match
              if (k < gclause->num_terms)
                continue;

              group->num_members += 1;

              group->members = (char **)
              realloc(group->members,
                      sizeof(char *)*group->num_members);

              // assign entry in filtered_records to group
              group->members[group->num_members-1] = **record_iter;

              // set filtered_recordset_copy[i] to NULL
              **record_iter = NULL;
            }
          }

          // unlink all the local filtered recordset copy from the flow data
          filtered_recordset_copy[i] = NULL;
        }

        free(filtered_recordset_copy); filtered_recordset_copy = NULL;

        // unlink the sorted records from the flow data
        for (int i = 0; i < fresult->num_filtered_records; i++)
          intermediate_result->sorted_recordset_reference[i] = NULL;
        free(intermediate_result->sorted_recordset_reference);
        intermediate_result->sorted_recordset_reference = NULL;

        // unlink the uniq records from the flow data
        gtype->dealloc_uniqresult(intermediate_result->uniq_result);
        free(intermediate_result); intermediate_result = NULL;
        free(gtype);
      }
    }
  }


#ifdef GROUPERAGGREGATIONS

  /* assign a specific uintX_t function depending on aggrule->op */
  for (int j = 0; j < num_aggr_clause_terms; j++){

    struct aggr_term* term = aggr_clause_termset[j];
    assign_aggr_func(term);
  }

  for (int i = 0; i < gresult->num_groups; i++) {

    struct group* group = gresult->groupset[i];

    group->aggr_result = grouper_aggregations(
                                              num_filter_clauses,
                                              filter_clauseset,

                                              num_grouper_clauses,
                                              grouper_clauseset,

                                              num_aggr_clause_terms,
                                              aggr_clause_termset,

                                              group,
                                              rec_size
                                              );
    if (group->aggr_result == NULL)
      errExit("grouper_aggregations(...) returned NULL");
    else
      group = NULL;
  }

#endif

  return gresult;
}


#if defined(__APPLE__) || defined(__FreeBSD__)
  #define comp(size) \
  int comp_##size(void *thunk, const void *e1, const void *e2) {\
    size x, y; \
    x = *(size *)(**(char ***)e1+*(size_t *)thunk); \
    y = *(size *)(**(char ***)e2+*(size_t *)thunk); \
    return (x > y) - (y > x); \
  }
#elif defined(__linux)
  #define comp(size) \
  int comp_##size(const void *e1, const void *e2, void *thunk) {\
    size x, y; \
    x = *(size *)(**(char ***)e1+*(size_t *)thunk); \
    y = *(size *)(**(char ***)e2+*(size_t *)thunk); \
    return (x > y) - (y > x); \
  }
#endif

comp(uint8_t);
comp(uint16_t);
comp(uint32_t);
comp(uint64_t);

#define comp_p(size) \
int comp_##size##_p(void *thunk, const void *e1, const void *e2) \
{ \
size x, y; \
x = *(size *)((char *)e1+*(size_t *)thunk); \
y = ((struct tree_item_uint32_t *)e2)->value; \
return (x > y) - (y > x); \
}

comp_p(uint8_t);
comp_p(uint16_t);
comp_p(uint32_t);
comp_p(uint64_t);
