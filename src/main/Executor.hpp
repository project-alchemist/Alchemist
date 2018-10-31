#ifndef ALCHEMIST__EXECUTOR_HPP
#define ALCHEMIST__EXECUTOR_HPP

#include <omp.h>
#include <dlfcn.h>
#include <ctime>
#include <iostream>
#include <string>
#include <unistd.h>
#include "Alchemist.hpp"
#include "utility/logging.hpp"

namespace alchemist {

using std::string;

typedef std::shared_ptr<Library> Library_ptr;

struct LibraryInfo {
	LibraryInfo() :
		name(" "), path(" "), lib_ptr(nullptr), lib(nullptr) { }
	LibraryInfo(std::string _name, std::string _path, void * _lib_ptr, Library_ptr _lib) :
		name(_name), path(_path), lib_ptr(_lib_ptr), lib(_lib) { }

	std::string name;
	std::string path;

	void * lib_ptr;

	Library_ptr lib;
};

class Executor : public std::enable_shared_from_this<Executor>
{
public:
	std::map<std::string, LibraryInfo> libraries;

	virtual ~Executor() {}

//	void set_log(Log_ptr _log);

//	virtual int process_input_parameters(Parameters &) = 0;
//	virtual int process_output_parameters(Parameters &) = 0;

	int load_library(std::string);
	int run_task(std::string, Parameters &);
	int unload_libraries();

//	int receive_new_matrix();
//	int get_transpose();
//	int matrix_multiply();
//	int get_matrix_rows();
//	int read_HDF5();

	void deserialize_parameters(std::string, Parameters &);
	std::string serialize_parameters(const Parameters &) const;
//
//private:
//	Log_ptr log;
};

}

#endif
