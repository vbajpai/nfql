import options
from copy import deepcopy
from validator_common import *
from groupfilter import Rule as RuleImpl
from groupfilter import GroupFilter as GroupFilterImpl
from groupfilter import AcceptGroupFilter as AcceptGroupFilterImpl
from operators import NOT
import pytables
from timeindex import TimeIndex

class GroupFilterValidator(object):
    def __init__(self, parser, grouper_validator):
        self.parser = parser
        self.grouper_validator = grouper_validator
        self.filters = deepcopy(parser.group_filters)
        self.branches_fields = self.get_branches_fields()
        self.br_name_to_grouper = grouper_validator.br_name_to_grouper
        self.br_name_to_gr_filter = {}
        self.impl = self.create_impl()

    def check_duplicate_filter_names(self):
            duplicates = {}
            for filter in self.filters:
                old_val = duplicates.setdefault(filter.name, 0)
                duplicates[filter.name] = old_val + 1
                
            duplicate_names = [k for k,v in duplicates.iteritems() if v > 1]
            if len(duplicate_names) > 0:
                msg = "Group filter(s) %s"%duplicate_names
                msg += " is/are all defined more than once."
                raise SyntaxError(msg)

    def check_field_refs(self):
        "Check record field references, for unknown fields"
        for filter in self.filters:
            for rule in iterate_rules(filter):
                for branch in filter.branches:
                    check_rule_fields(rule, self.branches_fields[branch])
    

    def get_branches_fields(self):
        branches_fields = {}
        for grouper in self.grouper_validator.impl:
            branches_fields[grouper.branch_name] = grouper.group_record_fields
        return branches_fields

    def validate(self):
        self.check_for_unused_filters()
        self.check_field_refs()
        self.check_duplicate_filter_names()
    
    def check_for_unused_filters(self):
        for filter in self.filters:
            if len(filter.branches) == 0:
                msg = "Warning groupfilter %s "%filter.name
                msg += "defined on line %s"%filter.line
                msg += " is not used in any branch."
                print msg
                continue # skips unused filters
    
    def get_rule_impl(self, rule):
        op = find_op(rule)
        args = [self.get_rule_impl(arg) if type(arg) == Rule else arg
                for arg in rule.args]
        impl = RuleImpl(None, NOT(op) if rule.NOT else op, args)
        
        return impl
    
    def get_rules_impl(self, filter):
        replace_bound_rules(filter)
        replace_with_vals(filter)
        rules_impl = []
        for or_rule in filter.rules:
            or_rule_list = []
            for rule in or_rule:
                impl = self.get_rule_impl(rule)
                or_rule_list.append(impl)
            rules_impl.append(or_rule_list)
        
        return rules_impl

    def create_impl(self):
        self.validate()
        group_filters_impl = []
        for filter in self.filters:
            rules_impl = self.get_rules_impl(filter)
            for br_name in filter.branches:
                records = self.br_name_to_grouper[br_name]
                index = TimeIndex(5000)
                grouper = records
                field_types = dict(zip(grouper.group_record_fields,
                                       grouper.group_record_types))
                fname = options.temp_path + options.groups_file_prefix
                fname += br_name+".h5"
                if options.delete_temp_files: if_exists_delete(fname)
                file = pytables.create_table_file(fname, field_types)
                groups_table = pytables.FlowRecordsTable(fname)
                filt_impl = GroupFilterImpl(rules_impl, records, br_name,
                                            groups_table, index)
                group_filters_impl.append(filt_impl)
        
        self.br_name_to_gr_filter = dict((filt.branch_name, filt) 
                                           for filt in group_filters_impl)
        
        # Check for branches that don't have group filters and and put accept
        # filters on them
        for br_name in self.br_name_to_grouper.keys():
            if br_name not in self.br_name_to_gr_filter.keys():
                records = self.br_name_to_grouper[br_name]
                index = TimeIndex(5000)
                grouper = records
                field_types = dict(zip(grouper.group_record_fields,
                                       grouper.group_record_types))
                fname = options.temp_path + options.groups_file_prefix
                fname += br_name+".h5"
                if options.delete_temp_files: if_exists_delete(fname)
                file = pytables.create_table_file(fname, field_types)
                groups_table = pytables.FlowRecordsTable(fname)
                filt_impl = AcceptGroupFilterImpl(records, br_name,
                                            groups_table, index)
                self.br_name_to_gr_filter[br_name] = filt_impl
                group_filters_impl.append(filt_impl)
        return group_filters_impl