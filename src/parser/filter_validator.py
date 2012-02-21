from validator_common import *
from copy import deepcopy
from record import RecordReader
from statement import FilterRef
from filter import Rule as RuleImpl
from filter import Filter as FilterImpl
from filter import BranchMask

class FilterValidator(object):
    def __init__(self, parser):
        self.parser = parser
        self.n_real_branches = len(self.parser.branch_names)
        self.filters = deepcopy(parser.filters)
        self.filter_names = dict((filter.name, filter)
                                 for filter in self.filters)
        self.branch_names = self.parser.branch_names # note! not a copy
        self.fields = get_input_fields_types(
                                    get_input_reader(self.parser)).keys()
        self.pseudo_branches = {}
        self.input_reader = RecordReader(get_input_reader(parser))
        self.impl = self.create_impl()

    def check_for_unused_filters(self):
        for filter in self.filters:
            if len(filter.branches) == 0:
                msg = "Warning filter %s "%filter.name
                msg += "defined on line %s"%filter.line
                msg += " is not used in any branch."
                print msg
                continue # skips unused filters

    def check_duplicate_filter_names(self):
        duplicates = {}
        for filter in self.filters:
            old_val = duplicates.setdefault(filter.name, 0)
            duplicates[filter.name] = old_val + 1
            
        duplicate_names = [k for k,v in duplicates.iteritems() if v > 1]
        if len(duplicate_names) > 0:
            msg = "Filter(s) %s"%duplicate_names
            msg += " is/are all defined more than once."
            raise SyntaxError(msg)

    def check_field_refs(self):
        "Check record field references, for unknown fields"
        for filter in self.filters:
            for rule in iterate_rules(filter):
                check_rule_fields(rule, self.fields)

    def change_branch_names_to_id(self):
        """
        Turn branch names into numerical ids. This helps with mask creation.
        """
        # create numerical branch id's:
        self.branches_ids = dict((branch, id) 
                         for id, branch in enumerate(self.parser.branch_names))
        self.ids_branches = dict((id, branch) 
                         for id, branch in enumerate(self.parser.branch_names))
        for filter in self.filters:
            filter.branches = [self.branches_ids[br] for br in filter.branches]

    def create_pseudobranches(self):
        """
        Finds all Filter ref's and adds their branches to the referenced
        filters. If a filter is ORed with another a new branch is created for
        each OR-ed rule.
        """

        max_id = len(self.branches_ids)
        for filter in self.filters:
            for or_rule in filter.rules:
                if type(or_rule[0]) is not FilterRef:
                    # Not a composite rule, so there can't be need for
                    # pseudo branches
                    break
                if len(or_rule) == 1:
                    # Not an ORed FilterRef. Just add FilterRef's branches
                    # to the referenced filter
                    ref_filt = self.parser.names[or_rule[0].name]
                    ref_filt.branches.update(filter.branches)
                else:
                    # ORed FilteRef create pseudo branches
                    pseudo_branch_group = []
                    for br in filter.branches:
                        for filter_ref in or_rule:
                            try:
                                ref_filt = self.filter_names[filter_ref.name]
                            except KeyError, ex:
                                msg = "Filter %s referenced in "%ex.message
                                msg += "%s is not defined"%filter.name
                                raise SyntaxError(msg)
                            id = max_id
                            max_id += 1
                            self.branch_names.add(id)
                            ref_filt.branches.append(id)
                            pseudo_branch_group.append((id, filter_ref.NOT))
                        ps_br_set = self.pseudo_branches.setdefault(br, [])
                        ps_br_set.append(pseudo_branch_group)

    def create_masks(self):
        branches_masks = {}
        rule_masks = {}
        for filter in self.filters:
            if type(filter.rules[0][0]) is FilterRef:
                continue
            for branch in filter.branches:

                for or_rule in filter.rules:
                    if len(or_rule) == 1:
                        #not an OR rule:
                        branches_masks.setdefault(branch,[True])[0] = True
                        sub_br_id = 0
                    else:
                        branches_masks.setdefault(branch,
                                                  [True]).append(False)
                        sub_br_id = len(branches_masks[branch]) - 1

                    for rule in or_rule:
                        rule_masks.setdefault(rule,[]).append((branch,
                                                               sub_br_id,
                                                               rule.NOT))

        self.branches_masks = branches_masks
        self.rule_masks = rule_masks

    def create_rule_implementations(self):
        rules = []
        for rule, br_mask in self.rule_masks.iteritems():
            self.replace_nested_rules(rule)
            op = find_op(rule)
            args = rule.args
            rules.append(RuleImpl(br_mask, op, args))

        return rules

    def replace_nested_rules(self, rule):
        if Rule not in map(type, rule.args):
            op = find_op(rule)
            args = rule.args
            return RuleImpl(None, op, args)
        
        for i, arg in enumerate(rule.args):
            if type(arg) is Rule:
                rule.args[i] = self.replace_nested_rules(arg)

    def validate(self):
        self.check_duplicate_filter_names()
        self.check_field_refs()
        self.change_branch_names_to_id()
        for filter in self.filters:
            replace_bound_rules(filter)
            replace_with_vals(filter)

        self.create_pseudobranches()
        self.check_for_unused_filters()
        self.create_masks()

    def create_impl(self):
        self.validate()
        rules = self.create_rule_implementations()
        pseudo_branches = self.pseudo_branches
        branch_masks = self.branches_masks
        br_mask = BranchMask(branch_masks, pseudo_branches,
                             self.n_real_branches)

        filter_impl = FilterImpl(rules, self.input_reader, br_mask,
                                      self.n_real_branches)
        return filter_impl

