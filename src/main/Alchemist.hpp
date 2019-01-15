#ifndef ALCHEMIST__ALCHEMIST_HPP
#define ALCHEMIST__ALCHEMIST_HPP

#include <cstdlib>
#include <deque>
#include <iostream>
#include <iomanip>
#include <list>
#include <memory>
#include <set>
#include <thread>
#include <utility>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <sstream>
#include <complex>
#include <cmath>
#ifdef ASIO_STANDALONE
#include <asio.hpp>
#else
#include <boost/version.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#endif
#include <El.hpp>
#include "mpi.h"
#include "utility/endian.hpp"
#include "utility/client_language.hpp"
#include "utility/command.hpp"
#include "utility/logging.hpp"
#include "utility/datatype.hpp"

#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define ALCHEMIST_PORT 24960
#define ALCHEMIST_VERSION_MAJOR 0
#define ALCHEMIST_VERSION_MINOR 5

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

typedef uint8_t Worker_ID;
typedef uint16_t Client_ID;
typedef uint16_t Group_ID;
typedef uint16_t Session_ID;
typedef uint16_t Matrix_ID;
typedef uint16_t Task_ID;
typedef uint8_t Library_ID;

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
	WorkerInfo(Worker_ID _ID):
		ID(_ID), hostname(string("0")), address(string("0")), port(0), active(false), group_ID(0) { }
	WorkerInfo(Worker_ID _ID, string _hostname, string _address, uint16_t _port) :
		ID(_ID), hostname(_hostname), address(_address), port(_port), active(false), group_ID(0)  { }

	Worker_ID ID;
	string hostname, address;
	uint16_t port;
	bool active;
	Group_ID group_ID;
};

struct MatrixInfo {
	Matrix_ID ID;

	string name;
	uint64_t num_rows, num_cols;
	bool sparse;
	uint8_t layout, num_partitions;

	Worker_ID * row_assignments;

	explicit MatrixInfo() : ID(0), name(""), num_rows(1), num_cols(1), sparse(false), layout(0), num_partitions(0), row_assignments(nullptr) {
		row_assignments = new Worker_ID[num_rows]();
	}

	MatrixInfo(Matrix_ID _ID, uint64_t _num_rows, uint64_t _num_cols) :
		ID(_ID), name(""), num_rows(_num_rows), num_cols(_num_cols), sparse(false), layout(0), num_partitions(0), row_assignments(nullptr) {
		row_assignments = new Worker_ID[num_rows]();
	}

	MatrixInfo(Matrix_ID _ID, string _name, uint64_t _num_rows, uint64_t _num_cols) :
		ID(_ID), name(_name), num_rows(_num_rows), num_cols(_num_cols), sparse(false), layout(0), num_partitions(0), row_assignments(nullptr) {
		row_assignments = new Worker_ID[num_rows]();
	}
	MatrixInfo(Matrix_ID _ID, string _name, uint64_t _num_rows, uint64_t _num_cols, bool _sparse, uint8_t _layout, uint8_t _num_partitions) :
		ID(_ID), name(_name), num_rows(_num_rows), num_cols(_num_cols), sparse(_sparse), layout(0), num_partitions(_num_partitions), row_assignments(nullptr) {
		row_assignments = new Worker_ID[num_rows]();
	}

	~MatrixInfo() {
		delete [] row_assignments; row_assignments = nullptr;
	}

	string to_string(bool display_layout=false) const {
		std::stringstream ss;

		ss << "Matrix " << name << " (ID: " << ID << ", dim: " << num_rows << " x " << num_cols << ", sparse: " << (uint16_t) sparse << ", # partitions: " << (uint16_t) num_partitions << ")";
		if (display_layout) {
			ss << std::endl << "Layout: " << std::endl;
			for (uint64_t i = 0; i < num_rows; i++) ss << (uint16_t) row_assignments[i] << " ";
		}

		return ss.str();
	}
};

typedef std::shared_ptr<MatrixInfo> MatrixInfo_ptr;

//inline bool exist_test (const std::string & name) {
//    struct stat buffer;
//    return (stat(name.c_str(), & buffer) == 0);
//}
}	// namespace alchemist


#endif
