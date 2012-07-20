# run nfdump
sudo python nfdump.py /home/nmelnikov/traces/nf/ queries/nfdump/filter/

# run flowtools
sudo python flowtools.py /home/nmelnikov/traces/ft/ queries/flowtools/filter/

# run silk
sudo python silk.py /home/nmelnikov/traces/silk/ queries/silk/filter/

# run nfql-engine
sudo python nfql-engine.py ../bin/engine /home/nmelnikov/traces/ft/ queries/nfql/filter/

# run nfql-filter
sudo python nfql-filter.py "../bin/filter --verbose=1" /home/nmelnikov/traces/ft/ queries/nfql/filter/
