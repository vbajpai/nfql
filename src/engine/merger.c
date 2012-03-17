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
merger(struct group ***filtered_groupset_collection, 
       size_t *num_filtered_groupset, 
       int num_threads, 
       struct merger_rule *filter, 
       int num_filter_rules) {
  
  struct group ***group_tuples;
  //    int num_group_tuples;
  //    struct group **temp_tuple;
  //    int i, j;
  int i;
  
  size_t* offsets = (size_t *)calloc(num_threads, sizeof(size_t));  
  if (offsets == NULL)
    errExit("calloc");
    
  struct permut_iter* iter = iter_init(offsets, num_filtered_groupset, num_threads);

  do {
    // break if any of the groups is already grouped
    for (i = 0; i < num_threads; i++) {
      if (filtered_groupset_collection[i][iter->array[i]] == NULL) goto cont;
    }

#ifdef PP    
    for (i = 0; i < num_filter_rules; i++) {
      printf("%p\n", filtered_groups[filter[i].branch1][iter->array[filter[i].branch1]]);
      printf("%p\n", filtered_groups[filter[i].branch1][iter->array[filter[i].branch1]]->aggr);
      /*
       if (!filter[i].func(filtered_groups[filter[i].branch1][iter->array[filter[i].branch1]], filter[i].field1,
       filtered_groups[filter[i].branch2][iter->array[filter[i].branch2]], filter[i].field2,
       filter[i].delta)) goto cont;
       */
    }
    /*
     for (i = 0; i < num_threads; i++) {
     printf("%zd ", iter->array[i]);
     }
     printf("\n");
     */
#endif    
    
  cont:   continue;
  } while (iter_next(iter));
  
  group_tuples = NULL;
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