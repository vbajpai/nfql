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

  
  /* -----------------------------------------------------------------------*/  
  /*                            local variables                             */
  /* -----------------------------------------------------------------------*/
  
  /* command line parsing variables */
  int                                 opt;
  static struct option                longopts[] = {
    { "debug",      no_argument,        NULL,           'd' },
    { "verbose",    required_argument,  NULL,           'v' },    
    {  NULL,        0,                  NULL,            0  }
  };  
  enum verbosity_levels               verbose_level;
  char*                               trace_filename;
  char*                               query_filename;
  
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
  int                                 i, ret;
  pthread_t*                          thread_ids;
  pthread_attr_t*                     thread_attrs;
  struct branch_info*                 binfos;
  
  /* ----------------------------------------------------------------------- */  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                  parsing the command line arguments                    */
  /* -----------------------------------------------------------------------*/
  
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

  /* ----------------------------------------------------------------------- */  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                  reading the input trace into struct                   */
  /* -----------------------------------------------------------------------*/
  
  fsock = open(trace_filename, O_RDONLY);
  if (fsock == -1)
    errExit("open"); 
  trace_data = ft_open(fsock);
  if (close(fsock) == -1)
    errExit("close");
  
  /* ----------------------------------------------------------------------- */  
  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                  reading the input query into mmap                     */
  /* -----------------------------------------------------------------------*/
  
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
  
  /* ----------------------------------------------------------------------- */  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                  parse the query json into struct                      */
  /* -----------------------------------------------------------------------*/
  
  filter_rules_params = calloc(1, sizeof(filter_rules_params));
  if (filter_rules_params == NULL)
    errExit("calloc");
  filter_offset = calloc(1, sizeof(filter_offset));
  if (filter_rules_params == NULL)
    errExit("calloc");
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
  
 /* ----------------------------------------------------------------------- */  
  
  
   
  
  
 
  
  
  /* -----------------------------------------------------------------------*/  
  /*                      allocating fquery and binfos                      */
  /* -----------------------------------------------------------------------*/
  
  fquery = (struct flowquery *)malloc(sizeof(struct flowquery));
  if (fquery == NULL) 
    errExit("malloc");
  fquery->num_branches = NUM_BRANCHES;
  binfos = (struct branch_info *)
            calloc(fquery->num_branches, 
                   sizeof(struct branch_info));
  if (binfos == NULL)
    errExit("calloc");
  fquery->branches = binfos;
  
 /* ----------------------------------------------------------------------- */


  

  
  
  
    
  /* -----------------------------------------------------------------------*/  
  /*                           pipeline rules                               */
  /* -----------------------------------------------------------------------*/  
 
  fquery->branches[0].num_filter_rules = NUM_FILTER_RULES_BRANCH1;
  fquery->branches[1].num_filter_rules = NUM_FILTER_RULES_BRANCH2;
  
  fquery->branches[0].num_group_modules = NUM_GROUPER_RULES_BRANCH1;
  fquery->branches[1].num_group_modules = NUM_GROUPER_RULES_BRANCH2;  
  
  fquery->branches[0].num_aggr = NUM_GROUPER_AGGREGATION_RULES_BRANCH1;
  fquery->branches[1].num_aggr = NUM_GROUPER_AGGREGATION_RULES_BRANCH2;  
  
  fquery->branches[0].num_gfilter_rules = NUM_GROUP_FILTER_RULES_BRANCH1;
  fquery->branches[1].num_gfilter_rules = NUM_GROUP_FILTER_RULES_BRANCH2;
  
  fquery->num_merger_rules = NUM_MERGER_RULES;

  
  /* rules for each branch */
  for (int i = 0; i < fquery->num_branches; i++) {
    
    fquery->branches[i].branch_id = i;
    fquery->branches[i].data = trace_data;
    
    
    struct filter_rule* frule = calloc(fquery->branches[i].num_filter_rules, 
                                       sizeof(struct filter_rule));    
    for (int j = 0; j < fquery->branches[i].num_filter_rules; j++) {
      
      if (i == 0)
        frule[j].field_offset     =         trace_data->offsets.dstport;
      else if (i == 1)
        frule[j].field_offset     =         trace_data->offsets.srcport;
      
        frule[j].value            =         filter_rules_params->off->value;
        frule[j].delta            =         filter_rules_params->delta;
        frule[j].op               =         RULE_EQ | RULE_S1_16;
        frule[j].func             =         NULL;      
    }    
    fquery->branches[i].filter_rules = frule;
    
    
    
    struct grouper_rule* grule = calloc(fquery->branches[i].num_group_modules, 
                                        sizeof(struct grouper_rule));
    for (int j = 0; j < fquery->branches[i].num_group_modules; j++) {
      switch (j) {
        case 0:
          grule[j].field_offset1     =        trace_data->offsets.srcaddr;      
          grule[j].field_offset2     =        trace_data->offsets.srcaddr;          
          break;
        case 1:
          grule[j].field_offset1     =        trace_data->offsets.dstaddr;      
          grule[j].field_offset2     =        trace_data->offsets.dstaddr;         
          break;
      }
      
         grule[j].delta            =         0;
         grule[j].op               =         RULE_EQ | RULE_S1_32 | RULE_S2_32 | RULE_NO;
         grule[j].func             =         NULL;
    }
    fquery->branches[i].group_modules = grule;    
    


    struct grouper_aggr* aggrule = calloc(fquery->branches[i].num_aggr, 
                                          sizeof(struct grouper_aggr));
    for (int j = 0; j < fquery->branches[i].num_aggr; j++) {      
      aggrule[j].module           =         0;      
      switch (j) {
        case 0:
          aggrule[j].field_offset =         trace_data->offsets.srcaddr;
          break;
        case 1:
          aggrule[j].field_offset =         trace_data->offsets.dstaddr;
          break;
        case 2:
          aggrule[j].field_offset =         trace_data->offsets.dPkts;
          break;
        case 3:
          aggrule[j].field_offset =         trace_data->offsets.dOctets;
          break;
      }
      switch (j) {
        case 0:
        case 1:
          aggrule[j].op           =         RULE_STATIC | RULE_S1_32;
          break;
        case 2:
        case 3:
          aggrule[j].op           =         RULE_SUM | RULE_S1_32;
          break;
      }      
      aggrule[j].func             =         NULL;
    }
    fquery->branches[i].aggr = aggrule;    


    
    struct gfilter_rule* gfrule = calloc(fquery->branches[i].num_gfilter_rules, 
                                         sizeof(struct gfilter_rule));
    for (int j = 0; j < fquery->branches[i].num_gfilter_rules; j++) {
      gfrule[j].field             =         trace_data->offsets.dPkts;
      gfrule[j].value             =         200;
      gfrule[j].delta             =         0;
      gfrule[j].op                =         RULE_GT | RULE_S1_32;
      gfrule[j].func              =         NULL;
    }    
    fquery->branches[i].gfilter_rules = gfrule;
    
  }
  
  
  struct merger_rule* mrule = calloc(fquery->num_merger_rules, 
                                     sizeof(struct merger_rule));
  for (int j = 0; j < fquery->num_merger_rules; j++) {    
    mrule[j].branch1               =         &fquery->branches[0];
    mrule[j].branch2               =         &fquery->branches[1];    
    switch (j) {        
      case 0:
        mrule[j].field1            =         trace_data->offsets.srcaddr;
        mrule[j].field2            =         trace_data->offsets.dstaddr;        
        break;
      case 1:
        mrule[j].field1            =         trace_data->offsets.dstaddr;
        mrule[j].field2            =         trace_data->offsets.srcaddr;        
        break;
    }
    mrule[j].delta                 =         0;
    mrule[j].op                    =         RULE_EQ | RULE_S1_32 | RULE_S2_32;
    mrule[j].func                  =         NULL;
  }
  fquery->mrules = mrule;
  
  /* deallocate the query buffers */
  free(filter_offset);
  free(filter_rules_params);
  
 /* -----------------------------------------------------------------------*/

  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*            fills filter_rule.func and grouper_rule.func                */
  /*                        from the grouper_rule.op                        */
  /*             by falling through a huge switch statement                 */
  /* -----------------------------------------------------------------------*/    
  
  assign_fptr(fquery, binfos, fquery->num_branches);
  
  /* -----------------------------------------------------------------------*/

  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*               splitter → filter → grouper → group-filter               */
  /* -----------------------------------------------------------------------*/   
  

  /* allocate space for a dedicated thread for each branch */
  thread_ids = (pthread_t *)calloc(fquery->num_branches, 
                                   sizeof(pthread_t));
  if (thread_ids == NULL)
    errExit("calloc");
  
  thread_attrs = (pthread_attr_t *)calloc(fquery->num_branches, 
                                          sizeof(pthread_attr_t));
  if (thread_attrs == NULL)
    errExit("calloc");
  
  for (i = 0; i < fquery->num_branches; i++) {

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
  for (i = 0; i < fquery->num_branches; i++) {
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
                                fquery->num_branches, 
                                mrule, 
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
    for (int i = 0; i < fquery->num_branches; i++) {
      
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