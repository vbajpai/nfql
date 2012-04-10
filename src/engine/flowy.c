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

struct parameters*
parse_cmdline_args(int argc, char**argv) {

  int                                 opt;
  char*                               shortopts = "v:d";
  static struct option                longopts[] = {
    { "debug",      no_argument,        NULL,           'd' },
    { "verbose",    required_argument,  NULL,           'v' },    
    {  NULL,        0,                  NULL,            0  }
  };
  enum verbosity_levels               verbose_level;
  
  /* free'd after calling read_param_data(...) */
  struct parameters* param = calloc(1, sizeof(struct parameters));
  if (param == NULL)
    errExit("calloc");

  while ((opt = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
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
    param->trace_filename = argv[optind];
    param->query_filename = argv[optind+1];
  }  
  return param;
}

struct parameters_data*
read_param_data(struct parameters* param) {
  
  int                                 fsock;
  
  /* param_data->query_mmap is free'd after calling parse_json_query(...)
   * param_data->query_mmap_stat is free'd after freeing param_data->query_mmap
   * param_data->trace is free'd in 2 stages:
   *    non-filtered records are free'd just after returning from the branch
   *    filtered records are free'd before exiting from main(...)
   * param_data is free'd before exiting from main(...)
   */ 
  struct parameters_data* param_data = calloc(1, 
                                              sizeof(struct parameters_data));
  if (param_data == NULL)
    errExit("calloc");
  
  fsock = open(param->trace_filename, O_RDONLY);
  if (fsock == -1)
    errExit("open"); 
  param_data->trace = ft_open(fsock);
  if (close(fsock) == -1)
    errExit("close");

  /* param_data->query_mmap_stat is free'd after freeing 
   * param_data->query_mmap  
   */
  param_data->query_mmap_stat = calloc(1, sizeof(struct stat));
  if (param_data->query_mmap_stat == NULL)
    errExit("calloc");
  fsock = open(param->query_filename, O_RDONLY);
  if (fsock == -1)
    errExit("open");
  if (fstat(fsock, param_data->query_mmap_stat) == -1)
    errExit("fstat");  

  /* param_data->query_mmap is free'd after calling parse_json_query(...) */
  param_data->query_mmap = mmap(NULL, 
                                param_data->query_mmap_stat->st_size, 
                                PROT_READ, 
                                MAP_PRIVATE, 
                                fsock, 0); 
  if (param_data->query_mmap == MAP_FAILED)
    errExit("mmap");
  if (close(fsock) == -1)
    errExit("close"); 
  
  return param_data;
}

struct json*
parse_json_query(char* query_mmap) {
  
  /* free'd after returning from prepare_flowquery(...) */
  struct json* json = calloc(1, sizeof(struct json));
  if (json == NULL)
    errExit("calloc");
  json->num_frules = 1;
  
  /* free'd after returning from prepare_flowquery(...) */
  json->fruleset = calloc(json->num_frules, 
                          sizeof(struct json_filter_rule*));
  if (json->fruleset == NULL)
    errExit("calloc");

  for (int i = 0; i < json->num_frules; i++) {
    /* free'd after returning from prepare_flowquery(...) */
    json->fruleset[i] = calloc(1, sizeof(struct json_filter_rule));
    if (json->fruleset[i] == NULL)
      errExit("calloc");
    
    /* free'd after returning from prepare_flowquery(...) */
    json->fruleset[i]->off = 
    calloc(1, sizeof(struct json_filter_rule_offset));
    if (json->fruleset[i]->off == NULL)
      errExit("calloc");
    
    /* free'd before returning from this function */
    struct json_object* query = json_tokener_parse(query_mmap);
    struct json_object* delta = json_object_object_get(query, "delta");
    struct json_object* op = json_object_object_get(query, "op");
    struct json_object* foffset = json_object_object_get(query, "offset");    
    struct json_object* fo_name = json_object_object_get(foffset, "name");
    struct json_object* fo_val = json_object_object_get(foffset, "value");
    struct json_object* fo_type = json_object_object_get(foffset, "datatype");
    
    json->fruleset[i]->delta = json_object_get_int(delta);  
    json->fruleset[i]->op = json_object_get_string(op);
    json->fruleset[i]->off->name = json_object_get_string(fo_name);  
    json->fruleset[i]->off->value = json_object_get_int(fo_val);
    json->fruleset[i]->off->datatype = json_object_get_string(fo_type);
    
    /* free the json objects */
    json_object_put(fo_name); fo_name = NULL;
    json_object_put(fo_val); fo_val = NULL;
    json_object_put(fo_type); fo_type = NULL;    
    json_object_put(foffset); foffset = NULL; 
    json_object_put(delta); delta = NULL;
    json_object_put(op); op = NULL;        
    json_object_put(query); query = NULL;    
  } 
  
  return json;
}

struct flowquery*
prepare_flowquery(struct ft_data* trace,
                  struct json* json_query) {
  
  
  /* -----------------------------------------------------------------------*/  
  /*                          allocating fquery                             */
  /* -----------------------------------------------------------------------*/
  
  /* TODO: when free'd? */
  struct flowquery* fquery = (struct flowquery *)
                              calloc(1, sizeof(struct flowquery));
  if (fquery == NULL) 
    errExit("calloc");
  
  /* TODO: hardcoded */
  fquery->num_branches = NUM_BRANCHES;
  
  /* TODO: when free'd? */
  fquery->branchset =  (struct branch_info *)
                       calloc(fquery->num_branches, 
                       sizeof(struct branch_info));
  if (fquery->branchset == NULL)
    errExit("calloc");
  
  /* ----------------------------------------------------------------------- */
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                       assigning branch rules                           */
  /* -----------------------------------------------------------------------*/  

  for (int i = 0; i < fquery->num_branches; i++) {
    
    struct branch_info* branch = &fquery->branchset[i];
    
    /* TODO: hardcoded */
    switch (i) {
      case 0:
        branch->num_filter_rules = NUM_FILTER_RULES_BRANCH1;
        branch->num_grouper_rules = NUM_GROUPER_RULES_BRANCH1;         
        branch->num_aggr_rules = NUM_GROUPER_AGGREGATION_RULES_BRANCH1;
        branch->num_gfilter_rules = NUM_GROUP_FILTER_RULES_BRANCH1;
        break;
      case 1:  
        branch->num_filter_rules = NUM_FILTER_RULES_BRANCH2;
        branch->num_grouper_rules = NUM_GROUPER_RULES_BRANCH2;
        branch->num_aggr_rules = NUM_GROUPER_AGGREGATION_RULES_BRANCH2;
        branch->num_gfilter_rules = NUM_GROUP_FILTER_RULES_BRANCH2;
        break;
    }
    
    branch->branch_id = i;
    branch->data = trace;
    
    /* filter rules are used in grouper aggregations */    
    /* therefore, free'd after returning from grouper(...) in branch.c */    
    struct filter_rule** fruleset = (struct filter_rule**) 
                                     calloc(branch->num_filter_rules, 
                                            sizeof(struct filter_rule*));
    if (fruleset == NULL)
      errExit("calloc");
    for (int j = 0; j < branch->num_filter_rules; j++) {

      /* free'd after returning from grouper(...) in branch.c */
      struct filter_rule* frule = calloc(1, sizeof(struct filter_rule));
      if (frule == NULL)
        errExit("calloc");      
      
      /* TODO: hardcoded */
      switch (i) {
        case 0:
          frule->field_offset      =         trace->offsets.dstport;
          break;
        case 1:          
          frule->field_offset      =         trace->offsets.srcport;
          break;
      }
      frule->value                 =         json_query->fruleset[0]->off->value;
      frule->delta                 =         json_query->fruleset[0]->delta;
      frule->op                    =         RULE_EQ | RULE_S1_16;
      frule->func                  =         NULL;      
      
      fruleset[j] = frule; frule = NULL;
    }
    branch->filter_ruleset = fruleset; fruleset = NULL;
    
    /* free'd after returning from grouper(...) in branch.c */
    struct grouper_rule** gruleset = (struct grouper_rule**)
                                      calloc(branch->num_grouper_rules, 
                                             sizeof(struct grouper_rule*));
    if (gruleset == NULL)
      errExit("calloc");
    for (int j = 0; j < branch->num_grouper_rules; j++) {
      
      /* free'd after returning from grouper(...) in branch.c */
      struct grouper_rule* grule = calloc(1, sizeof(struct grouper_rule));
      if (grule == NULL)
        errExit("calloc");
      
      /* TODO: hardcoded */
      switch (j) {
        case 0:
          grule->field_offset1     =        trace->offsets.srcaddr;      
          grule->field_offset2     =        trace->offsets.srcaddr;          
          break;
        case 1:
          grule->field_offset1     =        trace->offsets.dstaddr;      
          grule->field_offset2     =        trace->offsets.dstaddr;         
          break;
      }      
      grule->delta                 =         0;
      grule->op                    =         RULE_EQ | RULE_S1_32 | 
                                             RULE_S2_32 | RULE_NO;
      grule->func                  =         NULL;
      
      gruleset[j] = grule; grule = NULL;
    }
    branch->grouper_ruleset = gruleset; gruleset = NULL;
    
    /* free'd after returning from grouper(...) */
    struct aggr_rule** aruleset = (struct aggr_rule**)
                                   calloc(branch->num_aggr_rules, 
                                          sizeof(struct aggr_rule*));
    if (aruleset == NULL)
      errExit("calloc");
    for (int j = 0; j < branch->num_aggr_rules; j++) {
      
      /* free'd after returning from grouper(...) */
      struct aggr_rule* arule = calloc(1, sizeof(struct aggr_rule));
      if (arule == NULL)
        errExit("calloc");
      
      /* TODO: hardcoded */      
      switch (j) {
        case 0:
          arule->field_offset    =         trace->offsets.srcaddr;
          break;
        case 1:
          arule->field_offset    =         trace->offsets.dstaddr;
          break;
        case 2:
          arule->field_offset    =         trace->offsets.dPkts;
          break;
        case 3:
          arule->field_offset    =         trace->offsets.dOctets;
          break;
      }
      switch (j) {
        case 0:
        case 1:
          arule->op              =         RULE_STATIC | RULE_S1_32;
          break;
        case 2:
        case 3:
          arule->op              =         RULE_SUM | RULE_S1_32;
          break;
      }      
      arule->func                =         NULL;
      
      aruleset[j] = arule; arule = NULL;
    }
    branch->aggr_ruleset = aruleset; aruleset = NULL;
   
    /* free'd after returning from groupfilter(...) */
    struct gfilter_rule** gfruleset = (struct gfilter_rule**)
                                      calloc(branch->num_gfilter_rules, 
                                            sizeof(struct gfilter_rule*));
    if (gfruleset == NULL)
      errExit("calloc");    
    for (int j = 0; j < branch->num_gfilter_rules; j++) {
      
      /* free'd after returning from groupfilter(...) */
      struct gfilter_rule* gfrule = calloc(1, sizeof(struct gfilter_rule));
      if (gfrule == NULL)
        errExit("calloc");

      gfrule->field                =         trace->offsets.dPkts;
      gfrule->value                =         200;
      gfrule->delta                =         0;
      gfrule->op                   =         RULE_GT | RULE_S1_32;
      gfrule->func                 =         NULL;
      
      gfruleset[j] = gfrule; gfrule = NULL;
    }
    branch->gfilter_ruleset = gfruleset; gfruleset = NULL;    
  }

  /* ----------------------------------------------------------------------- */
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                       assigning merger rules                           */
  /* -----------------------------------------------------------------------*/  

  /* TODO: hardcoded */
  fquery->num_merger_rules = NUM_MERGER_RULES;

  /* free'd after returning from merger(...) */
  struct merger_rule** mruleset = (struct merger_rule**)
                                   calloc(fquery->num_merger_rules, 
                                          sizeof(struct merger_rule*));
  if (mruleset == NULL)
    errExit("calloc");
  
  for (int j = 0; j < fquery->num_merger_rules; j++) {

    /* free'd after returning from merger(...) */
    struct merger_rule* mrule = calloc(1, sizeof(struct merger_rule));
    if (mrule == NULL)
      errExit("calloc");
    
    mrule->branch1               =         &fquery->branchset[0];
    mrule->branch2               =         &fquery->branchset[1];    

    /* TODO: hardcoded */
    switch (j) {
      case 0:
        mrule->field1            =         trace->offsets.srcaddr;
        mrule->field2            =         trace->offsets.dstaddr;        
        break;
      case 1:
        mrule->field1            =         trace->offsets.dstaddr;
        mrule->field2            =         trace->offsets.srcaddr;        
        break;
    }
    mrule->delta                 =         0;
    mrule->op                    =         RULE_EQ | RULE_S1_32 | RULE_S2_32;
    mrule->func                  =         NULL;
    
    mruleset[j] = mrule; mrule = NULL;
  }
  fquery->mruleset = mruleset; mruleset = NULL;
  
  /* ----------------------------------------------------------------------- */
  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*             fills filter_rule.func, grouper_rule.func                  */
  /*               gfilter_rule.func, merger_rule.func by                   */
  /*              falling through a huge switch statement                   */
  /* -----------------------------------------------------------------------*/    
  
  assign_fptr(fquery);
  
  /* -----------------------------------------------------------------------*/
  
  
  
  
  
  
  
  
  
  return fquery;
}

pthread_t*
run_branch_async(struct flowquery* fquery){
  
  /* allocate space for a dedicated thread for each branch */
  /* free'd after returning from this function */
  pthread_t* threadset = (pthread_t *)calloc(fquery->num_branches, 
                                             sizeof(pthread_t));
  if (threadset == NULL)
    errExit("calloc");
  
  /* free'd before returning from this function */
  pthread_attr_t* thread_attrset = (pthread_attr_t *)
                                    calloc(fquery->num_branches, 
                                           sizeof(pthread_attr_t));
  if (thread_attrset == NULL)
    errExit("calloc");
  
  for (int i = 0, ret = 0; i < fquery->num_branches; i++) {
    
    pthread_t* thread = &threadset[i];
    pthread_attr_t* thread_attr = &thread_attrset[i];
    struct branch_info* branch = &fquery->branchset[i];
    
    /* initialize each thread attributes */
    ret = pthread_attr_init(thread_attr);
    if (ret != 0)
      errExit("pthread_attr_init");
    
    /* start each thread for a dedicated branch */      
    ret = pthread_create(thread, 
                         thread_attr, 
                         &branch_start, 
                         (void *)(branch));    
    if (ret != 0)
      errExit("pthread_create");
    
    
    /* destroy the thread attributes once the thread is created */  
    ret = pthread_attr_destroy(thread_attr);
    if (ret != 0) 
      errExit("pthread_attr_destroy");
  }
  
  free(thread_attrset);
  return threadset;
}

int 
main(int argc, char **argv) {
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                  parsing the command line arguments                    */
  /* -----------------------------------------------------------------------*/
  
  struct parameters* param = parse_cmdline_args(argc, argv);
  if (param == NULL)
    errExit("parse_cmdline_args(...) returned NULL");

  /* ----------------------------------------------------------------------- */  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                  reading the input trace and query                     */
  /* -----------------------------------------------------------------------*/
  
  struct parameters_data* param_data = read_param_data(param);
  if (param_data == NULL)
    errExit("read_param_data(...) returned NULL");
  else{
    free(param); param = NULL;
  }
    
  
  /* ----------------------------------------------------------------------- */  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                  parse the query json into struct                      */
  /* -----------------------------------------------------------------------*/
  
  struct json* json_query = parse_json_query(param_data->query_mmap);
  if (json_query == NULL)
    errExit("parse_json_query(...) returned NULL");
  else {
    /* free param_data->query_mmap */
    if (munmap(param_data->query_mmap,
               param_data->query_mmap_stat->st_size) == -1)
      errExit("munmap");
    param_data->query_mmap = NULL;
    free(param_data->query_mmap_stat); param_data->query_mmap_stat = NULL;
  }
  
 /* ----------------------------------------------------------------------- */  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                           prepare flowquery                            */
  /* -----------------------------------------------------------------------*/
  
  struct flowquery* fquery = prepare_flowquery(param_data->trace, 
                                               json_query); 
  if (fquery == NULL)
    errExit("prepare_flowquery(...) returned NULL");
  else {
    
    /* deallocate the json query buffers */
    for (int i = 0; i < json_query->num_frules; i++) {
      struct json_filter_rule* frule = json_query->fruleset[i];
      free(frule->off); frule->off = NULL;
      free(frule); frule = NULL;
    }
    free(json_query->fruleset); json_query->fruleset = NULL;
    free(json_query); json_query = NULL;
  }  
  
 /* -----------------------------------------------------------------------*/


  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                                branch                                  */  
  /*               splitter → filter → grouper → group-filter               */
  /* -----------------------------------------------------------------------*/   
  
  pthread_t* threadset = run_branch_async(fquery);
  if (threadset == NULL)
    errExit("run_branch_async(...) returned NULL");
  else {
    
    /* - wait for each thread to complete its branch
     * - save and print the number of filtered groups on completion
     * - save the filtered groups on completion
     */
    for (int i = 0, ret = 0; i < fquery->num_branches; i++) {
      pthread_t* thread = &threadset[i];
      ret = pthread_join(*thread, NULL);
      if (ret != 0)
        errExit("pthread_join");
    }    
    free(threadset);
    
    /* free all the records that were not filtered from the original trace */
    for (int i = 0; i < param_data->trace->num_records; i++) {
      
      bool if_filtered_record = false;
      char* record = param_data->trace->records[i];
      for (int j = 0; j < fquery->num_branches; j++) {

        struct branch_info* branch = &fquery->branchset[j];        
        for (int k = 0; k < branch->filter_result->num_filtered_records; k++) {
          
          char* filtered_record = branch->filter_result->filtered_recordset[k];
          if (record == filtered_record){
            if_filtered_record = true;
            break;
          }            
        }
        if (if_filtered_record)
          break;
      }
      if (!if_filtered_record){
        free(record); record = NULL;
        param_data->trace->records[i] = NULL;      
      }
      
    }
    
    /* print the filtered records if verbose mode is set */
    if (verbose_v) {
      echo_branch(fquery->num_branches,
                  fquery->branchset,
                  param_data->trace);
    }    
  }    
  
  /* -----------------------------------------------------------------------*/    
  
  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                              free memory                               */
  /* -----------------------------------------------------------------------*/  
  
  if (verbose_v) {    
    for (int i = 0; i < fquery->num_branches; i++) {
      struct branch_info* branch = &fquery->branchset[i];
      
      /* free grouper_result */
      if (verbose_vv) {
        
        /* free sorted records */
        for (int j = 0; j < branch->filter_result->num_filtered_records; j++)
          branch->grouper_result->sorted_recordset[j] = NULL;
        free(branch->grouper_result->sorted_recordset);
        branch->grouper_result->sorted_recordset = NULL;
              
        /* free unique records */
        for (int j = 0; j < branch->grouper_result->num_unique_records; j++)
          branch->grouper_result->unique_recordset[j] = NULL;
        free(branch->grouper_result->unique_recordset);
        branch->grouper_result->unique_recordset = NULL;        
      }
    }
  }
  
  /* -----------------------------------------------------------------------*/
  
  
  
  
  
  
  
  
  
  
#ifdef MERGER
  
  /* -----------------------------------------------------------------------*/  
  /*                                 merger                                 */
  /* -----------------------------------------------------------------------*/
  
  fquery->merger_result = merger(fquery->branchset,
                                 fquery->num_branches,
                                 
                                 fquery->mruleset, 
                                 fquery->num_merger_rules);
  
  if (fquery->merger_result == NULL)
    errExit("merger(...) returned NULL");
  else {
    
    /* free merger rules */
    for (int i = 0; i < fquery->num_merger_rules; i++) {
      struct merger_rule* mrule = fquery->mruleset[i];
      free(mrule); mrule = NULL; fquery->mruleset[i] = NULL;      
    }
    free(fquery->mruleset); fquery->mruleset = NULL;
    
    /* echo merger results, if verbose mode is SET */
    if (verbose_v)
      echo_merger(fquery, param_data->trace);
    
    /* free grouper and groupfilter parts */
    for (int i = 0; i < fquery->num_branches; i++) {
      struct branch_info* branch = &fquery->branchset[i];    
      
      /* free grouper aggregations */      
      for (int j = 0; j < branch->grouper_result->num_groups; j++) {        
        struct group* group = branch->grouper_result->groupset[j];        
        
#ifdef GROUPERAGGREGATIONS
        
        /* free group aggregations */        
        for (int x = 0; x < branch->num_aggr_rules; x++){
          struct aggr* aggr = group->aggrset[x];
          free(aggr->values); aggr->values = NULL;
          free(aggr); aggr = NULL; group->aggrset[x] = NULL;
        }  
        free(group->aggrset); group->aggrset = NULL;
        free(group->aggr_record); group->aggr_record = NULL;
#endif
      }
      
      /* free groupfilter result */      
      for (int j = 0; j < branch->gfilter_result->num_filtered_groups; j++) {          
        /* all the groups were already free'd by grouper just above */
        /* unlink the pointers */
        branch->gfilter_result->filtered_groupset[j] = NULL;
      }  
      free(branch->gfilter_result->filtered_groupset);
      branch->gfilter_result->filtered_groupset = NULL;          
      free(branch->gfilter_result);
      branch->gfilter_result = NULL;
    }
  }
  
  /* -----------------------------------------------------------------------*/    
  
#endif  
  
  

  
  
  
  
  
  
  
#ifdef UNGROUPER  
  
  /* -----------------------------------------------------------------------*/  
  /*                                ungrouper                               */
  /* -----------------------------------------------------------------------*/  
  
  if (fquery->merger_result->num_group_tuples != 0){
    fquery->streamset = ungrouper(fquery);
    if (fquery->streamset == NULL)
      errExit("ungrouper(...) returned NULL");
    else {
      
      /* echo ungrouper results */
      echo_results(fquery->merger_result->num_group_tuples,
                   fquery->streamset,
                   param_data->trace);      
      
      /* free grouper leftovers */
      for (int i = 0; i < fquery->num_branches; i++) {        
        struct branch_info* branch = &fquery->branchset[i];
        for (int j = 0; j < branch->grouper_result->num_groups; j++) {
          struct group* group = branch->grouper_result->groupset[j];        
          for (int k = 0; k < group->num_members; k++)
            group->members[k] = NULL;          
          free(group->members); group->members = NULL;          
          free(group); group = NULL; branch->grouper_result->groupset[j] = NULL;
        }
        free(branch->grouper_result->groupset); 
        branch->grouper_result->groupset = NULL;      
        free(branch->grouper_result); branch->grouper_result = NULL;
      }
      
      /* free ungrouper results */
      for (int j = 0; j < fquery->merger_result->num_group_tuples; j++) {
        struct stream* stream = fquery->streamset[j];
        for (int i = 0; i < stream->num_records; i++){
          char* record = stream->recordset[i];
          
          /* unlink the record, */
          /* all filtered records are free'd next at once */
          record = NULL; stream->recordset[i] = NULL;        
        }
        free(stream->recordset); stream->recordset = NULL;
        free(stream); stream = NULL;
      }
      free(fquery->streamset); fquery->streamset = NULL;
      
      /* free merger results */
      for (int i = 0; i < fquery->merger_result->num_group_tuples; i++) {
        struct group** matched_tuple = fquery->merger_result->group_tuples[i];
        for (int j = 0; j < fquery->num_branches; j++) {
          /* unlink the groups */
          matched_tuple[j] = NULL;
        }
        free(matched_tuple); 
        matched_tuple = NULL; fquery->merger_result->group_tuples[i] = NULL;      
      }
      free(fquery->merger_result->group_tuples); 
      fquery->merger_result->group_tuples = NULL;
      free(fquery->merger_result); fquery->merger_result = NULL;
      
    }    
  }
  
  /* -----------------------------------------------------------------------*/
  
#endif  
  
  
  
  
  
  
  
  
  
  
  /* -----------------------------------------------------------------------*/  
  /*                              free memory                               */
  /* -----------------------------------------------------------------------*/  
  
  /* free filter_result */
  for (int i = 0; i < fquery->num_branches; i++) {
    struct branch_info* branch = &fquery->branchset[i];
    for (int j = 0; j < branch->filter_result->num_filtered_records; j++) {
      char* record = branch->filter_result->filtered_recordset[j];
      free(record); record = NULL; 
      branch->filter_result->filtered_recordset[j] = NULL;
    }
    
    free(branch->filter_result->filtered_recordset);
    branch->filter_result->filtered_recordset = NULL;
    
    free(branch->filter_result); 
    branch->filter_result = NULL;
  }
  
  /* free flowquery */
  free(fquery->branchset); fquery->branchset = NULL;
  free(fquery); fquery = NULL;
  
  /* free param_data */
  ft_close(param_data->trace); param_data->trace = NULL;
  free(param_data); param_data = NULL;
  
  /* -----------------------------------------------------------------------*/  
  
  
  exit(EXIT_SUCCESS);
}