#ifndef ALCHEMIST__LIBRARYINFO_HPP
#define ALCHEMIST__LIBRARYINFO_HPP

#include <string>
#include "../Library.hpp"

namespace alchemist {

struct LibraryInfo {

	LibraryInfo(std::string _name, std::string _path, void * _lib_ptr, Library * _lib) :
		name(_name), path(_path), lib_ptr(_lib_ptr), lib(_lib) {}

	std::string name;
	std::string path;

	void * lib_ptr;

	Library * lib;
};

}

#endif
