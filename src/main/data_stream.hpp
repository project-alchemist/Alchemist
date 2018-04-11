#ifndef ALCHEMIST__DATA_STREAM_H
#define ALCHEMIST__DATA_STREAM_H

#include "Alchemist.hpp"
#include <iostream>
#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/tokenizer.hpp>

#define PORT 4960

namespace alchemist {

union DoubleBytes {
	uint64_t qword;
	double value;
};

inline uint64_t htond(double value) {
	DoubleBytes d;
	d.value = value;
	return be64toh(d.qword);
}

inline double ntohd(uint64_t qword) {
	DoubleBytes d;
	d.qword = htobe64(qword);
	return d.value;
}

struct DataInputStream {

//	std::shared_ptr<std::istream> is;
	std::istream * is;

	struct IOError : std::runtime_error {
		IOError() : std::runtime_error("DataInputStream::IOError") { }
	};

//	DataInputStream(std::shared_ptr<std::istream> _is) : is(_is) { }
	DataInputStream(std::istream * _is) : is(_is) { }

	DataInputStream() : is(nullptr) { }

	uint32_t read_int() {
		uint32_t val;
		is->read(reinterpret_cast<char*>(&val), sizeof(val));
		if (!is) throw IOError();
		val = ntohl(val);
		return val;
	}

	uint64_t read_long() {
		uint64_t val;
		is->read(reinterpret_cast<char*>(&val), sizeof(val));
		if (!is) throw IOError();
		val = htobe64(val);
		return val;
	}

	double read_double() {
		uint64_t val;
		is->read(reinterpret_cast<char*>(&val), sizeof(val));
		if (!is) throw IOError();
		return ntohd(val);
	}

	std::string read_string() {
		uint64_t stringLen = read_long();
		char * strin = new char [stringLen + 1];
		is->read(strin, stringLen);
		if (!is) throw IOError();
		strin[stringLen]='\0';
		std::string result = std::string(strin);
		delete[] strin;
		return result;
	}
};

struct DataOutputStream {

	std::ostream * os;

	struct IOError : std::runtime_error {
		IOError() : std::runtime_error("DataOutputStream::IOError") { }
	};

	DataOutputStream(std::ostream * _os) : os(_os) { }

	DataOutputStream() : os(nullptr) { }

	void write_int(uint32_t val) {
		val = htonl(val);
		os->write(reinterpret_cast<const char*>(&val), sizeof(val));
		if (!os) throw IOError();
	}

	void write_long(uint64_t val) {
		val = be64toh(val);
		os->write(reinterpret_cast<const char*>(&val), sizeof(val));
		if (!os) throw IOError();
	}

	void write_double(double val) {
		uint64_t word = htond(val);
		os->write(reinterpret_cast<const char*>(&word), sizeof(word));
		if (!os) throw IOError();
	}

	void write_string(const std::string &s) {
		write_long(s.size());
		os->write(&s[0], s.size());
		if (!os) throw IOError();
	}

	void flush() {
		os->flush();
		if (!os) throw IOError();
	}
};

} // namespace alchemist

#endif
