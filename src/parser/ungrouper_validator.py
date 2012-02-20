from copy import deepcopy
from pytables import create_table_file
from ungrouper import Ungrouper as UngrouperImpl
from record import RecordReader
from validator_common import *
from pytables import FlowRecordsTable, create_table_file
import options

class UngrouperValidator(object):
    def __init__(self, parser, merger_validator):
        self.ungroupers = deepcopy(parser.ungroupers)
        self.outputs = deepcopy(parser.outputs)
        self.input = deepcopy(parser.input)
        self.merger_validator = merger_validator
        self.name_to_merger = self.find_ungrouper_to_merger()
        self.name_to_output = self.find_name_to_otput()
        self.name_to_merger_table = self.find_name_to_merger_records_file()
        self.impl = self.create_impl()
    
    def find_ungrouper_to_merger(self):
        n_to_m = {}
        for ungrouper in self.ungroupers:
            m = None
            for merger in self.merger_validator.mergers:
                if ungrouper.branches == merger.branches:
                    for merger_impl in self.merger_validator.impl:
                        if merger_impl.name == merger.name:
                            m = merger_impl
                            break
                    break
            if not m:
                msg = "Could not find merger for Ungrouper %s,"%ungrouper.name
                raise SyntaxError(msg)
            else:
                n_to_m[ungrouper.name] = m
            
        return n_to_m
    
    def find_name_to_merger_records_file(self):
        n_to_merger_records_file = {}
        for name, m in self.name_to_merger.iteritems():
            file_name = m.merger_table.tuples_table.file_path
            table = FlowRecordsTable(file_name)
            n_to_merger_records_file[name] = RecordReader(table)
        return n_to_merger_records_file

    def find_name_to_groups(self, ungrouper):
        br_to_groups = {}
        for branch in ungrouper.branches:
            fname = options.temp_path + options.groups_file_prefix
            fname += branch + '.h5'
            br_to_groups[branch] = RecordReader(FlowRecordsTable(fname))
        return br_to_groups

    def find_name_to_otput(self):
        n_to_o = {}
        for ungrouper in self.ungroupers:
            for output in self.outputs:
                if output.branches == ungrouper.branches:
                    n_to_o[ungrouper.name] = output.name
                    break
        
        return n_to_o

    def find_name_to_gr_output(self, br_order, br_to_groups):
        br_to_gr_output = {}
        for br in br_order:
            fname = options.temp_path + options.groups_file_prefix
            fname += br + '-merged' + '.h5'
            src = br_to_groups[br].reader
            if options.delete_temp_files: if_exists_delete(fname)
            create_table_file(fname, dict(zip(src.fields, src.types)))
            br_to_gr_output[br] = FlowRecordsTable(fname)
        return br_to_gr_output

    def create_impl(self):
        ungr_impl = []
        for ungrouper in self.ungroupers:
            name = ungrouper.name
            merger_impl = self.name_to_merger[name]
            merger_name = merger_impl.name
            br_order = merger_impl.export_branches
            merger_file = options.temp_path + options.merger_file_prefix
            merger_file += merger_name + '.h5'
            merger = RecordReader(FlowRecordsTable(merger_file))
            br_to_groups = self.find_name_to_groups(ungrouper)
            file_name = self.name_to_output.get(name)
            records_table = FlowRecordsTable(self.input.name)
            records = RecordReader(records_table)
            output_file = self.name_to_output[name]
            if options.delete_temp_files: if_exists_delete(output_file)
            create_table_file(output_file, dict(zip(records_table.fields,
                                                    records_table.types)))
            br_to_gr_output = self.find_name_to_gr_output(br_order,
                                                          br_to_groups)
            output = FlowRecordsTable(output_file)
            ungr_impl.append(UngrouperImpl(name, file_name, merger, br_order,
                                           br_to_groups, records, output,
                                           br_to_gr_output))
        
        return ungr_impl