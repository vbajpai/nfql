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



  term1 = {'term': vars(FilterRule('protocolIdentifier', protocol('TCP'), 'RULE_S1_8', 0,
                                    'RULE_EQ'))}
  clause1 = {'clause': [term1]}
  filter1 = {'dnf-expr': [clause1]}

  term1 = {'term': vars(FilterRule('protocolIdentifier', protocol('TCP'), 'RULE_S1_8', 0,
                                    'RULE_EQ'))}
  clause1 = {'clause': [term1]}
  filter2 = {'dnf-expr': [clause1]}



  term1 = {'term': vars(GrouperRule('sourceIPv4Address', 'RULE_S1_32',
                                    'sourceIPv4Address', 'RULE_S2_32', 0,
                                    'RULE_EQ', 'RULE_REL'))}
  term2 = {'term': vars(GrouperRule('destinationIPv4Address', 'RULE_S1_32',
                                    'destinationIPv4Address', 'RULE_S2_32', 0,
                                    'RULE_EQ', 'RULE_REL'))}
  clause1 = {'clause': [term1] + [term2]}


  term1 = {'term': vars(AggregationRule('packetDeltaCount', 'RULE_S1_32',
                                        'RULE_SUM'))}
  term2 = {'term': vars(AggregationRule('octetDeltaCount', 'RULE_S1_32',
                                        'RULE_SUM'))}
  a1 = a2 = {'clause': [term1] + [term2]}

  grouper1 = {'dnf-expr': [clause1], 'aggregation': a1}
  grouper2 = {'dnf-expr': [clause1], 'aggregation': a2}





  term1 = {'term': vars(GroupFilterRule('packetDeltaCount', 20000,
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







  term1 = {'term': vars(MergerRule(0, 1, 'sourceIPv4Address', 'RULE_S1_32',
                                         'destinationIPv4Address', 'RULE_S2_32', 0,
                                                    'RULE_EQ',
                                                    'RULE_REL'))}

  term2 = {'term': vars(MergerRule(0, 1, 'destinationIPv4Address', 'RULE_S1_32',
                                         'sourceIPv4Address', 'RULE_S2_32', 0,
                                                    'RULE_EQ',
                                                    'RULE_REL'))}
  clause1 = {'clause': [term1] + [term2]}
  merger = {'dnf-expr': [clause1]}










  query = {'branchset': branchset, 'merger': merger, 'ungrouper': {}}
  fjson = json.dumps(query, indent=2)
  fsock = open('query-tcp-session.json', 'w')
  fsock.write(fjson)
  fsock.close
