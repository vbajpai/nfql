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

import sys, os, time, subprocess

def do_silk(trace_list, query_list):
  """runs silk on the given tracelist"""
  resdir = '%s/benchmarks/results/silk'%(os.getcwd())
  st = os.system('mkdir -p %s'%(resdir))
  output_trace = '/tmp/result.rwz'
  for query in query_list:
    basequery = os.path.splitext(os.path.basename(query))[0]
    for trace in trace_list:
      querycmd = open(query, 'r').read().replace('$INPUT', trace)
      querycmd = querycmd.replace('$OUTPUT', output_trace)
      basetrace = os.path.splitext(os.path.basename(trace))[0]
      resfile = '%s/silk-%s-%s.results'%(resdir, basequery, basetrace)
      print 'executing: [silk %s %s]: '%(basequery, basetrace),
      for iter in range(1, 11):
        # clear pagecache, dentries and inodes
        os.system('sync')
        try: 
          with open('/proc/sys/vm/drop_caches', 'w') as stream:
            stream.write('3\n')
        except IOError as e: 
          print '\nrun the script with sudo privileges'
          exit(1)
        # flush the stdout buffer
        sys.stdout.flush()
        print iter,
        # time the query execution
        try:
          start = time.time()
          st = os.system(querycmd)
          elapsed = time.time() - start
        except OSError as e: print e
        else:
          try:
            wsock = open(resfile, 'a')
            wsock.write('%f\n'%(elapsed))
            wsock.close()
          except: print 'cannot write to file'
      # calculate i/o ration
      try: input_flows = int(
                              os.popen(
                                        'rwstats --overall-stat %s | \
                                         grep "records" | \
                                         cut -d " " -f5'%(trace)
                                      ).read()
                            )
      except OSError as e: ratio = 0
      try: output_flows = int(
                              os.popen(
                                        'rwstats --overall-stat %s | \
                                         grep "records" | \
                                         cut -d " " -f5'%(output_trace)
                                      ).read()
                            )
      except OSError as e: ratio = 0
      else: ratio = float(output_flows)/input_flows

      # calculate timing avg
      try:
        fsock = open(resfile, 'r')
      except: print 'cannot open results file'
      else:
        try:
          floats = map(float, fsock.readlines())
          avgtime = reduce(lambda x,y: x+y, floats)/len(floats)
        except: avgtime = 0
        else:
          summaryfile = '%s/silk-summary.results'%(resdir)
          try:
            wsock = open(summaryfile, 'a')
            wsock.write('silk-%s-%s:%d output:%d ratio:%f avgtime:%f\n'
                                                               %(basequery,
                                                                 basetrace,
                                                                 input_flows,
                                                                 output_flows,
                                                                 ratio,
                                                                 avgtime))
          except IOError: pass
        finally:
          print '(#input:%d #output:%d #output/input:%f time:%f)'%(input_flows, 
                                                                   output_flows,
                                                                   ratio,
                                                                   avgtime)

def main(arg):
  """parses argument list and calls do_silk(...)"""

  def listdir(directory):
    """given a dir path, returns a list of absolute file paths"""
    filelist = os.listdir(directory)
    absfilelist = [os.path.abspath(os.path.join(directory, f)) 
                   for f in filelist]
    return absfilelist

  trace_dir = os.path.abspath(arg[0])
  query_dir = os.path.abspath(arg[1])
  trace_list = listdir(trace_dir)
  query_list = listdir(query_dir)
  print "benchmarking silk ..."
  do_silk(trace_list, query_list)

if __name__ == '__main__':
  """checks argument list for sanity and calls main(...)"""
  if len(sys.argv) != 3:
    print """usage: %s trace[s]/ querie[s]/"""%(sys.argv[0])
    exit(1)
  else:
    main(sys.argv[1:])
