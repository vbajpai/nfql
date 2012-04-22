import json

rule_map = {
  
  'RULE_S2_8' 	: 0,
  'RULE_S2_16' : 1,
  'RULE_S2_32' : 2,
  'RULE_S2_64' : 3,
  
  'RULE_S1_8' : 0,
  'RULE_S1_16' : 4,
  'RULE_S1_32' : 8,
  'RULE_S1_64' : 12,
  
  
  'RULE_ABS' : 0,
  'RULE_REL' : 16,
  'RULE_NO' : 32,
  
  
  'RULE_EQ' : 0,
  'RULE_NE' : 64,
  'RULE_GT' : 128,
  'RULE_LT' : 192,
  'RULE_LE' : 256,
  'RULE_GE' : 320,
  
  
  'RULE_STATIC' : 512,
  'RULE_COUNT' : 1024,
  'RULE_UNION' : 2048,  
  'RULE_MIN' : 4096,
  'RULE_MAX' : 8192,  
  'RULE_MEDIAN' : 16384, 
  'RULE_MEAN' : 32768,
  'RULE_STDDEV' : 65536,    
  'RULE_XOR' : 131072,
  'RULE_SUM' : 262144,
  'RULE_PROD' : 524288,  
  'RULE_AND' : 1048576,
  'RULE_OR' : 2097152,  
  
  
  'RULE_IN' : 4194304,
  'RULE_ALLEN_BF' : 8388608,
  'RULE_ALLEN_AF' : 16777216,
  'RULE_ALLEN_M' : 33554432,
  'RULE_ALLEN_MI' : 67108864,
  'RULE_ALLEN_O' : 134217728,
  'RULE_ALLEN_OI' : 268435456,
  'RULE_ALLEN_S' : 536870912,
  'RULE_ALLEN_SI' : 1073741824,
  'RULE_ALLEN_D' : 2147483648,
  'RULE_ALLEN_DI' : 4294967296,
  'RULE_ALLEN_F' : 8589934592,
  'RULE_ALLEN_FI' : 17179869184,
  'RULE_ALLEN_EQ' : 34359738368,  
}


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


if __name__ == '__main__':
  
  fruleset = []
  fruleset.append(vars(FilterRule('dstport', 80, rule_map['RULE_S1_16'], 0, 
                                                 rule_map['RULE_EQ'])))
  filter1 = {'num_rules': len(fruleset), 'ruleset': fruleset}

  fruleset = []
  fruleset.append(vars(FilterRule('srcport', 80, rule_map['RULE_S1_16'], 0, 
                                                 rule_map['RULE_EQ'])))    
  filter2 = {'num_rules': len(fruleset), 'ruleset': fruleset}
  
  gruleset = []
  gruleset.append(vars(GrouperRule('srcaddr', rule_map['RULE_S1_32'], 
                                   'srcaddr', rule_map['RULE_S2_32'], 0,  
                                    rule_map['RULE_EQ'], rule_map['RULE_ABS'])))
  gruleset.append(vars(GrouperRule('dstaddr', rule_map['RULE_S1_32'], 
                                   'dstaddr', rule_map['RULE_S2_32'], 0,  
                                   rule_map['RULE_EQ'], rule_map['RULE_ABS'])))
  grouper1 = {'num_rules': len(gruleset), 'ruleset': gruleset} 
  
  
  gruleset = []
  gruleset.append(vars(GrouperRule('srcaddr', rule_map['RULE_S1_32'], 
                                   'srcaddr', rule_map['RULE_S2_32'], 0,  
                                   rule_map['RULE_EQ'], rule_map['RULE_ABS'])))
  gruleset.append(vars(GrouperRule('dstaddr', rule_map['RULE_S1_32'], 
                                   'dstaddr', rule_map['RULE_S2_32'], 0,  
                                   rule_map['RULE_EQ'], rule_map['RULE_ABS'])))
  grouper2 = {'num_rules': len(gruleset), 'ruleset': gruleset}
  
  
  aruleset = []
  aruleset.append(vars(AggregationRule('srcaddr', rule_map['RULE_S1_32'], 
                                                  rule_map['RULE_STATIC'])))
  aruleset.append(vars(AggregationRule('dstaddr', rule_map['RULE_S1_32'], 
                                                  rule_map['RULE_STATIC'])))
  aruleset.append(vars(AggregationRule('dPkts', rule_map['RULE_S1_32'], 
                                                rule_map['RULE_SUM'])))
  aruleset.append(vars(AggregationRule('dOctets', rule_map['RULE_S1_32'], 
                                                  rule_map['RULE_SUM'])))
  a1 = {'num_rules' : len(aruleset), 'ruleset' : aruleset}

  aruleset = []
  aruleset.append(vars(AggregationRule('srcaddr', rule_map['RULE_S1_32'], 
                                                  rule_map['RULE_STATIC'])))
  aruleset.append(vars(AggregationRule('dstaddr', rule_map['RULE_S1_32'], 
                                                  rule_map['RULE_STATIC'])))
  aruleset.append(vars(AggregationRule('dPkts', rule_map['RULE_S1_32'], 
                                                rule_map['RULE_SUM'])))
  aruleset.append(vars(AggregationRule('dOctets', rule_map['RULE_S1_32'], 
                                                  rule_map['RULE_SUM'])))
  a2 = {'num_rules' : len(aruleset), 'ruleset' : aruleset}
  
  gfruleset = []
  gfruleset.append(vars(GroupFilterRule('dPkts', 200, 
                                        rule_map['RULE_S1_32'], 0,
                                        rule_map['RULE_GT'])))
  gfilter1 = {'num_rules' : len(gfruleset), 'ruleset' : gfruleset}
  
  gfruleset = []
  gfruleset.append(vars(GroupFilterRule('dPkts', 200, 
                                        rule_map['RULE_S1_32'], 0,
                                        rule_map['RULE_GT'])))
  gfilter2 = {'num_rules' : len(gfruleset), 'ruleset' : gfruleset}
  
  branchset = []
  branchset.append({'filter': filter1, 
                    'grouper': grouper1, 
                    'aggregation': a1,
                    'gfilter': gfilter1,
                   })
  
  branchset.append({'filter': filter2, 
                    'grouper': grouper2, 
                    'aggregation' : a2,
                    'gfilter': gfilter2,
                   })  
  
  mruleset = []
  mruleset.append(vars(MergerRule(0, 1, 'srcaddr', rule_map['RULE_S1_32'],
                                    'dstaddr', rule_map['RULE_S2_32'], 0, 
                                               rule_map['RULE_EQ'], 
                                               rule_map['RULE_ABS'])))
  
  mruleset.append(vars(MergerRule(0, 1, 'dstaddr', rule_map['RULE_S1_32'],
                                    'srcaddr', rule_map['RULE_S2_32'], 0, 
                                               rule_map['RULE_EQ'], 
                                               rule_map['RULE_ABS'])))
  
  merger = {'num_rules' : len(mruleset), 'ruleset' : mruleset}
  
  query = {'num_branches': len(branchset), 
           'branchset': branchset,
           'merger': merger}  
  
  fjson = json.dumps(query, indent=2)
  fsock = open('query.json', 'w')
  fsock.write(fjson)
  fsock.close