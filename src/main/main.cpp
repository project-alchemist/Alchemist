#include "Driver.hpp"

#define MIN_BUFFER_LENGTH 1000000

int main(int argc, char **argv)
{
	int32_t start_port = 24960;
	int64_t max_buffer_length = 0;				// No maximum buffer length set
	std::string output_dir = "";

	bool early_exit = false;

	for (int i = 1; i < argc-1; i += 2) {
		if (strcmp(argv[i], "--port") == 0)
			start_port = (int32_t) strtol(argv[i+1], NULL, 10);
		if (strcmp(argv[i], "--output_dir") == 0 || strcmp(argv[i], "--output-dir") == 0)
			output_dir = argv[i+1];
		if (strcmp(argv[i], "--max_buffer_length") == 0 || strcmp(argv[i], "--max-buffer-length") == 0)
			max_buffer_length = (int64_t) strtol(argv[i+1], NULL, 10);
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

	if (start_port < 1024 || start_port > 65535 - world_size + 1) {
		if (is_driver)
			fprintf(stderr, "ERROR: Alchemist starting port number must be between 1024 and %d\n", 65535 - world_size + 1);
		early_exit = true;
	}
	if (max_buffer_length < MIN_BUFFER_LENGTH && max_buffer_length != 0) {
		if (is_driver)
			fprintf(stderr, "ERROR: Maximum buffer length must be greater than %d (enter '0' for unspecified buffer length)\n", MIN_BUFFER_LENGTH);
		early_exit = true;
	}

	if (!early_exit) {
		try {
			if (is_driver) alchemist::Driver d(_io_context, start_port, max_buffer_length, output_dir);
			else alchemist::Worker w(_io_context, start_port+world_rank);
		}
		catch (std::exception & e) {
			std::cout << "Exception while starting Alchemist: " << e.what() << std::endl;
		}
	}

//	MPI_Barrier(world);

	El::Finalize();

	MPI_Finalize();

	return 0;
}

