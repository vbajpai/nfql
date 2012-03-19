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
       size_t num_branches, 
       struct merger_rule* m_rules, 
       size_t num_merger_rules,
       size_t *total_num_group_tuples,
       size_t *num_group_tuples) {
  
  /* initialize the iterator */
  struct permut_iter *iter = iter_init(binfo_set, num_branches);
  struct group*** group_tuples = NULL;
  *num_group_tuples = 0;
  
  /* iterate over all permutations */
  unsigned int index = 0;
  while(iter_next(iter)) {
    bool if_all_rules_matched = true;
    index++;

    /* match the groups against each merger rule */
    for (int i = 0; i < num_merger_rules; i++) {
      
      struct merger_rule rule = m_rules[i];
      size_t group1_id = iter->filtered_group_tuple[rule.branch1->branch_id];
      size_t group2_id = iter->filtered_group_tuple[rule.branch2->branch_id];                                                    

      if (!rule.func(rule.branch1->filtered_groupset[group1_id-1],
                     rule.field1,
                     rule.branch2->filtered_groupset[group2_id-1],
                     rule.field2,
                     0)){        
        if_all_rules_matched = false;
        break;
      }      
    }
    
    /* add the groups to the group tuple, if all rules matched */
    if(if_all_rules_matched){

      struct group **matched_tuple = (struct group **)
                                     calloc(num_branches, 
                                            sizeof(struct group *));
      if (matched_tuple == NULL)
        errExit("calloc");

      /* save the groups in the matched tuple */
      for (int j = 0; j < num_branches; j++){
        size_t groupID = iter->filtered_group_tuple[j];
        matched_tuple[j] = binfo_set[j].filtered_groupset[groupID-1];
      }
      
      *num_group_tuples += 1;
      group_tuples = (struct group ***)
                     realloc(group_tuples,
                             (*num_group_tuples)*sizeof(struct group**));
      if (group_tuples == NULL)
        errExit("realloc");      

      group_tuples[*num_group_tuples-1] = matched_tuple;
    }  
  };
  
  *total_num_group_tuples = index;
  iter_destroy(iter);
  return group_tuples;
}