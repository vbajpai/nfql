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

  fruleset = []
  fruleset.append(vars(FilterRule('prot', protocol('TCP'), 'RULE_S1_16', 0,
                                                           'RULE_EQ')))
  filter1 = {'ruleset': fruleset}

  gruleset = []
  grouper1 = {'ruleset': gruleset}

  aruleset = []
  a1 = {'ruleset' : aruleset}

  gfruleset = []
  gfilter1 = {'ruleset' : gfruleset}

  branchset = []
  branchset.append({'filter': filter1,
                    'grouper': grouper1,
                    'aggregation': a1,
                    'gfilter': gfilter1,
                   })

  mruleset = []

  merger = {'ruleset' : mruleset}
  query = {'branchset': branchset, 'merger': merger}

  fjson = json.dumps(query, indent=2)
  filename = os.path.splitext(sys.argv[0])[0]
  fsock = open('%s.json'%filename, 'w')
  fsock.write(fjson)
  fsock.close
