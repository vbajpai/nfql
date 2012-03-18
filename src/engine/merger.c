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

struct group ***
merger(struct branch_info* binfo_set, 
       int num_threads, 
       struct merger_rule* m_rules, 
       int num_merger_rules) {
  
  struct permut_iter *iter;
  size_t num_filtered_groups[] = {6, 3};
  
  iter = iter_init(num_filtered_groups, num_threads);
  unsigned int index = 0;
  do {
    index++;
    printf("%d: (%zu %zu)\n", index, iter->array[0], iter->array[1]);
  } while (iter_next(iter));
  
  iter_destroy(iter);
  
#ifdef OLDMERGER  
  
  size_t* offsets = (size_t *)calloc(num_threads, sizeof(size_t));  
  if (offsets == NULL)
    errExit("calloc");
    
  struct permut_iter* iter = iter_init(offsets, 
                                       num_filtered_groupset, 
                                       num_threads);

  do {
    // break if any of the groups is already grouped
    bool ifgrouped = false;
    for (int i = 0; i < num_threads; i++) {
      struct group** filtered_groupset = binfo_set[i].filtered_groupset;
      if (filtered_groupset[iter->array[i]] == NULL){
        ifgrouped = true;
        break;
      } 
    }  
    
    if(!ifgrouped) {    
      for (int i = 0; i < num_merger_rules; i++) {
        struct merger_rule rule = m_rules[i];
        if (!m_rules[i].func(binfo_set[i].filtered_groupset[j], 
                             rule.field1,
                             filtered_groupset_collection[rule.branch2][iter->array[rule.branch2]], 
                             rule.field2,
                             rule.delta))
          break;
      }      
    }

  } while (iter_next(iter));

#endif
  
  struct group ***group_tuples = NULL;
  
  //    int num_group_tuples;
  //    struct group **temp_tuple;
  //    int i, j;
  //    num_group_tuples = 0;
  
  /*
   for (i = 0; filtered_groups[0][i]->aggr != NULL; i++) {
   for (j = 0; filtered_groups[1][j]->aggr != NULL; j++) {
   if (!filter[0].func(filtered_groups[0][i], filter[0].field1, filtered_groups[1][j], filter[0].field2, filter[0].delta)
   || !filter[1].func(filtered_groups[0][i], filter[1].field1, filtered_groups[1][j], filter[1].field2, filter[1].delta)
   )
   continue;
   
   temp_tuple = (struct group **)malloc(sizeof(struct group *)*num_threads);
   if (temp_tuple == NULL) {
   perror("malloc");
   exit(EXIT_FAILURE);
   }
   
   temp_tuple[0] = filtered_groups[0][i];
   temp_tuple[1] = filtered_groups[1][j];
   
   group_tuples = (struct group ***)realloc(group_tuples, sizeof(struct group**)*(num_group_tuples+1));
   1if (group_tuples == NULL) {
   perror("malloc");
   exit(EXIT_FAILURE);
   }
   
   group_tuples[num_group_tuples] = temp_tuple;
   num_group_tuples++;
   }
   }
   
   group_tuples = (struct group ***)realloc(group_tuples, sizeof(struct group**)*(num_group_tuples+1));
   if (group_tuples == NULL) {
   perror("malloc");
   exit(EXIT_FAILURE);
   }
   group_tuples[num_group_tuples] = NULL;
   
   printf("number of group tuples: %d\n", num_group_tuples);
   
   */

  
  return group_tuples;
}