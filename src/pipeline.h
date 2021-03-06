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

#ifndef f_engine_pipeline_h
#define f_engine_pipeline_h

#include "base.h"

/* -----------------------------------------------------------------------*/
/*                              branch                                    */
/* -----------------------------------------------------------------------*/

struct group {
  uint32_t                                    num_members;
  char**                                      members;
  struct aggr_result*                         aggr_result;
};

struct aggr_result {
  struct aggr_record*                         aggr_record;
  struct aggr**                               aggrset;
};

struct aggr_record {
  char*                                       aggr_record;
  uint64_t                                    start_ts_msec;
  uint64_t                                    end_ts_msec;
};

struct filter_clause {
  size_t                                      num_terms;
  struct filter_term**                        termset;
};
struct filter_term {
  size_t                                      field_offset;
  uint64_t                                    value;
  uint64_t                                    delta;
  struct filter_op*                           op;
  bool (*func)(
               const char* const              record,
               size_t                         field_offset,
               uint64_t                       value,
               uint64_t                       delta
               );
  struct aggr* (*aggr_function)(
                                 char **records,
                                 char *group_aggregation,
                                 size_t num_records,
                                 size_t field_offset,
                                 bool if_aggr_common
                               );
};

struct grouper_clause {
  size_t                                      num_terms;
  struct grouper_term**                       termset;
  struct grouper_type**                       gtypeset;
};
struct grouper_term {
  size_t                                      field_offset1;
  size_t                                      field_offset2;
  uint64_t                                    delta;
  struct grouper_op*                          op;
  bool (*func)(
               const struct group* const      group,
               size_t                         field_offset1,
               const char* const              record2,
               size_t                         field_offset2,
               uint64_t                       delta
               );
  struct aggr* (*aggr_function)(
                                char **records,
                                char *group_aggregation,
                                size_t num_records,
                                size_t field_offset,
                                bool if_aggr_common
                               );
};
struct aggr_term {
  size_t                                      field_offset;
  struct aggr_op*                             op;
  struct aggr* (*func)(
                       char** const           group_records,
                       char*                  group_aggregation,
                       size_t                 num_records,
                       size_t                 field_offset,
                       bool                   if_aggr_common
                       );
};

struct groupfilter_clause {
  size_t                                      num_terms;
  struct groupfilter_term**                       termset;
};
struct groupfilter_term {
  size_t                                      field;
  uint64_t                                    value;
  uint64_t                                    delta;
  struct groupfilter_op*                      op;
  bool (*func)(
               const struct group* const      group,
               size_t                         field,
               uint64_t                       value,
               uint64_t                       delta
               );
};

struct branch {

  /* -----------------------------------------------------------------------*/
  /*                              inputs                                    */
  /* -----------------------------------------------------------------------*/
  int                                         branch_id;
  struct io_handler_s*                        io;
  struct io_reader_s*                         read_ctxt;
  struct io_writer_s*                         write_ctxt;
  struct io_data_s*                           data;

  size_t                                      num_filter_clauses;
  size_t                                      num_grouper_clauses;
  size_t                                      num_aggr_clause_terms;
  size_t                                      num_groupfilter_clauses;

  struct filter_clause**                      filter_clauseset;
  struct grouper_clause**                     grouper_clauseset;
  struct aggr_term**                          aggr_clause_termset;
  struct groupfilter_clause**                 groupfilter_clauseset;
  /* -----------------------------------------------------------------------*/



  /* -----------------------------------------------------------------------*/
  /*                               output                                   */
  /* -----------------------------------------------------------------------*/

  struct filter_result*                       filter_result;
  struct grouper_result*                      grouper_result;
  struct groupfilter_result*                  gfilter_result;

  /* -----------------------------------------------------------------------*/
};

struct filter_result {
  uint32_t                                    num_filtered_records;
  char**                                      filtered_recordset;
};
struct grouper_result {
  char**                                      sorted_recordset;
  uint32_t                                    num_groups;
  struct group**                              groupset;
};
struct groupfilter_result {
  uint32_t                                    num_filtered_groups;
  struct group**                              filtered_groupset;
};
struct aggr {
  size_t                                      num_values;
  uint64_t*                                   values;
};

/* -----------------------------------------------------------------------*/


struct flowquery {
  size_t                                      num_branches;
  size_t                                      num_merger_clauses;

  struct branch**                             branchset;
  struct merger_clause**                      merger_clauseset;
  struct merger_result*                       merger_result;
  struct ungrouper_result*                    ungrouper_result;
};

