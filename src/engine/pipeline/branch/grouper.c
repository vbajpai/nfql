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

char* 
grouper_aggregations(struct group* group, 
                     struct branch_info *binfo) {
  
  char* group_aggregation = (char *)calloc(1, binfo->data->rec_size);
  if (group_aggregation == NULL)
    errExit("calloc");
  
  group->aggr = (struct aggr *)calloc(binfo->num_aggr, sizeof(struct aggr));
  if (group->aggr == NULL)
    errExit("calloc");

  
  /* save common fields (coming from the filter rule) in aggregation record 
   * currently assumes that the filter rule was `eq`, such that each record
   * member has the same value for that field in the group, still need
   * to investigate how it might affect other filter operations */  
  for (int i = 0; i < binfo->num_filter_rules; i++) {
    
    size_t field_offset = binfo->filter_rules[i].field_offset;
    *((u_int16_t*)(group_aggregation + field_offset)) =
    *aggr_static_uint64_t(group->members,
                          group->num_members, 
                          field_offset, 
                          TRUE).values;          
  }  
  
  
  /* save common fields (coming from the grouper rule) in aggregation record 
   * currently assumes that the grouper rule was `eq`, such that each record
   * member has the same value for that field in the group, still need
   * to investigate how it might affect other grouper operations */
  for (int i = 0; i < binfo->num_group_modules; i++) {
    size_t group_offset_1 = binfo->group_modules[i].field_offset1;
    size_t group_offset_2 = binfo->group_modules[i].field_offset2;
    
    if(group_offset_1 != group_offset_2)
      *((u_int32_t*)(group_aggregation+group_offset_1)) = 
                  *aggr_static_uint64_t(group->members, 
                                        group->num_members, 
                                        group_offset_1, 
                                        TRUE).values;

    *((u_int32_t*)(group_aggregation+group_offset_2)) = 
    *aggr_static_uint64_t(group->members, 
                          group->num_members, 
                          group_offset_2, 
                          TRUE).values;      

  }
  
  
  /* aggregate the fields, but ignore fields used in grouper and filter 
   * module. Again it assume that the operation is `eq`. Need to investigate 
   * more on how it might affect for other type of operations */
  for (int i = 0; i < binfo->num_aggr; i++){    
    bool if_ignore_aggr_rule = false;
    size_t aggr_offset = binfo->aggr[i].field_offset;

    /* if aggr rule is same as any filter rule, just ignore it */
    for (int j = 0; j < binfo->num_filter_rules; j++) {
      
      size_t filter_offset = binfo->filter_rules[j].field_offset;      
      if (aggr_offset == filter_offset){
        if_ignore_aggr_rule = true;
        break;
      }
    }    
    
    /* if aggr rule is same as any grouper rule, just ignore it */
    for (int j = 0; j < binfo->num_group_modules; j++) {
      size_t group_offset_1 = binfo->group_modules[j].field_offset1;
      size_t group_offset_2 = binfo->group_modules[j].field_offset2;      

      if (aggr_offset == group_offset_1 || aggr_offset == group_offset_2){
        if_ignore_aggr_rule = true;
        break;
      }
    }

    group->aggr[i] = binfo->aggr[i].func(group->members, 
                                         group->num_members, 
                                         aggr_offset, 
                                         if_ignore_aggr_rule);
    if(!if_ignore_aggr_rule)
      *((u_int32_t*)(group_aggregation+aggr_offset)) = *group->aggr[i].values; 
  }
  
  
  return group_aggregation;  
}

