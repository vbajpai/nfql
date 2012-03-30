#!/bin/env python
#
# Copyright 2012 Vaibhav Bajpai <contact@vaibhavbajpai.com>
# Copyright 2011 Johannes 'josch' Schauer <j.schauer@email.de>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from sys import argv

operation_map = {
    'eq': '==',
    'ne': '!=',
    'lt': '<',
    'gt': '>',
    'le': '<=',
    'ge': '>=',
    'prod': '*',
    'sum': '+',
    'and': '&',
    'or': '|',
    'xor': '^'
}

aggr_map = {
  'RULE_STATIC' : 'static',
  'RULE_COUNT' : 'count',
  'RULE_UNION' : 'union',  
  'RULE_MIN' : 'min',
  'RULE_MAX' : 'max',  
  'RULE_MEDIAN' : 'median', 
  'RULE_MEAN' : 'mean',
  'RULE_STDDEV' : 'stddev',    
  'RULE_XOR' : 'xor',
  'RULE_SUM' : 'sum',
  'RULE_PROD' : 'prod',  
  'RULE_AND' : 'and',
  'RULE_OR' : 'or',
}

enum_map = {
  
    'RULE_S1_8': 'uint8_t',
    'RULE_S1_16': 'uint16_t',
    'RULE_S1_32': 'uint32_t',
    'RULE_S1_64': 'uint64_t',
  
    'RULE_S2_8': 'uint8_t',
    'RULE_S2_16': 'uint16_t',
    'RULE_S2_32': 'uint32_t',
    'RULE_S2_64': 'uint64_t',

  
    'RULE_ABS': 'abs',
    'RULE_REL': 'rel',
    'RULE_NO': 'no',
    'RULE_EQ': 'eq',
    'RULE_NE': 'ne',
    'RULE_GT': 'gt',
    'RULE_LT': 'lt',
    'RULE_LE': 'le',
    'RULE_GE': 'ge',
    'RULE_IN': 'in',

  
    'RULE_ALLEN_BF': 'allen_bf',
    'RULE_ALLEN_AF': 'allen_af', 
    'RULE_ALLEN_M': 'allen_m',  
    'RULE_ALLEN_MI': 'allen_mi', 
    'RULE_ALLEN_O': 'allen_o',  
    'RULE_ALLEN_OI': 'allen_oi', 
    'RULE_ALLEN_S': 'allen_s',  
    'RULE_ALLEN_SI': 'allen_si', 
    'RULE_ALLEN_D': 'allen_d',  
    'RULE_ALLEN_DI': 'allen_di', 
    'RULE_ALLEN_F': 'allen_f',  
    'RULE_ALLEN_FI': 'allen_fi', 
    'RULE_ALLEN_EQ': 'allen_eq'  
}

preamble = """
/*
 * This file was automatically generated by %s
 * DO NOT CHANGE!
 */

"""%argv[0]

header = open("auto_comps.h", "w")
source = open("auto_comps.c", "w")

header.write(preamble)
source.write(preamble)

header.write("#ifndef flowy_engine_auto_comps_h\n")
header.write("#define flowy_engine_auto_comps_h\n\n")

header.write('#include "pipeline.h"\n\n')
header.write('#include "error_handlers.h"\n\n')
header.write("#include <math.h>\n")

source.write('#include "auto_comps.h"\n')

source.write("""
int 
compar(const void *a, const void *b) {
    return (*(uint64_t *)a > *(uint64_t *)b) - (*(uint64_t *)b > *(uint64_t *)a);
}

""")

filter_proto = """bool 
                  filter_%s_%s(char *record, 
                               size_t field_offset, 
                               uint64_t value, 
                               uint64_t delta)"""
def filter_body(op, atype):
    result = " {\n\n"
    if op == "eq":
        result += "    %s rec = *(%s *)(record + field_offset);\n"%(atype, atype)
        result += "    return (rec >= value - delta) && (rec <= value + delta);\n"
    elif op == "ne":
        result += "    %s rec = *(%s *)(record + field_offset);\n"%(atype, atype)
        result += "    return (rec < value - delta) || (rec > value + delta);\n";
    elif op in ['lt', 'le']:
        result += "    return *(%s *)(record + field_offset) %s value + delta;\n"%(atype, operation_map[op])
    elif op in ['gt', 'ge']:
        result += "    return *(%s *)(record + field_offset) %s value - delta;\n"%(atype, operation_map[op])
    else:
        raise ValueError(op)
    result += "}\n\n"
    return result

