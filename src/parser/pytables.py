import tables
import os.path
from itertools import izip
from math import ceil, floor


default_fields = []
default_types = []

def create_flowtools_value_reader(fields):
        def get_fields(record):
            x = tuple(getattr(record,attr) for attr in fields)
            return x
        
        return get_fields

#class RecordsTable(object):
#    def __init__(self, file_path, tree_path, id_size):
#        if os.path.exists(file_path):
#            self.file_path = file_path
#        else:
#            raise IOError("File %s cannot be accessed."%file_path)
#        self.tree_path = tree_path
#        # open for reading
#        self.file = tables.openFile(self.file_path, mode="r+")
#        try:
#            #try to open the table as specified by path if node does not
#            #exist create it
#            self.table = self.file.getNode(self.tree_path)
#        except tables.exceptions.NoSuchNodeError:
#            raise IOError("file %s does not contain table %s"%
#                          (self.file_path,self.tree_path))
#        self.fields = tuple(self.table.description._v_names)
#        self.types = tuple([self.table.description._v_dtypes[name] 
#                           for name in self.fields])
#        # add the id field base on row number:
#        self.fields += ('rec_id',)
#        self.types += (id_size,)
#    
#    def __del__(self):
#        self.file.close()
#
#    def close(self):
#        self.file.close()
    


class Table(object):
    def __init__(self, file_path, id_size):
        if os.path.exists(file_path):
            self.file_path = file_path
        else:
            raise IOError("File %s cannot be accessed."%file_path)
        # open for reading
        self.file = tables.openFile(self.file_path, mode="r+")
        var_nodes  = ['/' + field 
                  for field in self.file.root._v_attrs.variable_fields]
        self.table = self.file.getNode("/fixed_length_fields")
        self.tables =[self.table.iterrows()] + map(self.file.getNode, var_nodes)
        self.fixed_fields = self.file.root._v_attrs.fixed_fields
        self.variable_fields = self.file.root._v_attrs.variable_fields
        # add the id field base on row number:
        self.fields = tuple(['rec_id'] +
                            self.fixed_fields +
                            self.variable_fields)
        self.types = tuple([id_size] +
                           self.file.root._v_attrs.fixed_types +
                           self.file.root._v_attrs.variable_types)
    
    def __del__(self):
        self.file.close()

    def close(self):
        self.file.close()

    def flush(self):
        self.file.flush()

class FlowRecordsTable(Table):
    """A reader object for an HDF table of flow records"""
    def __init__(self, file_path, expected_id_size = tables.UInt32Col()):
        Table.__init__(self, file_path, id_size = expected_id_size)

    def __iter__(self):
        for row in izip(self.iterate_fixed_fields(), *self.tables[1:]):
            yield row[0] + tuple(row[1:])

    def iterate_fixed_fields(self):
        for row in self.table:
            yield tuple([row.nrow] + 
                        [row[field] for field in self.fixed_fields])     
        raise StopIteration

    def read_row(self, row_n):
        row = [r for r in self.table.iterrows(row_n, row_n + 1)][0]
        fixed = tuple([row[field] for field in self.fixed_fields])
        variable = tuple(table.read(row_n)[0] for table in self.tables[1:])
        return (row_n,) + fixed + variable

    def read_rows_list(self, rows_list):
        for row_n in rows_list:
            yield self.read_row(row_n)

    def iter_ids(self, id_list):
        return self.table.readCoordinates(id_list)
    
    def get_record_by_id(self,id):
        return self.table[id]
    
    def __del__(self):
        self.file.close()

    def append(self, record):
        self.row = self.table.row
        for field in self.fixed_fields:
            self.row[field] = getattr(record, field)
        self.row.append()
        for field in self.variable_fields:
            getattr(self.file.root, field).append(getattr(record, field))

    def get_current_row(self):
        return self.row.nrow

    @property
    def len(self):
        return self.table.nrows

