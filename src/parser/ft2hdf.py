#!/usr/bin/python
from pytables import FlowRecordsTable
import pytables
import ftreader
import record
import os
from os.path import split, join, islink
import re
import sys
from bisect import bisect, bisect_left
from operator import itemgetter
from optparse import OptionParser

#def ft2hdf(ft_file, hdf_file):
#    ft_fields = ftreader.find_fields(ft_file)
#    fields = ftreader.translate_field_names(ft_fields,
#                                            ftreader.default_names_dict)
#    field_types = dict((field,pytables.default_ft_types[field]) 
#                                                        for field in fields)
##    print field_types
#    pytables.create_table_file(hdf_file, field_types)
#    rec_table = pytables.FlowRecordsTable(hdf_file)
#    # since pytables is initiated with dictionary there is no way to
#    # sort the fields order, so we have to translate back in order
#    # to keep the fields names order
#    ordered_ft_fields = ftreader.translate_field_names(rec_table.fields,
#                                                ftreader.reverse_names_dict)
#    flow_set = ftreader.FlowToolsReader(ft_file, ordered_ft_fields)
#    for flow in flow_set:
#        rec_table.append(flow)
#    rec_table.close()
    
    
def ft2hdf_single(ft_file, hdf_file):
    ft_fields = ftreader.find_fields(ft_file)
    fields = ftreader.translate_field_names(ft_fields,
                                            ftreader.default_names_dict)
    field_types = dict((field,pytables.default_ft_types[field]) 
                                                        for field in fields)
#    print field_types
    pytables.create_table_file(hdf_file, field_types)
    rec_table = pytables.FlowRecordsTable(hdf_file)
    # since pytables is initiated with dictionary there is no way to
    # sort the fields order, so we have to translate back in order
    # to keep the fields names order
    ordered_ft_fields = ftreader.translate_field_names(rec_table.fields,
                                                ftreader.reverse_names_dict)
    flow_set = ftreader.FlowToolsReader(ft_file,
                                    ordered_ft_fields, rec_table.fields[1:])
    rec_set = record.RecordReader(flow_set)
    for flow in rec_set:
        rec_table.append(flow)
    rec_table.close()

def ft2hdf(many_files, hdf_file):
    ft_file = many_files[0]
    ft_fields = ftreader.find_fields(ft_file)
    fields = ftreader.translate_field_names(ft_fields,
                                            ftreader.default_names_dict)
    field_types = dict((field,pytables.default_ft_types[field]) 
                                                        for field in fields)
#    print field_types
    pytables.create_table_file(hdf_file, field_types)
    rec_table = pytables.FlowRecordsTable(hdf_file)
    # since pytables is initiated with dictionary there is no way to
    # sort the fields order, so we have to translate back in order
    # to keep the fields names order
    ordered_ft_fields = ftreader.translate_field_names(rec_table.fields,
                                                ftreader.reverse_names_dict)
    for ft_file in many_files:
        flow_set = ftreader.FlowToolsReader(ft_file,
                                        ordered_ft_fields, rec_table.fields[1:])
        rec_set = record.RecordReader(flow_set)
        for flow in rec_set:
            rec_table.append(flow)
    rec_table.close()
    
def printHDF(hdf_file):
    r = pytables.FlowRecordsTable(hdf_file)
    recordReader = record.RecordReader(r)
    for rec in recordReader:
        print rec

class FSLoop(Exception):
    pass

def findFiles(path, start_time, end_time, filter_files = False):
    timeExp = re.compile(r"ft-v05\.(\d{4})-(\d{2})-(\d{2}).(\d{6}).(\d{4})")

    time_file_list = []
    dir_links = [path]
    def walkDirs(dir_links):
        file_list = []
        more_dir_links = []
        for link in dir_links:
            for root, dirs, files in os.walk(link):
                for file in files:
                    match = timeExp.search(file)
                    if match:
                        element = (int(''.join(match.groups()[:-1])), join(root,file))
                        if element in time_file_list:
                            raise FSLoop
                        file_list.append(element)
                for dir in dirs:
                    if islink(join(root,dir)):
                        print file
                        more_dir_links.append(join(root,dir))
        return file_list, more_dir_links
    
    while len(dir_links) > 0:
        tf, dir_links = walkDirs(dir_links)
        time_file_list.extend(tf)
        
    def cmp((a,x),(b,y)):
        if a-b < 0:
            return -1
        elif a-b>0:
            return 1
        else:
            return 0
    
    time_file_list.sort(cmp)

    if (filter_files):
        keys = [r[0] for r in time_file_list]
        begin = 0
        end = len(time_file_list)
        if start_time is not None:
            begin = bisect_left(keys, long(start_time))
        if end_time is not None:
            end = bisect(keys, long(end_time))
        # the start and end time must be converted to long
        time_file_list = time_file_list[begin:end]
        
    time_file_list = map(lambda (x,y):y,time_file_list)
    return time_file_list
    
def dateToInt(date):
    number_of_digits = [4, 2, 2, 2, 2, 2]
    separators = '[- :/]*'
    expr = "\d{%s}"%number_of_digits[0]
    for digit in number_of_digits[1:]:
        expr += separators + "(\d{%s})"%digit
    timeExp = re.compile(expr)
    result = timeExp.match(date)
    if result is None:
        raise ValueError("invalid date format")
    return date.translate(None, '- :/')

def lotsOfFolders(paths, start_time=None, end_time=None):
    full_file_paths=[]
    start_time, end_time = [dateToInt(d) if d != None else d for d in (start_time, end_time)]
    for path in paths:
        full_file_paths.extend(findFiles(path, start_time, end_time, True))
    # sort the results
    split_paths = map(split, full_file_paths)
    split_paths = set(split_paths)
    split_paths = sorted(split_paths, key=itemgetter(1))
    full_file_paths = [join(x, y) for x, y in split_paths]

    return full_file_paths

def main():
    usage = 'usage: %prog [options] input_path1 [input_path2 [...]] output_file.h5'
    p = OptionParser(usage)
    p.add_option('--start-time', '-s')
    p.add_option('--end-time', '-e')
    options, arguments = p.parse_args()
    start_time = options.start_time
    end_time = options.end_time
    folders = arguments[:-1]
    output = arguments[-1]
    if not (output[output.find('.h5'):] == '.h5'):
        sys.stderr.write('Output file should have an .h5 extension\n')
        exit(1)
    file_paths = lotsOfFolders(folders, start_time,end_time)
    if len(file_paths) < 1:
        sys.stderr.write('No flow-tools files found\n')
        exit(1)
    ft2hdf(file_paths, output)


if __name__ == "__main__":
    main()

