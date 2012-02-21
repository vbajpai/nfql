

class MergerStorage(object):
    def __init__(self, id, tuples_table, record_class):
        self.id = id
        self.tuples_table = tuples_table
        self.RecordClass = record_class

    def put(self, gr_rec_tuple):
        self.tuples_table.append(self.RecordClass(*gr_rec_tuple))
        
    def flush(self):
        self.tuples_table.flush()


class MergerRule(object):
    def __init__(self, op, args,  br_to_record):
        # The records are changed externally from branches:
        self.br_to_record = br_to_record
        self.args = args
        self.op = op

    def match(self):
        # The records are changed externally by another object
        args = []
        for arg in self.args:
            if type(arg) is MergerRule:
                args.append(arg.match())
            elif type(arg) is tuple:
                br, field = arg
                record = self.br_to_record[br]
                if field:
                    # normal rule get field of the record
                    args.append(getattr(record, field))
                else:
                    # allen rule, argument is the record
                    args.append(record)
            else:
                args.append(arg)
        return self.op(*args)

class Reject(Exception):
    pass

class Accept(Exception):
    pass

class Merger(object):
    def __init__ (self, name, br_name, records, name_to_branch,
                  next_branches_names, export_branches, br_to_record,
                  index, index_rules, rules, merger_table):
        self.name = name
        self.merger_table = merger_table
        self.records = records
        self.export_branches = export_branches
        self.br_name = br_name
        self.name_to_branch = name_to_branch
        self.rules = rules
        self.index = index
        self.br_to_record = br_to_record
        self.next_branches_names = next_branches_names
        self.remaining_rec = dict((name, None) for name 
                                              in next_branches_names)
        self.index_rules = index_rules

    @property
    def next_branch(self):
        if not self.next_branches_names:
            return False
        return self.name_to_branch[self.next_branches_names[0]]

    def match(self):
        for rule in self.rules:
            if not rule.match():
                return False
        return True
    
    def pass_allen_indices_down(self, record):
        new_br_remaining_rec = {}
        for rules in self.index_rules:
            br_name = rules[0].target
            rec_set = set()
            branch = self.name_to_branch[br_name]
            index = branch.index
            for rule in rules:
                interval = rule(record)
                rec_set.update(index.get_interval_records(*interval))
            
            # note {}.get(k) return none if {} has no key k
            set_from_parent = self.remaining_rec[br_name]
            if set_from_parent:
                # there is a set of records defined by parent
                # do an intersection
                new_br_remaining_rec[br_name] = rec_set & set_from_parent
            else:
                # no set from parent, just add this rec_set
                new_br_remaining_rec[br_name] = rec_set
            
        # pass to next branch
        if len(new_br_remaining_rec) == 0:
            self.next_branch.remaining_rec = self.remaining_rec
        else:
            self.next_branch.remaining_rec = new_br_remaining_rec
#        print "passing",self.next_branch.remaining_rec


    def go(self):
        for rec in self.records.record_reader:
            self.br_to_record[self.br_name] = rec
            self.pass_allen_indices_down(rec)
            self.next_branch.next()
        print "Finished merging branches: ",
        print [self.br_name] + self.next_branches_names
        self.merger_table.flush()
        self.merger_table.tuples_table.close()

class MergerBranch(Merger):
    def __init__ (self, br_name, records, name_to_branch, next_branches_names, 
                  export_branches, br_to_record ,index, index_rules, rules, 
                  merger_table):
        Merger.__init__(self, None, br_name, records, name_to_branch, 
                        next_branches_names, export_branches, br_to_record, 
                        index, index_rules, rules, merger_table)

    def next(self):
        remaining = self.remaining_rec[self.br_name]
        for rec in self.records.record_reader.read_rows_list(remaining):
            self.br_to_record[self.br_name] = rec
            if not self.match():
                continue

            self.pass_allen_indices_down(rec)
            try:
                self.next_branch.next()
            except Accept:
                # the reject modules did not reject this tuple
                res = tuple(self.br_to_record[br].rec_id for br 
                        in self.export_branches)
                self.merger_table.put(res)
            except Reject:
                # this tuple matched reject module so we go on
                pass

class MergerLastBranch(Merger):
    def __init__ (self, br_name, records, name_to_branch, next_branches_names, 
                  export_branches, br_to_record ,index, index_rules, rules, 
                  merger_table):
        Merger.__init__(self, None, br_name, records, name_to_branch, 
                        next_branches_names, export_branches, br_to_record, 
                        index, index_rules, rules, merger_table)
    def next(self):
        remaining = self.remaining_rec[self.br_name]
        for rec in self.records.record_reader.read_rows_list(remaining):
            self.br_to_record[self.br_name] = rec
            if not self.match():
                continue

            # last branch and no reject branches
            # append the record
            res = tuple(self.br_to_record[br].rec_id for br 
                        in self.export_branches)
            self.merger_table.put(res)

class MergerRejectBranch(Merger):
    def __init__ (self, br_name, records, name_to_branch, next_branches_names, 
              export_branches, br_to_record ,index, index_rules, rules, 
              merger_table):
        Merger.__init__(self, None, br_name, records, name_to_branch, 
                    next_branches_names, export_branches, br_to_record, 
                    index, index_rules, rules, merger_table)
    def next(self):
        remaining = self.remaining_rec[self.br_name]
        for rec in self.records.record_reader.read_rows_list(remaining):
            self.br_to_record[self.br_name] = rec
            if self.match():
                raise Reject # goes all the way up to last normal branch
            else:
                try:
                    if self.next_branch:
                        self.pass_allen_indices_down(rec)
                        self.next_branch.next()
                    else:
                        # this is the last branch, so go on
                        pass
                except Accept:
                    # this Accept is from lower reject-branch so just
                    # go on and raise Accept when this branch finishes
                    pass
        raise Accept