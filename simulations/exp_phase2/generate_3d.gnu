set terminal eps size 10,10 font "Verdana,20"
set output 'killmap3d_512.eps'
set border 4095 front linetype -1 linewidth 1.000
set samples 150, 150
set isosamples 400, 40

set xtics 0, 9
set ytics 0, 3
set xlabel "Max subscription time" offset 0,-1,0
set ylabel "Distribution" offset 4.0,-1,0


set autoscale xfix
set xtics add ( "2" 0)
set xtics add ( "3" 1)
set xtics add ( "4" 2)
set xtics add ( "6" 3)
set xtics add ( "7" 4)
set xtics add ( "8" 5)
set xtics add ( "9" 6)
set xtics add ( "10" 7)

set ytics add ( "Uniform(55,60)" 0)
set ytics add ( "Truncnormal" 1)
set ytics add ( "Weibull(36,37)" 2)
set ytics add ( "Normal(28)" 3)
set ytics add ( "Chi_square(22)" 4)
set ytics add ( "Erlang_k(1,15)" 5)
set ytics add ( "Exponential(7)" 6)

set zrange[0:50000]
set cbrange[0:50000]

set dgrid3d 7,8
set pm3d implicit at s
set pm3d interpolate 10,10

set xtics left offset -2,0
set ytics left offset 5,0
set key off

#Margins
set lmargin 0.2
set tmargin -2.0
set bmargin 1.5


splot 'dataset_512.dat' matrix with lines

