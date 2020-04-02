#!/bin/bash

NUM_SIMULATIONS=$2
currentSim=$1
msg=$3


echo 'RUNNING graph test common'
echo $currentSim '-' $NUM_SIMULATIONS


touch exp_phase3/dat/output_graph.dat
touch exp_phase3/dat/output_table.dat

now=$(date +"[%T - %d/%m/%Y]")
echo "##NewSim - $now: " $msg >> exp_phase3/dat/output_graph.dat
echo "##NewSim - $now: " $msg >> exp_phase3/dat/output_table.dat

$fileNumber=0
while [ $currentSim -lt $NUM_SIMULATIONS ]; do
	echo 'RUNNING graph test common: test_'$currentSim
		
	./run ScenarioTest/omnetpp_dist.ini -u Cmdenv -c test_$currentSim | grep '#___ini#' | sed 's/.*#___ini#//g'> exp_phase3/dat/output_$currentSim.dat

	let currentSim=currentSim+1 
done

echo 'END!'
