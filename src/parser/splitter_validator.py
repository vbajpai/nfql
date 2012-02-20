from copy import copy, deepcopy
from splitter import Branch as BranchImpl
from splitter import Splitter as SplitterImpl

class SplitterValidator(object):
    def __init__(self, parser, filter_validator):
        self.splitter = copy(parser.splitter)
        self.branches = deepcopy(parser.branches)
        self.branch_ids = filter_validator.branches_ids
        self.filter_impl = filter_validator.impl
        self.br_name_to_br = {}
        self.impl = self.create_impl()
    
    def sort_branches(self):
        id_to_branch = dict(zip(self.branch_ids.values(),
                                self.branch_ids.keys()))
        sorted_br = [id_to_branch[k] for k in sorted(id_to_branch.keys())]
        return sorted_br
    
    def create_impl(self):
        br_names = self.sort_branches()
        
        branches = [BranchImpl(name) for name in br_names]
        name_to_br = dict(zip(br_names, branches))
        self.br_name_to_br = name_to_br
        return SplitterImpl(name_to_br, self.filter_impl)