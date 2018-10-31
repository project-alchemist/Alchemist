#!/bin/bash

# Configuration file for building Alchemist

if [ "$SYSTEM" = "" ]; then
  export SYSTEM="MacOS"
fi

if [ "$SYSTEM" = "MacOS" ]; then
	export ALCHEMIST_PATH=$HOME/Projects/Alchemist2
	export ELEMENTAL_PATH=$HOME/Software/Elemental
	export EIGEN3_PATH=$HOME/Projects/eigen
	export ARPACK_PATH=$HOME/Software/arpack

elif [ "$SYSTEM" = "Linux" ]; then
	export ALCHEMIST_PATH=/usr/local/Alchemist
	export SPDLOG_PATH=/usr/local/spdlog
	export ELEMENTAL_PATH=/usr/local/elemental
	export EIGEN3_PATH=/usr/local/eigen
	export ARPACK_PATH=/usr/local/arpack
	export ASIO_PATH=/usr/local/asio

elif [ "$SYSTEM" = "Cori" ]; then
	export ALCHEMIST_PATH=$SCRATCH/Projects/Alchemist2
	export ELEMENTAL_PATH=$SCRATCH/Software/Elemental
	export SPDLOG_PATH=$SCRATCH/Software/SPDLog	
	export EIGEN3_PATH=$HOME/Software/Eigen3
	export ARPACK_PATH=$HOME/Software/ARPACK
	
elif [ "$SYSTEM" = "<your system here>" ]; then
	export ALCHEMIST_PATH=$SCRATCH/Projects/Alchemist
	export ELEMENTAL_PATH=$SCRATCH/Software/Elemental
	export SPDLOG_PATH=$SCRATCH/Software/SPDLog	
	export EIGEN3_PATH=$HOME/Software/Eigen3
	export ARPACK_PATH=$HOME/Software/ARPACK

fi

echo $ALCHEMIST_PATH