grouper_proto = """bool
                   grouper_%s_%s_%s_%s(struct group *group, 
                                       size_t field_offset1, 
                                       char *record2, 
                                       size_t field_offset2, 
                                       uint64_t delta)"""
def grouper_body(op, atype1, atype2, dtype):
    result = " {\n\n"
    if op == "eq":
        if dtype == 'no':
            result += "    return *(%s *)(group->members[0] + field_offset1) == *(%s *)(record2 + field_offset2);\n"%(atype1, atype2)
        else:
            if dtype == 'rel':
                result += "    %s rec1 = *(%s *)(group->members[group->num_members-1] + field_offset1);\n"%(atype1, atype1)
            else:
                result += "    %s rec1 = *(%s *)(group->members[0] + field_offset1);\n"%(atype1, atype1)
            result += "    %s rec2 = *(%s *)(record2 + field_offset2);\n"%(atype2, atype2)
            result += "    return (rec1 >= rec2 - delta) && (rec1 <= rec2 + delta);\n"
    elif op == "ne":
        if dtype == 'no':
            result += "    return *(%s *)(group->members[0] + field_offset1) != *(%s *)(record2 + field_offset2);\n"%(atype1, atype2)
        else:
            if dtype == 'rel':
                result += "    %s rec1 = *(%s *)(group->members[group->num_members-1] + field_offset1);\n"%(atype1, atype1)
            else:
                result += "    %s rec1 = *(%s *)(group->members[0] + field_offset1);\n"%(atype1, atype1)
            result += "    %s rec2 = *(%s *)(record2 + field_offset2);\n"%(atype2, atype2)
            result += "    return (rec1 < rec2 - delta) || (rec1 > rec2 + delta);\n"
    elif op in ['lt', 'le']:
        if dtype == 'no':
            result += "    return *(%s *)(group->members[0] + field_offset1) %s *(%s *)(record2 + field_offset2);\n"%(atype1, operation_map[op], atype2)
        elif dtype == 'rel':
            result += "    return *(%s *)(group->members[group->num_members-1] + field_offset1) %s *(%s *)(record2 + field_offset2) + delta;\n"%(atype1, operation_map[op], atype2)
        else:
            result += "    return *(%s *)(group->members[0] + field_offset1) %s *(%s *)(record2 + field_offset2) + delta;\n"%(atype1, operation_map[op], atype2)
    elif op in ['gt', 'ge']:
        if dtype == 'no':
            result += "    return *(%s *)(group->members[0] + field_offset1) %s *(%s *)(record2 + field_offset2);\n"%(atype1, operation_map[op], atype2)
        if dtype == 'rel':
            result += "    return *(%s *)(group->members[group->num_members-1] + field_offset1) %s *(%s *)(record2 + field_offset2) - delta;\n"%(atype1, operation_map[op], atype2)
        else:
            result += "    return *(%s *)(group->members[0] + field_offset1) %s *(%s *)(record2 + field_offset2) - delta;\n"%(atype1, operation_map[op], atype2)
    else:
        raise ValueError(op)
    result += "}\n\n"
    return result;

groupaggr_proto = """struct aggr 
                     aggr_%s_%s(char **records,
                                char *group_aggregation,
                                size_t num_records, 
                                size_t field_offset, 
                                bool if_aggr_common)"""

