#ifndef ALCHEMIST__MESSAGE_HPP
#define ALCHEMIST__MESSAGE_HPP

// Structure of Message:
//
//  client_command code (4 bytes)
//  Header (4 bytes - contains length of data format section)
//  Body (Variable bytes - contains data)

#include <cstdio>
#include <cstring>
#include <iostream>

#include "utility/command.hpp"
#include "utility/datatype.hpp"

namespace alchemist {

class Message
{
public:
	enum { header_length = 5 };
	enum { max_body_length = 65536 };

	Message() : cc(WAIT), body_length(0), read_index(0) {  }

	client_command cc;
	uint32_t body_length;
	uint32_t read_index;

	const uint32_t get_max_body_length() const { return max_body_length; }

	char data[header_length + max_body_length];

	const uint32_t length() const
	{
		return body_length + header_length;
	}

	const char * header() const
	{
		return &data[0];
	}

	char * header()
	{
		return &data[0];
	}

	const char * body() const
	{
		return &data[header_length];
	}

	char * body()
	{
		return &data[header_length];
	}

	bool decode_header()
	{
		memcpy(&cc, &data[0], sizeof(client_command));

		memcpy(&body_length, &data[sizeof(client_command)], 4);
		if (body_length > max_body_length) {
			body_length = 0;
			return false;
		}

		return true;
	}

	void clear()
	{
		cc = WAIT;
		body_length = 0;
		read_index = 0;
	}

	void start(const client_command & cc)
	{
		clear();
		add_client_command(cc);
	}

	void add_client_command(const client_command & cc)
	{
		memcpy(&data[0], &cc, sizeof(client_command));
	}

	void add_string(const std::string & _data)
	{
		auto string_length = _data.length();
		auto cdata = _data.c_str();

		datatype dt = STRING;

		memcpy(data + header_length + body_length, &dt, sizeof(datatype));
		body_length += sizeof(datatype);
		memcpy(data + header_length + body_length, &string_length, 4);
		body_length += 4;

		memcpy(data + header_length + body_length, cdata, string_length);
		body_length += string_length;

		memcpy(data + sizeof(client_command), &body_length, 4);
	}

	void add_unsigned_char(const unsigned char & data)
	{
		add(&data, 1, CHAR);
	}

	void add_uint16(const uint16_t & data)
	{
		add(&data, 1, UINT16_T);
	}

	void add_uint32(const uint32_t & data)
	{
		add(&data, 1, UINT32_T);
	}

	void add_float(const float & data)
	{
		add(&data, 1, FLOAT);
	}

	void add_double(const double & data)
	{
		add(&data, 1, DOUBLE);
	}

	void add_double(const double & data, uint32_t length)
	{
		add(&data, length, DOUBLE);
	}

	void add(const void * _data, const uint32_t & length, const datatype & dt)
	{
		memcpy(data + header_length + body_length, &dt, sizeof(datatype));
		body_length += sizeof(datatype);
		memcpy(data + header_length + body_length, &length, 4);
		body_length += 4;

		uint32_t data_length = get_word_length(length, dt);

		memcpy(data + header_length + body_length, _data, data_length);
		body_length += data_length;

		memcpy(data + sizeof(client_command), &body_length, 4);
	}

	uint32_t get_word_length(const uint32_t & length, const datatype & dt)
	{
		return get_datatype_length(dt)*length;
	}

	const datatype next_datatype()
	{
		datatype dt;

		memcpy(&dt, &data[header_length + read_index], sizeof(datatype));

		return dt;
	}

	const uint32_t next_data_length()
	{
		uint32_t data_length;

		memcpy(&data_length, &data[header_length + read_index + sizeof(datatype)], 4);

		return data_length;
	}

	inline void read_atom(const uint32_t & i, void * a, int psize)
	{
		memcpy(a, data + i, psize);
	}

	const signed char read_signed_char(const uint32_t & i)
	{
		signed char x;
		read_atom(i, &x, 1);

		return x;
	}

