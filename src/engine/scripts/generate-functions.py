#!/usr/bin/env python
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

header = open("auto-comps.h", "w")
source = open("auto-comps.c", "w")

header.write(preamble)
source.write(preamble)

header.write("#ifndef flowy_engine_auto_comps_h\n")
header.write("#define flowy_engine_auto_comps_h\n\n")

header.write('#include "base.h"\n')
header.write('#include <math.h>\n')
header.write('#include "pipeline.h"\n')
header.write('#include "errorhandlers.h"\n')
header.write('#include "utils.h"\n')
header.write('#include "grouper.h"\n')
header.write('struct grouper_intermediate_result;\n\n')

source.write('#include "auto-comps.h"\n')

source.write("""
  int 
  compar(const void *a, const void *b) {
  return (*(uint32_t *)a > *(uint32_t *)b) - (*(uint32_t *)b > *(uint32_t *)a);
  }
  
  """)

filter_proto = """bool 
  filter_%s_%s(const char* const record, 
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
  grouper_%s_%s_%s_%s(const struct group const *group, 
  size_t field_offset1, 
  const char* const record2, 
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

groupaggr_proto = """struct aggr* 
  aggr_%s_%s(char** const records,
  char* group_aggregation,
  size_t num_records, 
  size_t field_offset, 
  bool if_aggr_common)"""

def groupaggr_body(op, atype):
  result = " {\n\n"
  result += "   struct aggr *aggr = calloc(1, sizeof(struct aggr));\n"
  result += "  if (aggr == NULL)\n"
  result += '    errExit("calloc");\n'
  result += "    if (num_records == 0) {\n"
  result += "        aggr->num_values = 0;\n"
  result += "        aggr->values = NULL;\n"
  result += "    }else {\n"
  
  if op == 'static':
    result += "    if (if_aggr_common) {\n"
    result += "      aggr->num_values = 1;\n"
    result += "      aggr->values = (uint32_t *)calloc(1, sizeof(uint32_t));\n"
    result += "      if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "      aggr->values[0] = *(%s *)(records[0] + field_offset);\n"%atype
    result += "      *(uint32_t*)(group_aggregation + field_offset) = aggr->values[0];\n"
    result += "    } else {\n"
    result += "    free(aggr);\n"      
    result += """ aggr = aggr_union_uint32_t(records, 
      group_aggregation, 
      num_records, 
      field_offset, 
      if_aggr_common);  
      /* this is a SET */
      *(uint32_t*)(group_aggregation + field_offset) = 0;
      """
    result += "}\n"  
  elif op == 'count':
    result += "    aggr->num_values = 1;\n"
    result += "    aggr->values = (uint32_t *)calloc(aggr->num_values, sizeof(uint32_t));\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "    aggr->values[0] = num_records;\n"
    result += "      *(uint32_t*)(group_aggregation + field_offset) = aggr->values[0];\n"
  elif op in ['prod', 'sum', 'and', 'or', 'xor']:
    result += "    aggr->num_values = 1;\n"
    result += "    aggr->values = (uint32_t *)calloc(aggr->num_values, sizeof(uint32_t));\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "    for (int i = 0; i < num_records; i++)\n"
    result += "        aggr->values[0] %s= *(%s *)(records[i] + field_offset);\n"%(operation_map[op], atype)
    result += "    *(uint32_t*)(group_aggregation + field_offset) = aggr->values[0];\n"
  elif op == 'mean':
    result += "    aggr->num_values = 1;\n"
    result += "    aggr->values = (uint32_t *)calloc(aggr->num_values, sizeof(uint32_t));\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "    for (int i = 0; i < num_records; i++)\n"
    result += "        aggr->values[0] += *(%s *)(records[i] + field_offset);\n"%atype
    result += "    aggr->values[0] /= num_records;\n"
    result += "      *(uint32_t*)(group_aggregation + field_offset) = aggr->values[0];\n"
  elif op == 'stddev':
    result += "    uint32_t stddev;\n"
    result += "    aggr->num_values = 1;\n"
    result += "    aggr->values = (uint32_t *)calloc(aggr->num_values, sizeof(uint32_t));\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "    for (int i = 0; i < num_records; i++) {\n"
    result += "        aggr->values[0] += *(%s *)(records[i] + field_offset);\n"%atype
    result += "    }\n"
    result += "    aggr->values[0] /= num_records;\n"
    result += "    for (int i = 0; i < num_records; i++) {\n"
    result += "        stddev += (*(%s *)(records[i] + field_offset)-aggr->values[0])*(*(%s *)(records[i] + field_offset)-aggr->values[0]);\n"%(atype, atype)
    result += "    }\n"
    result += "    stddev /= num_records;\n"
    result += "    stddev = sqrt(stddev);\n"
    result += "    aggr->values[0] = stddev;\n"      
    result += "      *(uint32_t*)(group_aggregation + field_offset) = aggr->values[0];\n"
  elif op == 'union':
    result += "    uint32_t *temp;\n"
    result += "    uint32_t last;\n"
    result += "    temp = (uint32_t *)malloc(sizeof(uint32_t)*num_records);\n"
    result += "    if (temp == NULL)\n"
    result += "        errExit(\"malloc\");\n"
    result += "    for (int i=0; i < num_records; i++) {\n"
    result += "        temp[i] = *(%s *)(records[i] + field_offset);\n"%atype
    result += "    }\n"
    result += "    qsort(temp, num_records, sizeof(uint32_t), compar);\n"
    result += "    aggr->values = (uint32_t *)calloc(num_records, sizeof(uint32_t));\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "    aggr->values[0] = temp[0];\n"
    result += "    last = temp[0];\n"
    result += "    aggr->num_values = 1;\n"
    result += "    for (int i=1; i < num_records; i++) {\n"
    result += "        if (temp[i] != last) {\n"
    result += "            aggr->values[aggr->num_values++] = temp[i];\n"
    result += "            last = temp[i];\n"
    result += "        }\n"
    result += "    }\n"
    result += "    aggr->values = (uint32_t *)realloc(aggr->values, sizeof(uint64_t)*aggr->num_values);\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"malloc\");\n"
    result += "    free(temp);\n"
  elif op == 'median':
    result += "    uint32_t *temp;\n"
    result += "    temp = (uint32_t *)malloc(sizeof(uint32_t)*num_records);\n"
    result += "    if (temp == NULL)\n"
    result += "        errExit(\"malloc\");\n"
    result += "    for (int i=0; i < num_records; i++) {\n"
    result += "        temp[i] = *(%s *)(records[i] + field_offset);\n"%atype
    result += "    }\n"
    result += "    qsort(temp, num_records, sizeof(uint64_t), compar);\n"
    result += "    aggr->num_values = 1;\n"
    result += "    aggr->values = (uint32_t *)calloc(aggr->num_values, sizeof(uint32_t));\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "    aggr->values[0] = temp[num_records/2];"
    result += "    free(temp);\n"
    result += "      *(uint32_t*)(group_aggregation + field_offset) = aggr->values[0];\n"
  elif op == 'min':
    result += "    aggr->num_values = 1;\n"
    result += "    aggr->values = (uint32_t *)calloc(aggr->num_values, sizeof(uint32_t));\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "    aggr->values[0] = *(%s *)(records[0] + field_offset);\n"%atype
    result += "    for (int i = 1; i < num_records; i++) {\n"
    result += "        if (*(%s *)(records[0] + field_offset) < aggr->values[0]) {\n"%atype
    result += "            aggr->values[0] = *(%s *)(records[0] + field_offset);\n"%atype
    result += "        }\n"
    result += "    }\n"
    result += "      *(uint32_t*)(group_aggregation + field_offset) = aggr->values[0];\n"
  elif op == 'max':
    result += "    aggr->num_values = 1;\n"
    result += "    aggr->values = (uint32_t *)calloc(aggr->num_values, sizeof(uint32_t));\n"
    result += "    if (aggr->values == NULL)\n"
    result += "        errExit(\"calloc\");\n"
    result += "    aggr->values[0] = *(%s *)(records[0] + field_offset);\n"%atype
    result += "    for (int i = 1; i < num_records; i++) {\n"
    result += "        if (*(%s *)(records[0] + field_offset) > aggr->values[0]) {\n"%atype
    result += "            aggr->values[0] = *(%s *)(records[0] + field_offset);\n"%atype
    result += "        }\n"
    result += "    }\n"
    result += "      *(uint32_t*)(group_aggregation + field_offset) = aggr->values[0];\n"
  else:
    raise ValueError(op)
  result += "}\n"
  result += "    return aggr;\n"
  result += "}\n\n"
  return result


gfilter_proto = """bool 
  gfilter_%s_%s(const struct group const *group, 
  size_t field_offset, 
  uint64_t value, 
  uint64_t delta)"""

def gfilter_body(op, atype):
  result = " {\n\n"
  result += "%s* aggr_value = (%s*)(group->aggr_result->aggr_record + field_offset);\n"%(atype,atype)
  
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
  merger_%s_%s_%s(const struct group* const group1, 
  size_t field1_offset, 
  const struct group* const group2, 
  size_t field2_offset, 
  uint64_t delta)"""
