from validator_common import *
from copy import deepcopy
from tables import UIntAtom, UIntCol
from grouper import GrouperModule as GrouperModuleImpl
from grouper import Grouper as GrouperImpl
from grouper import GrouperRule as GrouperRuleImpl
from grouper import AggrOp as AggrOpImpl

class GrouperValidator(object):
    def __init__(self, parser, splitter_validator):
        self.parser = parser
        self.fields_types = get_input_fields_types(
                                    get_input_reader(self.parser))
        self.groupers = deepcopy(parser.groupers)
        self.br_name_to_br = splitter_validator.br_name_to_br
        self.br_name_to_grouper = {}
        self.impl = self.create_impl()

    def validate(self):
        self.check_field_refs()
        self.check_duplicate_grouper_names()
        for grouper in self.groupers:
            self.check_duplicate_module_names(grouper)
            for module in grouper.modules:
                replace_bound_rules(module)
                replace_with_vals(module)

    def check_duplicate_grouper_names(self):
        duplicates = {}
        for grouper in self.groupers:
            old_val = duplicates.setdefault(grouper.name, 0)
            duplicates[grouper.name] = old_val + 1
            
        duplicate_names = [k for k,v in duplicates.iteritems() if v > 1]
        if len(duplicate_names) > 0:
            msg = "Grouper(s) %s"%duplicate_names
            msg += " is/are all defined more than once."
            raise SyntaxError(msg)
        
    def check_duplicate_module_names(self, grouper):
        duplicates = {}
        for module in grouper.modules:
            old_val = duplicates.setdefault(module.name, 0)
            duplicates[module.name] = old_val + 1
            
        duplicate_names = [k for k,v in duplicates.iteritems() if v > 1]
        if len(duplicate_names) > 0:
            msg = "Module(s) %s"%duplicate_names
            msg += " is/are all defined more than once in grouper"
            msg += " %s."%grouper.name
            raise SyntaxError(msg)

    def check_field_refs(self):
        for grouper in self.groupers:
            for module in grouper.modules:
                for rule in module.rules:
                    check_rule_fields(rule[0], self.fields_types.keys())


            for aggr in grouper.aggr:
                for arg in aggr.args:
                    if type(arg) == Field:
                        mod, _, field = arg.name.partition('.')
                        if field != '':
                            if field not in self.fields_types.keys():
                                msg = 'There is no such field %s, '%arg.name
                                msg += 'referenced at line %s'%aggr.line
                                raise SyntaxError(msg)
                        else:
                            if mod not in self.fields_types.keys():
                                msg = 'There is no such field %s, '%arg.name
                                msg += 'referenced at line %s'%aggr.line
                                raise SyntaxError(msg)

    def create_grouper_rules_impl(self, grouper):
        modules_list = []
        for module in grouper.modules:
            rule_impl_list = self.convert_module_rules(module)
            aggr_ops_list = self.convert_module_aggr_ops(grouper, module)
            module_impl = GrouperModuleImpl(module.name, rule_impl_list,
                                       aggr_ops_list)
            modules_list.append(module_impl)
        
        grouper_aggr_ops = []
        for aggr in grouper.aggr:
            init_args = self.create_aggr_impl_init_args(aggr)
            spl = str.split(init_args[1], '.')
            if len(spl) > 1:
                msg = 'There is no such grouper module %s, '%spl
                msg += 'referenced on line %s'%aggr.line
                raise SyntaxError(msg)
            impl = AggrOpImpl(*init_args)
            grouper_aggr_ops.append(impl)
        
        groupers = [GrouperImpl(grouper.name, modules_list, grouper_aggr_ops,
                                self.br_name_to_br[br_name], br_name)
                                for br_name in grouper.branches]
        
        for grouper in groupers:
            self.br_name_to_grouper[grouper.branch_name] = grouper

        return groupers


    def convert_module_aggr_ops(self, grouper, module):
        aggr_ops_list = []
        del_list = []
        for aggr in grouper.aggr:
            op, field, gr_field, field_type = self.create_aggr_impl_init_args(
                                                                        aggr)
            mod_name, _, f = str.partition(field, '.')
            if f != '':
                if module.name == mod_name:
                    impl = AggrOpImpl(op, f, gr_field, field_type)
                    aggr_ops_list.append(impl)
                    del_list.append(aggr)
        
        for a in del_list:
            grouper.aggr.remove(a)
        
        return aggr_ops_list

    def create_aggr_impl_init_args(self, aggr):
        field = aggr.args[0].name
        if '.' in field:
            _, _, non_qid_field = field.partition('.')
        else:
            non_qid_field = field
        gr_field = aggr.args[1]
        if aggr.op == 'count':
            field_type = UIntCol(self.fields_types['rec_id'].itemsize)
        elif aggr.op == 'union':
            field_type = UIntAtom(self.fields_types[non_qid_field].itemsize)
        else:
            field_type = UIntCol(self.fields_types[non_qid_field].itemsize)
        
        op = find_op(aggr, 'aggr_operators')
        
        return op, field, gr_field, field_type

    def convert_module_rules(self, module):
        rule_impl_list = []
        for rules in module.rules:
            for rule in rules:
                op = find_op(rule)
                args = [arg.name if type(arg) is Field else arg
                        for arg in rule.args]
                rule_impl_list.append(GrouperRuleImpl(op, *args))
        return rule_impl_list
    
    def create_impl(self):
        self.validate()
        groupers_impls = []
        for grouper in self.groupers:
            groupers_impls.extend(self.create_grouper_rules_impl(grouper))
        
        for br_name in self.br_name_to_br.keys():
            if br_name not in self.br_name_to_grouper.keys():
                msg = 'There is no grouper for branch %s.'%br_name
                raise SyntaxError(msg)

        return groupers_impls