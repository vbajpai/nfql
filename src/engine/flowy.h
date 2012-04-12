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

/*
 * this header contains the public flowy interface
 */


#ifndef flowy_engine_flowy_h
#define flowy_engine_flowy_h

#include <pthread.h>
#include <getopt.h>
#include <json/json.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "branch.h"
#include "merger.h"
#include "ungrouper.h"

#include "auto_assign.h"
#include "error_handlers.h"
#include "ftreader.h"
#include "echo.h"



/* this should go away once the rules come from the JSON */
#define NUM_BRANCHES 2
#define NUM_FILTER_RULES_BRANCH1 1
#define NUM_FILTER_RULES_BRANCH2 1
#define NUM_GROUPER_RULES_BRANCH1 2 
#define NUM_GROUPER_RULES_BRANCH2 2
#define NUM_GROUPER_AGGREGATION_RULES_BRANCH1 4
#define NUM_GROUPER_AGGREGATION_RULES_BRANCH2 4
#define NUM_GROUP_FILTER_RULES_BRANCH1 1
#define NUM_GROUP_FILTER_RULES_BRANCH2 1
#define NUM_MERGER_RULES 2

struct parameters {
  char*                           query_filename;
  char*                           trace_filename;
};

struct parameters_data {
  char*                           query_mmap;
  struct stat*                    query_mmap_stat;  
  struct ft_data*                 trace;  
};

struct json {
  struct json_filter_rule**       fruleset;
  size_t                          num_frules;
};

struct json_filter_rule {
  const char*                     op;
  uint64_t                        delta;
  struct json_filter_rule_offset* off;
};

struct json_filter_rule_offset{
  const char*                     name;
  uint64_t                        value;
  const char*                     datatype;
};


struct parameters*
parse_cmdline_args(int argc, char** const argv);

struct parameters_data*
read_param_data(const struct parameters* const param);

struct json*
parse_json_query(const char* const query_mmap);

struct flowquery*
prepare_flowquery(struct ft_data* const trace,
                  const struct json* const json_query);

pthread_t*
run_branch_async(const struct flowquery* const fquery);

#endif