struct uniq_records_tree *
build_record_trees(struct branch_info *binfo,
                   char **filtered_records, 
                   size_t num_filtered_records, 
                   struct grouper_rule *group_modules) {

  struct uniq_records_tree*           uniq_records_trees;
  struct tree_item_uint32_t*          uniq_records;
  size_t                              num_uniq_records;
  char***                             sorted_records;
  int                                 i;
  
  sorted_records = (char ***)malloc((num_filtered_records+1)*sizeof(char **));
  if (sorted_records == NULL)
    errExit("malloc");  
  for (i = 0; i < num_filtered_records; i++)
    sorted_records[i] = &filtered_records[i];
  
  // order by right hand side of comparison
  // TODO: different comp func sizes
  qsort_r(sorted_records, 
          num_filtered_records, 
          sizeof(char **), 
          (void *)&group_modules[0].field_offset2,
          comp_uint32_t);
  
  if(verbose_vv){  
    binfo->sorted_records = (char**)
                             malloc(num_filtered_records * sizeof(char*));
    if (binfo->sorted_records == NULL)
      errExit("malloc");
    
    for (int i = 0; i < num_filtered_records; i++)
      binfo->sorted_records[i] = *sorted_records[i];
    binfo->num_filtered_records = num_filtered_records;
    
    if_group_modules_exist = true;
  }
  
  uniq_records = (struct tree_item_uint32_t *)
                 malloc(num_filtered_records*sizeof(struct tree_item_uint32_t));
  if (uniq_records == NULL)
    errExit("malloc");  
  
  uniq_records[0].value = *(uint32_t *)
                          (*sorted_records[0] + group_modules[0].field_offset2);
  uniq_records[0].ptr = &sorted_records[0];
  num_uniq_records = 1;
  for (i = 0; i < num_filtered_records; i++) {
    if (*(uint32_t *)(*sorted_records[i] + 
      group_modules[0].field_offset2) != 
      uniq_records[num_uniq_records-1].value) {
      uniq_records[num_uniq_records].value = *(uint32_t *)
                                              (*sorted_records[i] + 
                                               group_modules[0].field_offset2);
      uniq_records[num_uniq_records].ptr = &sorted_records[i];
      num_uniq_records++;
    }
  }

  uniq_records = (struct tree_item_uint32_t *)
                  realloc(uniq_records, 
                          num_uniq_records*sizeof(struct tree_item_uint32_t));
  if (uniq_records == NULL)
    errExit("realloc");
  
  // mark the end of sorted records
  sorted_records[num_filtered_records] = NULL;
  
  uniq_records_trees = (struct uniq_records_tree *)
                        malloc(1 * sizeof(struct uniq_records_tree));
  if (uniq_records_trees == NULL)
    errExit("malloc");
  
  uniq_records_trees[0].type = UINT32_T;
  uniq_records_trees[0].num_uniq_records = num_uniq_records;
  uniq_records_trees[0].tree_item.uniq_records32 = uniq_records;
  uniq_records_trees[0].sorted_records = sorted_records;
  
  return uniq_records_trees;
}

