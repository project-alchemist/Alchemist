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
#include <dlfcn.h>
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

typedef El::Matrix<double> Array;
typedef El::DistMatrix<double> DistMatrix;
typedef std::shared_ptr<El::AbstractDistMatrix<double>> DistMatrix_ptr;

typedef uint16_t WorkerID;
typedef uint16_t ClientID;
typedef uint16_t GroupID;
typedef uint16_t SessionID;
typedef uint16_t ArrayID;
typedef uint16_t MatrixID;
typedef uint16_t TaskID;
typedef uint8_t LibraryID;

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

enum { IS_PARAMETER = true };

struct Coordinate {
	Coordinate() : row(0), col(0) { }
	Coordinate(uint16_t _row, uint16_t _col) : row(_row), col(_col) { }

	uint16_t row, col;
};

struct WorkerInfo {
	WorkerInfo(WorkerID ID) :
		WorkerInfo(ID, string("0"), string("0"), 0, 0) { }
	WorkerInfo(WorkerID ID, string _hostname, string _address, uint16_t _port) :
		WorkerInfo(ID, _hostname, _address, _port, 0) { }
	WorkerInfo(WorkerID ID, string _hostname, string _address, uint16_t _port, GroupID groupID) :
		ID(ID), hostname(_hostname), address(_address), port(_port), groupID(groupID), grid_row(0), grid_col(0) { }

	WorkerID ID;
	string hostname, address;
	uint16_t port;
	GroupID groupID;
	uint16_t grid_row, grid_col;

	string to_string(bool include_allocation=true) const {
		std::stringstream ss;
		char buffer[4];

		sprintf(buffer, "%03d", ID);
		ss << "Worker-" << string(buffer) << " running on " << hostname << " at " << address << ":" << port;
		if (include_allocation)
			(groupID > 0) ? ss << " - ACTIVE (group " << groupID << ")" : ss << " - IDLE";

		return ss.str();
	}
};

struct Parameter {
	Parameter(string _name, datatype _dt, void * _p) : name(_name), dt(_dt), p(_p) { }

	string name;
	datatype dt;
	void * p;
};

typedef std::shared_ptr<Parameter> Parameter_ptr;

struct Library {

	Library(MPI_Comm & _world);

	virtual ~Library() { }

	Log_ptr log;

	MPI_Comm & world;

	virtual int load() = 0;
	virtual int unload() = 0;
	virtual int run(string & task_name, std::vector<Parameter_ptr> & in, std::vector<Parameter_ptr> & out) = 0;
};

typedef void * create_t(MPI_Comm &);
typedef void destroy_t(void *);

typedef std::shared_ptr<Library> Library_ptr;

class Parameter_ {
public:
	string name;
	datatype dt;
	void * value;

	Parameter_(string _name, datatype _dt, void * _value) {
		name = _name;
		dt = _dt;
		value = _value;
	}

	template <typename T>
	T * get_value() {
		return static_cast<T *>(value);
	}

	template <typename T>
	void set_value(T * _value) {
		value = static_cast<void *>(_value);
	}
};

struct MatrixInfo {
	MatrixID ID;

	string name;
	uint64_t num_rows, num_cols;
	uint8_t sparse;
	layout l;
	uint16_t num_grid_rows, num_grid_cols;

	map<WorkerID, Coordinate> grid;

	explicit MatrixInfo() : ID(0), name(""), num_rows(1), num_cols(1), sparse(0), l(MC_MR), num_grid_rows(1), num_grid_cols(1) { }

	MatrixInfo(MatrixID ID, uint64_t _num_rows, uint64_t _num_cols) :
		ID(ID), name(""), num_rows(_num_rows), num_cols(_num_cols), sparse(0), l(MC_MR), num_grid_rows(1), num_grid_cols(1) { }

	MatrixInfo(MatrixID ID, string _name, uint64_t _num_rows, uint64_t _num_cols) :
		ID(ID), name(_name), num_rows(_num_rows), num_cols(_num_cols), sparse(0), l(MC_MR), num_grid_rows(1), num_grid_cols(1) { }

	MatrixInfo(MatrixID ID, string _name, uint64_t _num_rows, uint64_t _num_cols, uint8_t _sparse, layout _l) :
		ID(ID), name(_name), num_rows(_num_rows), num_cols(_num_cols), sparse(_sparse), l(_l), num_grid_rows(1), num_grid_cols(1) { }

	MatrixInfo(MatrixID ID, string _name, uint64_t _num_rows, uint64_t _num_cols, uint8_t _sparse, layout _l, uint16_t _num_grid_rows, uint16_t _num_grid_cols) :
		ID(ID), name(_name), num_rows(_num_rows), num_cols(_num_cols), sparse(_sparse), l(_l), num_grid_rows(_num_grid_rows), num_grid_cols(_num_grid_cols) {
	}

