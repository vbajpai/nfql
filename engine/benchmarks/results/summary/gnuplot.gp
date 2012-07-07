set terminal push
plot '$0' using 1:2 title "NFQL" smooth csplines with lines linetype 2, \
     '$0' using 1:2 with points notitle pointtype 5 pointsize 0.5, \
     '$0' using 1:3 title "SiLK" smooth csplines with lines linetype 7, \
     '$0' using 1:3 with points notitle pointtype 5 pointsize 0.5
set xlabel "Number of Flow Records"
set ylabel "Time (secs)"
set key top left
set title '$1'
set grid
set terminal png
set output '$0.png'
replot
set output
set terminal pop
