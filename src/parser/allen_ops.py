from math import floor, ceil

def inv_op_str(op_name_string):
    inverse = {
               'LT' : 'GT',
               'GT' : 'LT',
               'm' : 'mi',
               'mi' : 'm',
               'o' : 'oi',
               'oi' : 'o',
               's' : 'si',
               'si' : 's',
               'd' : 'di',
               'di' : 'd',
               'f' : 'fi',
               'fi' : 'f',
               '=' : '='
               }
    return inverse[op_name_string]

class AllenOpIndex(object):
    def __init__(self, index):
        self.index

    def LT(self, x, delta):
        """
        X < Y
        x before y
        """
        return x.etime, x.etime + delta
    
    def GT(self, x, delta):
        """
        X > Y
        x after y
        """
        return x.stime - delta, x.stime
    
    def m(self, x, delta=1):
        """
        X m Y
        x meets y (x starts before y)
        y should occur at end of x
        """
        return x.etime, x.etime + delta
    
    def mi(self, x, delta=1):
        """
        X mi Y
        inverse x meets y (x starts after y)
        y should occur at the beginning of x
        """
        return x.stime - delta, x.stime
    
    def o(self, x, delta=1):
        """
        X o Y
        x overlaps y (x starts before y)
        y should occur at the end of x
        """
        return x.etime-delta, x.etime+delta
    
    def oi(self, x, delta=1):
        """
        X oi Y
        inverse x overlaps y (x starts after y)
        """
        return x.stime, x.stime
    
    def d(self, x, delta=0):
        """
        X d Y
        x during y
        """
        return x.stime, x.stime
    
    def di(self, x, delta=0):
        """
        X di Y
        inverse x during y (y during x)
        """
        return x.stime, x.etime
    
    
    def f(self, x, delta=1):
        """
        X f Y
        x finishes y (x starts after y, x and y end together)
        """
        # delta disregarded here
        return x.etime - delta, x.etime + delta
    
    def fi(self, x, delta=1):
        """
        X fi Y
        inverse x finishes y (x is finished by y)
        """
        return x.etime - delta, x.etime + delta
    
    def s(self, x, delta=1):
        """
        X s Y
        x starts y (x ends before y, x and y starts together)
        """
        return x.stime - delta, x.stime + delta
    
    def si(self, x, delta=1):
        """
        X si Y
        inverse x starts y (x is started by y)
        """
        # delta disregarded here
        return x.stime - delta, x.stime + delta
    
    def EQ(self, x, delta=1):
        """
        X = Y
        X lasts the same time as Y
        """
        # delta disregarded here
        return int((x.stime + x.etime)/2) - delta, int((x.stime +
                                                        x.etime)/2) + delta
                                                        
    def composite_intervals(self, op_x_delta_tuples):
        intervals = set()
        for op_x_delta in op_x_delta_tuples:
            op = op_x_delta[0]
            args = op_x_delta[1:]
            intervals.update(getattr(self, op)(*args))
        
        res = list(intervals)
        res.sort()
        return res
    

def LT(x, y, delta=0):
    """
    X < Y
    x before y
    """
    return x.etime < y.stime

def GT(x, y, delta=1):
    """
    X > Y
    x after y
    """
    return x.stime > y.etime

def m(x, y, delta=1):
    """
    X m Y
    x meets y (x starts before y)
    y should occur at end of x
    """
    return abs(x.etime - y.stime) < delta

def mi(x, y, delta=1):
    """
    X mi Y
    inverse x meets y (x starts after y)
    y should occur at the beginning of x
    """
    return abs(x.stime - y.etime) < delta

def o(x, y, delta=1):
    """
    X o Y
    x overlaps y (x starts before y)
    y should occur at the end of x
    """
    return y.stime < x.etime < y.etime

def oi(x, y, delta=1):
    """
    X oi Y
    inverse x overlaps y (x starts after y)
    """
    return y.stime < x.stime < y.etime

def d(x, y, delta=0):
    """
    X d Y
    x during y
    """
    return y.stime < x.stime and x.etime < y.etime

def di(x, y, delta=0):
    """
    X di Y
    inverse x during y (y during x)
    """
    return y.stime > x.stime and x.etime > y.etime


def f(x, y, delta=1):
    """
    X f Y
    x finishes y (x starts after y, x and y end together)
    """
    # delta disregarded here
    return x.stime > y.etime and abs(x.etime - y.etime) < delta

def fi(x, y, delta=1):
    """
    X fi Y
    inverse x finishes y (x is finished by y)
    """
    return x.stime < y.etime and abs(x.etime - y.etime) < delta

def s(x, y, delta=1):
    """
    X s Y
    x starts y (x ends before y, x and y start together)
    """
    return x.etime < y.etime and abs(x.stime - y.stime) < delta

def si(x, y, delta=1):
    """
    X si Y
    inverse x starts y (x is started by y)
    """
    # delta disregarded here
    return x.etime > y.etime and abs(x.stime - y.stime) < delta

def EQ(x, y, delta=1):
    """
    X fi Y
    inverse x finishes y (x is finished by y)
    """
    # delta disregarded here
    return abs(x.stime - y.stime) < delta and abs(x.etime - y.etime) < delta
