#!/usr/bin/python
import options
from optparse import OptionParser
import flowy_exec
import sys
import ply

if __name__ == '__main__':
    usage = 'usage: %prog [options] input_file.flw'
    p = OptionParser(usage)
    option_names = ['--time_index_interval_ms', '--max_unsatisfiable_deltas',
                    '--unsat_delta_threshold_mul', '--do_not_expand_groups']
    for opt_name in option_names:
        p.add_option(opt_name)
    opts, arguments = p.parse_args()
    
    for opt_name in map(lambda x: x[2:], option_names): 
        opt = getattr(opts, opt_name)
        if opt:
            setattr(options, opt_name, opt)

    if len(arguments) != 1:
        sys.stderr.write('Exactly one argument expected\n')
        sys.stderr.write(usage)
        exit(1)
        
    file = arguments[0]

    try:
        flowy_exec.run(file)
    except (ply.yacc.SyntaxError, SyntaxError) as e:
        import sys
        sys.stderr.write(str(e)+'\n')