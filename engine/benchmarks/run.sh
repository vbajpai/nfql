# run flowtools
sudo python nfdump.py /home/vbajpai/traces/nf/ july/queries/nfdump/filter/

# run flowtools
sudo python flowtools.py /home/vbajpai/traces/ft/ july/queries/flowtools/filter/

# run silk
sudo python silk.py /home/vbajpai/traces/silk/ july/queries/silk/filter/

# run nfql
sudo nfql.py bin/engine /home/vbajpai/traces/ft/ july/queries/nfql/filter/
