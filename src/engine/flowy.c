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
#include "flowy.h"

int 
main(int argc, char **argv) {
  
  /* command line parsing variables */
  int                                 opt;
  enum verbosity_levels               verbose_level;
  char*                               trace_filename;
  char*                               query_filename;
  static struct option                longopts[] = {
    { "debug",      no_argument,        NULL,           'd' },
    { "verbose",    required_argument,  NULL,           'v' },    
    {  NULL,        0,                  NULL,            0  }
  };

  
  /* ftreader variables */
  struct ft_data*                     trace_data;
  int                                 fsock;
  
  /* json parser variables */
  char*                               query_mmap_data;
  struct stat                         sb;
  struct json_object*                 query_json;
  struct json_object*                 filter_offset_json;  
  struct filter_offset*               filter_offset;
  struct filter_rules_params*         filter_rules_params;
  
  /* flowquery variables */
  struct flowquery*                   fquery;
  
  
  /* branch_info variables */
  int                                 num_threads;
  int                                 i, ret;
  pthread_t*                          thread_ids;
  pthread_attr_t*                     thread_attrs;
  struct branch_info*                 binfos;
  
  
  
  
  
  
  /* ----------------------------------------------*/  
  /*         parsing command line arguments        */
  /* ----------------------------------------------*/
  
  while ((opt = getopt_long(argc, argv, "v:d", longopts, NULL)) != -1) {
    switch (opt) {
      case 'd': debug = TRUE; verbose_level = HIGH;
      case 'v': if (optarg)
                  verbose_level = atoi(optarg); 
                switch (verbose_level) {
                  case HIGH: verbose_vvv = TRUE;                       
                  case MEDIUM: verbose_vv = TRUE;                       
                  case LOW: verbose_v = TRUE; break;
                  default: errExit("valid verbosity levels: (1-3)");                      
                }
                break;        
      case ':': usageError(argv[0], "Missing argument", optopt);
      default: exit(EXIT_FAILURE);
    }
  }
  
  if (argc != optind + 2)
    usageErr("%s $TRACE $QUERY\n", argv[0], argv[0]);
  else {
    trace_filename = argv[optind];
    query_filename = argv[optind+1];
  }

  /* ----------------------------------------------*/
  
  
  
  
  
  
  
  /* ----------------------------------------------*/  
  /*     reading the input trace into a struct     */
  /* ----------------------------------------------*/   
  
  fsock = open(trace_filename, O_RDONLY);
  if (fsock == -1)
    errExit("open"); 
  trace_data = ft_open(fsock);
  if (close(fsock) == -1)
    errExit("close");
  
  /* ----------------------------------------------*/
  
  
  
  
  
  
  
  
  /* ----------------------------------------------*/  
  /*     reading the input query into a mmap       */
  /* ----------------------------------------------*/    
  
  fsock = open(query_filename, O_RDONLY);
  if (fsock == -1)
    errExit("open");
  if (fstat(fsock, &sb) == -1)
    errExit("fstat");
  query_mmap_data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fsock, 0); 
  if (query_mmap_data == MAP_FAILED)
    errExit("mmap");
  if (close(fsock) == -1)
    errExit("close");
  
  /* ----------------------------------------------*/
  
  
  
  
  
  
  
  /* ----------------------------------------------*/  
  /*        parse the json into a struct           */
  /* ----------------------------------------------*/   

  filter_rules_params = calloc(1, sizeof(filter_rules_params));
  filter_offset = calloc(1, sizeof(filter_offset));
  filter_rules_params->off = filter_offset; 
  
  query_json = json_tokener_parse(query_mmap_data);
  filter_rules_params->delta = 
  json_object_get_int(json_object_object_get(query_json, "delta"));
  filter_rules_params->op = 
  json_object_get_string(json_object_object_get(query_json, "op"));  
  filter_offset_json = 
  json_object_object_get(query_json, "offset");
  filter_rules_params->off->name = 
  json_object_get_string(json_object_object_get(filter_offset_json, "name"));  
  filter_rules_params->off->value = 
  json_object_get_int(json_object_object_get(filter_offset_json, "value"));
  filter_rules_params->off->datatype = 
  json_object_get_string(json_object_object_get(filter_offset_json, "datatype"));

  json_object_object_del(filter_offset_json, "offset"); 
  json_object_object_del(query_json, "");  
  if (munmap(query_mmap_data, sb.st_size) == -1)
    errExit("munmap");
  
  /* ----------------------------------------------*/ 
  
  
  
   
  
  
  
  
  
  
  /* ---------------------------------------------*/  
  /*    creating branch_info for each thread      */
  /* ---------------------------------------------*/  
  
  fquery = (struct flowquery *)malloc(sizeof(struct flowquery));
  if (fquery == NULL) 
    errExit("malloc");
  num_threads = 2;
  binfos = (struct branch_info *)
            calloc(num_threads, 
                   sizeof(struct branch_info));
  if (binfos == NULL)
    errExit("calloc");
  
  fquery->num_branches = num_threads;
  fquery->branches = binfos;
  
  /* ----------------------------------------------*/


  
  
  
    
  /* -----------------------------------------------------------------------*/  
  /*    filter, grouper, grouper aggregation rules for branch 1             */
  /* -----------------------------------------------------------------------*/  
 
  
  /* array of filter rules, with one filter */
  struct filter_rule filter_rules_branch1[1] = {
      { trace_data->offsets.dstport, 
        filter_rules_params->off->value, 
        filter_rules_params->delta,
        RULE_EQ | RULE_S1_16, 
        NULL
      },
  };
  
  
  /* array of grouper rules, with 2 groupers */
  struct grouper_rule group_module_branch1[2] = {
    
    // shouldn't this be _16 ?
    { trace_data->offsets.srcaddr, trace_data->offsets.srcaddr, 0, 
      RULE_EQ | RULE_NO | RULE_S2_32 | RULE_S1_32, NULL },
    
    { trace_data->offsets.dstaddr, trace_data->offsets.dstaddr, 0, 
      RULE_EQ | RULE_NO | RULE_S2_32 | RULE_S1_32, NULL },
    
    //{ data->offsets.Last, data->offsets.First, 1, grouper_lt_uint32_t_rel }
  };  
  

  /* array of grouper aggregation rules, with 4 grouper aggrs */
  struct grouper_aggr group_aggr_branch1[4] = {
    
    // shouldn't the first field be -1 as indicated?
    { 0, trace_data->offsets.srcaddr, RULE_STATIC | RULE_S1_32, NULL },
    { 0, trace_data->offsets.dPkts, RULE_SUM | RULE_S1_32, NULL },
    { 0, trace_data->offsets.dOctets, RULE_SUM | RULE_S1_32, NULL },
    { 0, trace_data->offsets.tcp_flags, RULE_OR | RULE_S1_16, NULL }
  };

  
  /* array of grouper filter rules, with 0 filters */
  struct gfilter_rule gfilter_branch1[1] = {
    {trace_data->offsets.dPkts, 200, 0, RULE_GT, NULL}
  };
  
  
  /* filling up the branch_info struct */
  binfos[0].branch_id = 0;
  binfos[0].data = trace_data;
  binfos[0].filter_rules = filter_rules_branch1;
  binfos[0].num_filter_rules = 1;
  binfos[0].group_modules = group_module_branch1;
  binfos[0].num_group_modules = 2;
  binfos[0].aggr = group_aggr_branch1;
  binfos[0].num_aggr = 4;
  binfos[0].gfilter_rules = gfilter_branch1;
  binfos[0].num_gfilter_rules = 1;
  
 /* -----------------------------------------------------------------------*/
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*    filter, grouper, grouper aggregation rules for branch 2             */
  /* -----------------------------------------------------------------------*/  
  
  /* array of filter rules, with one filter */
  struct filter_rule filter_rules_branch2[1] = {
      { trace_data->offsets.srcport,
        filter_rules_params->off->value, 
        filter_rules_params->delta,
        RULE_EQ | RULE_S1_16, 
        NULL},    
  };


  /* array of grouper rules, with 2 groupers */
  struct grouper_rule group_module_branch2[2] = {
    { trace_data->offsets.srcaddr, trace_data->offsets.srcaddr, 0, 
      RULE_EQ | RULE_NO | RULE_S2_32 | RULE_S1_32, NULL },
    { trace_data->offsets.dstaddr, trace_data->offsets.dstaddr, 0, 
      RULE_EQ | RULE_NO | RULE_S2_32 | RULE_S1_32, NULL },
    
    //{ data->offsets.Last, data->offsets.First, 1, grouper_lt_uint32_t_rel },
  };

  
  /* array of grouper aggregation rules, with 4 grouper aggrs */
  struct grouper_aggr group_aggr_branch2[4] = {
    { 0, trace_data->offsets.srcaddr, RULE_STATIC | RULE_S1_32, NULL },
    { 0, trace_data->offsets.dPkts, RULE_SUM | RULE_S1_32, NULL },
    { 0, trace_data->offsets.dOctets, RULE_SUM | RULE_S1_32, NULL },
    { 0, trace_data->offsets.tcp_flags, RULE_OR | RULE_S1_16, NULL }
  };

  
  /* array of grouper filter rules, with 0 filters */
  struct gfilter_rule gfilter_branch2[1] = {
    {trace_data->offsets.dPkts, 200, 0, RULE_GT, NULL}
  };

  
  /* filling up the branch_info struct */  
  binfos[1].branch_id = 1;
  binfos[1].data = trace_data;
  binfos[1].filter_rules = filter_rules_branch2;
  binfos[1].num_filter_rules = 1;
  binfos[1].group_modules = group_module_branch2;
  binfos[1].num_group_modules = 2;
  binfos[1].aggr = group_aggr_branch2;
  binfos[1].num_aggr = 4;
  binfos[1].gfilter_rules = gfilter_branch2;
  binfos[1].num_gfilter_rules = 1;
  
  
  /* deallocate the query buffers */
  free(filter_offset);
  free(filter_rules_params);

  /* -----------------------------------------------------------------------*/
  
  
  

  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                           merger rules                                 */
  /* -----------------------------------------------------------------------*/
  
  
  struct merger_rule mfilter[2] = {
    {&binfos[0], trace_data->offsets.srcaddr, 
     &binfos[1], trace_data->offsets.dstaddr, 
     RULE_EQ | RULE_S1_32 | RULE_S2_32,  
     0,
     NULL},
    
    {&binfos[0], trace_data->offsets.dstaddr, 
     &binfos[1], trace_data->offsets.srcaddr,
     RULE_EQ | RULE_S1_32 | RULE_S2_32, 
     0,
     NULL},
  };
  
  fquery->num_merger_rules = 2;
  fquery->mrules = mfilter;

  /* -----------------------------------------------------------------------*/  
  
  
  

  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*            fills filter_rule.func and grouper_rule.func                */
  /*                        from the grouper_rule.op                        */
  /*             by falling through a huge switch statement                 */
  /* -----------------------------------------------------------------------*/    
  
  assign_fptr(fquery, binfos, num_threads);
  
  /* -----------------------------------------------------------------------*/

  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*               splitter → filter → grouper → group-filter               */
  /* -----------------------------------------------------------------------*/   
  

  /* allocate space for a dedicated thread for each branch */
  thread_ids = (pthread_t *)calloc(num_threads, sizeof(pthread_t));
  if (thread_ids == NULL)
    errExit("malloc");
  thread_attrs = (pthread_attr_t *)calloc(num_threads, sizeof(pthread_attr_t));
  if (thread_attrs == NULL)
    errExit("malloc");
  
  for (i = 0; i < num_threads; i++) {

    /* initialize each thread attributes */
    ret = pthread_attr_init(&thread_attrs[i]);
    if (ret != 0)
      errExit("pthread_attr_init");
    
    /* start each thread for a dedicated branch */      
    ret = pthread_create(&thread_ids[i], &thread_attrs[i], 
                         &branch_start, (void *)(&binfos[i]));    
    if (ret != 0)
      errExit("pthread_create");
    
    
    /* destroy the thread attributes once the thread is created */  
    ret = pthread_attr_destroy(&thread_attrs[i]);
    if (ret != 0) 
      errExit("pthread_attr_destroy");
  }
  
  /* - wait for each thread to complete its branch
   * - save and print the number of filtered groups on completion
   * - save the filtered groups on completion */
  for (i = 0; i < num_threads; i++) {
    ret = pthread_join(thread_ids[i], NULL);
    if (ret != 0)
      errExit("pthread_join");
  }
  
  free(thread_ids);
  free(thread_attrs);
  
  /* -----------------------------------------------------------------------*/    
  
  
  
  
  
  
  
  
  
  
