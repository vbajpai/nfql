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
#define STR(x) #x
#define XSTR(x) STR(x)
#define VERSION 2.5
#include "f.h"

struct parameters*
parse_cmdline_args(int argc, char** const argv) {

  int                                 opt;
  char*                               shortopts = "z:d:v:DhV";
  static struct option                longopts[] = {
    { "zlevel",     required_argument,  NULL,           'z' },
    { "dirpath",    required_argument,  NULL,           'd' },
    { "debug",      no_argument,        NULL,           'D' },
    { "verbose",    required_argument,  NULL,           'v' },
    { "version",    no_argument,        NULL,           'V' },
    { "help",       no_argument,        NULL,           'h' },
    {  NULL,        0,                  NULL,            0  }
  };
  enum verbosity_levels               verbose_level;
  const char usage_string[] =
  "%s [OPTIONS] queryfile tracefile\t\t query the specified trace\n"
  "   or: %s [OPTIONS] queryfile -\t\t\t read the trace from stdin\n\n"
  "OPTIONS:\n"
  "-z, --zlevel\t\t change the compression level (default:5)\n"
  "-d, --dirpath\t\t save the results as flow-tools files in given dirpath\n"
  "-D, --debug\t\t enable debugging mode\n"
  "-v, --verbose\t\t increase the verbosity level\n"
  "-V, --version\t\t output version information and exit\n"
  "-h, --help\t\t display this help and exit\n";


  /* free'd after calling read_param_data(...) */
  struct parameters* param = calloc(1, sizeof(struct parameters));
  if (param == NULL)
    errExit("calloc");

  while ((opt = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
    switch (opt) {
      case 'D': debug = TRUE; verbose_level = HIGH;
      case 'v': if (optarg) verbose_level = atoi(optarg);
        switch (verbose_level) {
          case HIGH: verbose_vvv = TRUE;
          case MEDIUM: verbose_vv = TRUE;
          case LOW: verbose_v = TRUE; break;
          default: errExit("valid verbosity levels: (1-3)");
        }
        break;
      case 'z':
        errno = 0;
        zlevel = (int) strtol(optarg, (char**) NULL, 10);
        if(errno != 0) errExit("invalid zlevel");
        else if(zlevel < 0 || zlevel > 9) errExit("valid zlevels: (0-9)");
        break;
      case 'd': file = TRUE; dirpath = optarg; break;
      case 'h': usageErr(usage_string, argv[0], argv[0]);
      case 'V': fprintf(stdout, "%s version %s", argv[0], XSTR(VERSION));
                exit(EXIT_SUCCESS);
      case ':': usageError(argv[0], "Missing argument", optopt);
      default: exit(EXIT_FAILURE);
    }
  }

  if (argc != optind + 2)
    usageErr(usage_string, argv[0], argv[0]);
  else {
    param->query_filename = argv[optind];
    param->trace_filename = argv[optind+1];
  }
  return param;
}

struct parameters_data*
open_trace_read_query(const struct parameters* const param) {

  /* param_data->query_mmap is free'd after calling parse_json_query(...)
   * param_data->query_mmap_stat is free'd after freeing param_data->query_mmap
   * param_data->trace is free'd before exiting from main(...)
   * param_data is free'd before exiting from main(...)
   */
  struct parameters_data* param_data = calloc(1,
                                              sizeof(struct parameters_data));
  if (param_data == NULL)
    errExit("calloc");

  int fsock;
  if(!strcmp(param->trace_filename,"-"))
    fsock = STDIN_FILENO;
  else {
    fsock = open(param->trace_filename, O_RDONLY);
    if (fsock == -1)
      errExit("open");
  }

  param_data->trace = ft_init(fsock);
  if (param_data->trace == NULL)
    errExit("ft_init(...) returned NULL");
  else
    param_data->trace_fsock = fsock;

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
  param_data->query_mmap =
  mmap(
       NULL,                                   /* starting page address */
       param_data->query_mmap_stat->st_size,   /* bytes to be mappped */
       PROT_READ,                              /* region accessibility */
       MAP_PRIVATE,                            /* flags */
       fsock,                                  /* file object */
       0                                       /* offset */
      );
  if (param_data->query_mmap == MAP_FAILED)
    errExit("mmap");
  if (close(fsock) == -1)
    errExit("close");

  return param_data;
}

struct json*
parse_json_query(const char* const query_mmap) {

  struct json_tokener* tok = json_tokener_new();
  struct json_object* query = json_tokener_parse_ex(tok, query_mmap, -1);
  if(tok->err != json_tokener_success)
    errExit("json_tokener_parse_ex(...)");
  json_tokener_free(tok);

  /* free'd after returning from prepare_flowquery(...) */
  struct json* json = calloc(1, sizeof(struct json));
  if (json == NULL)
    errExit("calloc");

  struct json_object* branchset = json_object_object_get(query, "branchset");
  json->num_branches = (size_t)json_object_array_length(branchset);

  /* free'd after returning from prepare_flowquery(...) */
  json->branchset = calloc(json->num_branches,
                           sizeof(struct json_branch_rules*));
  if (json->branchset == NULL)
    errExit("calloc");





  /* -----------------------------------------------------------------------*/
  /*                        parse branch rules                              */
  /* -----------------------------------------------------------------------*/

  for (int i = 0; i < json->num_branches; i++) {

    struct json_object* branch_json = json_object_array_get_idx(branchset, i);

    /* free'd after returning from prepare_flowquery(...) */
    struct json_branch_rules* branch = calloc(1,
                                              sizeof(struct json_branch_rules));
    if (branch == NULL)
      errExit("calloc");
    json->branchset[i] = branch;


#ifdef FILTER

  /* -----------------------------------------------------------------------*/
  /*                         parse filter rules                             */
  /* -----------------------------------------------------------------------*/

    struct json_object* filter = json_object_object_get(branch_json, "filter");
    struct json_object* fruleset = json_object_object_get(filter, "ruleset");
    branch->num_frules = json_object_array_length(fruleset);

    /* free'd after returning from prepare_flowquery(...) */
    branch->fruleset = calloc(branch->num_frules,
                              sizeof(struct json_filter_rule*));
    if (branch->fruleset == NULL)
      errExit("calloc");


    for (int i = 0; i < branch->num_frules; i++) {

      /* free'd after returning from prepare_flowquery(...) */
      struct json_filter_rule*
      frule = calloc(1, sizeof(struct json_filter_rule));
      if (frule == NULL)
        errExit("calloc");
      branch->fruleset[i] = frule;

      /* free'd after returning from prepare_flowquery(...) */
      frule->off = calloc(1, sizeof(struct json_filter_rule_offset));
      if (frule->off == NULL)
        errExit("calloc");

      struct json_object* frule_json = json_object_array_get_idx(fruleset, i);

      /* free'd before returning from this function */
      struct json_object*
      delta = json_object_object_get(frule_json, "delta");
      struct json_object*
      op = json_object_object_get(frule_json, "op");
      struct json_object*
      foffset = json_object_object_get(frule_json, "offset");
      struct json_object*
      fo_name = json_object_object_get(foffset, "name");
      struct json_object*
      fo_val = json_object_object_get(foffset, "value");
      struct json_object*
      fo_type = json_object_object_get(foffset, "datatype");

      frule->delta = json_object_get_int(delta);
      frule->op = strdup(json_object_get_string(op));
      if (frule->op == NULL) errExit("strdup");

      frule->off->name = strdup(json_object_get_string(fo_name));
      if (frule->off->name == NULL) errExit("strdup");

      frule->off->value = json_object_get_int(fo_val);
      frule->off->datatype = strdup(json_object_get_string(fo_type));
      if (frule->off->datatype == NULL) errExit("strdup");
    }

    /* -----------------------------------------------------------------------*/

#endif





#ifdef GROUPER

    /* -----------------------------------------------------------------------*/
    /*                        parse grouper rules                             */
    /* -----------------------------------------------------------------------*/

    struct json_object* grouper =
    json_object_object_get(branch_json, "grouper");
    struct json_object* gruleset =
    json_object_object_get(grouper, "ruleset");
    branch->num_grules = json_object_array_length(gruleset);

    /* free'd after returning from prepare_flowquery(...) */
    branch->gruleset = calloc(branch->num_grules,
                            sizeof(struct json_grouper_rule*));
    if (branch->gruleset == NULL)
      errExit("calloc");

    for (int i = 0; i < branch->num_grules; i++) {

      /* free'd after returning from prepare_flowquery(...) */
      struct json_grouper_rule*
      grule = calloc(1, sizeof(struct json_grouper_rule));
      if (grule == NULL)
        errExit("calloc");
      branch->gruleset[i] = grule;

      /* free'd after returning from prepare_flowquery(...) */
      grule->off = calloc(1, sizeof(struct json_grouper_rule_offset));
      if (grule->off == NULL)
        errExit("calloc");

      /* free'd after returning from prepare_flowquery(...) */
      grule->op = calloc(1, sizeof(struct json_grouper_rule_op));
      if (grule->op == NULL)
        errExit("calloc");

      struct json_object* grule_json = json_object_array_get_idx(gruleset, i);

      /* free'd before returning from this function */
      struct json_object*
      delta = json_object_object_get(grule_json, "delta");
      struct json_object*
      op = json_object_object_get(grule_json, "op");
      struct json_object*
      offset = json_object_object_get(grule_json, "offset");

      struct json_object*
      f1_name = json_object_object_get(offset, "f1_name");
      struct json_object*
      f2_name = json_object_object_get(offset, "f2_name");
      struct json_object*
      f1_datatype = json_object_object_get(offset, "f1_datatype");
      struct json_object*
      f2_datatype = json_object_object_get(offset, "f2_datatype");

      struct json_object*
      op_type = json_object_object_get(op, "type");
      struct json_object*
      op_name = json_object_object_get(op, "name");

      grule->delta = json_object_get_int(delta);

      grule->op->name = strdup(json_object_get_string(op_name));
      if (grule->op->name == NULL) errExit("strdup");

      grule->op->type = strdup(json_object_get_string(op_type));
      if (grule->op->type == NULL) errExit("strdup");

      grule->off->f1_name = strdup(json_object_get_string(f1_name));
      if (grule->off->f1_name == NULL) errExit("strdup");

      grule->off->f2_name = strdup(json_object_get_string(f2_name));
      if (grule->off->f2_name == NULL) errExit("strdup");

      grule->off->f1_datatype = strdup(json_object_get_string(f1_datatype));
      if (grule->off->f1_datatype == NULL) errExit("strdup");

      grule->off->f2_datatype = strdup(json_object_get_string(f2_datatype));
      if (grule->off->f2_datatype == NULL) errExit("strdup");
    }

    /* -----------------------------------------------------------------------*/

#endif














#ifdef GROUPERAGGREGATIONS

    /* -----------------------------------------------------------------------*/
    /*                        parse aggr rules                                */
    /* -----------------------------------------------------------------------*/

    struct json_object* aggr =
    json_object_object_get(branch_json, "aggregation");
    struct json_object* aruleset =
    json_object_object_get(aggr, "ruleset");
    branch->num_arules = json_object_array_length(aruleset);

    /* free'd after returning from prepare_flowquery(...) */
    branch->aruleset = calloc(branch->num_arules,
                              sizeof(struct json_aggr_rule*));
    if (branch->aruleset == NULL)
      errExit("calloc");

    for (int i = 0; i < branch->num_arules; i++) {

      /* free'd after returning from prepare_flowquery(...) */
      struct json_aggr_rule*
      arule = calloc(1, sizeof(struct json_aggr_rule));
      if (arule == NULL)
        errExit("calloc");
      branch->aruleset[i] = arule;

      /* free'd after returning from prepare_flowquery(...) */
      arule->off = calloc(1, sizeof(struct json_aggr_rule_offset));
      if (arule->off == NULL)
        errExit("calloc");

      struct json_object* arule_json = json_object_array_get_idx(aruleset, i);

      /* free'd before returning from this function */
      struct json_object*
      op = json_object_object_get(arule_json, "op");
      struct json_object*
      offset = json_object_object_get(arule_json, "offset");

      struct json_object*
      name = json_object_object_get(offset, "name");
      struct json_object*
      datatype = json_object_object_get(offset, "datatype");

      arule->op = strdup(json_object_get_string(op));
      if (arule->op == NULL) errExit("strdup");

      arule->off->name = strdup(json_object_get_string(name));
      if (arule->off->name == NULL) errExit("strdup");

      arule->off->datatype = strdup(json_object_get_string(datatype));
      if (arule->off->datatype == NULL) errExit("strdup");
    }


    /* -----------------------------------------------------------------------*/

#endif















#ifdef GROUPFILTER

    /* -----------------------------------------------------------------------*/
    /*                        parse gfilter rules                             */
    /* -----------------------------------------------------------------------*/

    struct json_object*
    gfilter = json_object_object_get(branch_json, "gfilter");
    struct json_object* gfruleset =
    json_object_object_get(gfilter, "ruleset");
    branch->num_gfrules = json_object_array_length(gfruleset);

    /* free'd after returning from prepare_flowquery(...) */
    branch->gfruleset = calloc(branch->num_gfrules,
                               sizeof(struct json_gfilter_rule*));
    if (branch->gfruleset == NULL)
      errExit("calloc");


    for (int i = 0; i < branch->num_gfrules; i++) {

      /* free'd after returning from prepare_flowquery(...) */
      struct json_gfilter_rule*
      gfrule = calloc(1, sizeof(struct json_gfilter_rule));
      if (gfrule == NULL)
        errExit("calloc");
      branch->gfruleset[i] = gfrule;

      /* free'd after returning from prepare_flowquery(...) */
      gfrule->off = calloc(1, sizeof(struct json_gfilter_rule_offset));
      if (gfrule->off == NULL)
        errExit("calloc");

      struct json_object* gfrule_json = json_object_array_get_idx(gfruleset, i);

      /* free'd before returning from this function */
      struct json_object*
      delta = json_object_object_get(gfrule_json, "delta");
      struct json_object*
      op = json_object_object_get(gfrule_json, "op");
      struct json_object*
      offset = json_object_object_get(gfrule_json, "offset");
      struct json_object*
      fo_name = json_object_object_get(offset, "name");
      struct json_object*
      fo_val = json_object_object_get(offset, "value");
      struct json_object*
      fo_type = json_object_object_get(offset, "datatype");

      gfrule->delta = json_object_get_int(delta);
      gfrule->op = strdup(json_object_get_string(op));
      if (gfrule->op == NULL) errExit("strdup");
      gfrule->off->name = strdup(json_object_get_string(fo_name));
      if (gfrule->off->name == NULL) errExit("strdup");
      gfrule->off->value = json_object_get_int(fo_val);
      gfrule->off->datatype = strdup(json_object_get_string(fo_type));
      if (gfrule->off->datatype == NULL) errExit("strdup");
    }


    /* -----------------------------------------------------------------------*/

#endif

  }

  /* -----------------------------------------------------------------------*/















#ifdef MERGER

  /* -----------------------------------------------------------------------*/
  /*                        parse merger rules                              */
  /* -----------------------------------------------------------------------*/

  struct json_object* merger = json_object_object_get(query, "merger");
  struct json_object* mruleset = json_object_object_get(merger, "ruleset");
  json->num_mrules = (size_t)json_object_array_length(mruleset);

  /* free'd after returning from prepare_flowquery(...) */
  json->mruleset = calloc(json->num_mrules, sizeof(struct json_merger_rule*));
  if (json->mruleset == NULL)
    errExit("calloc");

  for (int i = 0; i < json->num_mrules; i++) {

    /* free'd after returning from prepare_flowquery(...) */
    struct json_merger_rule*
    mrule = calloc(1, sizeof(struct json_merger_rule));
    if (mrule == NULL)
      errExit("calloc");
    json->mruleset[i] = mrule;

    /* free'd after returning from prepare_flowquery(...) */
    mrule->off = calloc(1, sizeof(struct json_merger_rule_offset));
    if (mrule->off == NULL)
      errExit("calloc");

    /* free'd after returning from prepare_flowquery(...) */
    mrule->op = calloc(1, sizeof(struct json_merger_rule_op));
    if (mrule->op == NULL)
      errExit("calloc");

    struct json_object* mrule_json = json_object_array_get_idx(mruleset, i);

    /* free'd before returning from this function */
    struct json_object*
    delta = json_object_object_get(mrule_json, "delta");
    struct json_object*
    op = json_object_object_get(mrule_json, "op");
    struct json_object*
    offset = json_object_object_get(mrule_json, "offset");
    struct json_object*
    b1_id = json_object_object_get(mrule_json, "branch1_id");
    struct json_object*
    b2_id = json_object_object_get(mrule_json, "branch2_id");

    struct json_object*
    f1_name = json_object_object_get(offset, "f1_name");
    struct json_object*
    f2_name = json_object_object_get(offset, "f2_name");
    struct json_object*
    f1_datatype = json_object_object_get(offset, "f1_datatype");
    struct json_object*
    f2_datatype = json_object_object_get(offset, "f2_datatype");

    struct json_object*
    op_type = json_object_object_get(op, "type");
    struct json_object*
    op_name = json_object_object_get(op, "name");

    mrule->delta = json_object_get_int(delta);
    mrule->b1_id = json_object_get_int(b1_id);
    mrule->b2_id = json_object_get_int(b2_id);

    mrule->op->name = strdup(json_object_get_string(op_name));
    if (mrule->op->name == NULL) errExit("strdup");
    mrule->op->type = strdup(json_object_get_string(op_type));
    if (mrule->op->type == NULL) errExit("strdup");

    mrule->off->f1_name = strdup(json_object_get_string(f1_name));
    if (mrule->off->f1_name == NULL) errExit("strdup");
    mrule->off->f2_name = strdup(json_object_get_string(f2_name));
    if (mrule->off->f2_name == NULL) errExit("strdup");
    mrule->off->f1_datatype = strdup(json_object_get_string(f1_datatype));
    if (mrule->off->f1_datatype == NULL) errExit("strdup");
    mrule->off->f2_datatype = strdup(json_object_get_string(f2_datatype));
    if (mrule->off->f2_datatype == NULL) errExit("strdup");
  }

  /* -----------------------------------------------------------------------*/

#endif










  /* call put(...) only on the root to decremenet the reference count */
  json_object_put(query); query = NULL;

  return json;
}

struct flowquery*
prepare_flowquery(struct ft_data* const trace,
                  const struct json* const json_query) {


  /* -----------------------------------------------------------------------*/
  /*                          allocating fquery                             */
  /* -----------------------------------------------------------------------*/

  /* free'd just before exiting from main(...) */
  struct flowquery* fquery = (struct flowquery *)
                              calloc(1, sizeof(struct flowquery));
  if (fquery == NULL)
    errExit("calloc");

  fquery->num_branches = json_query->num_branches;

  /* free'd just before exiting from main(...) */
  fquery->branchset =  (struct branch**)
                       calloc(fquery->num_branches,
                       sizeof(struct branch*));
  if (fquery->branchset == NULL)
    errExit("calloc");

  /* ----------------------------------------------------------------------- */







  /* -----------------------------------------------------------------------*/
  /*                       assigning branch rules                           */
  /* -----------------------------------------------------------------------*/

  for (int i = 0; i < fquery->num_branches; i++) {

    /* free'd just before exiting from main(...) */
    struct branch* branch = (struct branch*)
                             calloc(1, sizeof(struct branch));
    if (branch == NULL)
      errExit("calloc");
    fquery->branchset[i] = branch;

    struct json_branch_rules* json_branch = json_query->branchset[i];

    branch->branch_id = i;
    branch->data = trace;

#ifdef FILTER

    branch->num_filter_rules = json_branch->num_frules;

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

      /* free'd after returning from grouper(...) in branch.c */
      struct filter_op* op = calloc(1, sizeof(struct filter_op));
      if (op == NULL)
        errExit("calloc");
      else
        frule->op = op; op = NULL;

      struct json_filter_rule* frule_json = json_branch->fruleset[j];

      size_t offset = get_offset(frule_json->off->name, &trace->offsets);
      if (offset == -1)
        errExit("get_offset(frule_json) returned -1");

      uint64_t op_enum = get_enum(frule_json->op);
      if (op_enum == -1)
        errExit("get_enum(frule_json->op) returned -1");

      uint64_t type_enum = get_enum(frule_json->off->datatype);
      if (type_enum == -1)
        errExit("get_enum(frule_json->off->datatype) returned -1");

      frule->field_offset          =         offset;
      frule->value                 =         frule_json->off->value;
      frule->delta                 =         frule_json->delta;
      frule->op->op                =         op_enum;
      frule->op->field_type        =         type_enum;
      frule->func                  =         NULL;

      fruleset[j] = frule; frule = NULL; frule_json = NULL;
    }
    branch->filter_ruleset = fruleset; fruleset = NULL;

#endif

#ifdef GROUPER

    branch->num_grouper_rules = json_branch->num_grules;

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

      /* free'd after returning from grouper(...) in branch.c */
      struct grouper_op* op = calloc(1, sizeof(struct grouper_op));
      if (op == NULL)
        errExit("calloc");
      else
        grule->op = op; op = NULL;

      struct json_grouper_rule* grule_json = json_branch->gruleset[j];

      size_t f1_offset = get_offset(grule_json->off->f1_name,
                                    &trace->offsets);
      if (f1_offset == -1)
        errExit("get_offset(grule_json->off->f1_name) returned -1");

      size_t f2_offset = get_offset(grule_json->off->f2_name,
                                    &trace->offsets);
      if (f2_offset == -1)
        errExit("get_offset(grule_json->off->f2_name) returned -1");

      uint64_t op_name_enum = get_enum(grule_json->op->name);
      if (op_name_enum == -1)
        errExit("get_enum(grule_json->op->name) returned -1");

      uint64_t f1_type_enum = get_enum(grule_json->off->f1_datatype);
      if (f1_type_enum == -1)
        errExit("get_enum(grule_json->off->f1_datatype) returned -1");

      uint64_t f2_type_enum = get_enum(grule_json->off->f2_datatype);
      if (f1_type_enum == -1)
        errExit("get_enum(grule_json->off->f2_datatype) returned -1");

      uint64_t op_type_enum = get_enum(grule_json->op->type);
      if (op_type_enum == -1)
        errExit("get_enum(grule_json->op->type) returned -1");

      grule->field_offset1         =        f1_offset;
      grule->field_offset2         =        f2_offset;
      grule->delta                 =        grule_json->delta;
      grule->op->op                =        op_name_enum;
      grule->op->field1_type       =        f1_type_enum;
      grule->op->field2_type       =        f2_type_enum;
      grule->op->optype            =        op_type_enum;
      grule->func                  =        NULL;

      gruleset[j] = grule; grule = NULL;
      grule_json = NULL;
    }
    branch->grouper_ruleset = gruleset; gruleset = NULL;

#endif

#ifdef GROUPERAGGREGATIONS

    branch->num_aggr_rules = json_branch->num_arules;

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

      /* free'd after returning from grouper(...) */
      struct aggr_op* op = calloc(1, sizeof(struct aggr_op));
      if (op == NULL)
        errExit("calloc");
      else
        arule->op = op; op = NULL;

      struct json_aggr_rule* arule_json = json_branch->aruleset[j];
      size_t offset = get_offset(arule_json->off->name,
                                 &trace->offsets);
      if (offset == -1)
        errExit("get_offset(arule_json->off->name) returned -1");

      uint64_t op_enum = get_enum(arule_json->op);
      if (op_enum == -1)
        errExit("get_enum(arule_json->op) returned -1");

      uint64_t type_enum = get_enum(arule_json->off->datatype);
      if (type_enum == -1)
        errExit("get_enum(arule_json->off->datatype) returned -1");

      arule->field_offset        =         offset;
      arule->op->op              =         op_enum;
      arule->op->field_type      =         type_enum;
      arule->func                =         NULL;

      aruleset[j] = arule; arule = NULL;
    }
    branch->aggr_ruleset = aruleset; aruleset = NULL;

