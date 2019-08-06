#!/bin/bash

# Configuration file for building Alchemist

export BUILD=release         # release or debug

# export SYSTEM="MacOS"                # Options: MacOS, Cori, Linux
if [ "$SYSTEM" = "" ]; then
  export SYSTEM="MacOS"
fi

if [ "$SYSTEM" = "MacOS" ]; then
	export ALCHEMIST_PATH=$HOME/Projects/Alchemist2
	export ELEMENTAL_PATH=$HOME/Software/Elemental

elif [ "$SYSTEM" = "Linux" ]; then
	export ALCHEMIST_PATH=/usr/local/Alchemist
	export ELEMENTAL_PATH=/usr/local/elemental
	export SPDLOG_PATH=/usr/local/spdlog

elif [ "$SYSTEM" = "Cori" ]; then
	export ALCHEMIST_PATH=$SCRATCH/Projects/Alchemist
	export ELEMENTAL_PATH=$SCRATCH/lib/Elemental
	export SPDLOG_PATH=$SCRATCH/lib/spdlog	
	export EIGEN3_PATH=$SCRATCH/lib/Eigen
	export ARPACK_PATH=$SCRATCH/lib/ARPACK
	export ASIO_PATH=$SCRATCH/lib/asio
	
elif [ "$SYSTEM" = "<your system here>" ]; then
	export ALCHEMIST_PATH=$HOME/Projects/Alchemist
	export ELEMENTAL_PATH=$HOME/Software/Elemental
	export SPDLOG_PATH=$HOME/Software/SPDLog	
	export EIGEN3_PATH=$HOME/Software/Eigen3
	export ARPACK_PATH=$HOME/Software/ARPACK

fi

echo $ALCHEMIST_PATH
