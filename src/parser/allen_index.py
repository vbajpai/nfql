class LT(object):
    """
    X < Y
    x before y
    """
    def __init__(self, src, target, delta):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.etime, x.etime + self.delta
    
class GT(object):
    """
    X > Y
    x after y
    """
    def __init__(self, src, target, delta):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.stime - self.delta, x.stime

class m(object):
    """
    X m Y
    x meets y (x starts before y)
    y should occur at end of x
    """
    def __init__(self, src, target, delta=1):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.etime, x.etime + self.delta

class mi(object):
    """
    X mi Y
    inverse x meets y (x starts after y)
    y should occur at the beginning of x
    """
    def __init__(self, src, target, delta=1):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.stime - self.delta, x.stime

class o(object):
    """
    X o Y
    x overlaps y (x starts before y)
    y should occur at the end of x
    """
    def __init__(self, src, target, delta=0):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.etime-self.delta, x.etime+self.delta

class oi(object):
    """
    X oi Y
    inverse x overlaps y (x starts after y)
    """
    def __init__(self, src, target, delta=0):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.stime, x.stime

class d(object):
    """
    X d Y
    x during y
    """
    def __init__(self, src, target, delta=0):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.stime, x.stime

class di(object):
    """
    X di Y
    inverse x during y (y during x)
    """
    def __init__(self, src, target, delta=0):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.stime, x.etime


class f(object):
    """
    X f Y
    x finishes y (x starts after y, x and y end together)
    """
    def __init__(self, src, target, delta=1):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.etime - self.delta, x.etime + self.delta

class fi(object):
    """
    X fi Y
    inverse x finishes y (x is finished by y)
    """
    def __init__(self, src, target, delta=1):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.etime - self.delta, x.etime + self.delta

class s(object):
    """
    X s Y
    x starts y (x ends before y, x and y starts together)
    """
    def __init__(self, src, target, delta=0):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.stime - self.delta, x.stime + self.delta

class si(object):
    """
    X si Y
    inverse x starts y (x is started by y)
    """
    def __init__(self, src, target, delta=1):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.stime - self.delta, x.stime + self.delta

class EQ(object):
    """
    X = Y
    X lasts the same time as Y and both start together.
    """
    def __init__(self, src, target, delta=1):
        self.delta = delta
        self.src = src
        self.target = target

    def __call__(self, x):
        return x.stime - self.delta, x.stime + self.delta
