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

struct groupfilter_result*
groupfilter(
            size_t num_gfilter_rules,
            struct gfilter_rule** const ruleset,
            
            const struct grouper_result* const gresult
            ) {
  
  /* free'd just before calling ungrouper(...) */
  struct groupfilter_result* 
  gfilter_result = calloc(1, sizeof(struct groupfilter_result));
  if (gfilter_result == NULL)
    errExit("calloc");
  
  /* iterate over each group */
  for (int i = 0, j = 0; i < gresult->num_groups; i++) {
    
    struct group* group = gresult->groupset[i];
    
    /* iterate over each group filter rule */
    for (j = 0; j < num_gfilter_rules; j++) {
      
      struct gfilter_rule* rule = ruleset[j];
      
      /* assign a specific uintX_t function depending on rule->op */
      assign_gfilter_func(rule);
      
      /* break out if any one of the rules does NOT match */
      if (
          !rule->func(
                      group, 
                      rule->field, 
                      rule->value, 
                      rule->delta
                     )
         )
        break;
    }
    
    /* continue, if it did not pass the filter rules  */
    if (j < num_gfilter_rules) 
      continue;
    
    /* otherwise add the group to the filtered groupset */
    else {      
      gfilter_result->num_filtered_groups += 1;

      /* free'd just after returning from ungrouper(...) */
      gfilter_result->filtered_groupset = (struct group**)
      realloc(gfilter_result->filtered_groupset, 
             (gfilter_result->num_filtered_groups) * sizeof(struct group*));
      if (gfilter_result->filtered_groupset == NULL)
        errExit("realloc");
      gfilter_result->filtered_groupset
      [gfilter_result->num_filtered_groups-1] = gresult->groupset[i];
    }
  }
  
  return gfilter_result;
}