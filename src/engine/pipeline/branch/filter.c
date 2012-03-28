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

char ** 
filter(struct ft_data *data, struct filter_rule *filter_rules, 
       int num_filter_rules, size_t *num_filtered_records) {
  
  char** filtered_records = (char **)
                            calloc(*num_filtered_records,
                                   sizeof(char *));
  if (filtered_records == NULL)
    errExit("calloc");
  
  /* process each record */
  for (int i = 0; i < data->num_records; i++) {
    /* process each filter rule, for each record */
    int j = 0;
    for (j = 0; j < num_filter_rules; j++) {
      /* run the comparator function of the filter rule on the record */
      if (!filter_rules[j].func(data->records[i], 
                                filter_rules[j].field_offset, 
                                filter_rules[j].value, 
                                filter_rules[j].delta))
        break;
    }
    
    /* if any rule is not satisfied, move on to another record */
    if (j < num_filter_rules)
      continue;
    /* else, increment the filter counter, and save this record */
    else {      
      (*num_filtered_records)++;
      filtered_records = (char **)
      realloc(filtered_records,
              (*num_filtered_records)*sizeof(char *));
      if (filtered_records == NULL)
        errExit("malloc");
      
      filtered_records[*num_filtered_records-1] = data->records[i];
    }
  }
  
  /* if nothing got filtered, 
   * free the memory allocated and return NULL
   */
  if (*num_filtered_records == 0) {
    if (filtered_records != NULL) {
      free(filtered_records);
      filtered_records = NULL;
    }
  }  
  return filtered_records;
}