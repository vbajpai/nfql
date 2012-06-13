from statement import Rule, GrouperRule, AllenRule, Field, Arg
from pytables import FlowRecordsTable
import os

def flatten(l):
    if isinstance(l,list):
        return sum(map(flatten,l))
    else:
        return l

def if_exists_delete(path):
    if os.path.exists(path):
        if path[path.find('h5'):] == 'h5':
            os.remove(path)
def iterate_subrules(rule):
    for arg in rule.args:
        if type(arg) in [Rule, GrouperRule, AllenRule]:
            for r in iterate_subrules(arg):
                yield r
            yield arg

def iterate_args(rule):
    for arg in rule.args:
        yield arg

def iterate_rules(filter):
    for rule_list in filter.rules:
        for rule in rule_list:
            if type(rule) not in [Rule, GrouperRule, AllenRule]:
                continue
            yield rule
            for r in iterate_subrules(rule):
                yield r

def find_op(rule, module='operators'):
    imp = __import__(module)
    op_name = rule.op
    try:
        return getattr(imp, op_name)
    except AttributeError:
        try:
            external_imp = getattr(imp, 'external_import')
            return getattr(external_imp, op_name)
        except AttributeError:
            raise SyntaxError('Uknown operator %s at line %s.'%(op_name,
                                                            rule.line))

def get_input_reader(parser):
    """Returns a reader for the parser's input"""
    return FlowRecordsTable(parser.input.name)

def get_input_fields_types(input_reader):
    return dict((f, t) for f, t in zip(input_reader.fields,
                                       input_reader.types))

def check_rule_fields(rule, fields):
    for arg in rule.args:
        if type(arg) is Field: 
            if arg.name in fields:
                continue
            else:
                msg = 'There is no such field %s, '%arg.name
                msg += 'referenced at line %s'%rule.line
                raise SyntaxError(msg)

def replace_bound_rules(filter):
    '''
    Iterate over the rules replacing evaluatable rules with their values
    until there's nothing left to replace i.e. all remaining rules have 
    field references.
    '''
    def evaluate_rule(rule):
            '''
            Evaluates rule if possible i.e. rule contains no record field 
            references. If evaluation is not possible returns the original
            rule.
            '''
            arg_types = [type(x) for x in rule.args]
            if Rule in arg_types or Field in arg_types:
                return rule
            else:
                # no references to record fields evaluate now
                replace_bound_rules.count += 1
                op = find_op(rule)
                # hasattr() takes care of already replaced values
                args = [a.value if hasattr(a, 'value') else a 
                        for a in rule.args]
                result = op(*args) if not rule.NOT else not op(*args)

                return result
    replace_bound_rules.count = 0
    for rule in iterate_rules(filter):
        newargs = [evaluate_rule(arg) if type(arg) is Rule else arg 
                   for arg in rule.args]
        rule.args = newargs

        # we have to break because nasty stuff happens
        # because the iterable is being changed
        if replace_bound_rules.count !=0:
            break

    if replace_bound_rules.count != 0:
        replace_bound_rules.count = 0
        replace_bound_rules(filter)

def replace_with_vals(filter):
    for rule in iterate_rules(filter):
        for i, arg in enumerate(rule.args):
            if type(arg) is Arg:
                rule.args[i] = arg.value
                    
