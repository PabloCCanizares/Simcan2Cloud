### output
set terminal postscript enhanced "Verdana" 22
set output "salida.eps"

### Main title
#set title "XXX users accesing a data-center with XX nodes" font "Verdana, 20"
set key on inside top center horizontal box  font "Verdana, 18"

### x-title and font
set xtics font "Verdana,10"
#set xlabel "Users" font "Verdana, 22"

### y-title and font
set ytics font "Verdana,10"
#set ylabel "Average response time" font "Verdana, 22"

### Ranges and scale

#set xrange[0:50]
#set yrange[0:100000]

#Margins
set rmargin 2
#set lmargin 2.2
set tmargin 0.5
#set bmargin 1.5

#key
set key outside center bottom box lt 1 lc 0 font "Verdana, 10"

#Styles
set style line 11 lt 9 lc rgb "black" lw 3 
set style line 12 lt 9 lc rgb "black" lw 3
set style line 13 lt 9 lc rgb "blue" lw 2
set style line 14 lt 2 lc rgb "#00008B" lw 6
set style line 15 lt 2 lc rgb "#8B0000" lw 8
set style line 143 lt 2 lc rgb "#F0008B" lw 6
set style line 154 lt 2 lc rgb "#8BF000" lw 8
set style line 16 lt 9 lc rgb "orange" lw 0.5

#Bottom
set style line 2 lc rgb '#0060ad' lt 1 lw 1 pt 7 ps 0.5
set style line 3 lc rgb '#E9967A' lt 1 lw 1 pt 7 ps 0.5

#Top
set style line 4 lc rgb '#0000FF' lt 1 lw 1 pt 7 ps 1.0
set style line 5 lc rgb '#FF0000' lt 1 lw 1 pt 7 ps 1.0
set style line 6 lc rgb '#FFF000' lt 1 lw 1 pt 7 ps 1.0
set style line 7 lc rgb '#00FF00' lt 1 lw 1 pt 7 ps 1.0

#Middle
set style line 8 lc rgb '#48D1CC' lt 1 lw 1 pt 6 ps 1.0
set style line 6 lc rgb '#FFB6C1' lt 1 lw 1 pt 6 ps 1.0
set style line 63 lc rgb '#DAA520' lt 1 lw 1 pt 6 ps 1.0
set style line 10 lc rgb '#ADFF2F' lt 1 lw 1 pt 6 ps 1.0

#launching time
set style line 1 lc rgb '#000000' lt 1 lw 1 pt 7 ps 0.5


### Plot
plot "dat/output_0.dat" using 2:1 ls 10 title "All at once",\
"dat/output_1.dat" using 2:1 ls 1 title "Exponential",\
"dat/output_2.dat" using 2:1 ls 2 title "Normal",\
"dat/output_3.dat" using 2:1 ls 13 title "Uniform",\
"dat/output_4.dat" using 2:1 ls 14 title "Erlang",\
"dat/output_5.dat" using 2:1 ls 5 title "Truncnormal",\
"dat/output_6.dat" using 2:1 ls 6 title "Chi-square",\
"dat/output_7.dat" using 2:1 ls 8 title "Weibull"





