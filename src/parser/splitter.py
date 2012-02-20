from Queue import Queue
from Queue import Empty

class Splitter(object):
    def __init__(self, name_to_br, filter):
        self.branches = name_to_br.values()
        self.name_to_branch = name_to_br
        self.filter = filter
    
    def go(self):
        for rec, branch in self.filter:
            self.split(branch, rec)

        self.ready()
        
    def split(self, branch_mask, record):
#        print zip(self.branches, branch_mask)
        for branch, active in zip(self.branches, branch_mask):
            if active:
                branch.put(record)
#                if branch.name == 'A': print record
    
    def ready(self):
        print "Filters ready"
        for br in self.branches:
            br.ready = True
            


class Branch(Queue):
    def __init__(self, name):
        Queue.__init__(self, 0)
        self.name = name
        self.ready = False
        
    def __iter__(self):
        while(True):
            if self.empty() and self.ready:
                raise StopIteration
            try:
                record = self.get(timeout=3)
                yield record
                self.task_done()
            except Empty:
                if self.ready:
                    raise StopIteration
