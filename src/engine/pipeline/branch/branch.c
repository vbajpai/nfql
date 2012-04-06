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
  
  struct branch_info* branch = (struct branch_info *)arg;  
  
  
  
  
  
  
  
  
  
  
#ifdef FILTER  
  
  /* -----------------------------------------------------------------------*/  
  /*                                filter                                  */
  /* -----------------------------------------------------------------------*/  
  
  branch->filter_result = filter(branch);
  if (branch->filter_result == NULL)
    errExit("filter(...) returned NULL");
  
  /* -----------------------------------------------------------------------*/
  
#endif  
  
  
  
  
  
  
  
  
  
  
#ifdef GROUPER  
  
  /* -----------------------------------------------------------------------*/  
  /*                               grouper                                  */
  /* -----------------------------------------------------------------------*/  
  
  
//  branch->grouper_result = grouper(branch->filter_result->num_filtered_records, 
//                                   branch,
//                                   branch->group_modules, 
//                                   branch->num_group_modules, 
//                                   branch->aggr, 
//                                   branch->num_aggr);
  
  branch->grouper_result = grouper(branch);
  if (branch->grouper_result == NULL)
    errExit("grouper(...) returned NULL");
  else {

    /* free filter rules */
    for (int i = 0; i < branch->num_filter_rules; i++) {
      struct filter_rule* frule = branch->filter_ruleset[i];
      free(frule); frule = NULL; branch->filter_ruleset[i] = NULL;      
    }
    free(branch->filter_ruleset); branch->filter_ruleset = NULL;

    /* free grouper rules */
    for (int i = 0; i < branch->num_grouper_rules; i++) {
      struct grouper_rule* grule = branch->grouper_ruleset[i];
      free(grule); grule = NULL; branch->grouper_ruleset[i] = NULL;
    }
    free(branch->grouper_ruleset); branch->grouper_ruleset = NULL;
    
    /* free grouper aggregation rules */
    for (int i = 0; i < branch->num_aggr_rules; i++) {
      struct aggr_rule* arule = branch->aggr_ruleset[i];
      free(arule); arule = NULL; branch->aggr_ruleset[i] = NULL;
    }
    free(branch->aggr_ruleset); branch->aggr_ruleset = NULL;
  }
  
  /* -----------------------------------------------------------------------*/
  
#endif  
  
  
  
  
  
  
  
  
  
  
#ifdef GROUPFILTER
  
  /* -----------------------------------------------------------------------*/  
  /*                            grouper-filter                              */
  /* -----------------------------------------------------------------------*/  
  
  binfo->filtered_groupset = groupfilter(binfo->groupset, 
                                          binfo->num_groups, 
                                          binfo->gfilter_rules, 
                                          binfo->num_gfilter_rules, 
                                          &binfo->num_filtered_groups);
  
  /* -----------------------------------------------------------------------*/
  
#endif  
  
  
  
  
  
  
  
  
  
  
  pthread_exit(NULL);
}