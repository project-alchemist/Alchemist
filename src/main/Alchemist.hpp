#ifndef ALCHEMIST__ALCHEMIST_HPP
#define ALCHEMIST__ALCHEMIST_HPP

//#include <omp.h>
//#include <El.hpp>
//#include <cassert>
//#include <cstdlib>
//#include <cstdio>
//#include <memory>
//#include <thread>
//#include <fstream>
//#include <sstream>
//#include <unistd.h>
////#include <arpa/inet.h>
//#include <sys/stat.h>
//#include <iostream>
//#include <string>
//#include <map>
//#include <random>

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>

#include "mpi.h"
#include "utility/endian.hpp"
#include "Message.hpp"
#include "Parameters.hpp"

#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#ifndef NDEBUG
#define ENSURE(x) assert(x)
#else
#define ENSURE(x) do { if(!(x)) { \
  fprintf(stderr, "FATAL: invariant violated: %s:%d: %s\n", __FILE__, __LINE__, #x); fflush(stderr); abort(); } while(0)
#endif

namespace alchemist {

typedef El::Matrix<double> Matrix;
typedef El::AbstractDistMatrix<double> DistMatrix;

struct MatrixHandle {
  uint32_t ID;
};

struct MatrixDescriptor {
	MatrixHandle handle;
	size_t num_rows;
	size_t num_cols;

	explicit MatrixDescriptor() :
		num_rows(0), num_cols(0) {
	}

	MatrixDescriptor(MatrixHandle handle, size_t num_rows, size_t num_cols) :
		handle(handle), num_rows(num_rows), num_cols(num_cols) {
	}
};

inline bool operator < (const MatrixHandle & lhs, const MatrixHandle & rhs) {
  return lhs.ID < rhs.ID;
}

//inline bool exist_test (const std::string & name) {
//    struct stat buffer;
//    return (stat(name.c_str(), & buffer) == 0);
//}
}	// namespace alchemist


#endif
