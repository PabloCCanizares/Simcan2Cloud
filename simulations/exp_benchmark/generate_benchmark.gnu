set terminal svg size 800,800
set output 'killmap3d.svg'
set border 4095 front linetype -1 linewidth 1.000
set samples 25, 25
set isosamples 40, 40
set title "Reservoir Title"
set xlabel "Max subscription time"
#set xrange [ 0.0000 : 4 ] noreverse nowriteback
set ylabel "User distribution"
#set yrange [ 0.0000 : 4 ] noreverse nowriteback
#set zrange [ 0.00000 : 4 ] noreverse nowriteback
set pm3d implicit at s
#set pm3d interpolate 50,50
set grid
set hidden3d
splot 'output_graph2.dat' matrix with points pointtype 0