	~MatrixInfo() { }

	void add_worker_assignment(WorkerID workerID, const Coordinate c) {
		grid.insert(std::make_pair(workerID, c));
	}

	string to_string(bool display_grid=false) const {
		std::stringstream ss;

		ss << "Matrix " << name << " (ID: " << ID << ", dim: " << num_rows << " x " << num_cols << ", sparse: " << (uint16_t) sparse << ")";
		if (display_grid) {
			ss << std::endl << "Grid (" << (uint16_t) num_grid_rows << " x " << (uint16_t) num_grid_cols << "):" << std::endl;
			for (auto it = grid.begin(); it != grid.end(); it++)
				ss << (uint16_t) it->first << ": " << it->second.row << ", " << it->second.col << "\n";
		}

		return ss.str();
	}
};

template <typename T>
struct MatrixBlock {
	MatrixBlock(uint64_t _rows[3], uint64_t _cols[3]) : i(0), start(nullptr), T_length(sizeof(T))
	{
		for (uint8_t i = 0; i < 3; i++) rows[i] = _rows[i];
		for (uint8_t i = 0; i < 3; i++) cols[i] = _cols[i];
		size = std::ceil((1.0*rows[1] - rows[0] + 1.0)/rows[2]) * std::ceil((1.0*cols[1] - cols[0] + 1.0)/cols[2]);
	}

	MatrixBlock(MatrixBlock<T> & block) :  i(0), start(nullptr), T_length(sizeof(T))
	{
		for (uint8_t i = 0; i < 3; i++) rows[i] = block.rows[i];
		for (uint8_t i = 0; i < 3; i++) cols[i] = block.cols[i];
		size = block.size;
	}

	~MatrixBlock() { }

	uint64_t i, size, rows[3], cols[3];
	size_t T_length;
	char * start;

	void read_next(T * value)
	{
		if (i < size) {
			memcpy(value, start + T_length*i, T_length);
			i += 1;
		}
		else value = nullptr;
	}

	void write_next(T * value)
	{
		if (i < size) {
			memcpy(start + T_length*i, value, T_length);
			i += 1;
		}
		else value = nullptr;
	}

	void reset_counter() { i = 0; }

	bool compare(T * A)
	{
		T temp;
		for (uint64_t i = 0; i < size; i++) {
			memcpy(&temp, start + T_length*i, T_length);
			if (temp != A[i]) return false;
		}
		return true;
	}

	double reverse_double(double * x)
	{
		char * x_char = (char *) x;
		char temp;
		temp = x_char[0]; x_char[0] = x_char[7]; x_char[7] = temp;
		temp = x_char[1]; x_char[1] = x_char[6]; x_char[6] = temp;
		temp = x_char[2]; x_char[2] = x_char[5]; x_char[5] = temp;
		temp = x_char[3]; x_char[3] = x_char[4]; x_char[4] = temp;
		x = (double *) x_char;

		return *x;
	}

	string to_string(bool print_data = true) {
		std::stringstream ss;
		T temp;
		double td = 0.0;

		ss << "Size: " << size << std::endl;
		ss << "Rows: start = " << rows[0] << ", end = " << rows[1] << ", skip = " << rows[2] << std::endl;
		ss << "Cols: start = " << cols[0] << ", end = " << cols[1] << ", skip = " << cols[2] << std::endl;
		if (print_data) {
			ss << "Data: ";
			uint64_t m = 0;
			if (size > 0) {
				for (uint64_t j = 0; j < std::ceil((1.0*rows[1] - rows[0] + 1)/rows[2]); j++) {
					for (uint64_t k = 0; k < std::ceil((1.0*cols[1] - cols[0] + 1)/cols[2]); k++) {

						memcpy(&temp, start + T_length*m, T_length);
						td = (double) temp;
						ss << reverse_double(&td) << " ";
						m++;
					}
					ss << std::endl;
				}
			}
		}

		return ss.str();
	}
};

struct ArrayInfo {
	ArrayID ID;

	string name;
	uint64_t num_rows, num_cols;
	uint8_t sparse, layout;
	uint16_t num_partitions;

	map<WorkerID, uint32_t> worker_assignments;

	explicit ArrayInfo() : ID(0), name(""), num_rows(1), num_cols(1), sparse(0), layout(0), num_partitions(1) { }

	ArrayInfo(ArrayID ID, uint64_t _num_rows, uint64_t _num_cols) :
		ID(ID), name(""), num_rows(_num_rows), num_cols(_num_cols), sparse(0), layout(0), num_partitions(1) { }

	ArrayInfo(ArrayID ID, string _name, uint64_t _num_rows, uint64_t _num_cols) :
		ID(ID), name(_name), num_rows(_num_rows), num_cols(_num_cols), sparse(0), layout(0), num_partitions(1) { }

