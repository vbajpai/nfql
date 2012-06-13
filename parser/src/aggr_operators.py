import options
from tables import UInt32Col, UInt64Col

if options.import_grouper_ops:
    external_import = __import__(options.import_grouper_ops)

class last(object):
    __slots__ = ['field', 'gr_field', 'field_type', 'last']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.last = None
        
    def __call__(self, record = None):
        if record == None:
            return self.last
        else:
            self.last = getattr(record, self.field)
            return self.last


class sum(object):
    __slots__ = ['field', 'gr_field', 'field_type','sum']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.sum = 0
        
    def __call__(self, record = None):
        if record == None:
            return self.sum
        else:
            self.sum += getattr(record, self.field)
            return self.sum

class avg(object):
    __slots__ = ['field', 'gr_field', 'field_type','sum','n','avg']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.sum = 0
        self.n = 0
        self.avg = None
        
    def __call__(self, record = None):
        if record == None:
            if str(self.field_type).find('Int') != -1:
                return int(round(self.avg))
            else:
                return self.avg
        else:
            self.sum += getattr(record, self.field)
            self.n += 1
            self.avg = self.sum / self.n
            return self.avg
        
class max(object):
    __slots__ = ['field', 'gr_field', 'field_type','max']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.max = float("-inf")

    def __call__(self, record = None):
        if record == None:
            return self.max
        else:
            new_val = getattr(record, self.field)
            if self.max < new_val:
                self.max = new_val
            return self.max
        
class min(object):
    __slots__ = ['field', 'gr_field', 'field_type','min']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.min = float("inf")

    def __call__(self, record = None):
        if record == None:
            return self.min
        else:
            new_val = getattr(record, self.field)
            if self.min > new_val:
                self.min = new_val
            return self.min

class count(object):
    __slots__ = ['field', 'gr_field', 'field_type','count']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.count = 0

    def __call__(self, record = None):
        if record == None:
            return self.count
        else:
            self.count += 1
            return self.count
        
class union(object):
    __slots__ = ['field', 'gr_field', 'field_type','union']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.union = []

    def __call__(self, record = None):
        if record == None:
            return sorted(set(self.union))
        else:
            self.union.append(getattr(record, self.field))
            return self.union
        
class bitAND(object):
    __slots__ = ['field', 'gr_field', 'field_type','bitAND']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.bitAND = pow(2,field_type.size) - 1 # all 1s for the given size

    def __call__(self, record = None):
        if record == None:
            return self.bitAND
        else:
            self.bitAND &= getattr(record, self.field)
            return self.bitAND
        
class bitOR(object):
    __slots__ = ['field', 'gr_field', 'field_type','bitOR']
    def __init__(self, field, gr_field, field_type):
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        self.bitOR = 0

    def __call__(self, record = None):
        if record == None:
            return self.bitOR
        else:
            self.bitOR |= getattr(record, self.field)
            return self.bitOR
