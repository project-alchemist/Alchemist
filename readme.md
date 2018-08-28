Alchemist is a framework for easily and efficiently calling MPI-based codes from Apache Spark. 

# Alchemist Framework Structure

THIS DISCUSSION WILL BE UPDATED SOON

The Alchemist framework has a modular structure in order to achieve flexibility and ease of use. We distinguish between three "layers":
* The core Alchemist system
* The library layer, consisting of one or more MPI-based libraries, as well as two interfaces for each of these libraries, one for bridging Apache Spark and Alchemist, and another one for bridging the MPI library and Alchemist.
* The application layer, consisting of some Spark-based code that wishes to call one or more MPI libraries.

We also distinguish between the Spark side and the MPI side: 
* The Spark (or application) side consists of the Spark application, the Spark-Alchemist interface for each library, and the Alchemist subsystem that takes in a Spark-based data structure. This part of the core Alchemist code is written in Scala, and it is expected that the Spark application and interface are written in Scala as well, although this requirement may be eased at some point.
* The MPI (or library) side consists of the MPI libraries, the library-Alchemist interface for each library, and the Alchemist subsystem that takes in the data from the Spark side and creates the distributed data structures that are needed by the libraries, and then returns the computed results to the Spark side. This subsystem is written in C++ and it is expected that the library and its interface are also written in C++, although this requirement may also be eased at some point. 

For now we require that the MPI libraries use the Elemental library and MPI.

# Dependencies

Alchemist requires an implementation of MPI 3.0, for instance MPICH or Open-MPI. Don't install more than one implementation.

Alchemist also requires the following supporting libraries:
* Elemental: For distributing the matrices between Alchemist processes and distributed linear algebra
* SPDLog: For thread-safe logging during execution

# Installation instructions

## MacOS

### Install prerequisites

The following prerequisites are needed by the Alchemist framework. Assuming that the XCode command line tools and Homebrew have been installed:

```
brew install gcc
brew install make --with-default-names
brew install cmake
```

### Install dependencies

If some or all of the dependencies listed above have not yet been installed on the system, follow the instructions below. 

#### Install an MPI implementation
```
brew install mpich
```
OR
```
brew install open-mpi
```

#### Install Elemental
```
export ELEMENTAL_PATH=(/desired/path/to/Elemental/directory)
cd $ELEMENTAL_PATH
git clone https://github.com/elemental/Elemental.git
cd Elemental
git checkout 0.87
mkdir build
cd build
CC=gcc-7 CXX=g++-7 FC=gfortran-7 cmake -DCMAKE_BUILD_TYPE=Release -DEL_IGNORE_OSX_GCC_ALIGNMENT_PROBLEM=ON -DCMAKE_INSTALL_PREFIX=$ELEMENTAL_PATH ..
nice make -j8
make install
cd ../..
rm -rf Elemental
```

#### Install SPDLog
```
export SPDLOG_PATH=(/desired/path/to/SPDLog/directory)
cd $SPDLOG_PATH
git clone https://github.com/gabime/spdlog.git
cd spdlog
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$SPDLOG_PATH ..
make install
cd ../..
rm -rf spdlog
```

### Clone the Alchemist repo
```
export ALCHEMIST_PATH=(/desired/path/to/Alchemist/directory)
cd $ALCHEMIST_PATH
git clone https://github.com/project-alchemist/Alchemist.git
```

### Update configuration file

In the config.sh file:
* change SYSTEM to the system you are working on;
* set ALCHEMIST_PATH, ELEMENTAL_PATH, and SPDLOG_PATH to the appropriate paths.

It may also be a good idea to add the above paths to the bash profile.

# Building Alchemist

Assuming the above dependencies have been installed and the configuration file updated, Alchemist can be built using
```
./build.sh
```

# Testing Alchemist

For now, testing is done using the Alchemist-Client Interface (ACI) located at 
```
github.com/project-alchemist/ACI
```
