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
                                            'examples/query-http-octets.json',
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
       - number of filtered records should be 332"""
    num = [332]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups should be 1"""
    num = [1]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters should be 0"""
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
       - number of filtered records should be 16009"""
    num = [16009]
    self.iternum(
                 num,
                 self.__class__.pattern['filter'],
                 stdout[self.tracename]
                )

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups should be 1"""
    num = [1]
    self.iternum(
                 num,
                 self.__class__.pattern['grouper'],
                 stdout[self.tracename]
                )

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters should be 0"""
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

if __name__ == '__main__':
  unittest.main()
