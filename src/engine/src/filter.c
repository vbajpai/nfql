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

#include "filter.h"

struct filter_result*
filter(
       size_t num_records,
       struct record** const recordset,
       
       size_t num_filter_rules,
       struct filter_rule** const filter_ruleset
      ) {
  
  /* free'd before exiting from main(...) */
  struct filter_result* fresult = calloc(1, sizeof(struct filter_result));
  if (fresult == NULL)
    errExit("calloc");
  
  /* free'd before exiting from main(...) */
  fresult->filtered_recordset = (char **)
                                 calloc(fresult->num_filtered_records,
                                        sizeof(char *));
  if (fresult->filtered_recordset == NULL)
    errExit("calloc");
  
  /* process each record */
  for (int i = 0, j = 0; i < num_records; i++) {
    
    char* record = recordset[i]->record;
    
    /* process each filter rule, for each record */
    for (j = 0; j < num_filter_rules; j++) {
      
      struct filter_rule* const frule = filter_ruleset[j];
      
      /* get a uintX_t specific function depending on frule->op */
      assign_filter_func(frule);
      
      /* run the comparator function of the filter rule on the record */
      if (!frule->func(
                       record, 
                       frule->field_offset, 
                       frule->value, 
                       frule->delta
                      ))
        break;
    }
    
    /* if any rule is not satisfied */
    if (j < num_filter_rules)
      continue;
    /* else, increment the filter counter, and save this record */
    else {

      recordset[i]->if_filtered = true;
      fresult->num_filtered_records += 1;
      fresult->filtered_recordset = (char **) 
                       realloc(fresult->filtered_recordset,
                              (fresult->num_filtered_records)*sizeof(char *));
      if (fresult->filtered_recordset == NULL)
        errExit("realloc");      
      fresult->filtered_recordset[fresult->num_filtered_records - 1] = record;
    }
  }
  
  if (fresult->num_filtered_records == 0) {
    free(fresult->filtered_recordset); fresult->filtered_recordset = NULL;
  }  
  return fresult;
}