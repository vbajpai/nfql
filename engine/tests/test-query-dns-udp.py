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
                                      'examples/query-dns-udp.json',
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
       - number of filtered records in branch A should be 95
       - number of filtered records in branch B should be 97"""
    num = [95, 97]
    self.iternum(num, self.__class__.pattern['filter'], stdout[0])

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 1
       - number of groups in branch B should be 1"""
    num = [1, 1]
    self.iternum(num, self.__class__.pattern['grouper'], stdout[0])

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 1
       - number of group filters in branch B should be 1"""
    num = [1, 1]
    self.iternum(num, self.__class__.pattern['groupfilter'], stdout[0])

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 1"""
    num = [1]
    self.iternum(num, self.__class__.pattern['merger'], stdout[0])

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 1
       - number of records in stream 1 should be 192"""
    num = [1]
    self.iternum(num, self.__class__.pattern['ungrouper'][0], stdout[0])

    num = [192]
    self.iternum(num, self.__class__.pattern['ungrouper'][1], stdout[0])


class Trace2012(Trace, unittest.TestCase):

  def testFilters(self):
    """expected number of filtered records in each branch
       - number of filtered records in branch A should be 17452
       - number of filtered records in branch B should be 16820"""
    num = [17452, 16820]
    self.iternum(num, self.__class__.pattern['filter'], stdout[1])

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 5
       - number of groups in branch B should be 5"""
    num = [5, 5]
    self.iternum(num, self.__class__.pattern['grouper'], stdout[1])

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 1
       - number of group filters in branch B should be 2"""
    num = [1, 2]
    self.iternum(num, self.__class__.pattern['groupfilter'], stdout[1])

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 1"""
    num = [1]
    self.iternum(num, self.__class__.pattern['merger'], stdout[1])

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 1
       - number of records in stream 1 should be 34267"""
    num = [1]
    self.iternum(num, self.__class__.pattern['ungrouper'][0], stdout[1])

    num = [34267]
    self.iternum(num, self.__class__.pattern['ungrouper'][1], stdout[1])

if __name__ == '__main__':
 unittest.main()
