set terminal push
set terminal postscript eps font "Times" color

plot '$0' using 1:3 title "z-level" smooth csplines with lines, \
     '$0' using 1:3 with points notitle pointtype 5 pointsize 1

set xlabel "z-level"
set ylabel "Time (secs)"
set key top left
set title '$1'
set grid
set output '$0.eps'
replot
set output
set terminal pop
