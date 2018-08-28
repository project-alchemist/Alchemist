#ifndef ALCHEMIST__LIBRARYMANAGER_HPP
#define ALCHEMIST__LIBRARYMANAGER_HPP

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
		name(" "), path(" "), lib(nullptr) { }
	LibraryInfo(std::string _name, std::string _path, void * _lib_ptr, Library_ptr _lib) :
		name(_name), path(_path), lib(_lib) { }

	string name;
	string path;

	Library_ptr lib;
};

class LibraryManager : public std::enable_shared_from_this<LibraryManager>
{
public:
//	MPI_Comm & group;
//	MPI_Comm & peers;

	std::map<std::string, LibraryInfo> libraries;
//
//	LibraryManager();
	LibraryManager(Log_ptr &);

//	virtual int process_input_parameters(Parameters &) = 0;
//	virtual int process_output_parameters(Parameters &) = 0;

	int load_library(MPI_Comm & group, string library_name, string library_path);
	int run_task(string library_name, Parameters &);
	int unload_library(string library_name);

	int unload_libraries();

//	int receive_new_matrix();
//	int get_transpose();
//	int matrix_multiply();
//	int get_matrix_rows();
//	int read_HDF5();

	void deserialize_parameters(std::string, Parameters &);
	std::string serialize_parameters(const Parameters &) const;

private:
	Log_ptr log;
};

}

#endif
