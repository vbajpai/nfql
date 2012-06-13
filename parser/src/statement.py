class Splitter(object):
    def __init__(self, name, line, branches=None):
        self.name = name
        self.line = line
        self.branches = branches if branches else set()
    
    def __repr__(self):
        str = "Splitter('%s', %s, %s)"%(self.name, self.line, self.branches)
        return str

class Ungrouper(object):
    def __init__(self, name, line, branches=None):
        self.name = name
        self.line = line
        self.branches = branches if branches else set()
    
    def __repr__(self):
        str = "Ungrouper('%s', %s, %s)"%(self.name, self.line, self.branches)
        return str

class Input(object):
    def __init__(self, name, line, inputs=None, outputs=None, branches=None):
        self.name = name
        self.line = line
        self.branches = branches if branches else set()
        self.inputs = inputs if inputs != None else set()
        self.outputs = outputs if outputs != None else set()

    def __repr__(self):
        str = "Input('%s', %s, %s, %s, %s)"%(self.name, self.line, 
                                             self.branches, self.inputs,
                                             self.outputs)
        return str

class Output(object):
    def __init__(self, name, line, inputs=None, outputs=None, branches=None):
        self.name = name
        self.line = line
        self.branches = branches if branches else set()
        self.inputs = inputs if inputs != None else set()
        self.outputs = outputs if outputs != None else set()

    def __repr__(self):
        str = "Output('%s', %s, %s, %s, %s)"%(self.name, self.line,
                                            self.branches, self.inputs,
                                                self.outputs)
        return str

class Branch(object):
    def __init__(self, name, line, members=None, inputs=None, outputs=None):
        self.name = name
        self.members = members
        self.line = line
        self.inputs = members if members != None else set()
        self.inputs = inputs if inputs != None else set()
        self.outputs = outputs if outputs != None else set()

    def __repr__(self):
        str = "Branch('%s', %s, %s, %s, %s)"%(self.name, self.line,
                                              self.members, self.inputs,
                                              self.outputs)
        return str

class BranchNode(object):
    def __init__(self, name, line, inputs=None, outputs=None):
        self.name = name
        self.line = line
        self.inputs = inputs if inputs != None else set()
        self.outputs = outputs if outputs != None else set()
    
    def __repr__(self):
        str = "BranchNode('%s', %s, %s, %s)"%(self.name, self.line ,
                                                self.inputs, self.outputs)
        return str

class Rule(object):
    def __init__(self, op, line, args, NOT=False):
        self.op = op
        self.args = args
        self.NOT = False
        self.line = line

    def __repr__(self):
        str = "Rule('%s', %s, %s, %s)"%(self.op, self.line, 
                                        self.args, self.NOT)
        return str
    
    def __str__(self):
        return "%s%s" % (self.op, self.args)

    def __eq__(self, other):
        return str(self)== str(other)
    
    def __hash__(self):
        return hash(str(self))
    
class AllenRule(Rule):
    def __repr__(self):
        str = "AllenRule('%s', %s, %s, %s)"%(self.op, self.line, 
                                        self.args, self.NOT)
        return str

class Field(object):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return "Field('%s')"%self.name

class GrouperRule(object):
    def __init__(self, op, line, args):
        self.line = line
        self.args = args
        self.op = op

    def __repr__(self):
        str = "GrouperRule('%s', %s, %s)"%(self.op, self.line, self.args)
        return str

class Filter(object):
    def __init__(self, name, line, rules, branches=None):
        self.name = name
        self.rules = rules
        self.line = line
        self.branches = branches if branches else set()

    def __repr__(self):
        str = "Filter('%s', %s, %s, %s)"%(self.name, self.line, self.rules,
                                        self.branches)
        return str

class Module(Filter):
    def __repr__(self):
        str = "Module('%s', %s, %s, %s)"%(self.name, self.line,
                                          self.rules, self.branches)
        return str

class Grouper(object):
    def __init__(self, name, line, modules, aggr, branches=None):
        self.name = name
        self.aggr = aggr
        self.modules = modules
        self.line = line
        self.branches = branches if branches else set()

    def __repr__(self):
        str = "Grouper('%s', %s, %s, %s, %s)"%(self.name, self.line,
                                      self.modules, self.aggr, self.branches)
        return str

class Merger(object):
    def __init__(self, name, line, modules, export, branches=None):
        self.name = name
        self.export = export
        self.modules = modules
        self.line = line
        self.branches = branches if branches else set()

    def __repr__(self):
        str = "Merger('%s', %s, %s, %s, %s)"%(self.name, self.line,
                                      self.modules, self.export, self.branches)
        return str

class FilterRef(object):
    def __init__(self, name, line, NOT=False):
        self.name = name
        self.NOT = NOT
        self.line = line

    def __repr__(self):
        str = "FilterRef('%s', %s, %s)"%(self.name, self.line, self.NOT)
        return str

class Arg(object):
    def __init__(self, type, value, str=''):
        self.type = type
        self.value = value
        self.str = str

    def __repr__(self):
        str = "Arg('%s', %s, '%s')"%(self.type, repr(self.value), self.str)
        return str