struct group **
grouper(char **filtered_records, 
        size_t num_filtered_records,
        struct branch_info *binfo,
        struct grouper_rule *group_modules, 
        int num_group_modules,
        struct grouper_aggr *aggr, 
        size_t num_group_aggr, 
        size_t *num_groups) {
  
  struct group**                      groupset = NULL;
  struct group*                       group = NULL;
  struct uniq_records_tree*           uniq_records_trees;
  
  groupset = (struct group **)malloc(sizeof(struct group *));
  
  /* club all filtered records into one group, if no group modules are defined */
  if (num_group_modules == 0) {  
    
    /* groupset with space for 1 group */
    (*num_groups) = 1;
    groupset = (struct group **)malloc(sizeof(struct group*)**num_groups);
    if (groupset == NULL)
      errExit("malloc");      
    group = (struct group *)malloc(sizeof(struct group));      
    if (group == NULL)
      errExit("malloc");
    
    groupset[*num_groups - 1] = group;
    group->num_members = num_filtered_records;
    group->members = (char **)malloc(group->num_members * sizeof(char *));
    if (group->members == NULL)
      errExit("malloc");    
    
    for (int i = 0; i < num_filtered_records; i++) {
      group->members[i] = filtered_records[i];
      filtered_records[i] = NULL;
    }    
    
    /* save the start and finish times of the extreme members */
    group->start = *(u_int32_t*)(group->members[0] +
                                 (binfo->data->offsets).First);
    group->end = *(u_int32_t*)(group->members[group->num_members-1] +
                               (binfo->data->offsets).Last); 
    
    free(filtered_records);
  } 
  else {
    uniq_records_trees = build_record_trees(binfo,
                                            filtered_records,
                                            num_filtered_records, 
                                            group_modules);

    if(verbose_vv){  
      binfo->num_unique_records = uniq_records_trees->num_uniq_records;
      binfo->unique_records = (char**)
      malloc(binfo->num_unique_records * sizeof(char*));      
      if (binfo->unique_records == NULL)
        errExit("malloc");
      
      for (int i = 0; i < binfo->num_unique_records; i++)
        binfo->unique_records[i] = 
        **uniq_records_trees->tree_item.uniq_records32[i].ptr;
    }
    
    for (int i = 0; i < num_filtered_records; i++) {
      if (filtered_records[i] == NULL)
        continue;
      
      (*num_groups) += 1;
      
      groupset = (struct group **)
                  realloc(groupset, sizeof(struct group*) * (*num_groups));
      if (groupset == NULL)
        errExit("realloc");      
      group = (struct group *)malloc(sizeof(struct group));
      if (group == NULL)
        errExit("malloc");

      groupset[*num_groups-1] = group;
      group->num_members = 1;
      group->members = (char **)malloc(sizeof(char *));
      if (group->members == NULL)
        errExit("malloc");      
      group->members[0] = filtered_records[i];      

      // search for left hand side of comparison in records ordered by right
      // hand side of comparison
      char ***record_iter = 
              ((struct tree_item_uint32_t *)
              bsearch_r(filtered_records[i],
                        (void *)uniq_records_trees[0].tree_item.uniq_records32,
                        uniq_records_trees[0].num_uniq_records,
                        sizeof(struct tree_item_uint32_t),
                        (void *)&group_modules[0].field_offset1,
                        comp_uint32_t_p
                       ))->ptr;
      
      // iterate until terminating NULL in sorted_records
      for (;*record_iter != NULL; record_iter++) {
        // already processed record from filtered_records
        if (**record_iter == NULL)
          continue;
        
        // do not group with itself
        if (**record_iter == filtered_records[i])
          continue;
        
        // check all module filter rules for those two records
        int k;
        for (k = 0; k < num_group_modules; k++) {
          if (!group_modules[k].func(group, 
                                     group_modules[k].field_offset1,
                                     **record_iter, 
                                     group_modules[k].field_offset2, 
                                     group_modules[k].delta))
            break;
        }
        
        // first rule didnt match
        if (k == 0)
          break;
        
        // one of the other rules didnt match
        if (k < num_group_modules)
          continue;
        
        group->num_members += 1;
        
        group->members = (char **)
                            realloc(group->members, 
                                    sizeof(char *)*group->num_members);
        
        // assign entry in filtered_records to group
        group->members[group->num_members-1] = **record_iter; 
        **record_iter = NULL; // set entry in filtered_records to NULL
      }
      
      // unlink the filtered records from the flow data
      filtered_records[i] = NULL;
    }
    
    /* save the start and finish times of the extreme members */
    group->start = *(u_int32_t*)(group->members[0] +
                              (binfo->data->offsets).First);
    group->end = *(u_int32_t*)(group->members[group->num_members-1] +
                            (binfo->data->offsets).Last);    
    
    free(filtered_records);    
    
    // unlink the sorted records from the flow data
    for (int i = 0; i < num_filtered_records; i++)
      uniq_records_trees[0].sorted_records[i] = NULL;    
    free(uniq_records_trees[0].sorted_records);
    
    // unlink the uniq records from the flow data
    for (int i = 0; i < uniq_records_trees->num_uniq_records; i++)
      uniq_records_trees[0].tree_item.uniq_records32->ptr = NULL;
    free(uniq_records_trees[0].tree_item.uniq_records32);    
    free(uniq_records_trees);
  }

  return groupset;
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