def groupaggr_body(op, atype):
    result = " {\n\n"
    result += "    struct aggr aggr;\n"
    result += "    if (num_records == 0) {\n"
    result += "        aggr.num_values = 0;\n"
    result += "        aggr.values = NULL;\n"
    result += "        return aggr;\n"
    result += "    }\n"
    if op == 'static':
        result += "    if (if_aggr_common) {\n"
        result += "      aggr.num_values = 1;\n"
        result += "      aggr.values = (uint64_t *)malloc(sizeof(uint64_t));\n"
        result += "      if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "      aggr.values[0] = *(%s *)(records[0] + field_offset);\n"%atype
        result += "    }\n"
    elif op == 'count':
        result += "    aggr.num_values = 1;\n"
        result += "    aggr.values = (uint64_t *)malloc(sizeof(uint64_t)*aggr.num_values);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    aggr.values[0] = num_records;\n"
    elif op in ['prod', 'sum', 'and', 'or', 'xor']:
        result += "    int i;\n"
        result += "    aggr.num_values = 1;\n"
        result += "    aggr.values = (uint64_t *)malloc(sizeof(uint64_t)*aggr.num_values);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    for (i = 0; i < num_records; i++) {\n"
        result += "        aggr.values[0] %s= *(%s *)(records[i] + field_offset);\n"%(operation_map[op], atype)
        result += "    }\n"
    elif op == 'mean':
        result += "    int i;\n"
        result += "    aggr.num_values = 1;\n"
        result += "    aggr.values = (uint64_t *)malloc(sizeof(uint64_t)*aggr.num_values);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    for (i = 0; i < num_records; i++) {\n"
        result += "        aggr.values[0] += *(%s *)(records[i] + field_offset);\n"%atype
        result += "    }\n"
        result += "    aggr.values[0] /= num_records;\n"
    elif op == 'stddev':
        result += "    int i;\n"
        result += "    uint64_t stddev;\n"
        result += "    aggr.num_values = 1;\n"
        result += "    aggr.values = (uint64_t *)malloc(sizeof(uint64_t)*aggr.num_values);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    for (i = 0; i < num_records; i++) {\n"
        result += "        aggr.values[0] += *(%s *)(records[i] + field_offset);\n"%atype
        result += "    }\n"
        result += "    aggr.values[0] /= num_records;\n"
        result += "    for (i = 0; i < num_records; i++) {\n"
        result += "        stddev += (*(%s *)(records[i] + field_offset)-aggr.values[0])*(*(%s *)(records[i] + field_offset)-aggr.values[0]);\n"%(atype, atype)
        result += "    }\n"
        result += "    stddev /= num_records;\n"
        result += "    stddev = sqrt(stddev);\n"
    elif op == 'union':
        result += "    int i;\n"
        result += "    uint64_t *temp;\n"
        result += "    uint64_t last;\n"
        result += "    temp = (uint64_t *)malloc(sizeof(uint64_t)*num_records);\n"
        result += "    if (temp == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    for (i=0; i < num_records; i++) {\n"
        result += "        temp[i] = *(%s *)(records[i] + field_offset);\n"%atype
        result += "    }\n"
        result += "    qsort(temp, num_records, sizeof(uint64_t), compar);\n"
        result += "    aggr.values = (uint64_t *)malloc(sizeof(uint64_t)*num_records);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    aggr.values[0] = temp[0];\n"
        result += "    last = temp[0];\n"
        result += "    aggr.num_values = 1;\n"
        result += "    for (i=1; i < num_records; i++) {\n"
        result += "        if (temp[i] != last) {\n"
        result += "            aggr.values[aggr.num_values++] = temp[i];\n"
        result += "            last = temp[i];\n"
        result += "        }\n"
        result += "    }\n"
        result += "    aggr.values = (uint64_t *)realloc(aggr.values, sizeof(uint64_t)*aggr.num_values);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    free(temp);\n"
    elif op == 'median':
        result += "    int i;\n"
        result += "    uint64_t *temp;\n"
        result += "    temp = (uint64_t *)malloc(sizeof(uint64_t)*num_records);\n"
        result += "    if (temp == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    for (i=0; i < num_records; i++) {\n"
        result += "        temp[i] = *(%s *)(records[i] + field_offset);\n"%atype
        result += "    }\n"
        result += "    qsort(temp, num_records, sizeof(uint64_t), compar);\n"
        result += "    aggr.num_values = 1;\n"
        result += "    aggr.values = (uint64_t *)malloc(sizeof(uint64_t)*aggr.num_values);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    aggr.values[0] = temp[num_records/2];"
        result += "    free(temp);\n"
    elif op == 'min':
        result += "    int i;\n"
        result += "    aggr.num_values = 1;\n"
        result += "    aggr.values = (uint64_t *)malloc(sizeof(uint64_t)*aggr.num_values);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    aggr.values[0] = *(%s *)(records[0] + field_offset);\n"%atype
        result += "    for (i = 1; i < num_records; i++) {\n"
        result += "        if (*(%s *)(records[0] + field_offset) < aggr.values[0]) {\n"%atype
        result += "            aggr.values[0] = *(%s *)(records[0] + field_offset);\n"%atype
        result += "        }\n"
        result += "    }\n"
    elif op == 'max':
        result += "    int i;\n"
        result += "    aggr.num_values = 1;\n"
        result += "    aggr.values = (uint64_t *)malloc(sizeof(uint64_t)*aggr.num_values);\n"
        result += "    if (aggr.values == NULL)\n"
        result += "        errExit(\"malloc\");\n"
        result += "    aggr.values[0] = *(%s *)(records[0] + field_offset);\n"%atype
        result += "    for (i = 1; i < num_records; i++) {\n"
        result += "        if (*(%s *)(records[0] + field_offset) > aggr.values[0]) {\n"%atype
        result += "            aggr.values[0] = *(%s *)(records[0] + field_offset);\n"%atype
        result += "        }\n"
        result += "    }\n"
    else:
        raise ValueError(op)
    result += "    *(%s*)(group_aggregation + field_offset) = aggr.values[0];"%atype
    result += "    return aggr;\n"
    result += "}\n\n"
    return result


