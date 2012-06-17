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
