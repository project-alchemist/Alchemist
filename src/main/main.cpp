#include "Driver.hpp"

int main(int argc, char **argv)
{
	int provided;
	MPI_Init_thread(&argc, &argv, 3, &provided);
	int threads_ok = provided >= 3;

	El::Initialize();

	MPI_Comm world = MPI_COMM_WORLD;

	// Get the rank and size in the original communicator
	int world_rank, world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(world, &world_size);

	bool is_driver = world_rank == 0;

	io_context _io_context;

	try {
		if (is_driver) alchemist::Driver d(_io_context, ALCHEMIST_PORT);
		else alchemist::Worker w(_io_context, ALCHEMIST_PORT+world_rank);
	}
	catch (std::exception & e) {
		std::cout << "Exception while starting Alchemist: " << e.what() << std::endl;
	}

//	MPI_Barrier(world);

	El::Finalize();

	MPI_Finalize();

	return 0;
}