class TimeIndex(FlowRecordsTable):
    def __init__(self, fname, id_size_bytes=4):
        FlowRecordsTable.__init__(self, fname, id_size_bytes)
        self.start_time = self.file.root._v_attrs.start_time
        self.delta = self.file.root._v_attrs.delta
        self.id_size = id_size_bytes
        self.index = self.tables[0]


    def get_intervals_list(self, stime, etime):
        start_interval = int(floor((stime - self.start_time) / self.delta))
        end_interval = int(ceil((etime - self.start_time) / self.delta))
        if start_interval < 1 or end_interval < 1:
            raise ValueError("Something's wrong with index intervals")

        return xrange(start_interval, end_interval)
    
    def get_intervals_before(self, record, time_before):
        res = self.get_intervals_list(record.stime - time_before, record.stime)
        return res

    def get_intervals_after(self, record, time_after):
        res = self.get_intervals_list(record.etime, record.etime + time_after)
        return res

    def during(self, record):
        return self.index.get_intervals_list
    
    def time_to_index_row(self, time):
        return int(floor((time - self.start_time) / self.delta))

    def index(self, record):
        for i in self.get_intervals_list(record.stime, record.etime):
            self.index[i] = self.index[i].append(record.rec_id)

#class FlowRecordsTable(RecordsTable):
#    """A reader object for an HDF table of flow records"""
#    def __init__(self, file_path, expected_id_size = tables.UInt32Col()):
#        RecordsTable.__init__(self, file_path, "/flow_records", 
#                              id_size = expected_id_size)
#
#    def __iter__(self):
#        for row in self.table:
#            yield row[:] + (row.nrow,) # tuple concatenation
#        
#        raise StopIteration
#    
#    def iter_ids(self, id_list):
#        return self.table.readCoordinates(id_list)
#    
#    def get_record_by_id(self,id):
#        return self.table[id]
#    
#    def __del__(self):
#        self.file.close()
#
#    def append(self,args):
#        self.row = self.table.row
##        print zip(self.fields, args)
#        for field, val in zip(self.fields, args):
#            self.row[field]= val
#        self.row.append()

def create_Table(file, fields, table_name, field_types, filters):
    file.createTable(file.root, table_name, field_types,
                     "Records Table", filters=filters)

def create_VLArray(file, name, atom, description, filters):
    array = file.createVLArray(file.root, name,
                              atom,
                              "variable length field "+name,
                              filters=filters)
    array.flavor = 'python'

#def create_table_file(file_path, field_types, table_name="flow_records",
#                      complib='lzo', complevel=9):
#    if os.path.exists(file_path):
#        raise IOError("File %s already exists"%file_path)
#
#    file = tables.openFile(file_path, mode="w")
#    filters = tables.Filters(complevel=complevel, complib=complib)
#    file.createTable(file.root, table_name, field_types,
#                     "Records Table", filters=filters)
#    file.close()

def create_index_file(file_path, start_time, delta, id_size_bytes,
                      complib='lzo', complevel=9, itemsize_in_bytes = 4):
    if os.path.exists(file_path):
        raise IOError("File %s already exists"%file_path)
    
    file = tables.openFile(file_path, mode="w")
    filters = tables.Filters(complevel=complevel, complib=complib)
    array = create_VLArray(file.root, 'time_index',
                              tables.UIntAtom(itemsize=itemsize_in_bytes),
                              "time_index", filters=filters)
    array.flavor = 'python'
    file.root._v_attrs.variable_fields = ['time_index']
    file.root._v_attrs.variable_types = [
                                tables.UIntAtom(itemsize=itemsize_in_bytes)]
    file.root._v_attrs.start_time = start_time
    file.root._v_attrs.delta = delta
    file.close()


def create_table_file(file_path, field_types,
                      complib='lzo', complevel=9):
    if os.path.exists(file_path):
        raise IOError("File %s already exists"%file_path)
    
    file = tables.openFile(file_path, mode="w")
    filters = tables.Filters(complevel=complevel, complib=complib)
#    filters = tables.Filters()
    if 'rec_id' in field_types:
        del field_types['rec_id']
    fixed_length_fields = {}
    variable_length_fields = {}
    for k, v in field_types.iteritems():
#        print str(type(v)), str(type(v)).find('atom')
        if str(type(v)).find('atom') == -1:
            fixed_length_fields[k] = v
        else:
            variable_length_fields[k] = v

    file.createTable(file.root, "fixed_length_fields",  fixed_length_fields,
                     "Records Table", filters=filters)

    for field_name, atom in variable_length_fields.iteritems():
        array = file.createVLArray(file.root, field_name, atom, "field "
                                   + field_name, filters)
        array.flavor = 'python'
    file.root._v_attrs.fixed_fields = fixed_length_fields.keys()
    file.root._v_attrs.fixed_types = fixed_length_fields.values()
    
    file.root._v_attrs.variable_fields = variable_length_fields.keys()
    file.root._v_attrs.variable_types = variable_length_fields.values()
    
    file.close()

