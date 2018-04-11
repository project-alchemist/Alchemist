#!/bin/bash

source ./config.sh

export ALCHEMIST_EXE=$ALCHEMIST_PATH/target/alchemist

CURR_DIR=$PWD

if [ "$SYSTEM" == "MacOS" ]
then
	mpiexec -n 3 ALCHEMIST_EXE
	
elif [ "$SYSTEM" == "Cori" ]
then
	srun -n 5 ALCHEMIST_EXE
	
elif [ "$SYSTEM" == "<your system here>" ]
then
	srun -n 5 ALCHEMIST_EXE
	
fi