gfilter_proto = """bool 
                   gfilter_%s_%s(struct group *group, 
                                 size_t field_offset, 
                                 uint64_t value, 
                                 uint64_t delta)"""

def gfilter_body(op, atype):
    result = " {\n\n"
    result += "%s* aggr_value = (%s*)(group->group_aggr_record + field_offset);\n"%(atype,atype)
  
    if op == "eq":
        result += "    return (*aggr_value >= value - delta) && (*aggr_value <= value + delta);\n"
    elif op == "ne":
        result += "    return (*aggr_value < value - delta) || (*aggr_value > value + delta);\n";
    elif op in ['lt', 'le']:
        result += "    return *aggr_value %s value + delta;\n"%operation_map[op]
    elif op in ['gt', 'ge']:
        result += "    return *aggr_value %s value - delta;\n"%operation_map[op]
    else:
        raise ValueError(op)
    result += "}\n\n"
    return result

#merger: part I: operation on field offsets
merger1_proto = """bool 
                  merger_%s_%s_%s(struct group *group1, 
                                  size_t field1_offset, 
                                  struct group *group2, 
                                  size_t field2_offset, 
                                  uint64_t delta)"""
def merger1_body(op, atype1, atype2):
    result = " {\n\n"
    if op in ['eq', 'ne', 'lt', 'gt', 'le', 'ge', 'in']:
        result += "    if (*(%s*)(group1->group_aggr_record + field1_offset) == 0 ||\n"%atype1
        result += "        *(%s*)(group2->group_aggr_record + field2_offset) == 0)\n"%atype2
        result += "      return false;\n\n"
    if op == "eq":
      result += """    return (*(%s*)(group1->group_aggr_record + field1_offset) >= 
                               *(%s*)(group2->group_aggr_record + field2_offset) - delta)
                               && 
                               (*(%s*)(group1->group_aggr_record + field1_offset) <= 
                               *(%s*)(group2->group_aggr_record + field2_offset) + delta);  
          
                  """%(atype1, atype2, atype1, atype2)
    elif op == "ne":
      result += """    return (*(%s*)(group1->group_aggr_record + field1_offset) < 
                               *(%s*)(group2->group_aggr_record + field2_offset) - delta) 
                              || 
                              (*(%s*)(group1->group_aggr_record + field1_offset) > 
                              *(%s*)(group2->group_aggr_record + field2_offset) + delta);          
                  """%(atype1, atype2, atype1, atype2)
    elif op in ['lt', 'le']:
        result += """  return (*(%s*)(group1->group_aggr_record + field1_offset) %s 
                              *(%s*)(group2->group_aggr_record + field2_offset) + delta);\n
                  """%(atype1, operation_map[op], atype2)
    elif op in ['gt', 'ge']:
        result += """  return (*(%s*)(group1->group_aggr_record + field1_offset) %s 
                              *(%s*)(group2->group_aggr_record + field2_offset) - delta);\n          
                  """%(atype1, operation_map[op], atype2)
    #TODO: need to cross-check
    elif op == 'in':
        result += "    for (int i=0; i<group2->aggr[field2_offset].num_values; i++) {\n"
        result += "        if (group1->aggr[field1_offset].values[0] >= group2->aggr[field2_offset].values[i] - delta && group1->aggr[field1_offset].values[0] <= group2->aggr[field2_offset].values[i] + delta) {\n"
        result += "            return true;\n"
        result += "        }\n"
        result += "    }\n"
        result += "    return false;\n"
    else:
        raise ValueError(op)
    result += "}\n\n"
    return result;

