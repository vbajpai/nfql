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
                     subprocess.check_output(
                                     [
                                      'bin/nfql',
                                      'examples/query-https-tcp-session.json',
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
       - number of filtered records in branch A should be 0
       - number of filtered records in branch B should be 0"""
    num = [0, 0]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 0
       - number of groups in branch B should be 0"""
    num = [0, 0]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 0
       - number of group filters in branch B should be 0"""
    num = [0, 0]
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
       - number of filtered records in branch A should be 10549
       - number of filtered records in branch B should be 10311"""
    num = [10549, 10311]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 346
       - number of groups in branch B should be 343"""
    num = [346, 343]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 96
       - number of group filters in branch B should be 89"""
    num = [96, 89]
    self.iternum(
                 num,
                 self.__class__.pattern['groupfilter'],
                 stdout[self.tracename]
                )

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 88"""
    num = [88]
    self.iternum(
                 num,
                 self.__class__.pattern['merger'],
                 stdout[self.tracename]
                )

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 88
       - number of records in stream 1 should be 66
       - number of records in stream 2 should be 420
       - number of records in stream 3 should be 48
       - number of records in stream 4 should be 8955
       - number of records in stream 5 should be 133
       - number of records in stream 6 should be 86
       - number of records in stream 7 should be 181
       - number of records in stream 8 should be 42
       - number of records in stream 9 should be 1579
       - number of records in stream 10 should be 774
       - number of records in stream 11 should be 105
       - number of records in stream 12 should be 142
       - number of records in stream 13 should be 62
       - number of records in stream 14 should be 64
       - number of records in stream 15 should be 169
       - number of records in stream 16 should be 248
       - number of records in stream 17 should be 147
       - number of records in stream 18 should be 32
       - number of records in stream 19 should be 68
       - number of records in stream 20 should be 32
       - number of records in stream 21 should be 144
       - number of records in stream 22 should be 119
       - number of records in stream 23 should be 4
       - number of records in stream 24 should be 3
       - number of records in stream 25 should be 70
       - number of records in stream 26 should be 3
       - number of records in stream 27 should be 42
       - number of records in stream 28 should be 576
       - number of records in stream 29 should be 2
       - number of records in stream 30 should be 2
       - number of records in stream 31 should be 121
       - number of records in stream 32 should be 50
       - number of records in stream 33 should be 18
       - number of records in stream 34 should be 24
       - number of records in stream 35 should be 72
       - number of records in stream 36 should be 28
       - number of records in stream 37 should be 22
       - number of records in stream 38 should be 26
       - number of records in stream 39 should be 20
       - number of records in stream 40 should be 37
       - number of records in stream 41 should be 38
       - number of records in stream 42 should be 30
       - number of records in stream 43 should be 56
       - number of records in stream 44 should be 22
       - number of records in stream 45 should be 24
       - number of records in stream 46 should be 21
       - number of records in stream 47 should be 36
       - number of records in stream 48 should be 2
       - number of records in stream 49 should be 6
       - number of records in stream 50 should be 38
       - number of records in stream 51 should be 90
       - number of records in stream 52 should be 32
       - number of records in stream 53 should be 3
       - number of records in stream 54 should be 4
       - number of records in stream 55 should be 3
       - number of records in stream 56 should be 82
       - number of records in stream 57 should be 14
       - number of records in stream 58 should be 16
       - number of records in stream 59 should be 2
       - number of records in stream 60 should be 20
       - number of records in stream 61 should be 78
       - number of records in stream 62 should be 132
       - number of records in stream 63 should be 194
       - number of records in stream 64 should be 210
       - number of records in stream 65 should be 98
       - number of records in stream 66 should be 348
       - number of records in stream 67 should be 139
       - number of records in stream 68 should be 130
       - number of records in stream 69 should be 80
       - number of records in stream 70 should be 48
       - number of records in stream 71 should be 98
       - number of records in stream 72 should be 377
       - number of records in stream 73 should be 121
       - number of records in stream 74 should be 194
       - number of records in stream 75 should be 210
       - number of records in stream 76 should be 23
       - number of records in stream 77 should be 38
       - number of records in stream 78 should be 30
       - number of records in stream 79 should be 53
       - number of records in stream 80 should be 54
       - number of records in stream 81 should be 48
       - number of records in stream 82 should be 62
       - number of records in stream 83 should be 64
       - number of records in stream 84 should be 432
       - number of records in stream 85 should be 6
       - number of records in stream 86 should be 22
       - number of records in stream 87 should be 101
       - number of records in stream 88 should be 50"""
    num = [88]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][0],
                 stdout[self.tracename]
                )

    num = [
            66,  420,  48,  8955,  133,  86,  181,
            42,  1579,  774,  105,  142,  62,  64,
            169,  248,  147,  32,  68,  32,  144,
            119,  4,  3,  70,  3,  42,  576,  2,  2,
            121,  50,  18,  24,  72,  28,  22,  26,
            20,  37,  38,  30,  56,  22,  24,  21,
            36,  2,  6,  38,  90,  32,  3,  4,  3,
            82,  14,  16,  2,  20,  78,  132,  194,
            210,  98,  348,  139,  130,  80,  48,
            98,  377,  121,  194,  210,  23,  38,
            30,  53,  54,  48,  62,  64,  432,  6,
            22,  101,  50
          ]
    self.iternum(
                 num,
                 self.__class__.pattern['ungrouper'][1],
                 stdout[self.tracename]
                )

if __name__ == '__main__':
 unittest.main()
