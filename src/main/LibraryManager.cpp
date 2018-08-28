#include "LibraryManager.hpp"

namespace alchemist {

//void Executor::set_log(Log_ptr _log)
//{
//	log = _log;
//}

//LibraryManager::LibraryManager() { }
//
LibraryManager::LibraryManager(Log_ptr & _log) : log(_log) { }

int LibraryManager::load_library(MPI_Comm & group, string library_name, string library_path) {

	char * cstr = new char [library_path.length()+1];
	std::strcpy(cstr, library_path.c_str());

	log->info("Loading library {} located at {}", library_name, library_path);

	void * lib = dlopen(library_path.c_str(), RTLD_NOW);
	if (lib == NULL) {
		log->info("dlopen failed: {}", dlerror());

		return -1;
	}

	dlerror();			// Reset errors

    create_t * create_library = (create_t*) dlsym(lib, "create");
    const char * dlsym_error = dlerror();
    	if (dlsym_error) {
//    		log->info("dlsym with command \"load\" failed: {}", dlerror());

//        	delete [] cstr;
//        	delete dlsym_error;

    		return -1;
    	}

    	Library * library = create_library(group);

//    	libraries.insert(std::make_pair(library_name, LibraryInfo(library_name, library_path, lib, library)));

    	library->load();

    	Parameters input;
    	Parameters output;

    	library->run("greet", input, output);

    	log->info("Library {} loaded", library_name);

    	delete [] cstr;
    	delete dlsym_error;

    	return 0;
}

int LibraryManager::run_task(std::string args, Parameters & output_parameters) {

//	log->info("Received: {}", args);
//
//	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
//	boost::char_separator<char> sep(" ");
//	tokenizer tok(args, sep);
//
//	tokenizer::iterator iter = tok.begin();
//
//	Parameters input_parameters = Parameters();
//	std::string input_parameter;
//
//	std::string library_name = *iter;
//	std::string task_name = *(++iter);
//
//	log->info("Received: {}", library_name);
//	log->info("Received: {}", task_name);
//
//	for (++iter; iter != tok.end(); ++iter) {
//		input_parameter = *(iter);
//		log->info("Received: {}", input_parameter);
//		deserialize_parameters(input_parameter, input_parameters);
//	}

//	process_input_parameters(input_parameters);
//
//	log->info("Calling {}::run with '{}'", library_name, task_name);
//
//	libraries.find(library_name)->second.lib->run(task_name, input_parameters, output_parameters);
//
//	log->info("Finished call to {}::run with '{}'", library_name, task_name);
//
////	world.barrier();
//	MPI_Barrier(world);
//
//	process_output_parameters(output_parameters);

	return 0;
}

int LibraryManager::unload_library(string library_name) {


	return 0;
}

int LibraryManager::unload_libraries() {

//	for (auto const & library : libraries) {
//
////		log->info("Closing library {}", library.second.name);
//
//		destroy_t* destroy_library = (destroy_t*) dlsym(library.second.lib_ptr, "destroy");
//	    const char* dlsym_error = dlerror();
//	    	if (dlsym_error) {
////	    		log->info("dlsym with command \"close\" failed: {}", dlerror());
//	    		return -1;
//	    	}
//
//	    	destroy_library(library.second.lib);
//	    	dlclose(library.second.lib_ptr);
//	}

	return 0;
}

void LibraryManager::deserialize_parameters(std::string input_parameter, Parameters & input_parameters) {

//	boost::char_separator<char> sep("()");
//	boost::tokenizer<boost::char_separator<char> > tok(input_parameter, sep);
//	auto tok_iter = tok.begin();
//
//	std::string parameter_name = *tok_iter;
//	std::string parameter_type = *(++tok_iter);
//	std::string parameter_value = *(++tok_iter);
//
//	if (parameter_type.compare("i") == 0)
//		input_parameters.add_int(parameter_name, std::stoi(parameter_value));
//	else if (parameter_type.compare("l") == 0)
//		input_parameters.add_long(parameter_name, std::stol(parameter_value));
//	else if (parameter_type.compare("ll") == 0)
//		input_parameters.add_longlong(parameter_name, std::stoll(parameter_value));
//	else if (parameter_type.compare("u") == 0)
//		input_parameters.add_unsigned(parameter_name, std::stoi(parameter_value));
//	else if (parameter_type.compare("ul") == 0)
//		input_parameters.add_unsignedlong(parameter_name, std::stoul(parameter_value));
//	else if (parameter_type.compare("ull") == 0)
//		input_parameters.add_unsignedlonglong(parameter_name, std::stoull(parameter_value));
//	else if (parameter_type.compare("ld") == 0)
//		input_parameters.add_longdouble(parameter_name, std::stold(parameter_value));
//	else if (parameter_type.compare("d") == 0)
//		input_parameters.add_double(parameter_name, std::stod(parameter_value));
//	else if (parameter_type.compare("f") == 0)
//		input_parameters.add_float(parameter_name, std::stof(parameter_value));
//	else if (parameter_type.compare("b") == 0)
//		input_parameters.add_bool(parameter_name, parameter_value.compare("t") == 0);
//	else if (parameter_type.compare("c") == 0)
//		input_parameters.add_char(parameter_name, parameter_value[0]);
//	else if (parameter_type.compare("s") == 0)
//		input_parameters.add_string(parameter_name, parameter_value);
//	else if (parameter_type.compare("mh") == 0)
//		input_parameters.add_matrixhandle(parameter_name, std::stoi(parameter_value));
}

std::string LibraryManager::serialize_parameters(const Parameters & output) const {

	return output.to_string();
}

}
