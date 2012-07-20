# run nfdump
# sudo python nfdump.py /home/vbajpai/traces/nf/ queries/nfdump/

# run flowtools
# sudo python flowtools.py /home/vbajpai/traces/ft/ queries/flowtools/

# run silk
# sudo python silk.py /home/vbajpai/traces/silk/ queries/silk/

# run nfql-engine
# sudo python nfql-engine.py /home/vbajpai/nfql/engine/bin/engine /home/vbajpai/traces/ft/ queries/nfql/

# run nfql-filter
sudo python nfql-filter.py "/home/vbajpai/nfql/engine/bin/filter --verbose=1" /home/vbajpai/traces/ft/ queries/nfql/
