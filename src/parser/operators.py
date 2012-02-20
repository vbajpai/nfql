import options
from socket import getprotobyname


if options.import_ops:
    external_import = __import__(options.import_ops)

def NOT(op):
    def not_op(*args):
        op_result = op(*args)
        return not op_result
    
    return not_op

def and_op(*args, **kwargs):
    res = True
    
    for arg in args:
        res = res and arg
    
    for arg in kwargs.values():
        res = res and arg
        
    return res

def bitAND(*args):
    res = args[0]
    
    for arg in args[1:]:
        res &= arg
    
    return res

def bitOR(*args):
    res = args[0]
    
    for arg in args[1:]:
        res |= arg

    return res

def or_op(*args, **kwargs):
    res = False
    
    for arg in args:
        res = res or arg
    
    for arg in kwargs.values():
        res = res or arg
        
    return res


def protocol(name):
    return getprotobyname(name)

def SUM(*args):
    sum = 0
    for arg in args:
        sum += arg
    return sum

def EQ(*args):
    prev_arg = args[0]
    result = True
    for arg in args[1:]:
        result = result and prev_arg == arg
        prev_arg = arg
    return result

def LT(*args):
    prev_arg = args[0]
    result = True
    for arg in args[1:]:
        result = result and prev_arg < arg
        prev_arg = arg
    return result

def GT(*args):
    prev_arg = args[0]
    result = True
    for arg in args[1:]:
        result = result and prev_arg > arg
        prev_arg = arg
    return result

def GTEQ(*args):
    prev_arg = args[0]
    result = True
    for arg in args[1:]:
        result = result and prev_arg >= arg
        prev_arg = arg
    return result

def LTEQ(*args):
    prev_arg = args[0]
    result = True
    for arg in args[1:]:
        result = result and prev_arg <= arg
        prev_arg = arg
    return result

def IN(*args):
    last_arg = args[-1] # probably subnet mask
    result = True
    for arg in args[:-1]:
        result = result and arg & last_arg
    return result

def true(*args):
    return True