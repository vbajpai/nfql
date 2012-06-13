#from pytables import FlowRecordsTable
#from pytables import create_table_file
#from tables import UIntAtom
from math import floor

#class TimeIndex(object):
#    def __init__(self, start_time, delta, id_size_bytes=4):
#        self.start_time = start_time
#        self.delta = delta
#        self.id_size = id_size_bytes
#        self.index = self.get_index_file()
#
#    def get_index_file(self):
#        if self.index: return self.index
#        create_table_file(self.file, {'t': UIntAtom(self.id_size)})
#        
#        self.index = FlowRecordsTable(self.file)
#        self.index.
#        return self.index

class TimeIndex(object):
    def __init__(self, interval=1000, maxsize=10**5):
        self.interval = float(interval)
        self.index = {}
        self.maxsize = maxsize
        self.mintime = float('inf') # later replaced with int
        self.maxtime = float('-inf') # later replaced with int

    @property
    def len(self):
        return len(self.index)

    def get_interval(self, stime, etime):
        start = int(floor(stime/self.interval))
        end = int(floor(etime/self.interval) + 1)
        return xrange(start, end)

    def update_min_max_time(self, record):
        if self.mintime > record.stime:
            self.mintime = record.stime
        if self.maxtime < record.etime:
            self.maxtime = record.etime
            
    def get_total_interval(self):
        return self.get_interval(self.mintime, self.maxtime)


    def add(self, record):
        interval = self.get_interval(record.stime, record.etime)
        for i in interval:
            self.index.setdefault(i, set()).add(record.rec_id)
        
        self.update_min_max_time(record)
        if self.len > self.maxsize:
            print "Warning large index"
            
    def get_interval_records(self, stime, etime):
        res = set()
        for i in self.get_interval(stime, etime):
            res |= self.index.setdefault(i, set()) # set union

        return sorted(res)