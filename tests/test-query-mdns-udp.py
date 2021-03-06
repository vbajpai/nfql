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

import unittest, subprocess, re, glob, sys, os.path
from trace import Trace

traces = glob.glob('examples/*.ftz')
stdout = {}
for trace in traces:
  try:
    base = os.path.basename(trace)
    stdout[base] = (
                     subprocess.check_output (
                                     [
                                      'bin/nfql',
                                      'examples/query-mdns-udp.json',
                                      trace,
                                      '--debug'
                                     ]
                     )
                 )
  except subprocess.CalledProcessError as e:
    if e.output.find('segmentation fault'):
      print 'bin/nfql returned %s: segmentation fault' %(e.returncode)
    else:
      print e.output
  except OSError as e:
    print e
    sys.exit(1)

class Trace2009(Trace, unittest.TestCase):

  tracename = 'trace-2009.ftz'

  def testFilters(self):
    """expected number of filtered records in each branch
       - number of filtered records in branch A should be 0"""
    num = [0]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 0"""
    num = [0]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 0"""
    num = [0]
    self.iternum(
                 num,
                 self.__class__.pattern['groupfilter'],
                 stdout[self.tracename]
                )

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 0"""
    num = [0]
    self.iternum(
                 num,
                 self.__class__.pattern['merger'],
                 stdout[self.tracename]
                )

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 0"""
    num = [0]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][0],
                 stdout[self.tracename]
                )

class Trace2012(Trace, unittest.TestCase):

  tracename = 'trace-2012.ftz'

  def testFilters(self):
    """expected number of filtered records in each branch
       - number of filtered records in branch A should be 15386"""
    num = [16286]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 169"""
    num = [169]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 38"""
    num = [38]
    self.iternum(
                 num,
                 self.__class__.pattern['groupfilter'],
                 stdout[self.tracename]
                )

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 38"""
    num = [38]
    self.iternum(
                 num,
                 self.__class__.pattern['merger'],
                 stdout[self.tracename]
                )

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 38
       - number of records in stream 1 should be 304
       - number of records in stream 2 should be 434
       - number of records in stream 3 should be 286
       - number of records in stream 4 should be 274
       - number of records in stream 5 should be 421
       - number of records in stream 6 should be 314
       - number of records in stream 7 should be 365
       - number of records in stream 8 should be 299
       - number of records in stream 9 should be 257
       - number of records in stream 10 should be 27
       - number of records in stream 11 should be 338
       - number of records in stream 12 should be 408
       - number of records in stream 13 should be 173
       - number of records in stream 14 should be 285
       - number of records in stream 15 should be 245
       - number of records in stream 16 should be 64
       - number of records in stream 17 should be 304
       - number of records in stream 18 should be 156
       - number of records in stream 19 should be 381
       - number of records in stream 20 should be 398
       - number of records in stream 21 should be 159
       - number of records in stream 22 should be 344
       - number of records in stream 23 should be 298
       - number of records in stream 24 should be 217
       - number of records in stream 25 should be 258
       - number of records in stream 26 should be 322
       - number of records in stream 27 should be 252
       - number of records in stream 28 should be 299
       - number of records in stream 29 should be 316
       - number of records in stream 30 should be 215
       - number of records in stream 31 should be 427
       - number of records in stream 32 should be 135
       - number of records in stream 33 should be 308
       - number of records in stream 34 should be 356
       - number of records in stream 35 should be 50
       - number of records in stream 36 should be 420
       - number of records in stream 37 should be 270
       - number of records in stream 38 should be 269"""
    num = [38]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][0],
                 stdout[self.tracename]
                )

    num = [
            304,  434,  286,  274,  421,  314,  365,
            299,  257,  27,  338,  408,  173,  285,
            245,  64,  304,  156,  381,  398,  159,
            344,  298,  217,  258,  322,  252,  299,
            316,  215,  427,  135,  308,  356,  50,
            420,  270,  269
          ]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][1],
                 stdout[self.tracename]
                )

if __name__ == '__main__':
 unittest.main()
