#!/bin/bash

# Configuration file for building Alchemist

export SYSTEM="MacOS"                # Options: MacOS, Cori, <add your own>

if [ "$SYSTEM" == "MacOS" ]
then
	export ALCHEMIST_PATH=$HOME/Projects/Alchemist2
	
	export ELEMENTAL_PATH=$HOME/Software/Elemental
	export SPDLOG_PATH=$HOME/Software/SPDLog
elif [ "$SYSTEM" == "Cori" ]
then
	export ALCHEMIST_PATH=$SCRATCH/Projects/Alchemist2
	
	export ELEMENTAL_PATH=$SCRATCH/Software/Elemental
	export SPDLOG_PATH=$SCRATCH/Software/SPDLog	
	
elif [ "$SYSTEM" == "<your system here>" ]
then
	export ALCHEMIST_PATH=$SCRATCH/Projects/Alchemist
	
	export ELEMENTAL_PATH=$SCRATCH/Software/Elemental
	export SPDLOG_PATH=$SCRATCH/Software/SPDLog	
fi

echo $ALCHEMIST_PATH