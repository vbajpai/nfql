import options
class Ungrouper(object):
    def __init__(self, name, file_name, merger, br_order, br_to_groups,
                 records, output_file, br_to_gr_output):
        self.merger = merger
        self.name = name
        self.file_name = file_name
        self.br_order = br_order
        self.br_to_groups = br_to_groups
        self.flow_records = records
        self.output_file = output_file
        self.br_to_gr_output = br_to_gr_output
    
    def groups(self):
        for rec in self.merger:
            for br in self.br_order:
                gr_rec_id = getattr(rec, br)
                gr_rec = self.br_to_groups[br].read_row(gr_rec_id)
                yield gr_rec
                
    def records(self):
        for rec in self.merger:
            for br in self.br_order:
                gr_rec_id = getattr(rec, br)
                gr_rec = self.br_to_groups[br].read_row(gr_rec_id)
                for record in self.flow_records.read_rows_list(gr_rec.records):
                    yield record
                    
    def go(self):
        if options.do_not_expand_groups:
            for rec in self.merger:
                for br in self.br_order:
                    gr_rec_id = getattr(rec, br)
                    gr_rec = self.br_to_groups[br].read_row(gr_rec_id)
                    self.br_to_gr_output[br].append(gr_rec)
        else:
            for rec in self.merger:
                for br in self.br_order:
                    gr_rec_id = getattr(rec, br)
                    gr_rec = self.br_to_groups[br].read_row(gr_rec_id)
                    self.br_to_gr_output[br].append(gr_rec)
                    for record in self.flow_records.read_rows_list(
                                                            gr_rec.records):
                        self.output_file.append(record)
                        
        print 'Ungrouper %s finished exectution'%self.name