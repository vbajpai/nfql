from parser import Parser
from filter_validator import FilterValidator
from splitter_validator import SplitterValidator
from grouper_validator import GrouperValidator
from groupfilter_validator import GroupFilterValidator
from merger_validator import MergerValidator
from ungrouper_validator import UngrouperValidator
from threading import Thread
import options

def run(filename):
    p = Parser()

    file = open(filename)
    doc = file.read()
    
    p.parse(doc)
    
    #inps = get_inputs_list(p)
    #print get_input_fields_types(inps[0])
#    hdf_file = "../testFT2.h5"
#    r = pytables.FlowRecordsTable(hdf_file)
#    recordReader = record.RecordReader(r)
    f = FilterValidator(p)
    fl = f.impl
    s = SplitterValidator(p, f)
    spl = s.impl

    gr = GrouperValidator(p, s)
    grs = gr.impl
    

    gr_filt = GroupFilterValidator(p, gr)
    gr_filters = gr_filt.impl

    mr = MergerValidator(p, gr_filt)
    mergers = mr.impl
    
    splitter_thread = Thread(target=spl.go)
    gf_threads = [Thread(target=gf.go)for gf in gr_filters]
    splitter_thread.start()
    for gf_thread in gf_threads:
        gf_thread.start()
    
    splitter_thread.join()
    for gf_thread in gf_threads:
        gf_thread.join()

    merger_threads = [Thread(target=m.go()) for m in mergers]
    for merger_thread in merger_threads:
        merger_thread.start()
        
    for merger_thread in merger_threads:
        merger_thread.join()
        

    ung = UngrouperValidator(p, mr)
    ungroupers = ung.impl

    ungrouper_threads = [Thread(target=u.go) for u in ungroupers]
    for ungrouper_thread in ungrouper_threads:
        ungrouper_thread.start()
    
    for ungrouper_thread in ungrouper_threads:
        ungrouper_thread.join()
        
    print "FINISHED!"
#    fname = mergers[0].merger_table.tuples_table.file_path
#    print fname
    import ft2hdf



if __name__ == '__main__':
    options.delete_temp_files = True
    import ply
#    import profiler
#    profiler.profile_on()
    run('www_one_dir.flw')
#
#    
#    profiler.profile_off()
#    import pickle
#    stats = profiler.get_profile_stats()
#    sorted_stats = sorted(stats.iteritems(), key=lambda a: a[1][0])
#    for st in sorted_stats:
#        print st
#    
#    f = open('./profile_stats1', 'w')
#    pickle.dump(sorted_stats,f)