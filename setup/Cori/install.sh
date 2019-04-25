#!/bin/bash
set -o verbose

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

export ELEMENTAL_PATH=$SCRATCH/lib/Elemental
export SPDLOG_PATH=$SCRATCH/lib/SPDLog	
export EIGEN3_PATH=$SCRATCH/lib/Eigen3
export ARPACK_PATH=$SCRATCH/lib/ARPACK
export ASIO_PATH=$SCRATCH/lib/asio

export TEMP_DIR=$SCRATCH/lib/temp


mkdir -p $TEMP_DIR

# Install Elemental
RUN cd $TEMP_DIR && \
    git clone git://github.com/elemental/Elemental.git && \
    cd Elemental && \
    git checkout 0.87 && \
    mkdir build && \
    cd build && \
    CC=gcc-7 CXX=g++-7 FC=gfortran-7 cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ELEMENTAL_PATH .. && \
    nice make -j4  && \
    make install  && \
    cd $TEMP_DIR && \
    rm -rf Elemental

# Install ARPACK
RUN cd $TEMP_DIR && \
    git clone https://github.com/opencollab/arpack-ng && \
    cd arpack-ng && \
    git checkout 3.5.0 && \
    mkdir build && \
    cd build && \
    cmake -DMPI=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${ARPACK_PATH} .. && \
    nice make -j4 && \
    make install && \
    cd $TEMP_DIR && \
    rm -rf arrack-ng

# Install ARPACKPP
RUN cd $TEMP_DIR && \
    git clone https://github.com/m-reuter/arpackpp && \
    cd arpackpp && \
    git checkout 88085d99c7cd64f71830dde3855d73673a5e872b && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=${ARPACK_PATH} .. && \
    make install && \
    cd $TEMP_DIR && \
    rm -rf arpackpp

# Install Eigen3
RUN cd $TEMP_DIR && \
    curl -L http://bitbucket.org/eigen/eigen/get/3.3.4.tar.bz2 | tar xvfj - && \
    cd eigen-eigen-5a0156e40feb && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=${EIGEN3_PATH} .. && \
    nice make -j4 && \
    make install && \
    cd $TEMP_DIR && \
    rm -rf eigen-eigen-5a0156e40feb

# Install SPDlog
RUN cd $TEMP_DIR && \
    git clone https://github.com/gabime/spdlog.git && \
    cd spdlog && \
    mkdir $SPDLOG_PATH && cp -r include/ $SPDLOG_PATH/include/ && \
    cd $TEMP_DIR && rm -rf spdlog

# Install asio
RUN cd $TEMP_DIR && \
    wget -O asio-1.12.1.zip https://sourceforge.net/projects/asio/files/asio/1.12.1%20%28Stable%29/asio-1.12.1.zip/download && \
    unzip asio-1.12.1.zip && \
    cd asio-1.12.1 && ./configure --prefix=$ASIO_PATH && make install -j4 && \
    cd $TEMP_DIR && rm -rf asio-1.12.1 asio-1.12.1.zip

