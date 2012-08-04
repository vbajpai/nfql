#!/usr/bin/env python
#
# Copyright 2012 Vaibhav Bajpai <contact@vaibhavbajpai.com>
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


import json, sys, os, os.path
from pipeline import FilterRule, GrouperRule, AggregationRule
from pipeline import GroupFilterRule, MergerRule
from pipeline import protocol

if __name__ == '__main__':

  term1 = {'term': vars(FilterRule('dstport', 0, 'RULE_S1_16', 0,
                                   'RULE_GE'))}
  clause1 = {'clause': [term1]}
  filter1 = {'dnf-expr': [clause1]}

  term1 = {'term': vars(GrouperRule('prot', 'RULE_S1_8',
                                    'prot', 'RULE_S2_8', 0,
                                    'RULE_EQ', 'RULE_ABS'))}
  term2 = {'term': vars(GrouperRule('dPkts', 'RULE_S1_32',
                                    'dPkts', 'RULE_S2_32', 0,
                                    'RULE_EQ', 'RULE_ABS'))}
  term3 = {'term': vars(GrouperRule('dOctets', 'RULE_S1_32',
                                    'dOctets', 'RULE_S2_32', 0,
                                    'RULE_EQ', 'RULE_ABS'))}
  term4 = {'term': vars(GrouperRule('tcp_flags', 'RULE_S1_8',
                                    'tcp_flags', 'RULE_S2_8', 0,
                                    'RULE_EQ', 'RULE_ABS'))}
  term5 = {'term': vars(GrouperRule('First', 'RULE_S1_32',
                                    'First', 'RULE_S2_32', 0,
                                    'RULE_EQ', 'RULE_ABS'))}
  clause1 = {'clause': [term1] + [term2] + [term3] + [term4] + [term5]}
  term1 = {'term': vars(AggregationRule('dPkts', 'RULE_S1_32', 'RULE_SUM'))}
  term2 = {'term': vars(AggregationRule('dOctets', 'RULE_S1_32', 'RULE_SUM'))}
  a1 = {'clause': [term1] + [term2]}
  grouper1 = {'dnf-expr': [clause1], 'aggregation': a1}

  gfilter1 = {'dnf-expr' : []}

  branchset = []
  branchset.append({'filter': filter1,
                    'grouper': grouper1,
                    'groupfilter': gfilter1,
                   })

  merger = {'dnf-expr' : []}
  query = {'branchset': branchset, 'merger': merger}

  fjson = json.dumps(query, indent=2)
  filename = os.path.splitext(sys.argv[0])[0]
  fsock = open('%s.json'%filename, 'w')
  fsock.write(fjson)
  fsock.close
