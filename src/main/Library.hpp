#ifndef ALCHEMIST__LIBRARY_HPP
#define ALCHEMIST__LIBRARY_HPP

#include <dlfcn.h>
#include "Parameters.hpp"

namespace alchemist {

struct Library {

	Library(MPI_Comm & _world) : world(_world) { }

	virtual ~Library() { }

	MPI_Comm & world;

	virtual int load() = 0;
	virtual int unload() = 0;
	virtual int run(string & task_name, Parameters & in, Parameters & out) = 0;
};

typedef Library * create_t(MPI_Comm &);
typedef void destroy_t(Library *);

typedef std::shared_ptr<Library> Library_ptr;

}

#endif