#endif

#ifdef GROUPFILTER

    branch->num_gfilter_rules = json_branch->num_gfrules;

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

      /* free'd after returning from groupfilter(...) */
      struct gfilter_op* op = calloc(1, sizeof(struct gfilter_op));
      if (op == NULL)
        errExit("calloc");
      else
        gfrule->op = op; op = NULL;

      struct json_gfilter_rule* gfrule_json = json_branch->gfruleset[j];
      size_t offset = get_offset(gfrule_json->off->name,
                                 &trace->offsets);
      if (offset == -1)
        errExit("get_offset(gfrule_json->off->name) returned -1");

      uint64_t op_enum = get_enum(gfrule_json->op);
      if (op_enum == -1)
        errExit("get_enum(gfrule_json->op) returned -1");

      uint64_t type_enum = get_enum(gfrule_json->off->datatype);
      if (type_enum == -1)
        errExit("get_enum(gfrule_json->off->datatype) returned -1");

      gfrule->field                =         offset;
      gfrule->value                =         gfrule_json->off->value;
      gfrule->delta                =         gfrule_json->delta;
      gfrule->op->op               =         op_enum;
      gfrule->op->field_type       =         type_enum;
      gfrule->func                 =         NULL;

      gfruleset[j] = gfrule; gfrule = NULL;
    }
    branch->gfilter_ruleset = gfruleset; gfruleset = NULL;