class GroupsMembersTable(object):
    def __init__(self, file_path, tree_path):
        self.file_path = file_path
        self.tree_path = tree_path
        # open for reading
        self.file = tables.openFile(self.file_path, mode="r+")
        try:
            #try to open the table as specified by path if node does not
            #exist create it
            self.table = self.file.getNode(self.tree_path)
        except tables.exceptions.NoSuchNodeError:
            raise IOError("file %s does not contain table %s"%
                          (self.file_path,self.tree_path))
    
    def __iter__(self):
        for row in self.table:
            yield row
        
        raise StopIteration
    
    def iter_ids(self, id_list):
        for id in id_list:
            yield self.table[id]

    def get_group_by_id(self):
        return self.table[id]

    def __del__(self):
#        self.table.flush()
        self.file.close()

    def append(self, val_list):
        self.table.append(val_list)

class GroupsExpander(object):
    def __init__(self, groups_file_path, records_file_path):
        self.groups = GroupsMembersTable(groups_file_path, "gr1")
        self.records = FlowRecordsTable(self.records_file_path)
        
    
    def group_members(self,group_id):
        grp_member_ids = self.groups.get_group_by_id(group_id)
        return self.record.iter_ids(grp_member_ids)


default_ft_types = {
        'dFlows' : tables.UInt32Col(), 'bytes' : tables.UInt32Col(), 
        'dPkts' : tables.UInt32Col(), 'dst_as' : tables.UInt16Col(),
        'dst_mask' : tables.UInt8Col(), 'dst_tag' : tables.UInt32Col(),
        'dstip' : tables.UInt32Col(), 'dstport' : tables.UInt16Col(),
        'engine_id' : tables.UInt8Col(), 'engine_type' : tables.UInt8Col(),
        'exaddr' : tables.UInt32Col(), 'extra_pkts' : tables.UInt32Col(),
        'stime' : tables.UInt32Col(), 'in_encaps' : tables.UInt8Col(),
        'input' : tables.UInt16Col(), 'etime' : tables.UInt32Col(),
        'marked_tos' : tables.UInt8Col(), 'nexthop' : tables.UInt32Col(),
        'out_encaps' : tables.UInt8Col(), 'output' : tables.UInt16Col(),
        'peer_nexthop' : tables.UInt32Col(), 'prot' : tables.UInt8Col(),
        'router_sc' : tables.UInt32Col(), 'src_as' : tables.UInt16Col(),
        'src_mask' : tables.UInt8Col(), 'src_tag' : tables.UInt32Col(),
        'srcip' : tables.UInt32Col(), 'srcport' : tables.UInt16Col(),
        'sysUpTime' : tables.UInt32Col(), 'tcp_flags' : tables.UInt8Col(),
        'tos' : tables.UInt8Col(), 'unix_nsecs' : tables.UInt32Col(),
        'unix_secs' : tables.UInt32Col() 
                    }
#tab = FlowRecordsTable("../dynZip9.h5")

#for x in tab:
#    print x

#print tab.fields

#wr = TableWriter("../test.h5","/dumps/table1")

#create_group_file("../grptest.h5", "gr1")
#grp = GroupsMembersTable("../grptest.h5", "/gr1")
#grp.append([1,3,5])
#grp.append([2,4])
#grp.append([4324904231490123,98])
#
#for ls in grp.iter_ids([1,2]):
#    print ls

#grp.__del__()
#print [1,4,543,32]

#from os import remove
#fname = "../comp.h5"
#remove(fname)
#field_types = {'info': tables.UInt8Col(),
#               'records': tables.UInt8Atom(), 'info_list': tables.UInt8Atom()}
#create_complex_file(fname, field_types)
##h5f = tables.openFile(fname, 'r')
##print h5f.root._v_attrs.fixed_fields
##print h5f.root._v_attrs.fixed_types
##print h5f.root._v_attrs.variable_fields
##print h5f.root._v_attrs.variable_types
#
#cread = FlRecordsTable(fname)
#
#cread.append((999,[1,3],[1]))
#cread.append((2,[1,4],[2,4,999]))
#cread.close()
#
#read = FlRecordsTable(fname)
#for r in read:
#    print r

