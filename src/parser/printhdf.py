#!/usr/bin/python
from optparse import OptionParser
import pytables
import record
import sys

def printHDF(hdf_file):
    r = pytables.FlowRecordsTable(hdf_file)
    recordReader = record.RecordReader(r)
    for rec in recordReader:
        print rec
        
if __name__ == "__main__":
    usage = 'usage: %prog file_name.h5'
    p = OptionParser(usage)
    options, arguments = p.parse_args()
    if len(arguments) != 1:
        sys.stderr.write(usage)
        exit(1)
    
    printHDF(arguments[0])