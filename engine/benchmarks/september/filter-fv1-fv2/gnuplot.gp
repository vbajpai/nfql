set terminal push
set terminal postscript eps font "Times" color

plot '$0' using 3:4 title "nfql" smooth unique with lp pt 5, \
     '$1' using 3:4 title "nfql (optimized)" smooth unique with lp pt 5

set xlabel "Output Flows/Input Flows"
set ylabel "Time (secs)"
set key top left
set title '$2'
set grid
set output '$0.eps'
replot
set output
set terminal pop
