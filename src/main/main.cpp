#include "Driver.hpp"


using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {

	MPI_Init(NULL, NULL);

	El::Initialize();

	MPI_Comm peers, world = MPI_COMM_WORLD;

	// Get the rank and size in the original communicator
	int world_rank, world_size;
	MPI_Comm_rank(world, &world_rank);
	MPI_Comm_size(world, &world_size);

	bool is_driver = world_rank == 0;
	int color = is_driver ? 0 : 1; // Determine color based on row

	// Split the communicator based on the color and use the original rank for ordering
	MPI_Comm_split(world, color, world_rank, &peers);

	int peers_rank, peers_size;
	MPI_Comm_rank(peers, &peers_rank);
	MPI_Comm_size(peers, &peers_size);

//	char name[200];
//	int resultlen;

//	MPI_Get_processor_name(&name[0], &resultlen);
//	name[resultlen] = '\0';

//	std::string str(name, resultlen);
//
//	for (int i = 0; i < resultlen; i++)
//		printf("%c", name[i]);
//	std::cout << str;
//	printf(" %s \n", str.c_str());
//	str.append('/0');
//	printf("%s", str);

//	printf("WORLD RANK/SIZE: %d/%d \t PEERS RANK/SIZE: %d/%d\n", world_rank, world_size, peers_rank, peers_size);

	io_context _io_context;

	try {
		if (is_driver)
			alchemist::Driver d(world, peers, _io_context, ALCHEMIST_PORT);
		else
			alchemist::Worker w(world, peers, _io_context, ALCHEMIST_PORT+world_rank);
	}
	catch (std::exception & e) {
		std::cerr << "Exception while starting Alchemist: " << e.what() << std::endl;
	}

	MPI_Barrier(world);

	//alchemist::Worker(world, peers).run()

//	if (isDriver)
////		std::shared_ptr<spdlog::logger> log = start_log("alchemist");
//
////		log->info("Started Alchemist");
//
//		printf("Starting Alchemist\n");
//
//		char machine[255];
//		char port[255];
//
//		if (argc == 3) { // we are on a non-NERSC system, so passed in Spark driver machine name and port
////			log->info("Non-NERSC system assumed");
////			log->info("Connecting to Spark executor at {}:{}", argv[1], argv[2]);
//			std::strcpy(machine, argv[1]);
//			std::strcpy(port, argv[2]);
//		}
////		else { // assume we are on NERSC, so look in a specific location for a file containing the machine name and port
////			char const* tmp = std::getenv("SPARK_WORKER_DIR");
////			std::string sock_path;
////			if (tmp == NULL) {
//////				log->info("Couldn't find the SPARK_WORKER_DIR variable");
//////				world.abort(1);
////			} else {
////				sock_path = std::string(tmp) + "/connection.info";
////			}
//////			log->info("NERSC system assumed");
//////			log->info("Searching for connection information in file {}", sock_path);
////
////			while (!exist_test(sock_path)) {
////				boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
////			}
////			// now wait for a while for the connection file to be completely written, hopefully is enough time
////			// TODO: need a more robust way of ensuring this is the case
////			boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
////
////			std::string sock_spec;
////			std::ifstream infile(sock_path);
////			std::getline(infile, sock_spec);
////			infile.close();
////			boost::tokenizer<> tok(sock_spec);
////			boost::tokenizer<>::iterator iter = tok.begin();
////			std::string machine_str = *iter;
////			std::string port_str = *(++iter);
////
//////			log->info("Connecting to Spark executor at {}:{}", machine_str, port_str);
////			strcpy(machine, machine_str.c_str());
////			strcpy(port, port_str.c_str());
////		}
//
////		boost::asio::ip::tcp::iostream stream(machine, port);
////		ENSURE(stream);
////		stream.rdbuf()->non_blocking(false);
//
////		status = Driver(world, peers, stream, stream).run();
//
////		log->info("Alchemist has exited");
//	}
//	else {
//		printf("Calling worker\n");
//		//status = Worker(world, peers).run();
//	}

	El::Finalize();

	MPI_Comm_free(&peers);
	MPI_Finalize();

	return 0;
}

