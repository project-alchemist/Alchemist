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

#include "omp.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/version.hpp>
#include <boost/asio.hpp>
#include "mpi.h"
#include "Library.hpp"
#include "Parameters.hpp"
#include "utility/endian.hpp"
#include "utility/logging.hpp"

#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define ALCHEMIST_PORT 24960
#define ALCHEMIST_VERSION_MAJOR 0
#define ALCHEMIST_VERSION_MINOR 2

#ifndef NDEBUG
#define ENSURE(x) assert(x)
#else
#define ENSURE(x) do { if(!(x)) { \
  fprintf(stderr, "FATAL: invariant violated: %s:%d: %s\n", __FILE__, __LINE__, #x); fflush(stderr); abort(); } while(0)
#endif


#if BOOST_VERSION < 106600
typedef boost::asio::io_service io_context;
#else
typedef boost::asio::io_context io_context;
#endif

namespace alchemist {

typedef El::Matrix<double> Matrix;
typedef El::AbstractDistMatrix<double> DistMatrix;

const std::string get_Alchemist_version();
const std::string get_Boost_version();

typedef uint16_t Worker_ID;

struct WorkerInfo {
	WorkerInfo():
		hostname(string("0")), address(string("0")), port(0), active(false) { }
	WorkerInfo(string _hostname, string _address, uint16_t _port) :
		hostname(_hostname), address(_address), port(_port), active(true) { }

	string hostname, address;
	uint16_t port;
	bool active;
};

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
