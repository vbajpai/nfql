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

#include "groupfilter.h"

struct group **
groupfilter(struct group **groupset, 
             size_t num_groups, 
             struct gfilter_rule *rules, 
             size_t num_gfilter_rules,
             size_t *num_filtered_groups) {
  
  struct group **filtered_groupset;  
  filtered_groupset = (struct group **)
                       calloc(*num_filtered_groups, 
                              sizeof(struct group *));
  if (filtered_groupset == NULL)
    errExit("calloc");
  
  int i, j;
  /* iterate over each group */
  for (i = 0, j = 0; i < num_groups; i++) {
    
    struct group* group = groupset[i];
    
    /* iterate over each group filter rule */
    for (j = 0; j < num_gfilter_rules; j++) {
      
      /* break out if any one of the rules does NOT match */
      if (!rules[j].func(group, 
                         rules[j].field, 
                         rules[j].value, 
                         rules[j].delta))
        break;
    }
    
    /* free the group, if it did not pass the filter rules  */
    if (j < num_gfilter_rules) {
      
      /* free only when NO verbosity is set, otherwise free later. */
      if(!verbose_v){
        free(group->members);
        free(group->aggr);
        free(group);
        group = NULL;
      }      
    }
    
    /* otherwise add the group to the filtered groupset */
    else {
      
      (*num_filtered_groups)++;
      filtered_groupset = 
      (struct group **)realloc(filtered_groupset, 
                              (*num_filtered_groups)*sizeof(struct group *));
      if (filtered_groupset == NULL)
        errExit("realloc");      
      filtered_groupset[*num_filtered_groups-1] = groupset[i];
    }
  }
  return filtered_groupset;
}