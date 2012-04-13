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
                     size_t num_filter_rules,
                     struct filter_rule** const filter_ruleset,
                     
                     size_t num_grouper_rules,
                     struct grouper_rule** const grouper_ruleset,
                     
                     size_t num_aggr_rules,
                     struct aggr_rule** const aggr_ruleset,
                     
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
                           calloc(num_aggr_rules,
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
  bool ifgrouper = false;
  for (int j = 0; j < num_filter_rules; j++) {
    
    size_t field_offset = filter_ruleset[j]->field_offset;
    aggr_function = get_aggr_fptr(ifgrouper, filter_ruleset[j]->op); 
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
  
  
  /* save common fields (coming from the grouper rule) in aggregation record 
   * currently assumes that the grouper rule was `eq`, such that each record
   * member has the same value for that field in the group, still need
   * to investigate how it might affect other grouper operations */
  ifgrouper = true;
  for (int j = 0; j < num_grouper_rules; j++) {
    
    size_t goffset_1 = grouper_ruleset[j]->field_offset1;
    size_t goffset_2 = grouper_ruleset[j]->field_offset2;
    
    aggr_function = get_aggr_fptr(ifgrouper, grouper_ruleset[j]->op);
    if(aggr_function == NULL)
      errExit("get_aggr_fptr(...) returned NULL");
    
    if(goffset_1 != goffset_2){

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
  
  
  /* aggregate the fields, but ignore fields used in grouper and filter 
   * module. Again it assume that the operation is `eq`. Need to investigate 
   * more on how it might affect for other type of operations */
  for (int j = 0; j < num_aggr_rules; j++){    

    bool if_ignore_aggr_rule = false;
    size_t aggr_offset = aggr_ruleset[j]->field_offset;
    
    /* if aggr rule is same as any filter rule, just ignore it */
    for (int k = 0; k < num_filter_rules; k++) {
      
      size_t filter_offset = filter_ruleset[k]->field_offset;      
      if (aggr_offset == filter_offset){
        if_ignore_aggr_rule = true;
        break;
      }
    }    
    
    /* if aggr rule is same as any grouper rule, just ignore it */
    for (int k = 0; k < num_grouper_rules; k++) {

      size_t goffset_1 = grouper_ruleset[k]->field_offset1;
      size_t goffset_2 = grouper_ruleset[k]->field_offset2;      
      
      if (aggr_offset == goffset_1 || aggr_offset == goffset_2){
        if_ignore_aggr_rule = true;
        break;
      }
    }
    
    /* free'd just after returning from merger(...) */
    aggrset[j] = aggr_ruleset[j]->func(group->members,
                                       aggr_record,
                                       group->num_members, 
                                       aggr_offset, 
                                       if_ignore_aggr_rule);
  }    

  aggrset = NULL; aggr_record = NULL;
  
  return aresult;
}

struct grouper_intermediate_result *
get_grouper_intermediates(
                          size_t num_filtered_records,
                          char** const filtered_recordset_copy,                         
                          
                          size_t grule1_rhs_offset,                          
                          struct grouper_result* const gresult
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
   * item in the statement of the first grouper rule 
   * and save them in sorted_recordset_reference in place  
   * TODO: different comp func sizes
   */
  qsort_r(
          sorted_recordset_ref, 
          num_filtered_records, 
          sizeof(char **), 
          (void *)&grule1_rhs_offset,
          comp_uint32_t
         );
  
  if(verbose_vv){
    
    /* free'd just before calling merger(...) ?*/
    gresult->sorted_recordset = (char**) calloc(num_filtered_records, 
                                                sizeof(char*));
    if (gresult->sorted_recordset == NULL)
      errExit("calloc");
    
    for (int i = 0; i < num_filtered_records; i++)
      gresult->sorted_recordset[i] = *sorted_recordset_ref[i];
  }
  
  /* free'd just before returning from grouper(...) */
  struct tree_item_uint32_t* uniq_recordset = (struct tree_item_uint32_t *)
                                   calloc(num_filtered_records, 
                                          sizeof(struct tree_item_uint32_t));
  if (uniq_recordset == NULL)
    errExit("calloc");  
  
  /* TODO: uint32_t assumed 
   * unlinked uniq_recordset[0].ptr and free'd uniq_recordset
   * just before calling grouper_aggregations(...)
   */
  uniq_recordset[0].value = *(uint32_t *)(*sorted_recordset_ref[0] + 
                                          grule1_rhs_offset);
  uniq_recordset[0].ptr = &sorted_recordset_ref[0];
  size_t num_uniq_records = 1;

  for (int i = 0; i < num_filtered_records; i++) {
    if (
        *(uint32_t *)(*sorted_recordset_ref[i] + grule1_rhs_offset) != 
        uniq_recordset[num_uniq_records-1].value
       ) {
      
      uniq_recordset[num_uniq_records].value = 
      *(uint32_t *)(*sorted_recordset_ref[i] + grule1_rhs_offset);
      uniq_recordset[num_uniq_records].ptr = &sorted_recordset_ref[i];
      num_uniq_records++;
    }
  }

  /* TODO: UINT32_T assumed */
  uniq_recordset = (struct tree_item_uint32_t *)
                    realloc(uniq_recordset, 
                            num_uniq_records*sizeof(struct tree_item_uint32_t));
  if (uniq_recordset == NULL)
    errExit("realloc");
  
  // mark the end of sorted records
  sorted_recordset_ref[num_filtered_records] = NULL;
  

  /* free'd just before calling grouper_aggregations(...) */
  struct grouper_intermediate_result*
  intermediate_result = (struct grouper_intermediate_result *)
                        calloc(1 , sizeof(struct grouper_intermediate_result));
  if (intermediate_result == NULL)
    errExit("calloc");
  
  /* TODO: UINT32_T assumed */
  intermediate_result->type = UINT32_T;
  intermediate_result->num_uniq_records = num_uniq_records;
  intermediate_result->uniq_recordset.recordset_32 = uniq_recordset;
  uniq_recordset = NULL;
  intermediate_result->sorted_recordset_reference = sorted_recordset_ref;
  sorted_recordset_ref = NULL;
  
  return intermediate_result;
}


struct grouper_result*
grouper(struct branch* branch) {
  
  /* free'd just after returning from ungrouper(...) */
  struct grouper_result* gresult = calloc(1, sizeof(struct grouper_result));
  if (gresult == NULL)
    errExit("calloc");
  else
    branch->grouper_result = gresult;
  
  /* free'd just after returning from ungrouper(...) */
  struct group** groupset = (struct group **)calloc(1, sizeof(struct group *));
  if (groupset == NULL)
    errExit("calloc");
  else
    gresult->groupset = groupset;

  
  /* club all filtered records into one group, 
   * if no group modules are defined 
   */
  if (branch->num_grouper_rules == 0) {
    
    /* groupset with space for 1 group */
    gresult->num_groups = 1;
    
    /* free'd just after returning from ungrouper(...) */
    struct group* group = (struct group *)calloc(1, sizeof(struct group));      
    if (group == NULL)
      errExit("calloc");    
    
    groupset[gresult->num_groups - 1] = group;
    group->num_members = branch->filter_result->num_filtered_records;
    
    /* free'd after returning from ungrouper(...) */    
    group->members = (char **)calloc(group->num_members, sizeof(char *));
    if (group->members == NULL)
      errExit("calloc");    
    
    for (int i = 0; i < branch->filter_result->num_filtered_records; i++)
      group->members[i] = branch->filter_result->filtered_recordset[i];    
  }
  else {
    
    /* unlinked each item from original traces, ie filtered_recordset_copy[i] 
     * free'd filtered_recordset_copy just before exiting from this function 
     */
    char** filtered_recordset_copy = 
    calloc(branch->filter_result->num_filtered_records, sizeof(char*));    
    if (filtered_recordset_copy == NULL)
      errExit("calloc");      
    for (int i = 0; i < branch->filter_result->num_filtered_records; i++)
      filtered_recordset_copy[i] = branch->filter_result->filtered_recordset[i];
    
    struct grouper_intermediate_result* intermediate_result = 
    get_grouper_intermediates(
                              branch->filter_result->num_filtered_records,
                              filtered_recordset_copy,
                              
                              branch->grouper_ruleset[0]->field_offset2,
                              branch->grouper_result
                             );
    
    if (intermediate_result == NULL)
      errExit("get_grouper_intermediates(...) returned NULL");

    if(verbose_vv){  
      
      /* free'd just before calling merger(...) */      
      gresult->num_unique_records = intermediate_result->num_uniq_records;
      gresult->unique_recordset = (char**) 
                                  calloc(gresult->num_unique_records, 
                                         sizeof(char*));      
      if (gresult->unique_recordset == NULL)
        errExit("calloc");
      
      /* TODO: UINT32_T assumed */
      for (int i = 0; i < gresult->num_unique_records; i++)
        gresult->unique_recordset[i] = 
        **intermediate_result->uniq_recordset.recordset_32[i].ptr;
    }
    
    for (int i = 0; i < branch->filter_result->num_filtered_records; i++) {
      
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
      /* TODO: UINT32_T assumed */
      char ***record_iter = 
        (
          (struct tree_item_uint32_t *)
          bsearch_r(
                    filtered_recordset_copy[i],
                    (void *)intermediate_result[0].uniq_recordset.recordset_32,
                    intermediate_result[0].num_uniq_records,
                    sizeof(struct tree_item_uint32_t),
                    (void *)&branch->grouper_ruleset[0]->field_offset1,
                    comp_uint32_t_p
                   )
        )->ptr;
      
      // iterate until terminating NULL in sorted_records
      for (int k = 0; *record_iter != NULL; record_iter++) {
        
        // already processed record from filtered_records
        if (**record_iter == NULL)
          continue;
        
        // do not group with itself
        if (**record_iter == filtered_recordset_copy[i])
          continue;
        
        // check all module filter rules for those two records
        for (k = 0; k < branch->num_grouper_rules; k++) {
          if (
              !branch->grouper_ruleset[k]->func
                                   (
                                     group, 
                                     branch->grouper_ruleset[k]->field_offset1,
                                     **record_iter, 
                                     branch->grouper_ruleset[k]->field_offset2, 
                                     branch->grouper_ruleset[k]->delta
                                   )
            )
            break;
        }
        
        // first rule didnt match
        if (k == 0)
          break;
        
        // one of the other rules didnt match
        if (k < branch->num_grouper_rules)
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
      
      // unlink all the local filtered recordset copy from the flow data
      filtered_recordset_copy[i] = NULL;
    }
    
    free(filtered_recordset_copy); filtered_recordset_copy = NULL;    
    
    // unlink the sorted records from the flow data
    for (int i = 0; i < branch->filter_result->num_filtered_records; i++)
      intermediate_result->sorted_recordset_reference[i] = NULL;    
    free(intermediate_result->sorted_recordset_reference); 
    intermediate_result->sorted_recordset_reference = NULL;
    
    // unlink the uniq records from the flow data
    for (int i = 0; i < intermediate_result->num_uniq_records; i++)
      intermediate_result->uniq_recordset.recordset_32[i].ptr = NULL;
    free(intermediate_result->uniq_recordset.recordset_32);    
    intermediate_result->uniq_recordset.recordset_32 = NULL;
    free(intermediate_result); intermediate_result = NULL;    
  }


#ifdef GROUPERAGGREGATIONS
  
  for (int i = 0; i < branch->grouper_result->num_groups; i++) {
    
    struct group* group = branch->grouper_result->groupset[i];
    
    group->aggr_result = grouper_aggregations(
                                              branch->num_filter_rules,
                                              branch->filter_ruleset,
                                               
                                              branch->num_grouper_rules,
                                              branch->grouper_ruleset,
                                              
                                              branch->num_aggr_rules,
                                              branch->aggr_ruleset,
                                               
                                              group,
                                              branch->data->rec_size
                                             );
    if (group->aggr_result == NULL)
      errExit("grouper_aggregations(...) returned NULL");
    else
      group = NULL;
  }
#endif  
  
  return gresult;
}



#define comp(size) \
int comp_##size(void *thunk, const void *e1, const void *e2) \
{ \
size x, y; \
x = *(size *)(**(char ***)e1+*(size_t *)thunk); \
y = *(size *)(**(char ***)e2+*(size_t *)thunk); \
return (x > y) - (y > x); \
}

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