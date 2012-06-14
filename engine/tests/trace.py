import re

class Trace:

  pattern = {
              'filter' : 'No. of Filtered Records: (\d+).*',
              'grouper' : 'No. of Groups: (\d+).*',
              'groupfilter' : 'No. of Filtered Groups: (\d+).*',
              'merger' : 'No. of Merged Groups: (\d+).*',
              'ungrouper' : (
                              'No. of Streams: (\d+).*',
                              'No. of Records in Stream.+: (\d+).*'
                            )
            }

  def shortDescription(self):
    return self._testMethodDoc

  def iternum(self, num, pattern, out):
    for index, item in enumerate(num):
      lines = re.findall(pattern, out)
      num_recs = int(lines[index])
      self.assertEqual(num_recs, item)