#merger: part II: operation on allen intervals

merger2_proto = """bool 
                   merger_%s(struct group *group1, 
                             size_t field1_offset, 
                             struct group *group2, 
                             size_t field2_offset, 
                             uint64_t delta)"""

def merger2_body(op):
  result = " {\n\n"
  if op == 'allen_bf':
    result += "    return group1->end < group2->start;\n"
  elif op == 'allen_af':
    result += "    return group1->start > group2->end;\n"
  elif op == 'allen_m':
    result += "    return group1->end == group2->start;\n"
  elif op == 'allen_mi':
    result += "    return group1->start == group2->end;\n"
  elif op == 'allen_o':
    result += "    return group1->start < group2->start && group1->end > group2->start;\n"
  elif op == 'allen_oi':
    result += "    return group1->end > group2->end && group1->start < group2->end;\n"
  elif op == 'allen_s':
    result += "    return group1->start == group2->start && group1->end < group2->end;\n"
  elif op == 'allen_si':
    result += "    return group1->start == group2->start && group1->end > group2->end;\n"
  elif op == 'allen_d':
    result += "    return group1->start > group2->start && group1->end < group2->end;\n"
  elif op == 'allen_di':
    result += "    return group1->start < group2->start && group1->end > group2->end;\n"
  elif op == 'allen_f':
    result += "    return group1->end == group2->end && group1->start > group2->start;\n"
  elif op == 'allen_fi':
    result += "    return group1->end == group2->end && group1->start < group2->start;\n"
  elif op == 'allen_eq':
    result += "    return group1->start == group2->start && group1->end == group2->end;\n"
  else:
    raise ValueError(op)
  result += "}\n\n"
  return result;

# filter
for op in 'eq', 'ne', 'lt', 'gt', 'le', 'ge':
    for atype in 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t':
        header.write(filter_proto%(op,atype)+";\n")
        source.write(filter_proto%(op,atype))
        source.write(filter_body(op,atype))

# grouper
for op in 'eq', 'ne', 'lt', 'gt', 'le', 'ge':
    for atype1 in 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t':
        for atype2 in 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t':
            for dtype in 'no', 'abs', 'rel':
                header.write(grouper_proto%(op, atype1, atype2, dtype)+";\n")
                source.write(grouper_proto%(op, atype1, atype2, dtype))
                source.write(grouper_body(op, atype1, atype2, dtype))

# group aggr
for op in 'static', 'count', 'union', 'min', 'max', \
          'median', 'mean', 'stddev', 'prod', 'sum', 'and', 'or', 'xor':
    for atype in 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t':
        header.write(groupaggr_proto%(op, atype)+";\n")
        source.write(groupaggr_proto%(op, atype))
        source.write(groupaggr_body(op, atype))

# group filter
for op in 'eq', 'ne', 'lt', 'gt', 'le', 'ge':
  for atype in 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t':  
    header.write(gfilter_proto%(op, atype)+";\n")
    source.write(gfilter_proto%(op, atype))
    source.write(gfilter_body(op, atype))

