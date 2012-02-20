#!/usr/bin/python
from record import RecordReader
from pytables import FlowRecordsTable
from itertools import izip
from optparse import OptionParser

if __name__ == '__main__':
    usage = 'usage: %prog [options] input files'
    p = OptionParser(usage)
    opts, arguments = p.parse_args()

    mg_readers = [RecordReader(FlowRecordsTable(f)) for f in arguments]
    
    for rec_tuple in izip(*mg_readers):
        print ""
        for r in rec_tuple:
            print r