	const unsigned char read_unsigned_char(const uint32_t & i)
	{
		unsigned char x;
		read_atom(i, &x, 1);

		return x;
	}

	const signed char read_byte(const uint32_t & i)
	{
		signed char x;
		read_atom(i, &x, 1);

		return x;
	}

	const signed char read_char(const uint32_t & i)
	{
		signed char x;
		read_atom(i, &x, 1);

		return x;
	}

	const signed char read_character(const uint32_t & i)
	{
		signed char x;
		read_atom(i, &x, 1);

		return x;
	}

	const bool read_bool(const uint32_t & i)
	{
		bool x;
		read_atom(i, &x, 1);

		return x;
	}

	const bool read_logical(const uint32_t & i)
	{
		bool x;
		read_atom(i, &x, 1);

		return x;
	}

	const short int read_short(const uint32_t & i)
	{
		short int x;
		read_atom(i, &x, 2);

		return x;
	}

	const unsigned short int read_unsigned_short(const uint32_t & i)
	{
		unsigned short int x;
		read_atom(i, &x, 2);

		return x;
	}

	const long int read_long(const uint32_t & i)
	{
		long int x;
		read_atom(i, &x, 4);

		return x;
	}

	const unsigned long int read_unsigned_long(const uint32_t & i)
	{
		unsigned long int x;
		read_atom(i, &x, 4);

		return x;
	}

	const int8_t read_integer1(const uint32_t & i)
	{
		int8_t x;
		read_atom(i, &x, 1);

		return x;
	}

	const int16_t read_integer2(const uint32_t & i)
	{
		int16_t x;
		read_atom(i, &x, 2);

		return x;
	}

	const int32_t read_integer4(const uint32_t & i)
	{
		int32_t x;
		read_atom(i, &x, 4);

		return x;
	}

	const int64_t read_integer8(const uint32_t & i)
	{
		int64_t x;
		read_atom(i, &x, 8);

		return x;
	}

	const int8_t read_int8(const uint32_t & i)
	{
		int8_t x;
		read_atom(i, &x, 1);

		return x;
	}

	const uint8_t read_uint8(const uint32_t & i)
	{
		uint8_t x;
		read_atom(i, &x, 1);

		return x;
	}

	const int16_t read_int16(const uint32_t & i)
	{
		int16_t x;
		read_atom(i, &x, 2);

		return x;
	}

	const uint16_t read_uint16()
	{
		uint16_t x;

		read_index += sizeof(datatype);
		read_index += 4;

		memcpy(&x, &data[header_length + read_index], 2);
		read_index += 2;

		return x;
	}

	const void read_uint16(uint16_t * x)
	{
		uint32_t data_length;

		read_index += sizeof(datatype);
		memcpy(&data_length, &data[header_length + read_index], 4);
		read_index += 4;

		memcpy(x, &data[header_length + read_index], 2*data_length);

		read_index += 2*data_length;
	}

	const uint16_t read_uint16(const uint32_t & i)
	{
		uint16_t x;
		read_atom(i, &x, 2);

		return x;
	}

	const int32_t read_int32(const uint32_t & i)
	{
		int32_t x;
		read_atom(i, &x, 4);

		return x;
	}

	const uint32_t read_uint32(const uint32_t & i)
	{
		uint32_t x;
		read_atom(i, &x, 4);

		return x;
	}

	const int64_t read_int64(const uint32_t & i)
	{
		int64_t x;
		read_atom(i, &x, 8);

		return x;
	}

	const uint64_t read_uint64(const uint32_t & i)
	{
		uint64_t x;
		read_atom(i, &x, 8);

		return x;
	}

	const float read_float(const uint32_t & i)
	{
		float x;
		read_atom(i, &x, 4);

		return x;
	}

	const double read_double(const uint32_t & i)
	{
		double x;
		read_atom(i, &x, 8);

		return x;
	}

	const double read_real(const uint32_t & i)
	{
		return read_double(i);
	}

