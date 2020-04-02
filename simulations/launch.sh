#!/bin/bash

NUM_SIMULATIONS=$2
currentSim=$1


if [ "$1" == "" ]; then
	echo "Usage: $>launch.sh testInit testEnd"
elif [ "$2" == "" ]; then
	echo "Usage: $>launch.sh testInit testEnd"
else

	echo 'RUNNING graph test common'
	echo $currentSim '-' $NUM_SIMULATIONS

	while [ $currentSim -lt $NUM_SIMULATIONS ]; do
		echo 'RUNNING graph test common: test_'$currentSim
		echo "filename_ko='dat/test_ko_$currentSim.dat'"
		./run ScenarioTest/omnetpp.ini -u Cmdenv -c test_$currentSim | grep "#___#" > exp_phase1/test_$currentSim.dat
		cd exp_phase1
	
		echo 'Generating graphs test_'$currentSim
		cat test_$currentSim.dat | grep 'Success' | sed 's/.*Success //g' > dat/test_ok_$currentSim.dat
		cat test_$currentSim.dat | grep 'Timeout' | sed 's/.*Timeout //g'> dat/test_ko_$currentSim.dat
		gnuplot -e "filename_ok='dat/test_ok_$currentSim.dat'; filename_ko='dat/test_ko_$currentSim.dat'; output_filename='output_$currentSim.eps'" generate.gnu
		let currentSim=currentSim+1 
		cd ..
	done
fi
echo 'END!'
