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

if __name__ == '__main__':
  
  frule1 = FilterRule('dstport', 80, rule_map['RULE_S1_16'], 0, rule_map['RULE_EQ'])
  frule2 = FilterRule('srcport', 80, rule_map['RULE_S1_16'], 0, rule_map['RULE_EQ'])
  
  fruleset = {'www_request': vars(frule1), 'www_response': vars(frule2)}
  query = {'filter': fruleset}
  
  fjson = json.dumps(query, indent=2)
  fsock = open('query.json', 'w')
  fsock.write(fjson)
  fsock.close