	const std::string read_string()
	{
		uint32_t data_length;

		read_index += sizeof(datatype);
		memcpy(&data_length, &data[header_length + read_index], 4);
		read_index += 4;

		char cc[data_length+1];
		memcpy(cc, &data[header_length + read_index], data_length);
		cc[data_length] = '\0';
		read_index += data_length;

		return std::string(cc);
	}

	const std::string read_string(const uint32_t & i, const uint32_t & length)
	{
		char cc[length];
		memcpy(cc, &data[i], length);

		return std::string(cc);
	}

	const void read_next(std::stringstream & ss, uint32_t i, const datatype & dt)
	{
		switch (dt) {
			case BYTE:
				ss << read_byte(i);
				break;
			case CHAR:
				ss << read_signed_char(i);
				break;
			case UNSIGNED_CHAR:
				ss << read_unsigned_char(i);
				break;
			case SIGNED_CHAR:
				ss << read_signed_char(i);
				break;
			case CHARACTER:
				ss << read_character(i);
				break;
			case BOOL:
				ss << read_bool(i);
				break;
			case LOGICAL:
				ss << read_logical(i);
				break;
			case SHORT:
				ss << read_short(i);
				break;
			case UNSIGNED_SHORT:
				ss << read_unsigned_short(i);
				break;
			case LONG:
				ss << read_long(i);
				break;
			case UNSIGNED_LONG:
				ss << read_unsigned_long(i);
				break;
			case INTEGER1:
				ss << read_integer1(i);
				break;
			case INT8_T:
				ss << read_int8(i);
				break;
			case UINT8_T:
				ss << read_uint8(i);
				break;
			case INTEGER2:
				ss << read_integer2(i);
				break;
			case INT16_T:
				ss << read_int16(i);
				break;
			case UINT16_T:
				ss << read_uint16(i);
				break;
			case INTEGER4:
				ss << read_integer4(i);
				break;
			case INT32_T:
				ss << read_int32(i);
				break;
			case UINT32_T:
				ss << read_uint32(i);
				break;
			case INTEGER8:
				ss << read_integer8(i);
				break;
			case INT64_T:
				ss << read_int64(i);
				break;
			case UINT64_T:
				ss << read_uint64(i);
				break;
			case FLOAT:
				ss << read_float(i);
				break;
			case DOUBLE:
				ss << read_double(i);
				break;
			case REAL:
				ss << read_real(i);
				break;
			default:
				ss << "Invalid datatype";
				break;
			}
	}

	const std::string to_string()
	{
		std::stringstream ss;

		client_command _cc;
		uint32_t _body_length;
		memcpy(&_cc, &data[0], sizeof(client_command));
		memcpy(&_body_length, &data[sizeof(client_command)], 4);

		ss << "==============================================" << std::endl;
		ss << "Command code:        " << _cc << " (" << get_command_name(_cc) << ") " << std::endl;
		ss << "Message body length: " << _body_length << std::endl;
		ss << "----------------------------------------------" << std::endl;
		ss << std::endl;

		uint16_t datatype_length;
		uint32_t data_length;
		datatype dt;

		for (uint32_t i = 0; i < _body_length; ) {
			memcpy(&dt, &data[header_length + i], sizeof(datatype));
			i += sizeof(datatype);
			memcpy(&data_length, &data[header_length + i], 4);
			i += 4;

			ss << "datatype (length):    " << get_datatype_name(dt) << " (" << data_length << ")" << std::endl;
			ss << "Data:                 ";

			if (dt == STRING) {
				ss << read_string(header_length + i, data_length);
				i += data_length;
			}
			else {
				datatype_length = get_datatype_length(dt);

				for  (uint32_t j = 0; j < data_length; j++) {
					read_next(ss, header_length + i, dt);
					ss << " ";
					i += datatype_length;
				}
			}
			ss << std::endl << std::endl;
		}
		ss << "==============================================" << std::endl;

		return ss.str();
	}


};

}

#endif // ALCHEMIST__MESSAGE_HPP
