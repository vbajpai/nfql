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
       - number of filtered records in branch A should be 8130
       - number of filtered records in branch B should be 7879"""
    num = [8130, 7879]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 999
       - number of groups in branch B should be 1026"""
    num = [999, 1026]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 109
       - number of group filters in branch B should be 121"""
    num = [109, 121]
    self.iternum(
                 num,
                 self.__class__.pattern['groupfilter'],
                 stdout[self.tracename]
                )

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 105"""
    num = [105]
    self.iternum(
                 num,
                 self.__class__.pattern['merger'],
                 stdout[self.tracename]
                )

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 105
       - number of records in stream 1 should be 63
       - number of records in stream 2 should be 114
       - number of records in stream 3 should be 92
       - number of records in stream 4 should be 88
       - number of records in stream 5 should be 215
       - number of records in stream 6 should be 23
       - number of records in stream 7 should be 24
       - number of records in stream 8 should be 28
       - number of records in stream 9 should be 2
       - number of records in stream 10 should be 6
       - number of records in stream 11 should be 52
       - number of records in stream 12 should be 28
       - number of records in stream 13 should be 12
       - number of records in stream 14 should be 139
       - number of records in stream 15 should be 89
       - number of records in stream 16 should be 92
       - number of records in stream 17 should be 52
       - number of records in stream 18 should be 58
       - number of records in stream 19 should be 24
       - number of records in stream 20 should be 50
       - number of records in stream 21 should be 94
       - number of records in stream 22 should be 238
       - number of records in stream 23 should be 76
       - number of records in stream 24 should be 4
       - number of records in stream 25 should be 22
       - number of records in stream 26 should be 2
       - number of records in stream 27 should be 4
       - number of records in stream 28 should be 70
       - number of records in stream 29 should be 2
       - number of records in stream 30 should be 66
       - number of records in stream 31 should be 27
       - number of records in stream 32 should be 4
       - number of records in stream 33 should be 121
       - number of records in stream 34 should be 256
       - number of records in stream 35 should be 271
       - number of records in stream 36 should be 45
       - number of records in stream 37 should be 170
       - number of records in stream 38 should be 2
       - number of records in stream 39 should be 2
       - number of records in stream 40 should be 79
       - number of records in stream 41 should be 156
       - number of records in stream 42 should be 166
       - number of records in stream 43 should be 3
       - number of records in stream 44 should be 76
       - number of records in stream 45 should be 4
       - number of records in stream 46 should be 4
       - number of records in stream 47 should be 110
       - number of records in stream 48 should be 87
       - number of records in stream 49 should be 33
       - number of records in stream 50 should be 57
       - number of records in stream 51 should be 42
       - number of records in stream 52 should be 4
       - number of records in stream 53 should be 2
       - number of records in stream 54 should be 2
       - number of records in stream 55 should be 2
       - number of records in stream 56 should be 10
       - number of records in stream 57 should be 10
       - number of records in stream 58 should be 5
       - number of records in stream 59 should be 4
       - number of records in stream 60 should be 12
       - number of records in stream 61 should be 4
       - number of records in stream 62 should be 6
       - number of records in stream 63 should be 12
       - number of records in stream 64 should be 2
       - number of records in stream 65 should be 99
       - number of records in stream 66 should be 60
       - number of records in stream 67 should be 282
       - number of records in stream 68 should be 470
       - number of records in stream 69 should be 200
       - number of records in stream 70 should be 112
       - number of records in stream 71 should be 40
       - number of records in stream 72 should be 56
       - number of records in stream 73 should be 60
       - number of records in stream 74 should be 119
       - number of records in stream 75 should be 351
       - number of records in stream 76 should be 102
       - number of records in stream 77 should be 28
       - number of records in stream 78 should be 68
       - number of records in stream 79 should be 287
       - number of records in stream 80 should be 94
       - number of records in stream 81 should be 56
       - number of records in stream 82 should be 46
       - number of records in stream 83 should be 317
       - number of records in stream 84 should be 118
       - number of records in stream 85 should be 2
       - number of records in stream 86 should be 2
       - number of records in stream 87 should be 18
       - number of records in stream 88 should be 12
       - number of records in stream 89 should be 36
       - number of records in stream 90 should be 38
       - number of records in stream 91 should be 18
       - number of records in stream 92 should be 60
       - number of records in stream 93 should be 98
       - number of records in stream 94 should be 106
       - number of records in stream 95 should be 113
       - number of records in stream 96 should be 51
       - number of records in stream 97 should be 154
       - number of records in stream 98 should be 98
       - number of records in stream 99 should be 186
       - number of records in stream 100 should be 161
       - number of records in stream 101 should be 36
       - number of records in stream 102 should be 56
       - number of records in stream 103 should be 208
       - number of records in stream 104 should be 104
       - number of records in stream 105 should be 256"""
    num = [105]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][0],
                 stdout[self.tracename]
                )

    num = [63,  114,  92,  88,  215,  23,  24,
           28,  2,  6,  52,  28,  12,  139,  89,
           92,  52,  58,  24,  50,  94,  238,
           76,  4,  22,  2,  4,  70,  2,  66,
           27,  4,  121,  256,  271,  45,  170,
           2,  2,  79,  156,  166,  3,  76,  4,
           4,  110,  87,  33,  57,  42,  4,  2,
           2,  2,  10,  10,  5,  4,  12,  4,  6,
           12,  2,  99,  60,  282,  470,  200,
           112,  40,  56,  60,  119,  351,  102,
           28,  68,  287,  94,  56,  46,  317,
           118,  2,  2,  18,  12,  36,  38,  18,
           60,  98,  106,  113,  51,  154,  98,
           186,  161,  36,  56,  208,  104,  256
          ]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][1],
                 stdout[self.tracename]
                )

if __name__ == '__main__':
  unittest.main()
