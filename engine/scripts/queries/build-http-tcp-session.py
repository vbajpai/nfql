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


import json
from pipeline import FilterRule, GrouperRule, AggregationRule
from pipeline import GroupFilterRule, MergerRule
from pipeline import protocol

if __name__ == '__main__':




  term1 = {'term': vars(FilterRule('dstport', 80, 'RULE_S1_16', 0,
                                   'RULE_EQ'))}
  term2 = {'term': vars(FilterRule('prot', protocol('TCP'), 'RULE_S1_16', 0,
                                    'RULE_EQ'))}
  clause1 = {'clause': [term1] + [term2]}
  filter1 = {'dnf-expr': [clause1]}

  term1 = {'term': vars(FilterRule('srcport', 80, 'RULE_S1_16', 0,
                                   'RULE_EQ'))}
  term2 = {'term': vars(FilterRule('prot', protocol('TCP'), 'RULE_S1_16', 0,
                                    'RULE_EQ'))}
  clause1 = {'clause': [term1] + [term2]}
  filter2 = {'dnf-expr': [clause1]}






  term1 = {'term': vars(GrouperRule('srcaddr', 'RULE_S1_32',
                                    'srcaddr', 'RULE_S2_32', 0,
                                    'RULE_EQ', 'RULE_ABS'))}
  term2 = {'term': vars(GrouperRule('dstaddr', 'RULE_S1_32',
                                    'dstaddr', 'RULE_S2_32', 0,
                                    'RULE_EQ', 'RULE_ABS'))}
  grouper1_clause1 = {'clause': [term1] + [term2]}
  grouper2_clause1 = {'clause': [term2] + [term1]}

  term1 = {'term': vars(AggregationRule('srcaddr', 'RULE_S1_32',
                                        'RULE_STATIC'))}
  term2 = {'term': vars(AggregationRule('dstaddr', 'RULE_S1_32',
                                        'RULE_STATIC'))}
  term3 = {'term': vars(AggregationRule('dPkts', 'RULE_S1_32',
                                        'RULE_SUM'))}
  term4 = {'term': vars(AggregationRule('dOctets', 'RULE_S1_32',
                                        'RULE_SUM'))}
  a1 = a2 = {'clause': [term1] + [term2] + [term3] + [term4]}

  grouper1 = {'dnf-expr': [grouper1_clause1], 'aggregation': a1}
  grouper2 = {'dnf-expr': [grouper2_clause1], 'aggregation': a2}






  term1 = {'term': vars(GroupFilterRule('dPkts', 200,
                                        'RULE_S1_32', 0,
                                        'RULE_GT'))}
  clause1 = {'clause': [term1]}
  gfilter1 = gfilter2 = {'dnf-expr': [clause1]}






  branchset = []
  branchset.append({'filter': filter1,
                    'grouper': grouper1,
                    'groupfilter': gfilter1,
                   })

  branchset.append({'filter': filter2,
                    'grouper': grouper2,
                    'groupfilter': gfilter2,
                   })






  term1 = {'term': vars(MergerRule(0, 1, 'srcaddr', 'RULE_S1_32',
                                         'dstaddr', 'RULE_S2_32', 0,
                                                    'RULE_EQ',
                                                    'RULE_ABS'))}

  term2 = {'term': vars(MergerRule(0, 1, 'dstaddr', 'RULE_S1_32',
                                         'srcaddr', 'RULE_S2_32', 0,
                                                    'RULE_EQ',
                                                    'RULE_ABS'))}
  clause1 = {'clause': [term1] + [term2]}
  merger = {'dnf-expr': [clause1]}






  query = {'branchset': branchset, 'merger': merger, 'ungrouper': {}}
  fjson = json.dumps(query, indent=2)
  fsock = open('query-http-tcp-session.json', 'w')
  fsock.write(fjson)
  fsock.close