# merger: part I
for op in 'eq', 'ne', 'lt', 'gt', 'le', 'ge', 'in':
    for atype1 in 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t':
        for atype2 in 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t':      
            header.write(merger1_proto%(op,atype1,atype2)+";\n")
            source.write(merger1_proto%(op,atype1,atype2))
            source.write(merger1_body(op,atype1,atype2))


# merger: part II
for op in 'allen_bf', 'allen_af', 'allen_m', 'allen_mi', 'allen_o', \
          'allen_oi', 'allen_s', 'allen_si', 'allen_d', 'allen_di', \
          'allen_f', 'allen_fi', 'allen_eq':
    header.write(merger2_proto%(op)+";\n")
    source.write(merger2_proto%(op))
    source.write(merger2_body(op))


header.write("#endif\n")
header.close()
source.close()

header = open("auto_assign.h", 'w')
source = open("auto_assign.c", 'w')

header.write(preamble)
source.write(preamble)

header.write("#ifndef flowy_engine_auto_assign_h\n")
header.write("#define flowy_engine_auto_assign_h\n\n")

header.write("#include \"auto_comps.h\"\n\n")
header.write("""void
                assign_fptr(struct flowquery *fquery);\n\n""")

header.write("""struct aggr 
                (*get_aggr_fptr(bool ifgrouper,
                                uint64_t op))(char **records,
                                              char *group_aggregation,
                                              size_t num_records,
                                              size_t field_offset,
                                              bool if_aggr_common);\n
             """)


source.write("#include \"auto_assign.h\"\n")

source.write("""
void 
assign_fptr(struct flowquery *fquery) {
    for (int i = 0; i < fquery->num_branches; i++) {
    struct branch_info* branch = &fquery->branchset[i];
""")
  

# switch statement for the filter

source.write("""

        /* for loop for the filter */
        for (int j = 0; j < branch->num_filter_rules; j++) {
          struct filter_rule* frule = branch->filter_ruleset[j];          
          switch (frule->op) {

""")  
  
for op in 'RULE_EQ', 'RULE_NE', 'RULE_GT', 'RULE_LT', 'RULE_LE', 'RULE_GE':
  for atype in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
    source.write("                case %s | %s:\n"%(op, atype))
    source.write("                    frule->func = filter_%s_%s;\n"%(                                                                                                     enum_map[op], enum_map[atype]))
    source.write("                    break;\n")

source.write("""
            }
        }
""")
  
# switch statement for the grouper

source.write("""
  
      /* for loop for the grouper */
        for (int j = 0; j < branch->num_grouper_rules; j++) {
          struct grouper_rule* grule = branch->grouper_ruleset[j];  
          switch (grule->op) {
""")

for op in 'RULE_EQ', 'RULE_NE', 'RULE_GT', 'RULE_LT', 'RULE_LE', 'RULE_GE':
    for atype1 in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
        for atype2 in 'RULE_S2_8', 'RULE_S2_16', 'RULE_S2_32', 'RULE_S2_64':
            for dtype in 'RULE_ABS', 'RULE_REL', 'RULE_NO':
                source.write("                case %s | %s | %s | %s:\n"%(op, atype1, atype2, dtype))
                source.write("                    grule->func = grouper_%s_%s_%s_%s;\n"%(
                    enum_map[op], enum_map[atype1], enum_map[atype2], enum_map[dtype]))
                source.write("                    break;\n")

source.write("""
            }
        }
""")


# switch statement for the group-aggregation

source.write("""
  
  /* for loop for the group-aggregation */
  for (int j = 0; j < branch->num_aggr; j++) {
  struct grouper_aggr* aggrule = &branch->aggr[j];  
  switch (aggrule->op) {
  """)

for op in 'RULE_STATIC', \
          'RULE_COUNT',  \
          'RULE_UNION',  \
          'RULE_MIN',    \
          'RULE_MAX',    \
          'RULE_MEDIAN', \
          'RULE_MEAN',   \
          'RULE_STDDEV', \
          'RULE_XOR',    \
          'RULE_SUM',    \
          'RULE_PROD',   \
          'RULE_AND',    \
          'RULE_OR':     \
            
  for atype in 'RULE_S1_8',  \
               'RULE_S1_16', \
               'RULE_S1_32', \
               'RULE_S1_64': \
                  
        source.write("case %s | %s:\n"%(op, atype))
        source.write("aggrule->func = aggr_%s_%s;\n"
                     %(aggr_map[op], enum_map[atype]))
        source.write("break;\n")

