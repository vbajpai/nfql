set terminal push
set terminal postscript eps font "Times" color

plot '$0' using 3:4 title "nfql engine" smooth csplines with lines, \
     '$0' using 3:4 with points notitle pointtype 5 pointsize 1, \
     '$1' using 3:4 title "nfql filter" smooth csplines with lines, \
     '$1' using 3:4 with points notitle pointtype 5 pointsize 1, \
     '$2' using 3:4 title "flow-tools" smooth csplines with lines, \
     '$2' using 3:4 with points notitle pointtype 5 pointsize 1, \
		 '$3' using 3:4 title "nfdump" smooth csplines with lines, \
     '$3' using 3:4 with points notitle pointtype 5 pointsize 1, \
     '$4' using 3:4 title "silk" smooth csplines with lines, \
     '$4' using 3:4 with points notitle pointtype 5 pointsize 1

set xlabel "Output Flows/Input Flows"
set ylabel "Time (secs)"
set key top left
set title '$5'
set grid
set output '$0.eps'
replot
set output
set terminal pop
