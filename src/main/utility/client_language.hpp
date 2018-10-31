#ifndef ALCHEMIST__CLIENT_LANGUAGE_HPP
#define ALCHEMIST__CLIENT_LANGUAGE_HPP

#include <string>
#include <cstdlib>

namespace alchemist {

typedef enum _client_language : uint8_t {
	C = 0,
	CPP,
	SCALA,
	JAVA,
	PYTHON,
	JULIA
} client_language;

inline const std::string get_client_language_name(const client_language & cl)
{
	switch (cl) {
		case C:
			return "C";
		case CPP:
			return "C++";
		case SCALA:
			return "Scala";
		case JAVA:
			return "Java";
		case PYTHON:
			return "Python";
		case JULIA:
			return "Julia";
		}

	return "Unknown language";
}

}			// namespace alchemist

#endif