#endif

  }


  /* ----------------------------------------------------------------------- */





#ifdef MERGER

  /* -----------------------------------------------------------------------*/
  /*                       assigning merger rules                           */
  /* -----------------------------------------------------------------------*/

  fquery->num_merger_rules = json_query->num_mrules;

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

    /* free'd after returning from merger(...) */
    struct merger_op* op = calloc(1, sizeof(struct merger_op));
    if (op == NULL)
      errExit("calloc");
    else
      mrule->op = op; op = NULL;

    struct json_merger_rule* mrule_json = json_query->mruleset[j];

    size_t f1_offset = get_offset(mrule_json->off->f1_name,
                                  &trace->offsets);
    if (f1_offset == -1)
      errExit("get_offset(grule_json->off->f1_name) returned -1");

    size_t f2_offset = get_offset(mrule_json->off->f2_name,
                                  &trace->offsets);
    if (f2_offset == -1)
      errExit("get_offset(grule_json->off->f2_name) returned -1");

    uint64_t op_name = get_enum(mrule_json->op->name);
    if (op_name == -1)
      errExit("get_enum(mrule_json->op->name) returned -1");

    uint64_t op_type = get_enum(mrule_json->op->type);
    if (op_type == -1)
      errExit("get_enum(mrule_json->op->type) returned -1");

    uint64_t f1_type = get_enum(mrule_json->off->f1_datatype);
    if (f1_type == -1)
      errExit("get_enum(mrule_json->off->f1_datatype) returned -1");

    uint64_t f2_type = get_enum(mrule_json->off->f2_datatype);
    if (f2_type == -1)
      errExit("get_enum(mrule_json->off->f2_datatype) returned -1");

    mrule->field1                =         f1_offset;
    mrule->field2                =         f2_offset;
    mrule->op->op                =         op_name;
    mrule->op->field1_type       =         f1_type;
    mrule->op->field2_type       =         f2_type;
    mrule->op->optype            =         op_type;
    mrule->branch1               =         fquery->branchset[mrule_json->b1_id];
    mrule->branch2               =         fquery->branchset[mrule_json->b2_id];
    mrule->delta                 =         0;
    mrule->func                  =         NULL;

    mruleset[j] = mrule; mrule = NULL;
  }
  fquery->mruleset = mruleset; mruleset = NULL;

  /* ----------------------------------------------------------------------- */

