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
                                      'examples/query-mdns-udp.json',
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
       - number of filtered records in branch A should be 0"""
    num = [0]
    self.iternum(num, self.__class__.pattern['filter'], stdout[0])

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 0"""
    num = [0]
    self.iternum(num, self.__class__.pattern['grouper'], stdout[0])

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 0"""
    num = [0]
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
       - number of filtered records in branch A should be 15271"""
    num = [15271]
    self.iternum(num, self.__class__.pattern['filter'], stdout[1])

  def testGroupers(self):
    """expected number of groups in each branch
       - number of groups in branch A should be 156"""
    num = [156]
    self.iternum(num, self.__class__.pattern['grouper'], stdout[1])

  def testGroupFilter(self):
    """expected number of group filters in each branch
       - number of group filters in branch A should be 37"""
    num = [37]
    self.iternum(num, self.__class__.pattern['groupfilter'], stdout[1])

  def testMergers(self):
    """expected number of merged groups
       - number of merged groups should be 37"""
    num = [37]
    self.iternum(num, self.__class__.pattern['merger'], stdout[1])

  def testUngroupers(self):
    """expected number of streams and records in each stream
       - number of streams should be 37
       - number of records in stream 1 should be 245
       - number of records in stream 2 should be 338
       - number of records in stream 3 should be 427
       - number of records in stream 4 should be 420
       - number of records in stream 5 should be 408
       - number of records in stream 6 should be 421
       - number of records in stream 7 should be 314
       - number of records in stream 8 should be 381
       - number of records in stream 9 should be 365
       - number of records in stream 10 should be 304
       - number of records in stream 11 should be 322
       - number of records in stream 12 should be 308
       - number of records in stream 13 should be 344
       - number of records in stream 14 should be 135
       - number of records in stream 15 should be 252
       - number of records in stream 16 should be 356
       - number of records in stream 17 should be 434
       - number of records in stream 18 should be 304
       - number of records in stream 19 should be 257
       - number of records in stream 20 should be 173
       - number of records in stream 21 should be 269
       - number of records in stream 22 should be 258
       - number of records in stream 23 should be 159
       - number of records in stream 24 should be 286
       - number of records in stream 25 should be 156
       - number of records in stream 26 should be 215
       - number of records in stream 27 should be 299
       - number of records in stream 28 should be 270
       - number of records in stream 29 should be 398
       - number of records in stream 30 should be 217
       - number of records in stream 31 should be 316
       - number of records in stream 32 should be 50
       - number of records in stream 33 should be 299
       - number of records in stream 34 should be 64
       - number of records in stream 35 should be 274
       - number of records in stream 36 should be 27
       - number of records in stream 37 should be 285"""
    num = [37]
    self.iternum(num, self.__class__.pattern['ungrouper'][0], stdout[1])

    num = [
            245, 338, 427, 420, 408, 421, 314,
            381, 365, 304, 322, 308, 344, 135,
            252, 356, 434, 304, 257, 173, 269,
            258, 159, 286, 156, 215, 299, 270,
            398, 217, 316,  50, 299,  64, 274,
             27, 285
          ]
    self.iternum(num, self.__class__.pattern['ungrouper'][1], stdout[1])

if __name__ == '__main__':
 unittest.main()
