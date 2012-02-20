import_ops = "custops"
import_grouper_ops = None


delete_temp_files = True
time_index_interval_ms = 5000
unsat_delta_threshold_mul = 10
max_unsatisfiable_deltas = 20

do_not_expand_groups = False

temp_path = "./flowy-run/"
import os
try:
    os.mkdir(temp_path)
except OSError:
    pass
groups_file_prefix = "Groups"
merger_file_prefix = "Merged"
