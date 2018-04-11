#ifndef ALCHEMIST__LIBRARY_HPP
#define ALCHEMIST__LIBRARY_HPP

#include <El.hpp>
#include "mpi.h"
#include "Parameters.hpp"

namespace alchemist {

struct Library {

	Library(MPI_Comm & _world) : world(_world), grid(nullptr) { }

	virtual ~Library() { }

	MPI_Comm & world;
	std::shared_ptr<El::Grid> grid;

	virtual int load() = 0;
	virtual int unload() = 0;
	virtual int run(std::string, Parameters & in, Parameters & out) = 0;

	void load_grid(std::shared_ptr<El::Grid> _grid) { grid = _grid; }
};

typedef Library * create_t(MPI_Comm &);
typedef void destroy_t(Library *);

}

#endif
