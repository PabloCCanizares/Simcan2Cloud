#!/bin/bash

NUM_SIMULATIONS=$2
currentSim=$1
msg=$3


echo 'RUNNING graph test common'
echo $currentSim '-' $NUM_SIMULATIONS


touch exp_phase2/dat/output_graph.dat
touch exp_phase2/dat/output_table.dat

now=$(date +"[%T - %d/%m/%Y]")
echo "##NewSim - $now: " $msg >> exp_phase2/dat/output_graph.dat
echo "##NewSim - $now: " $msg >> exp_phase2/dat/output_table.dat

while [ $currentSim -lt $NUM_SIMULATIONS ]; do
	echo 'RUNNING graph test common: test_'$currentSim
		
	./run ScenarioTest/omnetpp_3d_4096.ini -u Cmdenv -c test_$currentSim | grep "#___" > exp_phase2/output.dat

	cd exp_phase2
	theRes=$(cat output.dat | grep '#___3d#' | sed 's/.*#___3d#//g')

	if [[ $(( ($currentSim+1) % 8 )) == 0 ]] && [ $currentSim -gt 0 ]; then	
		fullphrase="$fullphrase $theRes"

		echo $fullphrase >> dat/output_graph.dat
		fullphrase=""
	else 
		fullphrase="$fullphrase $theRes"
	fi

	cat  output.dat | grep '#___t#' | sed 's/.*#___t#//g'>> dat/output_table.dat

	cd ..
	let currentSim=currentSim+1 
done

echo 'END!'
