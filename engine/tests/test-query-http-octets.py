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
                                      'examples/query-http-octets.json',
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
       - number of filtered records in branch A should be 166
       - number of filtered records in branch B should be 166"""
    num = [166, 166]
    self.iternum(num, self.__class__.pattern['filter'], stdout[0])

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 1
       - number of groups in branch B should be 1"""
    num = [1, 1]
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
       - number of filtered records in branch A should be 7879
       - number of filtered records in branch B should be 8130"""
    num = [7879, 8130]
    self.iternum(num, self.__class__.pattern['filter'], stdout[1])

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 1
       - number of groups in branch B should be 1"""
    num = [1, 1]
    self.iternum(num, self.__class__.pattern['grouper'], stdout[1])

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 0
       - number of group filters in branch B should be 0"""
    num = [0, 0]
    self.iternum(num, self.__class__.pattern['groupfilter'], stdout[1])

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 0"""
    num = [0]
    self.iternum(num, self.__class__.pattern['merger'], stdout[1])

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 0"""
    num = [0]
    self.iternum(num, self.__class__.pattern['ungrouper'][0], stdout[1])

if __name__ == '__main__':
  unittest.main()