#endif







  return fquery;
}

struct ft_data*
read_trace(
           const struct parameters_data* const param_data,
           struct flowquery* fquery
          ) {

  struct ft_data* trace = ft_read(param_data->trace, fquery);
  if (trace == NULL)
    errExit("ft_read(...) returned NULL");

  if (close(param_data->trace_fsock) == -1)
    errExit("close");

  return trace;
}

pthread_t*
run_branch_async(const struct flowquery* const fquery){

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
    struct branch* branch = fquery->branchset[i];

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
  /*                         read the input query                           */
  /* -----------------------------------------------------------------------*/

  struct parameters_data* param_data = open_trace_read_query(param);
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

  struct flowquery* fquery = prepare_flowquery(param_data->trace, json_query);
  if (fquery == NULL)
    errExit("prepare_flowquery(...) returned NULL");
  else {

    for (int i = 0; i < json_query->num_branches; i++) {

      struct json_branch_rules* json_branch = json_query->branchset[i];

      /* deallocate the filter json query buffers */
      for (int j = 0; j < json_branch->num_frules; j++) {
        struct json_filter_rule* frule = json_branch->fruleset[j];
        free(frule->off->datatype); frule->off->datatype = NULL;
        free(frule->off->name); frule->off->name = NULL;
        free(frule->off); frule->off = NULL;
        free(frule->op); frule->op = NULL;
        free(frule); frule = NULL;
      }
      free(json_branch->fruleset); json_branch->fruleset = NULL;

      /* deallocate the grouper json query buffers */
      for (int j = 0; j < json_branch->num_grules; j++) {
        struct json_grouper_rule* grule = json_branch->gruleset[j];
        free(grule->off->f1_datatype); grule->off->f1_datatype = NULL;
        free(grule->off->f2_datatype); grule->off->f2_datatype = NULL;
        free(grule->off->f1_name); grule->off->f1_name = NULL;
        free(grule->off->f2_name); grule->off->f2_name = NULL;
        free(grule->off); grule->off = NULL;
        free(grule->op->name); grule->op->name = NULL;
        free(grule->op->type); grule->op->type = NULL;
        free(grule->op); grule->op = NULL;
        free(grule); grule = NULL;
      }
      free(json_branch->gruleset); json_branch->gruleset = NULL;

      /* deallocate the aggr json query buffers */
      for (int j = 0; j < json_branch->num_arules; j++) {
        struct json_aggr_rule* arule = json_branch->aruleset[j];
        free(arule->off->datatype); arule->off->datatype = NULL;
        free(arule->off->name); arule->off->name = NULL;
        free(arule->off); arule->off = NULL;
        free(arule->op); arule->op = NULL;
        free(arule); arule = NULL;
      }
      free(json_branch->aruleset); json_branch->aruleset = NULL;

      /* deallocate the group filter json query buffers */
      for (int j = 0; j < json_branch->num_gfrules; j++) {
        struct json_gfilter_rule* gfrule = json_branch->gfruleset[j];
        free(gfrule->off->datatype); gfrule->off->datatype = NULL;
        free(gfrule->off->name); gfrule->off->name = NULL;
        free(gfrule->off); gfrule->off = NULL;
        free(gfrule->op); gfrule->op = NULL;
        free(gfrule); gfrule = NULL;
      }
      free(json_branch->gfruleset); json_branch->gfruleset = NULL;

      free(json_branch); json_branch = NULL; json_query->branchset[i] = NULL;
    }

    /* deallocate the grouper json query buffers */
    for (int i = 0; i < json_query->num_mrules; i++) {
      struct json_merger_rule* mrule = json_query->mruleset[i];
      free(mrule->off->f1_datatype); mrule->off->f1_datatype = NULL;
      free(mrule->off->f2_datatype); mrule->off->f2_datatype = NULL;
      free(mrule->off->f1_name); mrule->off->f1_name = NULL;
      free(mrule->off->f2_name); mrule->off->f2_name = NULL;
      free(mrule->off); mrule->off = NULL;
      free(mrule->op->name); mrule->op->name = NULL;
      free(mrule->op->type); mrule->op->type = NULL;
      free(mrule->op); mrule->op = NULL;
      free(mrule); mrule = NULL;
    }
    free(json_query->mruleset); json_query->mruleset = NULL;
    free(json_query->branchset); json_query->branchset = NULL;
    free(json_query); json_query = NULL;
  }

 /* -----------------------------------------------------------------------*/








  /* -----------------------------------------------------------------------*/
  /*          read the input trace and prepare filtered recordsets          */
  /* -----------------------------------------------------------------------*/

  param_data->trace = read_trace(param_data, fquery);
  if(param_data->trace == NULL)
    errExit("read_trace(...) returned NULL");

  /* ----------------------------------------------------------------------- */










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
      void* res = NULL;
      pthread_t* thread = &threadset[i];
      ret = pthread_join(*thread, &res);
      if (ret != 0)
        errExit("pthread_join");
      else if (res != EXIT_SUCCESS)
        errExit("branch (%d) prematurely exited", i);
    }
    free(threadset);

    /* print the filtered records if verbose mode is set */
    if (verbose_v) {

      echo_branch(
                  fquery->num_branches,
                  fquery->branchset,
                  param_data->trace
                  );
    }
  }

  /* -----------------------------------------------------------------------*/








