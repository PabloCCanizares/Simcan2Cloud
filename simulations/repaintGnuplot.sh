#!/bin/bash

NUM_SIMULATIONS=$2
currentSim=$1



echo 'RUNNING graph test common'
echo $currentSim '-' $NUM_SIMULATIONS

	cd exp_phase1
while [ $currentSim -lt $NUM_SIMULATIONS ]; do
	
	echo 'Generating graphs test_'$currentSim
	gnuplot -e "filename_ok='dat/test_ok_$currentSim.dat'; filename_ko='dat/test_ko_$currentSim.dat'; output_filename='output_$currentSim.eps'" generate.gnu
	let currentSim=currentSim+1 
done
	cd ..
echo 'END!'
