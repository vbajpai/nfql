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

#include "grouper_fptr.h"

/*
 * variables are named "tree" but there are no trees, just arrays. But those
 * arrays are used to binary search them in a "tree-like" fashion and hence
 * the naming
 *
 */
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
  
  if(debug){  
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
  
  if (num_group_modules == 0) {    
    for (int i = 0; i < num_filtered_records; i++) {
      
      (*num_groups) += 1;
      groupset = (struct group **)realloc(groupset, 
                                          sizeof(struct group*)**num_groups);
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
      filtered_records[i] = NULL;
    }    
    free(filtered_records);
  } 
  else {
    uniq_records_trees = build_record_trees(binfo,
                                            filtered_records,
                                            num_filtered_records, 
                                            group_modules);

    if(debug){  
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

  
  
#ifdef GROUPAGGR
  for (int i = 0; i < *num_groups; i++) {
    groupset[i]->aggr = (struct aggr *)
                         malloc(sizeof(struct aggr)*num_group_aggr);
    if (groupset[i]->aggr == NULL)
      errExit("malloc");
    
    for (int j = 0; j < num_group_aggr; j++)
      groupset[i]->aggr[j] = aggr[j].func(groupset[i]->members, 
                                          groupset[i]->num_members, 
                                          aggr[j].field_offset);
  }
#endif
  
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