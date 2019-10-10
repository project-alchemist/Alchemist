#include "Driver.hpp"

int main(int argc, char **argv)
{
	int alchemist_port = 24960;
	for (int i = 1; i < argc-1; i++) {
		if (strcmp(argv[i], "--port"))
			alchemist_port = (int) strtol(argv[i+1], NULL, 10);
	}

	int provided;
	MPI_Init_thread(&argc, &argv, 3, &provided);
	int threads_ok = provided >= 3;

	El::Initialize();

	// Get the rank and size in the original communicator
	int world_rank, world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	bool is_driver = world_rank == 0;

	io_context _io_context;

	try {
		if (is_driver) alchemist::Driver d(_io_context, alchemist_port);
		else alchemist::Worker w(_io_context, alchemist_port+world_rank);
	}
	catch (std::exception & e) {
		std::cout << "Exception while starting Alchemist: " << e.what() << std::endl;
	}

//	MPI_Barrier(world);

	El::Finalize();

	MPI_Finalize();

	return 0;
}

