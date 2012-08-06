set terminal push
set terminal postscript eps font "Times" color

plot '$0' using 3:4 title "nfql engine" smooth unique with lp pt 5, \
     '$1' using 3:4 title "flow-tools" smooth unique with lp pt 5, \
		 '$2' using 3:4 title "nfdump" smooth unique with lp pt 5, \
     '$3' using 3:4 title "silk" smooth unique with lp pt 5

set xlabel "Output Flows/Input Flows"
set ylabel "Time (secs)"
set key top left
set title '$4'
set grid
set output '$0.eps'
replot
set output
set terminal pop
