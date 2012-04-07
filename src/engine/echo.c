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

#include "echo.h"

#ifdef DEBUGGGGGG  
void
echo_debug(){  

  /* -----------------------------------------------------------------------*/  
  /*                                debugging                               */
  /* -----------------------------------------------------------------------*/  
  
  if(verbose_v){
    
   
    /* merger */
    
#ifdef MERGER    
    if (verbose_vv) {
      
      struct permut_iter *iter = iter_init(fquery->branchset, 
                                           fquery->num_branches);
      printf("\nNo. of (to be) Matched Groups: %zu \n", 
             fquery->total_num_group_tuples);
      if (fquery->total_num_group_tuples != 0)      
        puts(FLOWHEADER);      
      while(iter_next(iter)) {
        for (int j = 0; j < fquery->num_branches; j++) {          
          flow_print_record(trace, 
                            fquery->branchset[j].
                            filtered_groupset[iter->filtered_group_tuple[j] - 1]
                            ->group_aggr_record);
        }
        printf("\n");
      }
      iter_destroy(iter);
    }
    
    printf("\nNo. of Merged Groups: %zu (Tuples)\n", 
           fquery->num_group_tuples);      
    if (fquery->num_group_tuples != 0)          
      puts(FLOWHEADER);
    
    for (int j = 0; j < fquery->num_group_tuples; j++) {
      struct group** group_tuple = fquery->group_tuples[j];
      for (int i = 0; i < fquery->num_branches; i++) {
        struct group* group = group_tuple[i];
        flow_print_record(trace, group->group_aggr_record);
      }
      printf("\n");
    }    
#endif
    
  }
  
  /* -----------------------------------------------------------------------*/      


}

#endif    


void
echo_filter(struct branch_info* branch){
  
  printf("\nNo. of Filtered Records: %zd\n", 
         branch->filter_result->num_filtered_records);      
  if (branch->filter_result->num_filtered_records != 0)
    puts(FLOWHEADER);      
  for (int j = 0; j < branch->filter_result->num_filtered_records; j++) {
    
    char* record = branch->filter_result->filtered_recordset[j];
    flow_print_record(branch->data, record);
  }
}

void
echo_grouper(struct branch_info* branch) {


  if(branch->num_grouper_rules > 0){
    
    printf("\nNo. of Sorted Records: %zd\n", 
           branch->filter_result->num_filtered_records);      
    if (branch->filter_result->num_filtered_records != 0)          
      puts(FLOWHEADER);      
    for (int j = 0; j < branch->filter_result->num_filtered_records; j++) {
      flow_print_record(branch->data, 
                        branch->grouper_result->sorted_recordset[j]);
      
      /* not free'd since they point to original records */
      branch->grouper_result->sorted_recordset[j] = NULL;
    }      
    free(branch->grouper_result->sorted_recordset);
    
    
    printf("\nNo. of Unique Records: %zd\n", branch->grouper_result->num_unique_records);      
    if (branch->grouper_result->num_unique_records != 0)          
      puts(FLOWHEADER);      
    for (int j = 0; j < branch->grouper_result->num_unique_records; j++) {
      flow_print_record(branch->data, 
                        branch->grouper_result->unique_recordset[j]);
      
      /* not free'd since they point to original records */
      branch->grouper_result->unique_recordset[j] = NULL;
    }      
    free(branch->grouper_result->unique_recordset);
  }      
  
  printf("\nNo. of Groups: %zu (Verbose Output)\n", 
         branch->grouper_result->num_groups);
  
  if (branch->grouper_result->num_groups > 0)
    puts(FLOWHEADER); 
  
  for (int j = 0; j < branch->grouper_result->num_groups; j++) {
    
    printf("\n");
    struct group* group = branch->grouper_result->groupset[j];
    
    /* print group members */ 
    for (int k = 0; k < group->num_members; k++)
      flow_print_record(branch->data, group->members[k]);
  }

}

void
echo_group_aggr(struct branch_info* branch){
  
  printf("\nNo. of Groups: %zu (Aggregations)\n", 
         branch->grouper_result->num_groups);
  
  if (branch->grouper_result->num_groups != 0)      
    puts(FLOWHEADER); 
  for (int j = 0; j < branch->grouper_result->num_groups; j++) {        
    struct group* group = branch->grouper_result->groupset[j];
    flow_print_record(branch->data, group->aggr_record);    
  }
}

void
echo_branch(size_t num_branches,
            struct branch_info* branchset,
            struct ft_data* trace){
  
  
  /* process each branch */
  for (int i = 0; i < num_branches; i++) {    
    struct branch_info* branch = &branchset[i];

#ifdef FILTER
    echo_filter(branch);
#endif
    

#ifdef GROUPER    
    if (verbose_vv){
      echo_grouper(branch);
    }
#endif    
    
    
#ifdef GROUPERAGGREGATIONS
    echo_group_aggr(branch);
#endif
    
    
#ifdef GROUPFILTER
    printf("\nNo. of Filtered Groups: %zu (Aggregations)\n", 
           branch->num_filtered_groups);      
    if (branch->num_filtered_groups != 0)      
      puts(FLOWHEADER); 
    
    for (int j = 0; j < branch->num_filtered_groups; j++) {
      
      struct group* filtered_group = branch->filtered_groupset[j];
      flow_print_record(branch->data, filtered_group->group_aggr_record);
    }
#endif
  }
}

void 
echo_results(size_t num_streams,
             struct stream** streamset,
             struct ft_data* trace) {
  
  
  /* -----------------------------------------------------------------------*/  
  /*                                results                                 */
  /* -----------------------------------------------------------------------*/  
  
  printf("\nNo. of Streams: %zu \n", num_streams);
  printf("----------------- \n");
  
  for (int j = 0; j < num_streams; j++) {
    
    struct stream* stream = streamset[j];
    printf("\nNo. of Records in Stream (%d): %zu \n",j+1, stream->num_records);
    if (stream->num_records != 0)
      puts(FLOWHEADER);
    for (int i = 0; i < stream->num_records; i++) {
      char* record = stream->recordset[i];
      flow_print_record(trace, record);
    }
    printf("\n");
  }
  
  /* -----------------------------------------------------------------------*/
  
}