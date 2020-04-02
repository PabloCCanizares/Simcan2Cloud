### output
set terminal postscript enhanced "Verdana" 22
set output 'salida.eps'

### Main title
set title "XXX users accesing a data-center with XX nodes" font "Verdana, 20"
set key on inside top center horizontal box  font "Verdana, 18"

### x-title and font
set xtics font "Verdana,20"
set xlabel "Users" font "Verdana, 22"

### y-title and font
set ytics font "Verdana,20"
set ylabel "Average response time" font "Verdana, 22"

### Ranges and scale

#set xrange[0:50]
#set yrange[0:10000]

#Styles
set style line 11 lt 9 lc rgb "black" lw 3 
set style line 12 lt 9 lc rgb "black" lw 3
set style line 13 lt 9 lc rgb "blue" lw 3
set style line 14 lt 9 lc rgb "red" lw 3
set style line 15 lt 9 lc rgb "yellow" lw 3
set style line 16 lt 9 lc rgb "orange" lw 0.5

#Bottom
set style line 1 lc rgb '#0060ad' lt 1 lw 1 pt 7 ps 0.5
set style line 2 lc rgb '#E9967A' lt 1 lw 1 pt 7 ps 0.5

#Top
set style line 3 lc rgb '#0000FF' lt 1 lw 1 pt 7 ps 1.0
set style line 4 lc rgb '#FF0000' lt 1 lw 1 pt 7 ps 1.0

#Middle
set style line 5 lc rgb '#48D1CC' lt 1 lw 1 pt 6 ps 1.0
set style line 6 lc rgb '#FFB6C1' lt 1 lw 1 pt 6 ps 1.0

set style line 7 lc rgb '#000000' lt 1 lw 1 pt 7 ps 0.5

### Plot
plot "test_a.dat" using 1:2 ls 7, \
 "test_b.dat" using 1:2 ls 7, \
 "test_a.dat" using 1:3 ls 3, \
 "test_b.dat" using 1:3 ls 4, \
 "test_a.dat" using 1:4 ls 5, \
 "test_b.dat" using 1:4 ls 6