	ArrayInfo(ArrayID ID, string _name, uint64_t _num_rows, uint64_t _num_cols, uint8_t _sparse, uint8_t _layout) :
		ID(ID), name(_name), num_rows(_num_rows), num_cols(_num_cols), sparse(_sparse), layout(0), num_partitions(1) { }

	ArrayInfo(ArrayID ID, string _name, uint64_t _num_rows, uint64_t _num_cols, uint8_t _sparse, uint8_t _layout, uint16_t _num_partitions) :
		ID(ID), name(_name), num_rows(_num_rows), num_cols(_num_cols), sparse(_sparse), layout(0), num_partitions(_num_partitions) {
	}

	~ArrayInfo() { }

	void add_worker_assignment(WorkerID workerID, uint64_t worker_assignment) {
		worker_assignments.insert(std::make_pair(workerID, worker_assignment));
	}

	string to_string(bool display_layout=false) const {
		std::stringstream ss;

		ss << "Array " << name << " (ID: " << ID << ", dim: " << num_rows << " x " << num_cols << ", sparse: " << (uint16_t) sparse << ", # partitions: " << (uint16_t) num_partitions << ")";
		if (display_layout) {
			ss << std::endl << "Layout: " << std::endl;
			for (auto it = worker_assignments.begin(); it != worker_assignments.end(); it++)
				ss << (uint16_t) it->first << " " << it->second << "\n";
		}

		return ss.str();
	}
};

template <typename T>
struct ArrayBlock {
	ArrayBlock(uint8_t _ndims) : i(0), size(0), nnz(0), ndims(_ndims), start(nullptr), T_length(sizeof(T))
	{
		for (int j = 0; j < 3; j++)
			dims[j] = new uint64_t[ndims];
	}

	ArrayBlock(ArrayBlock<T> & block) : i(0), size(block.size), nnz(block.nnz), ndims(block.ndims), start(nullptr), T_length(sizeof(T))
	{
		for (int j = 0; j < 3; j++) {
			dims[j] = new uint64_t[ndims];
			for (int i = 0; i < ndims; i++)
				dims[j][i] = block.dims[j][i];
		}

		size = 1;
		for (int i = 0; i < ndims; i++)
			size *= std::ceil((1.0*dims[1][i] - dims[0][i] + 1.0)/dims[2][i]);
	}

	~ArrayBlock()
	{
		for (int j = 0; j < 3; j++)
			delete [] dims[j];
	}

	uint64_t i, nnz, size, ndims;
	uint64_t* dims[3];
	size_t T_length = sizeof(T);

	char* start;

	void read_next(T * value)
	{
		if (i < size) {
			memcpy(value, start + T_length*i, T_length);
			i += 1;
		}
		else value = nullptr;
	}

	void write_next(T * value)
	{
		if (i < size) {
			memcpy(start + T_length*i, value, T_length);
			i += 1;
		}
		else value = nullptr;
	}

	void reset_counter() { i = 0; }

	bool compare(T * A)
	{
		T temp;
		for (uint64_t i = 0; i < size; i++) {
			memcpy(&temp, start + T_length*i, T_length);
			if (temp != A[i]) return false;
		}
		return true;
	}

	string to_string() const {
		std::stringstream ss;
		T temp;

		ss << "Size = " << size << std::endl;
		for (uint8_t j = 0; j < ndims; j++)
			ss << "Dim " << j+1 << ": start = " << dims[0][j] << ", end = " << dims[1][j] << ", skip = " << dims[2][j] << std::endl;
//		ss << "Data: ";
//		uint64_t m = 0;
//		if (size > 0) {
//			for (uint64_t j = 0; j < std::ceil((1.0*dims[1][0] - dims[0][0] + 1)/dims[2][0]); j++) {
//				for (uint64_t k = 0; k < std::ceil((1.0*dims[1][1] - dims[0][1] + 1)/dims[2][1]); k++) {
//
//					memcpy(&temp, start + T_length*m, T_length);
//					ss << m << " " << temp << " ";
//					m++;
//				}
//				ss << std::endl;
//			}
//		}

		return ss.str();
	}
};

typedef std::shared_ptr<WorkerInfo> WorkerInfo_ptr;
typedef std::shared_ptr<ArrayInfo> ArrayInfo_ptr;
typedef std::shared_ptr<ArrayBlock<float>> FloatArrayBlock_ptr;
typedef std::shared_ptr<ArrayBlock<double>> DoubleArrayBlock_ptr;
typedef std::shared_ptr<MatrixInfo> MatrixInfo_ptr;
typedef std::shared_ptr<MatrixBlock<double>> MatrixBlock_ptr;

//inline bool exist_test (const std::string & name) {
//    struct stat buffer;
//    return (stat(name.c_str(), & buffer) == 0);
//}
}	// namespace alchemist


#endif
