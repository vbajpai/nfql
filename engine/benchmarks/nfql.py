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

import sys, os, time

def do_nfql(nfql, trace_list, query_list):
  """runs nfql engine on the given tracelist and querylist"""
  resdir = '%s/benchmarks/results/nfql'%(os.getcwd())
  st = os.system('mkdir -p %s'%(resdir))
  for query in query_list:
    basequery = os.path.splitext(os.path.basename(query))[0]
    for trace in trace_list:
      basetrace = os.path.splitext(os.path.basename(trace))[0]
      print 'executing: [%s %s %s]: '%(nfql, basequery, basetrace),
      for iter in range(1, 11):
        sys.stdout.flush()
        print iter,
        try:
          resfile = '%s/nfql-%s-%s.results'%(resdir, basequery, basetrace)
          start = time.time()
          stmt = '%s %s %s > /dev/null'%(nfql, query, trace)
          st = os.system(stmt)
          elapsed = time.time() - start
        except OSError as e:
          print e
        else:
          try:
            wsock = open(resfile, 'a')
            wsock.write('%f\n'%(elapsed))
            wsock.close()
          except: print 'cannot write to file'
      try:
        fsock = open(resfile, 'r')
      except: print 'cannot open results file'
      else:
        try:
          floats = map(float, fsock.readlines())
          avgtime = reduce(lambda x,y: x+y, floats)/len(floats)
        except: avgtime = 0
        else:
          summaryfile = '%s/nfql-summary.results'%(resdir)
          try:
            wsock = open(summaryfile, 'a')
            wsock.write('nfql-%s-%s: avg=%f s\n'%(basequery,
                                                  basetrace,
                                                  avgtime))
          except IOError: pass
        finally: print '(%f secs)'%(avgtime)

def main(arg):
  """parses argument list and calls do_nfql(...)"""

  def listdir(directory):
    """given a dir path, returns a list of absolute file paths"""
    filelist = os.listdir(directory)
    absfilelist = [os.path.abspath(os.path.join(directory, f))
                   for f in filelist]
    return absfilelist

  nfql = os.path.abspath(arg[0])
  trace_dir = os.path.abspath(arg[1])
  query_dir = os.path.abspath(arg[2])

  trace_list = listdir(trace_dir)
  query_list = listdir(query_dir)
  print "benchmarking nfql ..."
  do_nfql(nfql, trace_list, query_list)

if __name__ == '__main__':
  """checks argument list for sanity and calls main(...)"""
  if len(sys.argv) != 4:
    print """usage: %s bin/engine trace[s]/ querie[s]/"""%(sys.argv[0])
    exit(1)
  else:
    main(sys.argv[1:])
