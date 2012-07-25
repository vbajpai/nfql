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

#ifndef f_engine_f_h
#define f_engine_f_h

#include <pthread.h>
#include <getopt.h>
#include <json/json.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "branch.h"
#include "merger.h"
#include "ungrouper.h"

#include "auto-assign.h"
#include "errorhandlers.h"
#include "ftreader.h"
#include "echo.h"

struct parameters {
  char*                           query_filename;
  char*                           trace_filename;
};
struct parameters_data {
  char*                           query_mmap;
  struct stat*                    query_mmap_stat;
  struct ft_data*                 trace;
  int                             trace_fsock;
};

struct json {
  size_t                          num_branches;
  size_t                          num_merger_clauses;

  struct json_branch**            branchset;
  struct json_merger_clause**     merger_clauseset;
};
struct json_branch {
  size_t                          num_filter_clauses;
  size_t                          num_grouper_clauses;
  size_t                          num_aggr_clause_terms;
  size_t                          num_groupfilter_clauses;

  struct json_filter_clause**     filter_clauseset;
  struct json_grouper_clause**    grouper_clauseset;
  struct json_aggr_term**         aggr_clause_termset;
  struct
  json_groupfilter_clause**       groupfilter_clauseset;
};

struct json_filter_clause {
  size_t                          num_terms;
  struct json_filter_term**       termset;
};
struct json_filter_term {
  char*                           op;
  uint64_t                        delta;
  struct json_filter_term_offset* off;
};
struct json_filter_term_offset {
  char*                           name;
  uint64_t                        value;
  char*                           datatype;
};

struct json_grouper_clause {
  size_t                          num_terms;
  struct json_grouper_term**      termset;
};
struct json_grouper_term {
  uint64_t                        delta;
  struct
  json_grouper_term_offset*       off;
  struct
  json_grouper_term_op*           op;
};
struct json_grouper_term_offset {
  char*                           f1_name;
  char*                           f2_name;

  char*                           f1_datatype;
  char*                           f2_datatype;
};
struct json_grouper_term_op {
  char*                           name;
  char*                           type;

};

struct json_aggr_term {
  char*                           op;
  struct json_aggr_term_offset*   off;
};
struct json_aggr_term_offset{
  char*                           name;
  char*                           datatype;
};

struct json_groupfilter_clause {
  size_t                          num_terms;
  struct json_groupfilter_term**  termset;
};
struct json_groupfilter_term {
  char*                           op;
  uint64_t                        delta;
  struct
  json_groupfilter_term_offset*   off;
};
struct json_groupfilter_term_offset{
  char*                           name;
  uint64_t                        value;
  char*                           datatype;
};

struct json_merger_clause {
  size_t                          num_terms;
  struct json_merger_term**       termset;
};
struct json_merger_term {

  uint64_t                        b1_id;
  uint64_t                        b2_id;
  uint64_t                        delta;

  struct
  json_grouper_term_offset*       off;
  struct
  json_grouper_term_op*           op;
};
struct json_merger_term_offset {
  char*                           f1_name;
  char*                           f2_name;
  char*                           f1_datatype;
  char*                           f2_datatype;
};
struct json_merger_term_op {
  char*                           name;
  char*                           type;
};

struct parameters*
parse_cmdline_args(int argc, char** const argv);

struct parameters_data*
open_trace_read_query(const struct parameters* const param);

struct json*
parse_json_query(const char* const query_mmap);

struct flowquery*
prepare_flowquery(struct ft_data* const trace,
                  const struct json* const json_query);

struct ft_data*
read_trace(const struct parameters_data* const param_data,
           struct flowquery* fquery);

pthread_t*
run_branch_async(const struct flowquery* const fquery);

#endif
