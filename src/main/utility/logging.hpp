#ifndef ALCHEMIST__LOGGING_HPP
#define ALCHEMIST__LOGGING_HPP

#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace alchemist {

#define SPACE "                                              ";
#define TAB "    ";

typedef std::shared_ptr<spdlog::logger> Log_ptr;

inline Log_ptr start_log(std::string name, std::string pattern)
{
	std::string logfile_name = name + ".log";

	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
	console_sink->set_level(spdlog::level::info);
	console_sink->set_pattern(pattern);

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(logfile_name, true);
	file_sink->set_level(spdlog::level::trace);

	Log_ptr log;
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(console_sink);
	sinks.push_back(file_sink);
	log = std::make_shared<spdlog::logger>(name, std::begin(sinks), std::end(sinks));
	return log;
}

}				// namespace alchemist

#endif			// ALCHEMIST__LOGGING_HPP