#ifdef MERGER
  
  /* -----------------------------------------------------------------------*/  
  /*                                 merger                                 */
  /* -----------------------------------------------------------------------*/
  
  fquery->group_tuples = merger(binfos,
                                num_threads, 
                                mfilter, 
                                fquery->num_merger_rules,
                                &fquery->total_num_group_tuples,
                                &fquery->num_group_tuples);
  
  /* -----------------------------------------------------------------------*/    
  
#endif  
  
  

  
  
  
  
  
  
  
#ifdef UNGROUPER  
  
  /* -----------------------------------------------------------------------*/  
  /*                                ungrouper                               */
  /* -----------------------------------------------------------------------*/  
  // TODO: free group_collections at some point
  /* -----------------------------------------------------------------------*/    
  
  
#endif  
  
  
  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                                debugging                               */
  /* -----------------------------------------------------------------------*/  

  if(verbose_v){
    
    /* process each branch */
    for (int i = 0; i < num_threads; i++) {
      
      printf("\nNo. of Filtered Records: %zd\n", binfos[i].num_filtered_records);      
      puts(FLOWHEADER);      
      for (int j = 0; j < binfos[i].num_filtered_records; j++) {
        flow_print_record(binfos[i].data, binfos[i].filtered_records[j]);
        
        /* not free'd since they point to original records */
        binfos[i].filtered_records[j] = NULL;
      }      
      free(binfos[i].filtered_records);
      
      if (verbose_vv){
        if(if_group_modules_exist){
          
          printf("\nNo. of Sorted Records: %zd\n", binfos[i].num_filtered_records);      
          puts(FLOWHEADER);      
          for (int j = 0; j < binfos[i].num_filtered_records; j++) {
            flow_print_record(binfos[i].data, binfos[i].sorted_records[j]);
            
            /* not free'd since they point to original records */
            binfos[i].sorted_records[j] = NULL;
          }      
          free(binfos[i].sorted_records);
          
          
          printf("\nNo. of Unique Records: %zd\n", binfos[i].num_unique_records);      
          puts(FLOWHEADER);      
          for (int j = 0; j < binfos[i].num_unique_records; j++) {
            flow_print_record(binfos[i].data, binfos[i].unique_records[j]);
            
            /* not free'd since they point to original records */
            binfos[i].unique_records[j] = NULL;
          }      
          free(binfos[i].unique_records);      
          
        }      
        
        printf("\nNo. of Groups: %zu (Verbose Output)\n", binfos[i].num_groups);
        puts(FLOWHEADER); 
        for (int j = 0; j < binfos[i].num_groups; j++) {
          
          printf("\n");
          struct group* group = binfos[i].groupset[j];
          
          /* print group members */ 
          for (int k = 0; k < group->num_members; k++) {
            flow_print_record(binfos[i].data, group->members[k]);
            group->members[k] = NULL;
          }     
          free(group->members);
        }
      }
      
      
#ifdef GROUPERAGGREGATIONS
      printf("\nNo. of Groups: %zu (Aggregations)\n", binfos[i].num_groups);
      puts(FLOWHEADER); 
      for (int j = 0; j < binfos[i].num_groups; j++) {        
        struct group* group = binfos[i].groupset[j];
        flow_print_record(binfos[i].data, group->group_aggr_record);  
        
        for (int x = 0; x < binfos[i].num_aggr; x++)
          free(group->aggr[x].values);
        free(group->aggr); 
      }
#endif
      
      
#ifdef GROUPFILTER
      printf("\nNo. of Filtered Groups: %zu (Aggregations)\n", 
             binfos[i].num_filtered_groups);      
      puts(FLOWHEADER); 
      
      for (int j = 0; j < binfos[i].num_filtered_groups; j++) {
        
        struct group* filtered_group = binfos[i].filtered_groupset[j];
        flow_print_record(binfos[i].data, filtered_group->group_aggr_record);
      }
#endif
      
      
      /* free memory */
      for (int j = 0; j < binfos[i].num_groups; j++) {        
        struct group* group = binfos[i].groupset[j];
        free(group->group_aggr_record);
        free(group);
      }
      free(binfos[i].groupset);
    }
    
    /* merger */

#ifdef MERGER    
    if (verbose_vv) {
      
      struct permut_iter *iter = iter_init(fquery->branches, 
                                           fquery->num_branches);
      printf("\nNo. of (to be) Matched Groups: %zu \n", 
             fquery->total_num_group_tuples);
      puts(FLOWHEADER);      
      while(iter_next(iter)) {
        for (int j = 0; j < fquery->num_branches; j++) {          
          flow_print_record(trace_data, 
                            fquery->branches[j].
                            filtered_groupset[iter->filtered_group_tuple[j] - 1]
                            ->group_aggr_record);
        }
        printf("\n");
      }
      iter_destroy(iter);
    }
    
    printf("\nNo. of Merged Groups: %zu (Tuples)\n", 
           fquery->num_group_tuples);      
    puts(FLOWHEADER);
    
    for (int j = 0; j < fquery->num_group_tuples; j++) {
      struct group** group_tuple = fquery->group_tuples[j];
      for (int i = 0; i < fquery->num_branches; i++) {
        struct group* group = group_tuple[i];
        flow_print_record(trace_data, group->group_aggr_record);
      }
      printf("\n");
    }    
#endif

  }    

  /* -----------------------------------------------------------------------*/      
  
  
  

/* free branch_info */
  
  
  exit(EXIT_SUCCESS);
}
