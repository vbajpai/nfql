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
  time_avg = 0
  for query in query_list:
    for trace in trace_list:
      print 'executing: %s %s %s'%(os.path.basename(nfql),
                                   os.path.basename(query),
                                   os.path.basename(trace))
      for iter in range(1, 11):
        try:
          avg = 0
          stmt = '%s %s %s'%(nfql, query, trace)
          start = time.time()
          result = subprocess.Popen(
                                    stmt,
                                    shell=True,
                                    stdout=open(os.devnull, 'w')
                                   )
          elapsed = time.time() - start
        except subprocess.CalledProcessError as e:
          print e.output
        except OSError as e:
          print e
        else:
          #print '%s: %s' %(iter, elapsed)
          avg += elapsed
      avg /= 10
      print 'avg: %s' %(avg)

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
    print 'usage: %s engine traces/ query/ '%(sys.argv[0])
    exit(1)
  else:
    main(sys.argv[1:])