source.write("""
  }
  }
  """)


# switch statement for the grouper-filter

source.write("""
  
  /* for loop for the group-filter */
  for (int j = 0; j < branch->num_gfilter_rules; j++) {
  struct gfilter_rule* gfrule = &branch->gfilter_rules[j];  
  switch (gfrule->op) {
  """)

for op in 'RULE_EQ', 'RULE_NE', 'RULE_GT', 'RULE_LT', 'RULE_LE', 'RULE_GE':
  for atype1 in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
    source.write("case %s | %s:\n"%(op,atype1))
    source.write("gfrule->func = gfilter_%s_%s;\n"%(enum_map[op], enum_map[atype1]))
    source.write("break;\n")

source.write("""
  }
  }
  }
  """)


# switch statement for the merger

source.write("""
  
  /* for loop for the merger */
  for (int j = 0; j < fquery->num_merger_rules; j++) {
  struct merger_rule* mrule = &fquery->mruleset[j];  
  switch (mrule->op) {
  """)

for op in 'RULE_EQ', 'RULE_NE', 'RULE_GT', 'RULE_LT', 'RULE_LE', 'RULE_GE', 'RULE_IN':
  for atype1 in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
    for atype2 in 'RULE_S2_8', 'RULE_S2_16', 'RULE_S2_32', 'RULE_S2_64':
      source.write("case %s | %s | %s:\n"%(op, atype1, atype2))
      source.write("mrule->func = merger_%s_%s_%s;\n"
                   %(enum_map[op], enum_map[atype1], enum_map[atype2]))
      source.write("break;\n")

for op in 'RULE_ALLEN_BF',  \
          'RULE_ALLEN_AF',  \
          'RULE_ALLEN_M',   \
          'RULE_ALLEN_MI',  \
          'RULE_ALLEN_O',   \
          'RULE_ALLEN_OI',  \
          'RULE_ALLEN_S',   \
          'RULE_ALLEN_SI',  \
          'RULE_ALLEN_D',   \
          'RULE_ALLEN_DI',  \
          'RULE_ALLEN_F',   \
          'RULE_ALLEN_FI',  \
          'RULE_ALLEN_EQ':  
  source.write("case %s:\n"%(op))
  source.write("mrule->func = merger_%s;\n"
               %(enum_map[op]))
  source.write("break;\n")


source.write("""
            }
        }
    }
""")


# get_aggr_fptr(...)

source.write("""
                struct aggr (*get_aggr_fptr(bool ifgrouper,
                                            uint64_t op))(char **records,
                                                          char *group_aggregation,
                                                          size_t num_records,
                                                          size_t field_offset,
                                                          bool if_aggr_common) {\n
                  struct aggr (*aggr_function)(char **records,
                                               char *group_aggregation,
                                               size_t num_records,
                                               size_t field_offset,
                                               bool if_aggr_common) = NULL;\n
             """)

source.write("if(!ifgrouper) {\n")
source.write("/* cases for the filter-stage */\n")
source.write("switch (op) {\n")
for atype in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
  for op in 'RULE_EQ', 'RULE_NE', 'RULE_GT', 'RULE_LT', 'RULE_LE', 'RULE_GE':  
    source.write("case %s | %s:\n"%(op, atype))
  source.write("aggr_function = aggr_static_%s;\n"%(enum_map[atype]))
  source.write("break;\n")
source.write("}\n} \nelse {\n")             

source.write("/* cases for the grouper-stage */\n")
source.write("switch (op) {\n")             
for atype1 in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
  for op in 'RULE_EQ', 'RULE_NE', 'RULE_GT', 'RULE_LT', 'RULE_LE', 'RULE_GE':
    for atype2 in 'RULE_S2_8', 'RULE_S2_16', 'RULE_S2_32', 'RULE_S2_64':
      for dtype in 'RULE_ABS', 'RULE_REL', 'RULE_NO':
        source.write("case %s | %s | %s | %s:\n"%(op, atype1, atype2, dtype))
  source.write("aggr_function = aggr_static_%s;\n"%(enum_map[atype1]))        
  source.write("break;\n")
