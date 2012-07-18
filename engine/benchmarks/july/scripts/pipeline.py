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


import json
import socket

def protocol(name):
  return socket.getprotobyname(name)

class FilterRule:

  def __init__(self, name, value, datatype, delta, op):

    self.offset = {
      'name': name,
      'value': value,
      'datatype': datatype
    }

    self.delta = delta
    self.op = op

class GrouperRule:

  def __init__(self, field1_name, field1_type, field2_name, field2_type,
               delta, op_name, op_type):

    self.offset = {
      'f1_name': field1_name,
      'f1_datatype': field1_type,
      'f2_name': field2_name,
      'f2_datatype': field2_type
    }

    self.delta = delta

    self.op = {
      'name' : op_name,
      'type' : op_type
    }

class AggregationRule:

  def __init__(self, name, datatype, op):

    self.offset = {
      'name': name,
      'datatype': datatype
    }

    self.op = op

class GroupFilterRule:

  def __init__(self, name, value, datatype, delta, op):

    self.offset = {
      'name': name,
      'value': value,
      'datatype': datatype
    }

    self.delta = delta
    self.op = op

class MergerRule:

  def __init__(self, branch1_id, branch2_id,
                     field1_name, field1_type,
                     field2_name, field2_type, delta,
                     op_name, op_type):

    self.offset = {
      'f1_name': field1_name,
      'f1_datatype': field1_type,
      'f2_name': field2_name,
      'f2_datatype': field2_type,
    }

    self.op = {
      'name' : op_name,
      'type' : op_type,
    }

    self.branch1_id = branch1_id;
    self.branch2_id = branch2_id;
    self.delta = delta
