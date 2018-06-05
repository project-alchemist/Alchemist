#ifndef ALCHEMIST__LOGGING_HPP
#define ALCHEMIST__LOGGING_HPP

#include <string>
#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"

namespace alchemist {

#define SPACE "                                              ";
#define TAB "    ";

typedef std::shared_ptr<spdlog::logger> Log_ptr;

inline Log_ptr start_log(std::string name)
{
	std::string logfile_name = name + ".log";

	Log_ptr log;
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::ansicolor_stderr_sink_st>());
	sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_st>(logfile_name));
	log = std::make_shared<spdlog::logger>(name, std::begin(sinks), std::end(sinks));
	log->flush_on(spdlog::level::info);
	log->set_level(spdlog::level::info); // only log stuff at or above info level, for production
	return log;
}

}				// namespace alchemist

#endif			// ALCHEMIST__LOGGING_HPP
