#!/bin/bash
#set -o verbose

# This script installs the prerequisites for building Alchemist on Cori
#
# How to use this script:
#
#   ssh cori
#   set ALCHEMIST_PATH environment variable
#   mkdir $ALCHEMIST_PATH
#   cd $ALCHEMIST_PATH/..
#   git clone https://github.com/project-alchemist/Alchemist.git
#   bash $ALCHEMIST_PATH/setup/Cori/install.sh
#

INSTALL_ELEMENTAL=1
INSTALL_ARPACK=1
INSTALL_EIGEN=1
INSTALL_SPDLOG=1
INSTALL_ASIO=1

export ELEMENTAL_PATH=$SCRATCH/lib/Elemental
export SPDLOG_PATH=$SCRATCH/lib/spdlog	
export EIGEN3_PATH=$SCRATCH/lib/Eigen
export ARPACK_PATH=$SCRATCH/lib/ARPACK
export ASIO_PATH=$SCRATCH/lib/asio

export LD_LIBRARY_PATH=$ELEMENTAL_PATH/lib:$ARPACK_PATH/lib:$LIBRARY_PATH
export CC="cc"
export CXX="CC"
export FC="ftn"
export TEMP_DIR=$SCRATCH/lib/temp

MAKE_THREADS=16

mkdir -p $TEMP_DIR
mkdir -p $ELEMENTAL_PATH
mkdir -p $SPDLOG_PATH
mkdir -p $EIGEN3_PATH
mkdir -p $ARPACK_PATH
mkdir -p $ASIO_PATH



# Setup
module unload darshan
module unload PrgEnv-intel
module load PrgEnv-gnu
module load gcc
module load java
module load python
module load boost
module load cmake
module load sbt
module load fftw
module load hdf5-parallel


# Install Elemental
if [ "$INSTALL_ELEMENTAL" = 1 ]; then

	# Check that the cmake toolchain file is where we expect
	TOOLCHAIN=$ALCHEMIST_PATH/setup/Cori/gnu.cmake
	[ -f "$TOOLCHAIN" ]

	echo " "
	echo "Installing Elemental"
	echo "--------------------"
	cd $TEMP_DIR
	git clone git://github.com/elemental/Elemental.git
	cd Elemental
    git checkout 0.87
    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX="$ELEMENTAL_PATH" -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-dynamic" -DCMAKE_CXX_FLAGS="-dynamic" -DCMAKE_Fortran_FLAGS="-dynamic" ..
    nice make -j"$MAKE_THREADS"
    make install
    cd $TEMP_DIR
    rm -rf Elemental
fi

# Install ARPACK
if [ "$INSTALL_ARPACK" = 1 ]; then
	echo " "
	echo "Installing Arpack-ng"
	echo "--------------------"
	cd $TEMP_DIR
    git clone https://github.com/opencollab/arpack-ng
    cd arpack-ng
    git checkout 3.5.0
    mkdir -p build
    cd build
    cmake -DMPI=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${ARPACK_PATH} ..
    nice make -j"$MAKE_THREADS"
    make install
    cd $TEMP_DIR
    rm -rf arrack-ng

	echo " "
	echo "Installing Arpackpp"
	echo "-------------------"
    cd $TEMP_DIR
    git clone https://github.com/m-reuter/arpackpp
    cd arpackpp
    git checkout 88085d99c7cd64f71830dde3855d73673a5e872b
    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=${ARPACK_PATH} ..
    make install
    cd $TEMP_DIR
    rm -rf arpackpp
fi

# Install Eigen
if [ "$INSTALL_EIGEN" = 1 ]; then
	echo " "
	echo "Installing Eigen"
	echo "----------------"
	cd $TEMP_DIR
    curl -L http://bitbucket.org/eigen/eigen/get/3.3.4.tar.bz2 | tar xvfj -
    cd eigen-eigen-5a0156e40feb
    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=${EIGEN3_PATH} ..
    nice make -j"$MAKE_THREADS"
    make install
    cd $TEMP_DIR
    rm -rf eigen-eigen-5a0156e40feb
fi

# Install spdlog
if [ "$INSTALL_SPDLOG" = 1 ]; then
	echo " "
	echo "Installing spdlog"
	echo "----------------"
	cd $TEMP_DIR
    git clone https://github.com/gabime/spdlog.git
    cd spdlog
    mkdir -p $SPDLOG_PATH
    cp -r include/ $SPDLOG_PATH/include/
    cd $TEMP_DIR
    rm -rf spdlog
fi

# Install asio
if [ "$INSTALL_ASIO" = 1 ]; then
	echo " "
	echo "Installing asio"
	echo "---------------"
	cd $TEMP_DIR
    wget -O asio-1.12.2.zip https://sourceforge.net/projects/asio/files/asio/1.12.2%20%28Stable%29/asio-1.12.2.zip/download
    unzip asio-1.12.2.zip
    cd asio-1.12.2
    ./configure --prefix=$ASIO_PATH --without-boost
    make install -j"$MAKE_THREADS"
    cd $TEMP_DIR
    rm -rf asio-1.12.2 asio-1.12.2.zip
fi