def merger1_body(op, atype1, atype2):
  result = " {\n\n"
  if op in ['eq', 'ne', 'lt', 'gt', 'le', 'ge', 'in']:
    result += "    if (*(%s*)(group1->aggr_result->aggr_record + field1_offset) == 0 ||\n"%atype1
    result += "        *(%s*)(group2->aggr_result->aggr_record + field2_offset) == 0)\n"%atype2
    result += "      return false;\n\n"
  if op == "eq":
    result += """    return (*(%s*)(group1->aggr_result->aggr_record + field1_offset) >= 
      *(%s*)(group2->aggr_result->aggr_record + field2_offset) - delta)
      && 
      (*(%s*)(group1->aggr_result->aggr_record + field1_offset) <= 
      *(%s*)(group2->aggr_result->aggr_record + field2_offset) + delta);  
      
      """%(atype1, atype2, atype1, atype2)
  elif op == "ne":
    result += """    return (*(%s*)(group1->aggr_result->aggr_record + field1_offset) < 
      *(%s*)(group2->aggr_result->aggr_record + field2_offset) - delta) 
      || 
      (*(%s*)(group1->aggr_result->aggr_record + field1_offset) > 
      *(%s*)(group2->aggr_result->aggr_record + field2_offset) + delta);          
      """%(atype1, atype2, atype1, atype2)
  elif op in ['lt', 'le']:
    result += """  return (*(%s*)(group1->aggr_result->aggr_record + field1_offset) %s 
      *(%s*)(group2->aggr_result->aggr_record + field2_offset) + delta);\n
      """%(atype1, operation_map[op], atype2)
  elif op in ['gt', 'ge']:
    result += """  return (*(%s*)(group1->aggr_result->aggr_record + field1_offset) %s 
      *(%s*)(group2->aggr_result->aggr_record + field2_offset) - delta);\n          
      """%(atype1, operation_map[op], atype2)
  #TODO: need to cross-check
  elif op == 'in':
    result += "    for (int i=0; i<group2->aggr_result->aggrset[field2_offset]->num_values; i++) {\n"
    result += "        if (group1->aggr_result->aggrset[field1_offset]->values[0] >= group2->aggr_result->aggrset[field2_offset]->values[i] - delta && group1->aggr_result->aggrset[field1_offset]->values[0] <= group2->aggr_result->aggrset[field2_offset]->values[i] + delta) {\n"
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
  merger_%s(const struct group* const group1, 
  size_t field1_offset, 
  const struct group* const group2, 
  size_t field2_offset, 
  uint64_t delta)"""


def merger2_body(op):
  result = " {\n\n"
  result += "uint32_t* t1 = (u_int32_t*)(group1->aggr_result->aggr_record + field1_offset);\n"
  result += "uint32_t* t2 = (u_int32_t*)(group2->aggr_result->aggr_record + field2_offset);\n"
  if op == 'allen_bf':
    result += "    return *t1 < *t2;\n"
  elif op == 'allen_af':
    result += "    return *t1 > *t2;\n"
  elif op == 'allen_m':
    result += "    return *t1 == *t2;\n"
  elif op == 'allen_mi':
    result += "    return *t1 == *t2;\n"
  elif op == 'allen_o':
    result += "    return *t1 < *t2 && *t1 > *t2;\n"
  elif op == 'allen_oi':
    result += "    return *t1 > *t2 && *t1 < *t2;\n"
  elif op == 'allen_s':
    result += "    return *t1 == *t2 && *t1 < *t2;\n"
  elif op == 'allen_si':
    result += "    return *t1 == *t2 && *t1 > *t2;\n"
  elif op == 'allen_d':
    result += "    return *t1 > *t2 && *t1 < *t2;\n"
  elif op == 'allen_di':
    result += "    return *t1 < *t2 && *t1 > *t2;\n"
  elif op == 'allen_f':
    result += "    return *t1 == *t2 && *t1 > *t2;\n"
  elif op == 'allen_fi':
    result += "    return *t1 == *t2 && *t1 < *t2;\n"
  elif op == 'allen_eq':
    result += "    return *t1 == *t2 && *t1 == *t2;\n"
  else:
    raise ValueError(op)
  result += "}\n\n"
  return result;


# alloc_uniqresult

alloc_uniqresult_proto = """ 
          struct uniq_recordset_result*
          alloc_uniqresult_%s(
                                    size_t num_filtered_records,
                                    struct grouper_rule** const grouper_ruleset,
                                    char*** const sorted_recordset_ref
                                   )"""
                         
def alloc_uniqresult_body(atype):
  result = " {\n\n"
  result += """
    
    /* free'd just before calling grouper_aggregations(...) */
    struct uniq_recordset_result* 
    uresult = calloc(1, sizeof(struct uniq_recordset_result));
    if (uresult == NULL)
    errExit("calloc"); """
    
  result += """
    
    /* free'd just before returning from grouper(...) */
    struct tree_item_%s* uniq_recordset = (struct tree_item_%s*)
    calloc(num_filtered_records, 
    sizeof(struct tree_item_%s));
    if (uniq_recordset == NULL)
    errExit("calloc");  
    
    /* unlinked uniq_recordset[0].ptr and free'd uniq_recordset
    * just before calling grouper_aggregations(...)
    */
    uniq_recordset[0].value = *(%s *)(*sorted_recordset_ref[0] + 
    grouper_ruleset[0]->field_offset2);
    uniq_recordset[0].ptr = &sorted_recordset_ref[0];
    size_t num_uniq_records = 1;
    
    for (int i = 0; i < num_filtered_records; i++) {
    if (
    *(%s *)(*sorted_recordset_ref[i] + 
    grouper_ruleset[0]->field_offset2) != 
    uniq_recordset[num_uniq_records-1].value
    ) {
    
    uniq_recordset[num_uniq_records].value = 
    *(%s *)(*sorted_recordset_ref[i] + 
    grouper_ruleset[0]->field_offset2);
    uniq_recordset[num_uniq_records].ptr = &sorted_recordset_ref[i];
    num_uniq_records++;
    }
    }
    
    uniq_recordset = (struct tree_item_%s *)
    realloc(uniq_recordset, 
    num_uniq_records*sizeof(struct tree_item_%s));
    if (uniq_recordset == NULL)
    errExit("realloc");
    
    uresult->num_uniq_records = num_uniq_records;
    uresult->uniq_recordset.recordset_%s = uniq_recordset;
   
            """%(enum_map[atype],
                 enum_map[atype],
                 enum_map[atype],
                 enum_map[atype],
                 enum_map[atype],
                 enum_map[atype],
                 enum_map[atype],
                 enum_map[atype],
                 enum_map[atype])
  
  result += "return uresult;\n}\n\n"
  return result;


# dealloc_uniqresult

dealloc_uniqresult_proto = """   
  void
  dealloc_uniqresult_%s(struct uniq_recordset_result* uniq_result)"""

def dealloc_uniqresult_body(atype):
  result = " {\n\n"
  result += """
    
    // unlink the uniq records from the flow data
    for (int i = 0; i < uniq_result->num_uniq_records; i++)
    uniq_result->uniq_recordset.recordset_%s[i].ptr = NULL;
    free(uniq_result->uniq_recordset.recordset_%s);    
    uniq_result->uniq_recordset.recordset_%s = NULL;    
   
    """%(enum_map[atype],
         enum_map[atype],
         enum_map[atype])
  
  result += "free(uniq_result);\n";
  result += "uniq_result = NULL;\n}\n\n"
  return result;


# get_uniq_record

get_uniq_record_proto = """
  char*
  get_uniq_record_%s(const struct uniq_recordset_result* const uniq_result,
  int index)"""

def get_uniq_record_body(atype):
  result = " {\n\n"
  result += """    
  return **uniq_result->uniq_recordset.recordset_%s[index].ptr;\n}\n\n
    """%(enum_map[atype])
  return result;


#bsearch_r_uintX_t wrappers

bsearch_proto = """ char*** 
  bsearch_%s(
  const char* const filtered_record,
  struct grouper_rule** const grouper_ruleset,
  const struct grouper_intermediate_result* const intermediate_result
  )"""

def bsearch_body(atype):
  result = " {\n\n"
  result += """
    char*** record_iter = 
    (
    (struct tree_item_%s *)
    bsearch_r(
    filtered_record,
    (void *)intermediate_result[0].uniq_result->uniq_recordset.recordset_%s,
    intermediate_result[0].uniq_result->num_uniq_records,
    sizeof(struct tree_item_%s),
    (void *)&grouper_ruleset[0]->field_offset1,
    comp_%s_p
    )
    )->ptr;
    
    """%(enum_map[atype],
         enum_map[atype],
         enum_map[atype],
         enum_map[atype])
  result += "return record_iter;\n}\n\n"
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

#bsearch_r
for atype in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
  header.write(bsearch_proto%(enum_map[atype])+";\n")
  source.write(bsearch_proto%(enum_map[atype]))
  source.write(bsearch_body(atype))

#alloc_uniqresult
for atype in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
  header.write(alloc_uniqresult_proto%(enum_map[atype])+";\n")
  source.write(alloc_uniqresult_proto%(enum_map[atype]))
  source.write(alloc_uniqresult_body(atype))

#dealloc_uniqresult
for atype in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
  header.write(dealloc_uniqresult_proto%(enum_map[atype])+";\n")
  source.write(dealloc_uniqresult_proto%(enum_map[atype]))
  source.write(dealloc_uniqresult_body(atype))

#get_uniq_record
for atype in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
  header.write(get_uniq_record_proto%(enum_map[atype])+";\n")
  source.write(get_uniq_record_proto%(enum_map[atype]))
  source.write(get_uniq_record_body(atype))

header.write("#endif\n")
header.close()
source.close()

header = open("auto-assign.h", 'w')
source = open("auto-assign.c", 'w')

header.write(preamble)
source.write(preamble)

header.write("#ifndef flowy_engine_auto_assign_h\n")
header.write("#define flowy_engine_auto_assign_h\n\n")

header.write('#include "auto-comps.h"\n')
header.write('#include "grouper.h"\n\n')
header.write('struct grouper_intermediate_result;\n\n')

header.write("""void 
  assign_filter_func(struct filter_rule* const frule);\n\n""")

header.write("""void 
  assign_grouper_func(struct grouper_rule* const grule);\n\n""")

header.write("""void 
  assign_aggr_func(struct aggr_rule* const arule);\n\n""")

header.write("""void 
  assign_gfilter_func(struct gfilter_rule* const gfrule);\n\n""")

header.write("""void 
  assign_merger_func(struct merger_rule* const mrule);\n\n""")

header.write("""struct aggr* 
  (*get_aggr_fptr(bool ifgrouper,
  uint64_t op))(char **records,
  char *group_aggregation,
  size_t num_records,
  size_t field_offset,
  bool if_aggr_common);\n
  """)

header.write("""struct grouper_type* get_gtype(uint64_t op);\n\n""");

source.write("#include \"auto-assign.h\"\n")


# assign_filter_func(...)

source.write("""  
  void 
  assign_filter_func(struct filter_rule* const frule) {  
  """)

source.write("""
  
  switch ( 
  frule->op->op | 
  frule->op->field_type
  ) {
  
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


# assign_grouper_func(...)

source.write("""  
  void 
  assign_grouper_func(struct grouper_rule* const grule) {
  """)

source.write("""
  
  switch (
          grule->op->op | 
          grule->op->field1_type | 
          grule->op->field2_type | 
          grule->op->optype
         ) {
  
  """)

for op in 'RULE_EQ', 'RULE_NE', 'RULE_GT', 'RULE_LT', 'RULE_LE', 'RULE_GE':
  for atype1 in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
    for atype2 in 'RULE_S2_8', 'RULE_S2_16', 'RULE_S2_32', 'RULE_S2_64':
      for dtype in 'RULE_ABS', 'RULE_REL', 'RULE_NO':
        source.write("case %s | %s | %s | %s:\n"%(op, atype1, atype2, dtype))
        source.write("grule->func = grouper_%s_%s_%s_%s;\n"%(
                                                             enum_map[op], 
                                                             enum_map[atype1], 
                                                             enum_map[atype2], 
                                                             enum_map[dtype]))
        source.write("break;\n")

source.write("""}\n}\n""")


# assign_aggr_func(...)

source.write("""  
  void 
  assign_aggr_func(struct aggr_rule* const arule) {
  """)

source.write("""
  
  switch (
          arule->op->op |
          arule->op->field_type
         ) {
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
        source.write("arule->func = aggr_%s_%s;\n"
                     %(aggr_map[op], enum_map[atype]))
        source.write("break;\n")

source.write("""}\n}\n""")


# assign_gfilter_func(...)

source.write("""  
  void 
  assign_gfilter_func(struct gfilter_rule* const gfrule) {
  """)

source.write("""

  switch (
          gfrule->op->op |
          gfrule->op->field_type
         ) {
  """)

for op in 'RULE_EQ', 'RULE_NE', 'RULE_GT', 'RULE_LT', 'RULE_LE', 'RULE_GE':
  for atype1 in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
    source.write("case %s | %s:\n"%(op,atype1))
    source.write("gfrule->func = gfilter_%s_%s;\n"%(enum_map[op], enum_map[atype1]))
    source.write("break;\n")

source.write("}\n}\n")



# assign_merger_func(...)

source.write("""  
  void 
  assign_merger_func(struct merger_rule* const mrule) {  
  """)

source.write("""
  
  switch (
           mrule->op->op |
           mrule->op->field1_type |
           mrule->op->field2_type          
         ) {
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
  }\n  
  """)


# get_aggr_fptr(...)

source.write("""
  struct aggr* (*get_aggr_fptr(bool ifgrouper,
  uint64_t op))(char **records,
  char *group_aggregation,
  size_t num_records,
  size_t field_offset,
  bool if_aggr_common) {\n
  struct aggr* (*aggr_function)(char **records,
  char *group_aggregation,
  size_t num_records,
  size_t field_offset,
  bool if_aggr_common) = NULL;\n
  """)

source.write("if(!ifgrouper) {\n")
source.write("/* cases for the filter-stage */\n")
source.write("switch (op) {\n")
for atype in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
  source.write("case %s:\n"%(atype))
  source.write("aggr_function = aggr_static_%s;\n"%(enum_map[atype]))
  source.write("break;\n")
source.write("}\n} \nelse {\n")             

source.write("/* cases for the grouper-stage */\n")
source.write("switch (op) {\n")             
for atype1 in 'RULE_S1_8', 'RULE_S1_16', 'RULE_S1_32', 'RULE_S1_64':
  source.write("case %s:\n"%(atype1))
  source.write("aggr_function = aggr_static_%s;\n"%(enum_map[atype1]))        
  source.write("break;\n")
source.write("}\n}\n")
source.write("return aggr_function;\n}\n")


# get_gtype(...)

source.write("""  
                  struct grouper_type* get_gtype(uint64_t op) {
                  
                  /* free'd just before calling grouper_aggregations(...) */
                  struct grouper_type* gtype = calloc(1, sizeof(struct grouper_type));
                  if(gtype == NULL)
                  errExit("calloc"); 
             """)

source.write('switch (op) {\n')

for atype in 'RULE_S2_8', 'RULE_S2_16', 'RULE_S2_32', 'RULE_S2_64':
  
  source.write("case %s:\n"%(atype))
  source.write("gtype->qsort_comp = comp_%s;\n"%enum_map[atype])
  source.write("gtype->bsearch = bsearch_%s;\n"%enum_map[atype])
  source.write("gtype->bsearch = bsearch_%s;\n"%enum_map[atype])  
  source.write("gtype->alloc_uniqresult = alloc_uniqresult_%s;\n"%enum_map[atype])  
  source.write("gtype->dealloc_uniqresult = dealloc_uniqresult_%s;\n"%enum_map[atype])    
  source.write("gtype->get_uniq_record = get_uniq_record_%s;\n"%enum_map[atype])      
  source.write("break;\n")

source.write('}\n')
source.write('return gtype;\n}\n\n');


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
