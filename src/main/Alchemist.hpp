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
#include <ctime>
#include <boost/version.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
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

using std::vector;

typedef El::Matrix<double> Matrix;
typedef El::AbstractDistMatrix<double> DistMatrix;

const std::string get_Alchemist_version();
const std::string get_Boost_version();

typedef uint16_t Worker_ID;
typedef uint16_t Session_ID;
typedef uint16_t Job_ID;
typedef uint16_t Matrix_ID;

struct WorkerInfo {
	WorkerInfo():
		hostname(string("0")), address(string("0")), port(0), active(false) { }
	WorkerInfo(string _hostname, string _address, uint16_t _port) :
		hostname(_hostname), address(_address), port(_port), active(true) { }

	string hostname, address;
	uint16_t port;
	bool active;
};

struct MatrixInfo {
	Matrix_ID ID;
	uint32_t num_rows;
	uint32_t num_cols;

	vector<Worker_ID> row_assignments;

	explicit MatrixInfo() : ID(0), num_rows(0), num_cols(0) { }

	MatrixInfo(Matrix_ID _ID, uint32_t _num_rows, uint32_t _num_cols) :
		ID(_ID), num_rows(_num_rows), num_cols(_num_cols) {
		row_assignments.resize(num_rows);
	}
};

//inline bool exist_test (const std::string & name) {
//    struct stat buffer;
//    return (stat(name.c_str(), & buffer) == 0);
//}
}	// namespace alchemist


#endif
