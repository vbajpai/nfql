set terminal push
set terminal postscript eps font "Times" mono dashed

plot '$0' using 1:2:xtic(4) title "NFQL" smooth csplines with lines, \
     '$0' using 1:2 with points notitle pointtype 5 pointsize 1, \
     '$0' using 1:3:xtic(4) title "SiLK" smooth csplines with lines, \
     '$0' using 1:3 with points notitle pointtype 5 pointsize 1
set xlabel "Number of Flow Records"
set ylabel "Time (secs)"
set key top left
set title '$1'
set grid
set output '$0.eps'
replot
set output
set terminal pop
