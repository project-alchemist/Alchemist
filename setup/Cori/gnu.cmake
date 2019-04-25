# The Cray wrappers
set(COMPILER_DIR $ENV{CRAYPE_DIR}/bin)
set(CMAKE_C_COMPILER       ${COMPILER_DIR}/cc)
set(CMAKE_CXX_COMPILER     ${COMPILER_DIR}/CC)
set(CMAKE_Fortran_COMPILER ${COMPILER_DIR}/ftn)

set(CMAKE_C_FLAGS "-dynamic")
set(CMAKE_CXX_FLAGS "-dynamic")
set(CMAKE_Fortran_FLAGS "-dynamic")

# This is just a hack, as this machine always uses the above wrappers
set(MPI_C_COMPILER ${CMAKE_C_COMPILER})
set(MPI_CXX_COMPILER ${CMAKE_CXX_COMPILER})
set(MPI_Fortran_COMPILER ${CMAKE_Fortran_COMPILER})

set(OpenMP_CXX_FLAGS "-fopenmp")

string(REPLACE " " ";" GNU_VERSIONS $ENV{PE_LIBSCI_GENCOMPS_GNU_x86_64})
list(GET GNU_VERSIONS 0 GNU_VERSION_NUMBER)
set(MATH_LIBS "-L/opt/cray/pe/libsci/default/GNU/${GNU_VERSION_NUMBER}/x86_64/lib -lsci_gnu")