import flowtools
from os.path import exists

default_names_dict = {
              'dFlows': 'dFlows', 'dOctets': 'bytes', 'dPkts': 'dPkts',
              'dst_as': 'dst_as', 'dst_mask': 'dst_mask', 'dst_tag': 'dst_tag',
              'dstaddr_raw': 'dstip', 'dstport': 'dstport',
              'engine_id': 'engine_id', 'engine_type': 'engine_type',
              'exaddr_raw': 'exaddr', 'extra_pkts': 'extra_pkts',
              'first_raw': 'stime', 'in_encaps': 'in_encaps',
              'input': 'input', 'last_raw': 'etime', 'marked_tos': 'marked_tos',
              'nexthop_raw': 'nexthop', 'out_encaps': 'out_encaps',
              'output': 'output', 'peer_nexthop_raw': 'peer_nexthop',
              'prot': 'prot', 'router_sc': 'router_sc', 'src_as': 'src_as',
              'src_mask': 'src_mask', 'src_tag': 'src_tag',
              'srcaddr_raw': 'srcip', 'srcport': 'srcport',
              'sysUpTime': 'sysUpTime', 'tcp_flags': 'tcp_flags', 
              'tos': 'tos', 'unix_nsecs': 'unix_nsecs', 
              'unix_secs': 'unix_secs'}

reverse_names_dict = dict(zip(default_names_dict.values(),
                              default_names_dict.keys()))

# list of the possible fields in the flow tools file
flow_tools_fields = ['dFlows', 'dOctets', 'dPkts', 'dst_as', 'dst_mask',
                     'dst_tag', 'dstaddr_raw', 'dstport', 'engine_id',
                     'engine_type', 'exaddr_raw', 'extra_pkts', 'first_raw',
                     'in_encaps', 'input', 'last_raw', 'marked_tos',
                     'nexthop_raw', 'out_encaps', 'output', 'peer_nexthop_raw',
                     'prot', 'router_sc', 'src_as', 'src_mask', 'src_tag',
                     'srcaddr_raw', 'srcport', 'sysUpTime', 'tcp_flags', 'tos',
                     'unix_nsecs', 'unix_secs']

def find_fields(flowtools_file, fields_of_interest=flow_tools_fields):
    '''
    Returns list of fields_of_interest which are present in
    flotools_file.
    Arguments:
        flowtools_file - path to flowtools records file
        fields_of_interest - names of the fields for which to check
            if none is given all possible fields are searched for.
    '''
    # read first record to see which fields are present:
    flowset = flowtools.FlowSet(flowtools_file)
    flow = iter(flowset).next()
    
    # Find which fields are present in the file
    # (The flow record should have these attributes):
    present_fields = [k for k in fields_of_interest if hasattr(flow,k)]
    return present_fields

def translate_field_names(fields_list, dictionary):
    '''
    Translates names of fields which have keys dictionary.
    For names not present in dictionary the name remains unchanged.
    '''
    return [dictionary[k] for k in fields_list if dictionary.has_key(k)]

def create_flowtools_value_reader(fields):
        def get_fields(record):
            x = tuple(getattr(record,attr) for attr in fields)
            return x
        
        return get_fields


class FlowToolsReader(object):
    
    def __init__(self, path, ft_fields=None, fields=None):
        self.ft_fields = ft_fields if ft_fields else flow_tools_fields
        self.fields = fields if fields else ft_fields
        self.fields = ('rec_id',) + self.fields
        self.get_vals = create_flowtools_value_reader(self.ft_fields)
        if exists(path):
            self.path = path
        else:
            raise IOError("File %s cannot be accessed."%path)
    
    def __iter__(self):
        flowset = flowtools.FlowSet(self.path)
        for id, flow in enumerate(flowset):
            yield (id,) + self.get_vals(flow)
        raise StopIteration



#ft_file = "../ft-v05.2008-10-02.120001+0200"
#ft_fields = find_fields(ft_file)
#print ft_fields
#fields = translate_field_names(ft_fields, default_names_dict)
#import pytables
#field_types = dict((field,pytables.default_ft_types[field]) for field in fields)
#ordered_ft_fields = translate_field_names(field_types.keys(), reverse_names_dict)
#print ordered_ft_fields
#flow_set = FlowToolsReader(ft_file, ft_fields, ft_fields)
#import record
#rec_set = record.RecordReader(flow_set)
#print len(flow_set.fields)
#unix_secs = 0
#sysuptime = 0
#uptime_set = set()
#for i, flow in enumerate(rec_set):
#    if sysuptime != flow.sysUpTime:
#        sysuptime = flow.sysUpTime
#        uptime_set.add(sysuptime)
#        print i, 'ut', flow.sysUpTime - flow.last_raw, 'usecs', flow.unix_secs, 'first - last', flow.last_raw - flow.first_raw
#
#print uptime_set