#ifdef GROUPER

  /* -----------------------------------------------------------------------*/
  /*                              free memory                               */
  /* -----------------------------------------------------------------------*/

  if (verbose_v) {
    for (int i = 0; i < fquery->num_branches; i++) {
      struct branch* branch = fquery->branchset[i];

      /* free grouper_result */
      if (verbose_vv) {

        /* free sorted records */
        if (branch->grouper_result->sorted_recordset != NULL) {
          for (int j = 0; j < branch->filter_result->num_filtered_records; j++)
            branch->grouper_result->sorted_recordset[j] = NULL;
          free(branch->grouper_result->sorted_recordset);
          branch->grouper_result->sorted_recordset = NULL;
        }

        /* free unique records */
        if (branch->grouper_result->unique_recordset != NULL) {
          for (int j = 0; j < branch->grouper_result->num_unique_records; j++)
            branch->grouper_result->unique_recordset[j] = NULL;
          free(branch->grouper_result->unique_recordset);
          branch->grouper_result->unique_recordset = NULL;
        }
      }
    }
  }

  /* -----------------------------------------------------------------------*/

#endif








#ifdef MERGER

  /* -----------------------------------------------------------------------*/
  /*                                 merger                                 */
  /* -----------------------------------------------------------------------*/

  fquery->merger_result = merger(
                                 fquery->num_merger_rules,
                                 fquery->mruleset,

                                 fquery->num_branches,
                                 fquery->branchset
                                );

  if (fquery->merger_result == NULL)
    errExit("merger(...) returned NULL");
  else {

    /* free merger rules */
    for (int i = 0; i < fquery->num_merger_rules; i++) {
      struct merger_rule* mrule = fquery->mruleset[i];
      free(mrule->op); mrule->op = NULL;
      free(mrule); mrule = NULL; fquery->mruleset[i] = NULL;
    }
    free(fquery->mruleset); fquery->mruleset = NULL;

    /* echo merger results, if verbose mode is SET */
    if (verbose_v)
      echo_merger(
                  fquery->num_branches,
                  fquery->branchset,

                  fquery->merger_result,
                  param_data->trace
                 );

    /* free grouper and groupfilter parts */
    for (int i = 0; i < fquery->num_branches; i++) {
      struct branch* branch = fquery->branchset[i];

      /* free grouper aggregations */
      for (int j = 0; j < branch->grouper_result->num_groups; j++) {
        struct group* group = branch->grouper_result->groupset[j];

#ifdef GROUPERAGGREGATIONS

        /* free group aggregations */
        for (int x = 0; x < branch->num_aggr_rules; x++){
          struct aggr* aggr = group->aggr_result->aggrset[x];
          free(aggr->values); aggr->values = NULL;
          free(aggr); aggr = NULL; group->aggr_result->aggrset[x] = NULL;
        }
        free(group->aggr_result->aggrset);
        group->aggr_result->aggrset = NULL;

        free(group->aggr_result->aggr_record);
        group->aggr_result->aggr_record = NULL;

        free(group->aggr_result);
        group->aggr_result = NULL;
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

  fquery->ungrouper_result = ungrouper(
                                       fquery->num_branches,
                                       fquery->merger_result
                                      );
  if (fquery->ungrouper_result == NULL)
    errExit("ungrouper(...) returned NULL");
  else {

    /* echo ungrouper results */
    echo_results(
                 fquery->ungrouper_result,
                 param_data->trace
                );
  }

  /* -----------------------------------------------------------------------*/

#endif










  /* -----------------------------------------------------------------------*/
  /*                              free memory                               */
  /* -----------------------------------------------------------------------*/

#ifdef GROUPER
  /* free grouper leftovers */
  for (int i = 0; i < fquery->num_branches; i++) {
    struct branch* branch = fquery->branchset[i];
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
#endif

#ifdef MERGER
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
#endif

#ifdef UNGROUPER
  /* free ungrouper results */
  for (int j = 0; j < fquery->ungrouper_result->num_streams; j++) {
    struct stream* stream = fquery->ungrouper_result->streamset[j];
    for (int i = 0; i < stream->num_records; i++){
      /* unlink the record, */
      /* all filtered records are free'd next at once */
      stream->recordset[i] = NULL;
    }
    free(stream->recordset); stream->recordset = NULL;
    free(stream); stream = NULL;
  }
  free(fquery->ungrouper_result->streamset);
  fquery->ungrouper_result->streamset = NULL;
  free(fquery->ungrouper_result); fquery->ungrouper_result = NULL;
#endif

#ifdef FILTER
  /* free filter_result */
  for (int i = 0; i < fquery->num_branches; i++) {
    struct branch* branch = fquery->branchset[i];
    for (int j = 0; j < branch->filter_result->num_filtered_records; j++) {
      free(branch->filter_result->filtered_recordset[j]);
      branch->filter_result->filtered_recordset[j] = NULL;
    }

    free(branch->filter_result->filtered_recordset);
    branch->filter_result->filtered_recordset = NULL;

    free(branch->filter_result);
    branch->filter_result = NULL;
  }
#endif

  /* free flowquery */
  for (int i = 0; i < fquery->num_branches; i++) {
    struct branch* branch = fquery->branchset[i];
    free(branch); branch = NULL; fquery->branchset[i] = NULL;
  }
  free(fquery->branchset); fquery->branchset = NULL;
  free(fquery); fquery = NULL;

  /* free param_data */
  ft_close(param_data->trace); param_data->trace = NULL;
  free(param_data); param_data = NULL;

  /* -----------------------------------------------------------------------*/


  exit(EXIT_SUCCESS);
}