source.write("}\n}\n")
source.write("return aggr_function;\n}\n")

header.write("#endif\n")
header.close()
source.close()

def switch_cases(op, atype1, atype2, dtype):
    result = ""
    if op == "eq":
        if dtype == 'no':
            result += "    rule_matches = (*(%s *)(newgroup->members[0] + group_modules[k].field_offset1) == *(%s *)(**record_iter + group_modules[k].field_offset2));\n"%(atype1, atype2)
        else:
            if dtype == 'rel':
                result += "    rec1_%s = *(%s *)(newgroup->members[newgroup->num_members-1] + group_modules[k].field_offset1);\n"%(atype1, atype1)
            else:
                result += "    rec1_%s = *(%s *)(newgroup->members[0] + group_modules[k].field_offset1);\n"%(atype1, atype1)
            result += "    rec2_%s = *(%s *)(**record_iter + group_modules[k].field_offset2);\n"%(atype2, atype2)
            result += "    rule_matches = (rec1_%s >= rec2_%s - group_modules[k].delta) && (rec1_%s <= rec2_%s + group_modules[k].delta);\n"%(atype1, atype2, atype1, atype2)
    elif op == "ne":
        if dtype == 'no':
            result += "    rule_matches = *(%s *)(newgroup->members[0] + group_modules[k].field_offset1) != *(%s *)(**record_iter + group_modules[k].field_offset2);\n"%(atype1, atype2)
        else:
            if dtype == 'rel':
                result += "    rec1_%s = *(%s *)(newgroup->members[newgroup->num_members-1] + group_modules[k].field_offset1);\n"%(atype1, atype1)
            else:
                result += "    rec1_%s = *(%s *)(newgroup->members[0] + group_modules[k].field_offset1);\n"%(atype1, atype1)
            result += "    rec2_%s = *(%s *)(**record_iter + group_modules[k].field_offset2);\n"%(atype2, atype2)
            result += "    rule_matches = (rec1_%s < rec2_%s - group_modules[k].delta) || (rec1_%s > rec2_%s + group_modules[k].delta);\n"%(atype1, atype2, atype1, atype2)
    elif op in ['lt', 'le']:
        if dtype == 'no':
            result += "    rule_matches = *(%s *)(newgroup->members[0] + group_modules[k].field_offset1) %s *(%s *)(**record_iter + group_modules[k].field_offset2);\n"%(atype1, operation_map[op], atype2)
        elif dtype == 'rel':
            result += "    rule_matches = *(%s *)(newgroup->members[newgroup->num_members-1] + group_modules[k].field_offset1) %s *(%s *)(**record_iter + group_modules[k].field_offset2) + group_modules[k].delta;\n"%(atype1, operation_map[op], atype2)
        else:
            result += "    rule_matches = *(%s *)(newgroup->members[0] + group_modules[k].field_offset1) %s *(%s *)(**record_iter + group_modules[k].field_offset2) + group_modules[k].delta;\n"%(atype1, operation_map[op], atype2)
    elif op in ['gt', 'ge']:
        if dtype == 'no':
            result += "    rule_matches = *(%s *)(newgroup->members[0] + group_modules[k].field_offset1) %s *(%s *)(**record_iter + group_modules[k].field_offset2);\n"%(atype1, operation_map[op], atype2)
        if dtype == 'rel':
            result += "    rule_matches = *(%s *)(newgroup->members[newgroup->num_members-1] + group_modules[k].field_offset1) %s *(%s *)(**record_iter + group_modules[k].field_offset2) - group_modules[k].delta;\n"%(atype1, operation_map[op], atype2)
        else:
            result += "    rule_matches = *(%s *)(newgroup->members[0] + group_modules[k].field_offset1) %s *(%s *)(**record_iter + group_modules[k].field_offset2) - group_modules[k].delta;\n"%(atype1, operation_map[op], atype2)
    else:
        raise ValueError(op)
    return result;