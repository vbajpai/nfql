"""
This module provides methods for dynamically creating flow and 
group record classes.
"""

def get_record_class(attributes, types=None, default_vals=None):
    '''
    Creates a record class for given attribute names.
    
    Arguments:
        attributes - a sequence of attribute names
        types - optional sequence of attribute types, which
            correspond to the attribute names in attributes.
            Types may be of any type, and are not used by the
            Record class, but are useful for external storage,
            where data type has to be predetermined.
        default_val - a sequence of default values which
            correspond to the attribute names in attributes
        
        Lists are used instead of dictionaries because the order
        may be important.
        
    Return:
        Record class which has attributes with the names given
        by attributes list. The class uses __slots__ to lower
        memory usage as potentially millions of instance will
        be present during runtime. The class has a constructor,
        which takes as argument values for the attributes ordered
        the same way as in the attributes list. If default values
        are specified there is a default(no argument) constructor
        as well.
        NOTE that this method returns a class not an instance.
        
    Raises:
        ValueError if number of types or default values doesn't
        match number of attributes.
    '''
    if default_vals and len(attributes) != len(default_vals):
        raise ValueError(
            "Number of attributes(%d) and number of default values(%d)"%
            (len(attributes),len(default_vals))+" don't match")
    if types and len(attributes) != len(types):
        raise ValueError(
            "Number of attributes(%d) and number of default types(%d)"%
            (len(attributes),len(default_vals))+" don't match")
    elif types:
        types_dict = dict(zip(attributes, types))
    else:
        types_dict = {}
    class Record(object):
        '''
        Record class contains flow or group record information.
        
        It uses __slots__ to save memory because potentially millions of
        FlowRecords will be used during run time.
        Attributes:
            attribute names are specified in cls.__slots__
            defaults - contains the default values for attributes used
                with default constructor.
            attr_types - contains a dictionary of the types of 
                the attributes.
        
        Methods:
            __init__ - when defaults is specified __init__()
                creates an object with default values. If no
                defaults are specified during class creation
                __init__() raises TypeError.
                __init__(*args) takes exactly the same number
                of arguments as the classes' number of attributes,
                and creates new instance with the given values.
                Argument order corresponds to the order of
                attributes in cls.__slots__
        
        '''
        # set slots to conserve memory
        # copy ([:]) don't reference to protect from unexpected changes
        __slots__ = attributes[:]
        attr_types = types_dict
        num_of_fields = len(__slots__)
        defaults = default_vals[:] if default_vals else None
        
        def __init__(self, *args):
            num_args = len(args)
            if num_args == self.num_of_fields:
                for name, value in zip(self.__slots__,args):
                    setattr(self, name, value)
            elif num_args == 0 and self.defaults != None:
                for name, value in zip(self.__slots__,self.defaults):
                    setattr(self, name, value)
            elif self.defaults == None:
                raise TypeError(
                    "__init__() takes %d arguments (%d given)"%
                    ( self.num_of_fields + 1, num_args+1))
            else:
                raise TypeError(
                    "__init__() takes either 1 or %d arguments (%d given)"%
                    ( self.num_of_fields + 1, num_args+1))
        
        def tuple(self):
            return tuple(getattr(self, field) for field in self.slots)
        
        def __repr__(self):
            res = "Recod("
            for field in self.__slots__:
                val = getattr(self, field)
                if type(val) is str:
                    val = "'" + str(val) + "'"
                else:
                    val = str(val)
                res += val + ", "
            res =res[:-2] + ")"
            return res
        
        def __str__(self):
            res = "Recod: "
            for field in self.__slots__:
                val = getattr(self, field)
                res += field + "->" + str(val) + ", "
            res =res[:-2]
            return res
    return Record


class RecordReader(object):
    def __init__(self, reader_object):
        self.reader = reader_object
        self.Record = get_record_class(self.reader.fields)

    def __iter__(self):
        for tuple in self.reader:
            yield self.Record(*tuple)
            
    def read_rows_list(self, rows_list):
        for tuple in self.reader.read_rows_list(rows_list):
            yield self.Record(*tuple)
    
    def read_row(self, row_n):
        tup = self.reader.read_row(row_n)
        return self.Record(*tup)

#from flowy import pytables
#ptread = pytables.FlowRecordsTable("../testFT.h5" )
#rr = RecordReader(ptread)
#for i in rr:
#    print i.dOctets

#
#        
#FlowRecord = get_record_class(["a","b"],["str","uint"],[1,6])
#
#def printSth(self):
#    print "sth"
#
#FlowRecord.p = printSth
#
#x = FlowRecord(1,6)
#
#
#print x.a, x.b
#print x.__slots__
#
#t = FlowRecord()
#print t.a
#t.p()