struct merger_clause {
  size_t                                     num_terms;
  struct merger_term**                       termset;
};
/*
 * merger term
 * a single term will always compare two branches
 * A.dstip = B.dstip = C.dstip should be broken down to
 * A.dstip = B.dstip
 * B.dstip = C.dstip
 */
struct merger_term {
  struct branch*                              branch1;
  size_t                                      field1;
  struct branch*                              branch2;
  size_t                                      field2;
  struct merger_op*                           op;
  uint64_t                                    delta;
  bool (*func)(
               const struct group* const      group1,
               size_t                         field1,
               const struct group* const      group2,
               size_t                         field2,
               uint64_t                       delta
               );
};

struct merger_result {
  uint32_t                                    num_group_tuples;
  size_t                                      total_num_group_tuples;
  struct group***                             group_tuples;
};
struct ungrouper_result {
  size_t                                      num_streams;
  struct stream**                             streamset;
};
struct stream {
  struct io_writer_s*                         writer_ctxt;
  uint32_t                                    num_records;
  char**                                      recordset;
};

struct filter_op {
  uint64_t                                    op;
  uint64_t                                    field_type;
};
struct grouper_op {
  uint64_t                                    op;
  uint64_t                                    field1_type;
  uint64_t                                    field2_type;
  uint64_t                                    optype;
};
struct aggr_op {
  uint64_t                                    op;
  uint64_t                                    field_type;
};
struct groupfilter_op {
  uint64_t                                    op;
  uint64_t                                    field_type;
};
struct merger_op {
  uint64_t                                    op;
  uint64_t                                    field1_type;
  uint64_t                                    field2_type;
  uint64_t                                    optype;
};

/*
 * struct grouper_rule.op is a uint16_t with the options ORed together so that
 * one can have a switch over them.
 *
 * the comparison constants come first as they are not of the size of a power
 * of two and hence, would create wholes in the range of possible values for
 * grouper_rule.op which in turn would make it harder for the compiler to
 * optimize the switch. It is generally better if the possible values for the
 * switch statement are continuous because then, a branchtable can be built.
 *
 * +---+---+---+---+---+---+---+---+---+
 * |    comp   | delta | size1 | size2 |
 * +---+---+---+---+---+---+---+---+---+
 *
 * comp:  eq/ne/gt/lt/le/ge comparison operation
 * delta:     relative/absolute/no delta
 * size1: 8/16/32/64 bits for first argument
 * size2: 8/16/32/64 bits for second argument
 */
enum {

  /* size2 */
  RULE_S2_8 = 0,
  RULE_S2_16 = 1,
  RULE_S2_32 = 2,
  RULE_S2_64 = 3,

  /* size1 */
  RULE_S1_8 = 0,
  RULE_S1_16 = 4,
  RULE_S1_32 = 8,
  RULE_S1_64 = 12,

  /* D */
  RULE_ABS = 0,
  RULE_REL = 16,
  RULE_NO = 32,

  /* comp */
  RULE_EQ = 0,
  RULE_NE = 64,
  RULE_GT = 128,
  RULE_LT = 192,
  RULE_LE = 256,
  RULE_GE = 320,

  /* aggregation options */
  RULE_STATIC = 512,
  RULE_COUNT = 1024,
  RULE_UNION = 2048,
  RULE_MIN = 4096,
  RULE_MAX = 8192,
  RULE_MEDIAN = 16384,
  RULE_MEAN = 32768,
  RULE_STDDEV = 65536,
  RULE_XOR = 131072,
  RULE_SUM = 262144,
  RULE_PROD = 524288,
  RULE_AND = 1048576,
  RULE_OR = 2097152,

  /* merger allen operations */
  RULE_IN = 4194304,
  RULE_ALLEN_BF = 8388608,
  RULE_ALLEN_AF = 16777216,
  RULE_ALLEN_M = 33554432,
  RULE_ALLEN_MI = 67108864,
  RULE_ALLEN_O = 134217728,
  RULE_ALLEN_OI = 268435456,
  RULE_ALLEN_S = 536870912,
  RULE_ALLEN_SI = 1073741824,
  RULE_ALLEN_D = 2147483648,
  RULE_ALLEN_DI = 4294967296,
  RULE_ALLEN_F = 8589934592,
  RULE_ALLEN_FI = 17179869184,
  RULE_ALLEN_EQ = 34359738368
};

#endif
