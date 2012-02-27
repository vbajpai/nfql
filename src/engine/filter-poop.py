import json

class FilterStatement: 
  
  def __init__(self, name, value, datatype, delta, op):
    self.offset = {
      'name': [name],
      'value': [value],
      'datatype': [datatype]
    }
    
    self.delta = delta
    self.op = op

if __name__ == '__main__':
  f = FilterStatement('dstport', 80, 'u_int16', 0, 'RULE_EQ')
  fjson = json.dumps(vars(f), indent=2)
  fsock = open('query.json', 'w')
  fsock.write(fjson)
  fsock.close