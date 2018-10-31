#ifndef ALCHEMIST__ALCHEMIST_HPP
#define ALCHEMIST__ALCHEMIST_HPP

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <thread>
#include <utility>
#include <ctime>
#include <string>
#ifdef ASIO_STANDALONE
#include <asio.hpp>
#else
#include <boost/version.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#endif
#include <eigen3/Eigen/Dense>
#include "arpackpp/arrssym.h"
#include "mpi.h"
#include "Library.hpp"
#include "DistMatrix.hpp"
#include "Parameters.hpp"
#include "utility/client_language.hpp"
#include "utility/command.hpp"
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

#ifdef ASIO_STANDALONE
typedef asio::io_context io_context;
typedef asio::error_code error_code;
#else
typedef boost::asio asio;
typedef boost::system::error_code error_code;
#if BOOST_VERSION < 106600
typedef asio::io_service io_context;
#else
typedef asio::io_context io_context;
#endif
#endif

namespace alchemist {

using std::array;
using std::vector;
using std::map;
using std::string;
using asio::ip::tcp;

const string get_Alchemist_version();
const string get_Boost_version();

typedef El::Matrix<double> Matrix;
typedef std::shared_ptr<DistMatrix> DistMatrix_ptr;

typedef uint16_t Worker_ID;
typedef uint16_t Client_ID;
typedef uint16_t Group_ID;
typedef uint16_t Session_ID;
typedef uint16_t Matrix_ID;
typedef uint16_t Library_ID;
typedef uint16_t Task_ID;

inline const string get_Alchemist_version()
{
	std::stringstream ss;
	ss << ALCHEMIST_VERSION_MAJOR << "." << ALCHEMIST_VERSION_MINOR;
	return ss.str();
}

#ifndef ASIO_STANDALONE
const string get_Boost_version()
{
	std::stringstream ss;
	ss << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "." << BOOST_VERSION % 100;
	return ss.str();
}
#endif

//const string make_daytime_string()
//{
//	std::time_t now = std::time(0);
//	return std::ctime(&now);
//}

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
	uint64_t num_rows;
	uint64_t num_cols;

	vector<Worker_ID> row_assignments;

	explicit MatrixInfo() : ID(0), num_rows(0), num_cols(0) { }

	MatrixInfo(Matrix_ID _ID, uint64_t _num_rows, uint64_t _num_cols) :
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
