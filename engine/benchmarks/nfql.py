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

import sys, os, subprocess, time

def do_nfql(nfql, trace_list, query_list):
  """docstring for do_nfql"""
  st = os.system('mkdir -p results/')
  for query in query_list:
    basequery = os.path.splitext(os.path.basename(query))[0]
    for trace in trace_list:
      basetrace = os.path.splitext(os.path.basename(trace))[0]
      print 'executing: %s %s %s'%(nfql, basequery, basetrace),
      for iter in range(1, 11):
        print iter,
        try:
          time = '/usr/bin/time -f "%e" --append -o '
          time += 'results/nfql-%s-%s.results'%(basequery, basetrace)
          stmt = '%s %s %s %s > /dev/null'%(time, nfql, query, trace)
          st = os.system(stmt)
        except OSError as e:
          print e
      print

def listdir(directory):
  """docstring for list_dir"""
  filelist = os.listdir(directory)
  absfilelist = [os.path.abspath(os.path.join(directory, f)) for f in filelist]
  return absfilelist

def main(arg):
  """docstring for main"""
  nfql = os.path.abspath(arg[0])
  trace_dir = arg[1]
  query_dir = arg[2]

  trace_list = listdir(trace_dir)
  query_list = listdir(query_dir)
  print "benchmarking nfql ..."
  do_nfql(nfql, trace_list, query_list)

if __name__ == '__main__':
  if len(sys.argv) != 4:
    print """usage: %s
       bin/engine
       benchmarks/traces/flow-tools/
       benchmarks/query/ """%(sys.argv[0])
    exit(1)
  else:
    main(sys.argv[1:])
