#!/bin/bash

NUM_SIMULATIONS=$2
currentSim=$1
msg=$3


echo '_test RUNNING graph test common'
echo $currentSim '-' $NUM_SIMULATIONS


touch exp_phase2/dat/output_graph_2.dat
touch exp_phase2/dat/output_table_2.dat

now=$(date +"[%T - %d/%m/%Y]")
echo "##NewSim - $now: " $msg >> exp_phase2/dat/output_graph_2.dat
echo "##NewSim - $now: " $msg >> exp_phase2/dat/output_table_2.dat

while [ $currentSim -lt $NUM_SIMULATIONS ]; do
	echo 'RUNNING graph test common: test_'$currentSim
		
	./run ScenarioTest/omnetpp_3d.ini -u Cmdenv -c test_$currentSim | grep "#___" > exp_phase2/output_2.dat

	cd exp_phase2
	theRes=$(cat output_2.dat | grep '#___3d#' | sed 's/.*#___3d#//g')

	if [[ $(( ($currentSim+1) % 4 )) == 0 ]] && [ $currentSim -gt 0 ]; then	
		fullphrase="$fullphrase $theRes"

		echo $fullphrase >> dat/output_graph_2.dat
		fullphrase=""
	else 
		fullphrase="$fullphrase $theRes"
	fi

	cat  output_2.dat | grep '#___t#' | sed 's/.*#___t#//g'>> dat/output_table_2.dat

	cd ..
	let currentSim=currentSim+1 
done

echo 'END!'
