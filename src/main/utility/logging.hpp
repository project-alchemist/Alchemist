#ifndef ALCHEMIST__LOGGING_HPP
#define ALCHEMIST__LOGGING_HPP

#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/ansicolor_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace alchemist {

#define SPACE "                                                 ";
#define TAB "    ";

// Formatting codes
const std::string regular = "";
const std::string reset = "\033[m";
const std::string bold = "\033[1m";
const std::string dark = "\033[2m";
const std::string italic = "\033[3m";
const std::string underline = "\033[4m";
const std::string blink = "\033[5m";
const std::string reverse = "\033[7m";
const std::string concealed = "\033[8m";
const std::string clear_line = "\033[K";

// Foreground colors
const std::string black = "\033[30m";
const std::string red = "\033[31m";
const std::string green = "\033[32m";
const std::string yellow = "\033[33m";
const std::string blue = "\033[34m";
const std::string magenta = "\033[35m";
const std::string cyan = "\033[36m";
const std::string white = "\033[37m";
const std::string iblack = "\033[90m";
const std::string ired = "\033[91m";
const std::string igreen = "\033[92m";
const std::string iyellow = "\033[93m";
const std::string iblue = "\033[94m";
const std::string ipurple = "\033[95m";
const std::string icyan = "\033[96m";
const std::string iwhite = "\033[97m";

// Background colors
const std::string on_black = "\033[40m";
const std::string on_red = "\033[41m";
const std::string on_green = "\033[42m";
const std::string on_yellow = "\033[43m";
const std::string on_blue = "\033[44m";
const std::string on_magenta = "\033[45m";
const std::string on_cyan = "\033[46m";
const std::string on_white = "\033[47m";
const std::string on_iblack = "\033[100m";
const std::string on_ired = "\033[101m";
const std::string on_igreen = "\033[102m";
const std::string on_iyellow = "\033[103m";
const std::string on_iblue = "\033[104m";
const std::string on_ipurple = "\033[105m";
const std::string on_icyan = "\033[106m";
const std::string on_iwhite = "\033[107m";

typedef std::shared_ptr<spdlog::logger> Log_ptr;

inline Log_ptr start_log(std::string name, std::string pattern, std::string format=regular, std::string fore_color="", std::string back_color="")
{
	std::string logfile_name = name + ".log";

	auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
	console_sink->set_level(spdlog::level::info);
	console_sink->set_pattern(pattern);
	console_sink->set_color(spdlog::level::info, format + fore_color + back_color);

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
