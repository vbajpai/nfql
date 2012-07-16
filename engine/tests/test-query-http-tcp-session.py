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
                                      'bin/engine',
                                      'examples/query-http-tcp-session.json',
                                      trace,
                                      '--debug'
                                     ]
                     )
                   )
  except subprocess.CalledProcessError as e:
    if e.output.find('segmentation fault'):
      print 'bin/engine returned %s: segmentation fault' %(e.returncode)
    else:
      print e.output
  except OSError as e:
    print e
    sys.exit(1)

class Trace2009(Trace, unittest.TestCase):

  tracename = 'trace-2009.ftz'

  def testFilters(self):
    """expected number of filtered records in each branch
       - number of filtered records in branch A should be 166
       - number of filtered records in branch B should be 166"""
    num = [166, 166]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 32
       - number of groups in branch B should be 32"""
    num = [32, 32]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 3
       - number of group filters in branch B should be 5"""
    num = [3, 5]
    self.iternum(
                 num,
                 self.__class__.pattern['groupfilter'],
                 stdout[self.tracename]
                )

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 3"""
    num = [3]
    self.iternum(
                 num,
                 self.__class__.pattern['merger'],
                 stdout[self.tracename]
                )

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 3
       - number of records in stream 1 should be 24
       - number of records in stream 2 should be 62
       - number of records in stream 3 should be 56"""
    num = [3]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][0],
                 stdout[self.tracename]
                )

    num = [24, 62, 56]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][1],
                 stdout[self.tracename]
                )


class Trace2012(Trace, unittest.TestCase):

  tracename = 'trace-2012.ftz'

  def testFilters(self):
    """expected number of filtered records in each branch
       - number of filtered records in branch A should be 8089
       - number of filtered records in branch B should be 2417"""
    num = [8089, 2417]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 1013
       - number of groups in branch B should be 1013"""
    num = [1013, 1013]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 109
       - number of group filters in branch B should be 49"""
    num = [109, 49]
    self.iternum(
                 num,
                 self.__class__.pattern['groupfilter'],
                 stdout[self.tracename]
                )

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 42"""
    num = [42]
    self.iternum(
                 num,
                 self.__class__.pattern['merger'],
                 stdout[self.tracename]
                )

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 42
       - number of records in stream 1 should be 110
       - number of records in stream 2 should be 114
       - number of records in stream 3 should be 208
       - number of records in stream 4 should be 161
       - number of records in stream 5 should be 104
       - number of records in stream 6 should be 154
       - number of records in stream 7 should be 98
       - number of records in stream 8 should be 113
       - number of records in stream 9 should be 6
       - number of records in stream 10 should be 98
       - number of records in stream 11 should be 76
       - number of records in stream 12 should be 186
       - number of records in stream 13 should be 170
       - number of records in stream 14 should be 63
       - number of records in stream 15 should be 215
       - number of records in stream 16 should be 256
       - number of records in stream 17 should be 271
       - number of records in stream 18 should be 121
       - number of records in stream 19 should be 36
       - number of records in stream 20 should be 106
       - number of records in stream 21 should be 4
       - number of records in stream 22 should be 56
       - number of records in stream 23 should be 88
       - number of records in stream 24 should be 92
       - number of records in stream 25 should be 10
       - number of records in stream 26 should be 33
       - number of records in stream 27 should be 51
       - number of records in stream 28 should be 2
       - number of records in stream 29 should be 42
       - number of records in stream 30 should be 4
       - number of records in stream 31 should be 2
       - number of records in stream 32 should be 12
       - number of records in stream 33 should be 2
       - number of records in stream 34 should be 3
       - number of records in stream 35 should be 4
       - number of records in stream 36 should be 4
       - number of records in stream 37 should be 12
       - number of records in stream 38 should be 12
       - number of records in stream 39 should be 10
       - number of records in stream 40 should be 2
       - number of records in stream 41 should be 4
       - number of records in stream 42 should be 5"""
    num = [42]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][0],
                 stdout[self.tracename]
                )

    num = [
           110, 114, 208, 161, 104, 154,  98,
           113,   6,  98,  76, 186, 170,  63,
           215, 256, 271, 121,  36, 106,   4,
            56,  88,  92,  10,  33,  51,   2,
            42,   4,   2,  12,   2,   3,   4,
             4,  12,  12,  10,   2,   4,   5
          ]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][1],
                 stdout[self.tracename]
                )

if __name__ == '__main__':
  unittest.main()
