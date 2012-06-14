import unittest, subprocess, re, glob, sys
from trace import Trace

traces = glob.glob('examples/*.ft')
stdout = []
for trace in traces:
  try:
    stdout.append(\
             subprocess.check_output(
                                     [
                                      'bin/engine',
                                      'examples/query-https-tcp-session.json',
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
  def testFilters(self):
    """expected number of filtered records in each branch
       - number of filtered records in branch A should be 0
       - number of filtered records in branch B should be 0"""
    num = [0, 0]
    self.iternum(num, self.__class__.pattern['filter'], stdout[0])

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 0
       - number of groups in branch B should be 0"""
    num = [0, 0]
    self.iternum(num, self.__class__.pattern['grouper'], stdout[0])

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 0
       - number of group filters in branch B should be 0"""
    num = [0, 0]
    self.iternum(num, self.__class__.pattern['groupfilter'], stdout[0])

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 0"""
    num = [0]
    self.iternum(num, self.__class__.pattern['merger'], stdout[0])

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 0"""
    num = [0]
    self.iternum(num, self.__class__.pattern['ungrouper'][0], stdout[0])


class Trace2012(Trace, unittest.TestCase):

  def testFilters(self):
    """expected number of filtered records in each branch
       - number of filtered records in branch A should be 10536
       - number of filtered records in branch B should be 5944"""
    num = [10536, 5944]
    self.iternum(num, self.__class__.pattern['filter'], stdout[1])

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 343
       - number of groups in branch B should be 343"""
    num = [343, 343]
    self.iternum(num, self.__class__.pattern['grouper'], stdout[1])

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 96
       - number of group filters in branch B should be 31"""
    num = [96, 31]
    self.iternum(num, self.__class__.pattern['groupfilter'], stdout[1])

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 31"""
    num = [31]
    self.iternum(num, self.__class__.pattern['merger'], stdout[1])

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 31
       - number of records in stream 1 should be 20
       - number of records in stream 2 should be 420
       - number of records in stream 3 should be 48
       - number of records in stream 4 should be 50
       - number of records in stream 5 should be 8955
       - number of records in stream 6 should be 121
       - number of records in stream 7 should be 101
       - number of records in stream 8 should be 18
       - number of records in stream 9 should be 248
       - number of records in stream 10 should be 169
       - number of records in stream 11 should be 147
       - number of records in stream 12 should be 72
       - number of records in stream 13 should be 66
       - number of records in stream 14 should be 24
       - number of records in stream 15 should be 32
       - number of records in stream 16 should be 144
       - number of records in stream 17 should be 37
       - number of records in stream 18 should be 32
       - number of records in stream 19 should be 62
       - number of records in stream 20 should be 26
       - number of records in stream 21 should be 64
       - number of records in stream 22 should be 22
       - number of records in stream 23 should be 14
       - number of records in stream 24 should be 16
       - number of records in stream 25 should be 28
       - number of records in stream 26 should be 30
       - number of records in stream 27 should be 22
       - number of records in stream 28 should be 56
       - number of records in stream 29 should be 23
       - number of records in stream 30 should be 68
       - number of records in stream 31 should be 38"""
    num = [31]
    self.iternum(num, self.__class__.pattern['ungrouper'][0], stdout[1])

    num = [
            20, 420,  48,  50, 8955, 121,  101,
            18, 248, 169, 147,   72,  66,   24,
            32, 144,  37,  32,   62,  26,   64,
            22,  14,  16,  28,   30,  22,   56,
            23,  68,  38
          ]

    self.iternum(num, self.__class__.pattern['ungrouper'][1], stdout[1])

if __name__ == '__main__':
 unittest.main()
