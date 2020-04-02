set terminal svg size 800,800
set output 'killmap3d.svg'
#set border 4095 front linetype -1 linewidth 1.000
#set samples 25, 25
#set isosamples 40, 40
#set title "Reservoir Title"
#set xlabel "Max subscription time"

set ylabel "User distribution"
#set xrange [ 0 : 3 ] noreverse nowriteback
#set yrange [ 0 : 3 ] noreverse nowriteback
set zrange [ 0 : 150 ] noreverse nowriteback
set xrange [0:20]
set yrange [0:20]

#set pm3d implicit at s
#set pm3d interpolate 0,0

set dgrid3d 30,30
set hidden3d

set multiplot
splot "datafile.dat" u 1:2:3 with lines

unset dgrid3d 
unset hidden3d

set border 4095 front linetype -1 linewidth 1.000
set samples 25, 25
set isosamples 40, 40
set title "Reservoir Title"
set xlabel "Max subscription time"

set pm3d implicit at s
set pm3d interpolate 0,0


splot 'output_graph1.dat' matrix with points pointtype 0, \
'output_graph2.dat' matrix with lines lc rgb "black"

unset multiplot
