import record
import options
from aggr_operators import count

class UnsatisfiableDelta(Exception):
    pass

class Grouper(object):
    def __init__(self, id, modules, aggr_ops, records, branch_name):
        self.modules = modules
        self.records = records
        self.aggr_ops = aggr_ops
        self.group_record_fields = self.create_gr_record_fields_list()
        self.group_record_fields = ('rec_id',) + self.group_record_fields
        self.group_record_types = self.create_gr_record_fields_types()
        self.group_records = []
        self.branch_name = branch_name
        self.Record = record.get_record_class(self.group_record_fields)


    def new_group(self, record):
        group = Group(record, self.modules, self.aggr_ops)
        return group
        
    def __iter__(self):
        count = 0
        for record in self.records:
#            print self.group_records
            matched = False
#            print len(self.group_records)
            del_list = []
            try:
                for i, group_record in enumerate(self.group_records):
                    if group_record.satisfiable:
                        if group_record.match(record):
                            matched = True
                            break
                    else:
                        yield  self.Record(*((count,)+group_record.export()))
                        count += 1
                        del_list.append(i)
            except ValueError:
                # Group Records list is empty
                # WARNING may catch ValueError from another place
                # group list is still empty
                matched = False # this will signal new group creation
            if not matched:
                self.group_records.append(self.new_group(record))
                
            # remove exported groups:
            for n_removed, i in enumerate(del_list):
                # count removed elements with n_removed and compensate
                # because positions change when removing elements
                # Fortunately del_list is sorted so '-' works as
                # a compensation, as all removed elements are before the
                # current one
                del self.group_records[i - n_removed]
        print "Finished grouping branch "+self.branch_name
        for group_record in self.group_records:
            yield self.Record(*((count,)+group_record.export()))
            count += 1
    
    def create_gr_record_fields_list(self):
        field_list = []
        for module in self.modules:
            for op in module.aggr_ops:
                field_list.append(op.gr_field)
        
        for op in self.aggr_ops:
            field_list.append(op.gr_field)
        return tuple(field_list)
    
    def create_gr_record_fields_types(self):
        type_list = [None]
        for module in self.modules:
            for op in module.aggr_ops:
                type_list.append(op.field_type)
        
        for op in self.aggr_ops:
            if type(op) == count:
                type_list[0] = op.field_type # set the type for rec_id
            type_list.append(op.field_type)
        return tuple(type_list)

class AggrOp(object):
    def __init__(self, op, field, gr_field, field_type):
        self.op = op
        self.field = field
        self.gr_field = gr_field
        self.field_type = field_type
        
    def new_op(self):
        return self.op(self.field, self.gr_field, self.field_type)

class GrouperModule(object):
    def __init__(self, name, rules, aggr_ops):
        self.name = name
        self.rules = rules
        self.aggr_ops = aggr_ops

    def match(self, record, group):
        for rule in self.rules:
            if not rule.match(record, group):
                return False
        return True

class GrouperRule(object):
    def __init__(self, op, old_rec_field, new_record_field,
                 delta=None, relative=False):
        self.op = op
        self.old_rec_field = old_rec_field
        self.new_rec_field = new_record_field
        self.delta = delta
        self.relative = relative
        self.is_shortcut = self.check_is_shortcut()
#        print self.op, self.old_rec_field, self.new_rec_field

    def check_is_shortcut(self):
        if self.delta:
            if (self.old_rec_field in ('stime', 'etime') and
                self.new_rec_field in ('stime', 'etime')):
                return True
            
        return False

    def match(self, record, group):
        new = getattr(record, self.new_rec_field)
        if self.relative:
            old = getattr(group.last_record, self.old_rec_field)
        else:
            old = getattr(group.first_record, self.old_rec_field)
            
        if self.delta:
            if self.op(abs(new - old), self.delta):
                return True
            elif (self.is_shortcut and 
                  not self.op(abs(new - old), 
                            self.delta * options.unsat_delta_threshold_mul )):
#                print abs(new - old)/1000.0, (self.delta * options.unsat_delta_threshold_mul)/1000.0
                raise UnsatisfiableDelta
            else:
                return True
        else:
            return self.op(old, new)

class Group(object):
    __slots__ = ['modules', 'modules_aggr_ops', 'aggr_ops', 'records',
                 'first_record', 'last_record', 'satisfiable',
                 'n_unsatisfiable_deltas', 'max_unsat_deltas']

    def __init__(self, first_record, modules, aggr_ops,
                 max_unsat_deltas=options.max_unsatisfiable_deltas):
        self.first_record = first_record
        self.last_record = first_record # changes with each new matched record
        self.modules = modules
        # list of lists of aggr_ops each corresponding to a module
        self.modules_aggr_ops = self.create_modules_aggr()
        self.aggr_ops = self.create_aggr_ops(aggr_ops)
        self.satisfiable = True
        self.n_unsatisfiable_deltas = 0
        self.max_unsat_deltas = max_unsat_deltas
    
    def create_modules_aggr(self):
        modules_aggr_ops = []
        for module in self.modules:
            aggr = [op.new_op() for op in module.aggr_ops]
            for op in aggr:
                op(self.first_record)
            modules_aggr_ops.append(aggr)
        return modules_aggr_ops
    
    def create_aggr_ops(self, aggr_ops):
        aggr = [op.new_op() for op in aggr_ops]
        for op in aggr:
            op(self.first_record)
        return aggr

    def match(self, record):
        matched = False
        for module, aggr_ops in zip(self.modules, self.modules_aggr_ops):
            try:
                if module.match(record, self):
                    for op in aggr_ops:
                        op(record)
                    matched = True
            except UnsatisfiableDelta:
                if matched:
                    continue
                self.n_unsatisfiable_deltas += 1
                if self.n_unsatisfiable_deltas > self.max_unsat_deltas:
                    self.satisfiable = False

        if matched:
            for aggr_op in self.aggr_ops:
                aggr_op(record)
#                print aggr_op.gr_field, aggr_op()
#            print self.records
            self.n_unsatisfiable_deltas = 0
            return True
        else:
            return False
        
    def export(self):
        fields = []
        for aggr_ops in self.modules_aggr_ops:
            for op in aggr_ops:
                fields.append(op())
        
        for op in self.aggr_ops:
            fields.append(op())

        return tuple(fields)
