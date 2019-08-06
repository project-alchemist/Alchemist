#!/bin/bash

source ./config.sh

export ALCHEMIST_EXE=$ALCHEMIST_PATH/target/$1/alchemist

echo $ALCHEMIST_EXE

CURR_DIR=$PWD

if [ "$SYSTEM" = "MacOS" ] || [ "$SYSTEM" = "Linux" ]; then
	mpiexec -n $2 $ALCHEMIST_EXE
	
elif [ "$SYSTEM" = "Cori" ]; then
	srun -n $2 $ALCHEMIST_EXE
	
elif [ "$SYSTEM" = "<your system here>" ]; then
	srun -n $2 $ALCHEMIST_EXE
fi
