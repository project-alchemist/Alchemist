#ifndef ALCHEMIST__MESSAGE_HPP
#define ALCHEMIST__MESSAGE_HPP

// Structure of Message:
//
//  client_command code (4 bytes)
//  Header (4 bytes - contains length of data format section)
//  Body (Variable bytes - contains data)

#include <codecvt>
#include <locale>
#include <sstream>
#include "Alchemist.hpp"

namespace alchemist {

using std::string;
using std::stringstream;

class Message
{
public:
	enum { header_length = 9 };

	uint32_t max_body_length;

	bool big_endian;

	char * data;

	uint16_t client_ID;
	uint16_t session_ID;
	client_command cc;

	client_language cl;

	uint32_t body_length;
	uint32_t read_pos;

	// For writing data
	datatype current_datatype;
	int32_t current_datatype_count;
	int32_t current_datatype_count_max;
	int32_t current_datatype_count_pos;
	int32_t write_pos;

	bool data_copied;

	Message() : Message(10000000) { }

	Message(uint32_t _max_body_length) : cc(WAIT), client_ID(0), session_ID(0), body_length(0), cl(C), read_pos(header_length), current_datatype(NONE),
				current_datatype_count(0), current_datatype_count_max(0), max_body_length(_max_body_length), current_datatype_count_pos(header_length+1),
				write_pos(header_length), data_copied(false) {

		data = new char[header_length + max_body_length]();

		big_endian = is_big_endian();
	}

	~Message() { delete [] data; }

	bool is_big_endian()
	{
	    union {
	        uint32_t i;
	        char c[4];
	    } bint = {0x01020304};

	    return bint.c[0] == 1;
	}

	const uint32_t get_max_body_length() const { return max_body_length; }

	const int32_t length() const
	{
		return body_length + header_length;
	}

	const int32_t get_body_length() const
	{
		return body_length;
	}

	const char * header() const
	{
		return &data[0];
	}

	char * header()
	{
		return &data[0];
	}

	void copy_body(const char * _body, const uint32_t _body_length)
	{
		log->info("Y 1");
		for (uint32_t i = 0; i < _body_length; i++) {
			data[header_length + i] = _body[i];
		}
		log->info("Y 2");

		data_copied = true;
		body_length = _body_length;
		log->info("Y 3");
		write_pos = body_length + header_length;
		log->info("Y 4");
	}

	void copy_data(const char * _data, const uint32_t _data_length)
	{
		for (uint32_t i = 0; i < _data_length; i++) data[i] = _data[i];

		data_copied = true;
		body_length = _data_length - header_length;
		write_pos = body_length + header_length;
	}

 	const char * body() const
	{
		return &data[header_length];
	}

	char * body()
	{
		return &data[header_length];
	}

	const char * get_data()
	{
        update_body_length();
        update_datatype_count();

		return &data[0];
	}

	bool decode_header()
	{
		memcpy(&client_ID, &data[0], 2);
		client_ID = be16toh(client_ID);
		memcpy(&session_ID, &data[2], 2);
		session_ID = be16toh(session_ID);
		memcpy(&cc, &data[4], sizeof(client_command));

		memcpy(&body_length, &data[4+sizeof(client_command)], 4);
		body_length = be32toh(body_length);

		if (body_length > max_body_length) {
			body_length = 0;
			return false;
		}

		return true;
	}

	void set_client_language(const client_language & _cl)
	{
		cl = _cl;
	}

	const client_language get_client_language()
	{
		return cl;
	}

	void clear()
	{
		client_ID = 0;
		session_ID = 0;
		cc = WAIT;
		body_length = 0;
		read_pos = header_length;

		current_datatype = NONE;
		current_datatype_count = 0;
		current_datatype_count_max = 0;
		current_datatype_count_pos = header_length + 1;
		write_pos = header_length;
	}

	void start(const uint16_t & _client_id, const uint16_t & _session_id, const client_command & cc)
	{
		clear();
		add_client_id(_client_id);
		add_session_id(_session_id);
		add_client_command(cc);
	}

	void add_client_id(const uint16_t _client_id)
	{
		uint16_t temp_client_id = htobe16(_client_id);
		memcpy(&data[0], &temp_client_id, 2);
	}

	void add_session_id(const uint16_t _session_id)
	{
		uint16_t temp_session_id = htobe16(_session_id);
		memcpy(&data[2], &temp_session_id, 2);
	}

	void add_client_command(const client_command & cc)
	{
		memcpy(&data[4], &cc, 1);
	}

	void add_string(const string & str)
	{
		switch(cl) {
		case C:
		case CPP:
			{
				check_datatype(STRING);

				uint32_t string_length = (uint32_t) str.length();
				uint32_t new_string_length = htobe32(string_length);
				memcpy(data + write_pos, &new_string_length, 4);
				write_pos += 4;
				auto cdata = str.c_str();
				memcpy(data + write_pos, cdata, string_length);
				write_pos += string_length;
			}
			break;
		case SCALA:
		case JAVA:
			{
				check_datatype(WSTRING);

				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter_1;
				std::wstring wstr = converter_1.from_bytes(str);

				std::wstring_convert<std::codecvt_utf16<wchar_t>> converter_2;
				string str_u16 = converter_2.to_bytes(wstr);

				int32_t string_length = (int32_t) str.length();
				int32_t new_string_length = htobe32(string_length);
				memcpy(data + write_pos, &new_string_length, 4);
				write_pos += 4;
				auto cdata = str_u16.c_str();
				memcpy(data + write_pos, cdata, string_length);
				write_pos += string_length;
			}
			break;
		case PYTHON:
		case JULIA:
			{
				check_datatype(STRING);

				int32_t string_length = (int32_t) str.length();
				int32_t new_string_length = htobe32(string_length);
				memcpy(data + write_pos, &new_string_length, 4);
				write_pos += 4;
				auto cdata = str.c_str();
				memcpy(data + write_pos, cdata, string_length);
				write_pos += string_length;
			}
			break;
		}

		current_datatype_count += 1;
	}

	void add_byte(const unsigned char * _data, uint32_t length)
	{
		check_datatype(BYTE);

		unsigned char temp_byte;

		for (uint32_t i = 0; i < length; i++) {
			temp_byte = _data[i];
			memcpy(data + write_pos, &temp_byte, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_byte(const unsigned char _data)
	{
		check_datatype(BYTE);

		unsigned char temp_byte = _data;
		memcpy(data + write_pos, &temp_byte, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_bool(const bool * _data, uint32_t length)
	{
		check_datatype(BOOL);

		bool temp_bool;

		for (uint32_t i = 0; i < length; i++) {
			temp_bool = _data[i];
			memcpy(data + write_pos, &temp_bool, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_bool(const unsigned char _data)
	{
		check_datatype(BOOL);

		bool temp_bool = _data;
		memcpy(data + write_pos, &temp_bool, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_logical(const bool * _data, uint32_t length)
	{
		check_datatype(LOGICAL);

		bool temp_bool;

		for (uint32_t i = 0; i < length; i++) {
			temp_bool = _data[i];
			memcpy(data + write_pos, &temp_bool, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_logical(const unsigned char _data)
	{
		check_datatype(LOGICAL);

		bool temp_bool = _data;
		memcpy(data + write_pos, &temp_bool, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_char(const char * _data, uint32_t length)
	{
		check_datatype(CHAR);

		unsigned char temp_char;

		for (uint32_t i = 0; i < length; i++) {
			temp_char = _data[i];
			memcpy(data + write_pos, &temp_char, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_char(const char _data)
	{
		check_datatype(CHAR);

		unsigned char temp_char = _data;
		memcpy(data + write_pos, &temp_char, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_unsigned_char(const unsigned char & _data)
	{
		switch(cl) {
		case C:
		case CPP:
			add_unsigned_char_(_data);
			break;
		case SCALA:
		case JAVA:
			add_char((char) _data);
			break;
		case PYTHON:
		case JULIA:
			add_char((char) _data);
			break;
		}
	}

	void add_unsigned_char(const unsigned char * _data, uint32_t length)
	{
		check_datatype(UNSIGNED_CHAR);

		unsigned char temp_uchar;

		for (uint32_t i = 0; i < length; i++) {
			temp_uchar = _data[i];
			memcpy(data + write_pos, &temp_uchar, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_unsigned_char_(const unsigned char _data)
	{
		check_datatype(UNSIGNED_CHAR);

		unsigned char temp_uchar = _data;
		memcpy(data + write_pos, &temp_uchar, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_wchar(const wchar_t * _data, uint32_t length)
	{
		check_datatype(WCHAR);

		wchar_t temp_wchar;

		for (uint32_t i = 0; i < length; i++) {
			temp_wchar = _data[i];
			temp_wchar = htobe16(temp_wchar);
			memcpy(data + write_pos, &temp_wchar, 2);
			write_pos += 2;
		}

		current_datatype_count += length;
	}

	void add_wchar(const wchar_t _data)
	{
		check_datatype(WCHAR);

		wchar_t temp_wchar = _data;
		temp_wchar = htobe16(temp_wchar);
		memcpy(data + write_pos, &temp_wchar, 2);
		write_pos += 2;

		current_datatype_count += 1;
	}

	void add_int(const int * _data, uint32_t length)
	{
		check_datatype(INT);

		int temp_int;

		for (uint32_t i = 0; i < length; i++) {
			temp_int = _data[i];
			memcpy(data + write_pos, &temp_int, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_int(const int _data)
	{
		check_datatype(INT);

		int temp_int = _data;
		memcpy(data + write_pos, &temp_int, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_integer1(const int8_t * _data, uint32_t length)
	{
		check_datatype(INTEGER1);

		int8_t temp_int8;

		for (uint32_t i = 0; i < length; i++) {
			temp_int8 = _data[i];
			memcpy(data + write_pos, &temp_int8, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_integer1(const int8_t _data)
	{
		check_datatype(INTEGER1);

		int8_t temp_int8 = _data;
		memcpy(data + write_pos, &temp_int8, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_integer2(const int16_t * _data, uint32_t length)
	{
		check_datatype(INTEGER2);

		int16_t temp_int16;

		for (uint32_t i = 0; i < length; i++) {
			temp_int16 = _data[i];
			memcpy(data + write_pos, &temp_int16, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_integer2(const int16_t _data)
	{
		check_datatype(INTEGER2);

		int16_t temp_int16 = _data;
		memcpy(data + write_pos, &temp_int16, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_integer4(const int32_t * _data, uint32_t length)
	{
		check_datatype(INTEGER4);

		int32_t temp_int32;

		for (uint32_t i = 0; i < length; i++) {
			temp_int32 = _data[i];
			memcpy(data + write_pos, &temp_int32, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_integer4(const int32_t _data)
	{
		check_datatype(INTEGER4);

		int16_t temp_int32 = _data;
		memcpy(data + write_pos, &temp_int32, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_integer8(const int64_t * _data, uint32_t length)
	{
		check_datatype(INTEGER8);

		int64_t temp_int64;

		for (uint32_t i = 0; i < length; i++) {
			temp_int64 = _data[i];
			memcpy(data + write_pos, &temp_int64, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_integer8(const int64_t _data)
	{
		check_datatype(INTEGER8);

		int16_t temp_int64 = _data;
		memcpy(data + write_pos, &temp_int64, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_int8(const int8_t * _data, uint32_t length)
	{
		check_datatype(INT8_T);

		int8_t temp_int8;

		for (uint32_t i = 0; i < length; i++) {
			temp_int8 = _data[i];
			memcpy(data + write_pos, &temp_int8, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_int8(const int8_t & _data)
	{
		check_datatype(INT8_T);

		memcpy(data + write_pos, &_data, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_uint8(const uint8_t & _data) {

		switch(cl) {
		case C:
		case CPP:
			add_uint8_(_data);
			break;
		case SCALA:
		case JAVA:
			add_int8((int8_t) _data);
			break;
		case PYTHON:
		case JULIA:
			add_int8((int8_t) _data);
			break;
		}
	}

	void add_uint8(const uint8_t * _data, uint32_t length)
	{
		check_datatype(UINT8_T);

		uint8_t temp_uint8;

		for (uint32_t i = 0; i < length; i++) {
			temp_uint8 = _data[i];
			memcpy(data + write_pos, &temp_uint8, 1);
			write_pos += 1;
		}

		current_datatype_count += length;
	}

	void add_uint8_(const uint8_t _data)
	{
		check_datatype(UINT8_T);

		uint8_t temp_uint8 = _data;
		memcpy(data + write_pos, &temp_uint8, 1);
		write_pos += 1;

		current_datatype_count += 1;
	}

	void add_int16(const int16_t * _data, uint32_t length)
	{
		check_datatype(INT16_T);

		int16_t temp_int16;

		for (uint32_t i = 0; i < length; i++) {
			temp_int16 = _data[i];
			temp_int16 = htobe16(temp_int16);
			memcpy(data + write_pos, &temp_int16, 2);
			write_pos += 2;
		}

		current_datatype_count += length;
	}

	void add_int16(const int16_t _data)
	{
		check_datatype(INT16_T);

		int16_t temp_int16 = htobe16(_data);
		memcpy(data + write_pos, &temp_int16, 2);
		write_pos += 2;

		current_datatype_count += 1;
	}

	void add_uint16(const uint16_t & _data) {

		switch(cl) {
		case C:
		case CPP:
			add_uint16_(_data);
			break;
		case SCALA:
		case JAVA:
			add_int16((int16_t) _data);
			break;
		case PYTHON:
		case JULIA:
			add_int16((int16_t) _data);
			break;
		}
	}

	void add_uint16(const uint16_t * _data, uint32_t length)
	{
		check_datatype(UINT16_T);

		uint16_t temp_uint16;

		for (uint32_t i = 0; i < length; i++) {
			temp_uint16 = _data[i];
			temp_uint16 = htobe16(temp_uint16);
			memcpy(data + write_pos, &temp_uint16, 2);
			write_pos += 2;
		}

		current_datatype_count += length;
	}

	void add_uint16_(const uint16_t _data)
	{
		check_datatype(UINT16_T);

		uint16_t temp_uint16 = htobe16(_data);
		memcpy(data + write_pos, &temp_uint16, 2);
		write_pos += 2;

		current_datatype_count += 1;
	}

	void add_int32(const int32_t * _data, uint32_t length)
	{
		check_datatype(INT32_T);

		int32_t temp_int32;

		for (uint32_t i = 0; i < length; i++) {
			temp_int32 = _data[i];
			temp_int32 = htobe32(temp_int32);
			memcpy(data + write_pos, &temp_int32, 4);
			write_pos += 4;
		}

		current_datatype_count += length;
	}

	void add_int32(const int32_t _data)
	{
		check_datatype(INT32_T);

		int32_t temp_int32 = _data;
		temp_int32 = htobe32(temp_int32);
		memcpy(data + write_pos, &temp_int32, 4);
		write_pos += 4;

		current_datatype_count += 1;
	}

	void add_uint32(const uint32_t & _data) {

		switch(cl) {
		case C:
		case CPP:
			add_uint32_(_data);
			break;
		case SCALA:
		case JAVA:
			add_int32((int32_t) _data);
			break;
		case PYTHON:
		case JULIA:
			add_int32((int32_t) _data);
			break;
		}
	}

	void add_uint32(const uint32_t * _data, uint32_t length)
	{
		check_datatype(UINT32_T);

		uint32_t temp_uint32;

		for (uint32_t i = 0; i < length; i++) {
			temp_uint32 = _data[i];
			temp_uint32 = htobe32(temp_uint32);
			memcpy(data + write_pos, &temp_uint32, 4);
			write_pos += 4;
		}

		current_datatype_count += length;
	}

	void add_uint32_(const uint32_t & _data)
	{
		check_datatype(UINT32_T);

		uint32_t temp_uint32 = htobe32(_data);
		memcpy(data + write_pos, &temp_uint32, 4);
		write_pos += 4;

		current_datatype_count += 1;
	}

	void add_int64(const int64_t * _data, uint32_t length)
	{
		check_datatype(INT64_T);

		uint64_t temp_int64;

		for (uint32_t i = 0; i < length; i++) {
			temp_int64 = _data[i];
			temp_int64 = htobe64(temp_int64);
			memcpy(data + write_pos, &temp_int64, 8);
			write_pos += 8;
		}

		current_datatype_count += length;
	}

	void add_int64(const int64_t _data)
	{
		check_datatype(INT64_T);

		int64_t temp_int64 = _data;
		temp_int64 = htobe64(temp_int64);
		memcpy(data + write_pos, &temp_int64, 8);
		write_pos += 8;

		current_datatype_count += 1;
	}

	void add_uint64(const uint64_t & _data) {

		switch(cl) {
		case C:
		case CPP:
			add_uint64_(_data);
			break;
		case SCALA:
		case JAVA:
			add_int64((int64_t) _data);
			break;
		case PYTHON:
		case JULIA:
			add_int64((int64_t) _data);
			break;
		}
	}

	void add_uint64(const uint64_t * _data, uint32_t length)
	{
		check_datatype(UINT64_T);

		uint64_t temp_uint64;

		for (uint32_t i = 0; i < length; i++) {
			temp_uint64 = _data[i];
			temp_uint64 = htobe64(temp_uint64);
			memcpy(data + write_pos, &temp_uint64, 8);
			write_pos += 8;
		}

		current_datatype_count += length;
	}

	void add_uint64_(const uint64_t & _data)
	{
		check_datatype(UINT64_T);

		uint64_t temp_uint64 = _data;
		temp_uint64 = htobe64(temp_uint64);
		memcpy(data + write_pos, &temp_uint64, 8);
		write_pos += 8;

		current_datatype_count += 1;
	}

	void add_float(const float * _data, uint32_t length)
	{
		check_datatype(FLOAT);

		float temp_float;

		for (uint32_t i = 0; i < length; i++) {
			temp_float = _data[i];
			reverse_float(&temp_float);
			memcpy(data + write_pos, &temp_float, 4);
			write_pos += 4;
		}

		current_datatype_count += length;
	}

	void add_float(const float _data)
	{
		check_datatype(FLOAT);

		float temp_float = _data;
		reverse_float(&temp_float);
		memcpy(data + write_pos, &temp_float, 4);
		write_pos += 4;

		current_datatype_count += 1;
	}

	void add_double(const double * _data, uint32_t length)
	{
		check_datatype(DOUBLE);

		double temp_double;

		for (uint32_t i = 0; i < length; i++) {
			temp_double = _data[i];
			reverse_double(&temp_double);
			memcpy(data + write_pos, &temp_double, 8);
			write_pos += 8;
		}

		current_datatype_count += length;
	}

	void add_double(const double & _data)
	{
		check_datatype(DOUBLE);

		double temp_double = _data;
		reverse_double(&temp_double);
		memcpy(data + write_pos, &temp_double, 8);
		write_pos += 8;

		current_datatype_count += 1;
	}

	void add_parameter()
	{
		check_datatype(PARAMETER);
	}

	void add_library_ID(const Library_ID & _data)
	{
		check_datatype(LIBRARY_ID);

		current_datatype_count += 1;

		switch(cl) {
		case C:
		case CPP:
			memcpy(data + write_pos, &_data, 1);
			break;
		case SCALA:
		case JAVA:
			{
				int8_t temp = (int8_t) _data;
				memcpy(data + write_pos, &temp, 1);
			}
			break;
		case PYTHON:
		case JULIA:
			{
				int8_t temp = (int8_t) _data;
				memcpy(data + write_pos, &temp, 1);
			}
			break;
		}
		write_pos += 1;
	}

	void add_matrix_ID(const Matrix_ID & _data)
	{
		check_datatype(MATRIX_ID);

		current_datatype_count += 1;

		switch(cl) {
		case C:
		case CPP:
			{
				uint16_t temp_uint16 = htobe16((uint16_t) _data);
				memcpy(data + write_pos, &temp_uint16, 2);
			}
			break;
		case SCALA:
		case JAVA:
			{
				int16_t temp_int16 = htobe16((int16_t) _data);
				memcpy(data + write_pos, &temp_int16, 2);
			}
			break;
		case PYTHON:
		case JULIA:
			{
				int16_t temp_int16 = htobe16((int16_t) _data);
				memcpy(data + write_pos, &temp_int16, 2);
			}
			break;
		}
		write_pos += 2;
	}

	void add_matrix_info(const MatrixInfo & matrix)
	{
		check_datatype(MATRIX_INFO);

		current_datatype_count += 1;

		switch(cl) {
		case C:
		case CPP:
			{
				// Matrix ID
				uint16_t matrix_ID = htobe16((uint16_t) matrix.ID);
				memcpy(data + write_pos, &matrix_ID, 2);
				write_pos += 2;
				// Matrix name
				uint32_t string_length = matrix.name.length();
				uint32_t string_length_ = htobe32(string_length);
				memcpy(data + write_pos, &string_length_, 4);
				write_pos += 4;
				auto name_c = matrix.name.c_str();
				memcpy(data + write_pos, name_c, string_length);
				write_pos += string_length;
				// Matrix num rows
				uint64_t num_rows = htobe64(matrix.num_rows);
				memcpy(data + write_pos, &num_rows, 8);
				write_pos += 8;
				// Matrix num cols
				uint64_t num_cols = htobe64(matrix.num_cols);
				memcpy(data + write_pos, &num_cols, 8);
				write_pos += 8;
				// Matrix sparse
				uint8_t sparse = (uint8_t) matrix.sparse;
				memcpy(data + write_pos, &sparse, 1);
				write_pos += 1;
				// Matrix layout
				memcpy(data + write_pos, &matrix.layout, 1);
				write_pos += 1;
				// Matrix num partitions
				memcpy(data + write_pos, &matrix.num_partitions, 1);
				write_pos += 1;
				// Matrix row assignments
				memcpy(data + write_pos, matrix.row_assignments, matrix.num_rows);
				write_pos += matrix.num_rows;
			}
			break;
		case SCALA:
		case JAVA:
			{
				// Matrix ID
				int16_t matrix_ID = htobe16((int16_t) matrix.ID);
				memcpy(data + write_pos, &matrix_ID, 2);
				write_pos += 2;
				// Matrix name
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter_1;
				std::wstring_convert<std::codecvt_utf16<wchar_t>> converter_2;

				std::wstring name_wide = converter_1.from_bytes(matrix.name);
				string name_u16 = converter_2.to_bytes(name_wide);

				int32_t string_length = name_u16.length();
				int32_t string_length_ = htobe32(string_length);
				memcpy(data + write_pos, &string_length_, 4);
				write_pos += 4;
				auto name_c = matrix.name.c_str();
				memcpy(data + write_pos, name_c, string_length);
				write_pos += string_length;
				// Matrix num rows
				int64_t num_rows = htobe64(matrix.num_rows);
				memcpy(data + write_pos, &num_rows, 8);
				write_pos += 8;
				// Matrix num cols
				int64_t num_cols = htobe64(matrix.num_cols);
				memcpy(data + write_pos, &num_cols, 8);
				write_pos += 8;
				// Matrix sparse
				int8_t sparse = (int8_t) matrix.sparse;
				memcpy(data + write_pos, &sparse, 1);
				write_pos += 1;
				// Matrix layout
				memcpy(data + write_pos, &matrix.layout, 1);
				write_pos += 1;
				// Matrix num partitions
				memcpy(data + write_pos, &matrix.num_partitions, 1);
				write_pos += 1;
				// Matrix row assignments
				memcpy(data + write_pos, matrix.row_assignments, matrix.num_rows);
				write_pos += matrix.num_rows;
			}
			break;
		case PYTHON:
		case JULIA:
			{
				std::cout << matrix.to_string(true) << std::endl;
				// Matrix ID
				int16_t matrix_ID = htobe16((int16_t) matrix.ID);
				memcpy(data + write_pos, &matrix_ID, 2);
				write_pos += 2;
				// Matrix name
				int32_t string_length = matrix.name.length();
				int32_t string_length_ = htobe32(string_length);
				memcpy(data + write_pos, &string_length_, 4);
				write_pos += 4;
				auto name_c = matrix.name.c_str();
				memcpy(data + write_pos, name_c, string_length);
				write_pos += string_length;
				// Matrix num rows
				int64_t num_rows = htobe64(matrix.num_rows);
				memcpy(data + write_pos, &num_rows, 8);
				write_pos += 8;
				// Matrix num cols
				int64_t num_cols = htobe64(matrix.num_cols);
				memcpy(data + write_pos, &num_cols, 8);
				write_pos += 8;
				// Matrix sparse
				int8_t sparse = (int8_t) matrix.sparse;
				memcpy(data + write_pos, &sparse, 1);
				write_pos += 1;
				// Matrix layout
				memcpy(data + write_pos, &matrix.layout, 1);
				write_pos += 1;
				// Matrix num partitions
				memcpy(data + write_pos, &matrix.num_partitions, 1);
				write_pos += 1;
				// Matrix row assignments
				memcpy(data + write_pos, matrix.row_assignments, matrix.num_rows);
				write_pos += matrix.num_rows;
			}
			break;
		}
	}

	void update_body_length()
	{
		body_length = write_pos - header_length;
		int32_t _body_length = htobe32(body_length);
		memcpy(data + 4 + sizeof(client_command), &_body_length, 4);
	}

	void update_datatype_count()
	{
		if (!data_copied) {
			int32_t _current_datatype_count = htobe32(current_datatype_count);
			memcpy(data + current_datatype_count_pos, &_current_datatype_count, 4);
		}
		data_copied = false;
 	}

	void check_datatype(const datatype new_datatype)
	{
		if (current_datatype != new_datatype) {
			current_datatype = new_datatype;

			update_datatype_count();

			memcpy(data + write_pos, &current_datatype, 1);

			current_datatype_count = 0;
			current_datatype_count_pos = write_pos + 1;
			write_pos += 5;
		}
//		else current_datatype_count += 1;
	}


//	void add(const void * _data, const uint32_t & length, const datatype & dt)
//	{
//		if (dt == current_datatype) {
//			uint32_t current_length;
//
//			memcpy(&current_length, data + current_length_pos, 4);
//			current_length = be32toh(current_length);
//			current_length += length;
//			current_length = htobe32(current_length);
//			memcpy(data + current_length_pos, &current_length, 4);
//		}
//		else {    // new datatype being written
//			current_datatype = dt;
//
//			memcpy(data + header_length + body_length, &dt, sizeof(datatype));
//			body_length += sizeof(datatype);
//
//			current_length_pos = header_length + body_length;
//
//			uint32_t length_temp = htobe32(length);
//			memcpy(data + current_length_pos, &length_temp, 4);
//			body_length += 4;
//		}
//
//		uint32_t word_length = get_word_length(1, dt);
//		uint32_t data_length = get_word_length(length, dt);
//
//		int16_t temp16;
//		int32_t temp32;
//		int64_t temp64;
//
//		for (uint32_t i = 0; i < data_length; i += word_length) {
//			if (word_length == 2) {
//				memcpy(
//				temp16 = &_data + i:
//			}
//			memcpy(data + header_length + body_length, &_data + i, word_length);
//		}
//
//		body_length += data_length;
//
//		body_length = htobe32(body_length);
//		memcpy(data + sizeof(client_command), &body_length, 4);
//		body_length = be32toh(body_length);
//
//	}
//
//	uint32_t get_word_length(const uint32_t & length, const datatype & dt)
//	{
//		return get_datatype_length(dt)*length;
//	}

	const datatype next_datatype()
	{
		datatype dt;

		memcpy(&dt, data + read_pos, sizeof(datatype));

		return dt;
	}

	const uint32_t next_data_length()
	{
		uint32_t data_length;

		memcpy(&data_length, data + read_pos + sizeof(datatype), 4);

		return be32toh(data_length);
	}

	inline void read_atom(uint32_t & i, void * a, int psize)
	{
		memcpy(a, data + i, psize);
	}

	const char read_char()
	{
		switch(cl) {
		case C:
		case CPP:
			return read_char_();
		case SCALA:
		case JAVA:
			return (char) read_char_();
		case PYTHON:
		case JULIA:
			return (char) read_char_();
		default:
			return (char) '0';
		}
	}

	const char read_char(uint32_t & i)
	{
		char x;
		read_atom(i, &x, 1);

		return x;
	}

	const char read_char_()
	{
		if (current_datatype != CHAR) {
			current_datatype = CHAR;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		signed char x;

		memcpy(&x, &data[read_pos], 1);
		read_pos += 1;

		return x;
	}

	const unsigned char read_signed_char()
	{
		switch(cl) {
		case C:
		case CPP:
			return read_signed_char_();
		case SCALA:
		case JAVA:
			return (signed char) read_signed_char_();
		case PYTHON:
		case JULIA:
			return (signed char) read_signed_char_();
		default:
			return (signed char) '0';
		}
	}

	const signed char read_signed_char(uint32_t & i)
	{
		signed char x;
		read_atom(i, &x, 1);

		return x;
	}

	const signed char read_signed_char_()
	{
		if (current_datatype != SIGNED_CHAR) {
			current_datatype = SIGNED_CHAR;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		signed char x;

		memcpy(&x, &data[read_pos], 1);
		read_pos += 1;

		return x;
	}

	const unsigned char read_unsigned_char()
	{
		switch(cl) {
		case C:
		case CPP:
			return read_unsigned_char_();
		case SCALA:
		case JAVA:
			return (unsigned char) read_unsigned_char_();
		case PYTHON:
		case JULIA:
			return (unsigned char) read_unsigned_char_();
		default:
			return (unsigned char) '0';
		}
	}

	const unsigned char read_unsigned_char(uint32_t & i)
	{
		unsigned char x;
		read_atom(i, &x, 1);

		return x;
	}

	const unsigned char read_unsigned_char_()
	{
		if (current_datatype != UNSIGNED_CHAR) {
			current_datatype = UNSIGNED_CHAR;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		unsigned char x;

		memcpy(&x, &data[read_pos], 1);
		read_pos += 1;

		return x;
	}

	const char read_character()
	{
		switch(cl) {
		case C:
		case CPP:
			return read_character_();
		case SCALA:
		case JAVA:
			return (char) read_character_();
		case PYTHON:
		case JULIA:
			return (char) read_character_();
		default:
			return (char) '0';
		}
	}

	const char read_character(uint32_t & i)
	{
		char x;
		read_atom(i, &x, 1);

		return x;
	}

	const char read_character_()
	{
		if (current_datatype != CHARACTER) {
			current_datatype = CHARACTER;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		char x;

		memcpy(&x, &data[read_pos], 1);
		read_pos += 1;

		return x;
	}

	const wchar_t read_wchar()
	{
		switch(cl) {
		case C:
		case CPP:
			return read_character_();
		case SCALA:
		case JAVA:
			return read_character_();
		case PYTHON:
		case JULIA:
			return read_character_();
		default:
			return (wchar_t) '0';
		}
	}

	const wchar_t read_wchar(uint32_t & i)
	{
		wchar_t x;
		read_atom(i, &x, 2);
		x = (wchar_t) be16toh(x);

		return x;
	}

	const wchar_t read_wchar_()
	{
		if (current_datatype != WCHAR) {
			current_datatype = WCHAR;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		wchar_t x;

		memcpy(&x, &data[read_pos], 2);
		read_pos += 2;

		return (wchar_t) x;
	}


	const uint8_t read_byte()
	{
		if (current_datatype != BYTE) {
			current_datatype = BYTE;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_byte(read_pos);
	}

	const uint8_t read_byte(uint32_t & i)
	{
		uint8_t x;
		memcpy(&x, data + i, 1);
		i += 1;

		return x;
	}

	const bool read_bool()
	{
		if (current_datatype != BOOL) {
			current_datatype = BOOL;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		bool x;
		memcpy(&x, data + read_pos, 1);
		read_pos += 1;

		return x;
	}

	const bool read_bool(uint32_t & i)
	{
		bool x;
		memcpy(&x, data + i, 1);
		i += 1;

		return x;
	}

	const bool read_logical()
	{
		return read_bool();
	}

	const bool read_logical(uint32_t & i)
	{
		return read_bool(i);
	}

	const short read_short()
	{
		return (short) read_int16();
	}

	const short read_short(uint32_t & i)
	{
		return (short) read_int16(i);
	}

	const unsigned short read_unsigned_short()
	{
		return (unsigned short) read_uint16();
	}

	const unsigned short read_unsigned_short(uint32_t & i)
	{
		return (unsigned short) read_uint16(i);
	}

	const int read_integer()
	{
		return (int) read_int32();
	}

	const int read_integer(uint32_t & i)
	{
		return (int) read_int32(i);
	}

	const int read_int()
	{
		return (int) read_int32();
	}

	const int read_int(uint32_t & i)
	{
		return (int) read_int32(i);
	}

	const unsigned int read_unsigned()
	{
		return (unsigned int) read_uint32();
	}

	const unsigned int read_unsigned(uint32_t & i)
	{
		return (unsigned int) read_uint32(i);
	}

	const unsigned int read_unsigned_int()
	{
		return (unsigned int) read_uint32();
	}

	const unsigned int read_unsigned_int(uint32_t & i)
	{
		return (unsigned int) read_uint32(i);
	}

	const long read_long()
	{
		return (long) read_int64();
	}

	const long read_long(uint32_t & i)
	{
		return (long) read_int64(i);
	}

	const unsigned long read_unsigned_long()
	{
		return (unsigned long) read_uint64();
	}

	const unsigned long read_unsigned_long(uint32_t & i)
	{
		return (unsigned long) read_uint64(i);
	}

	const long long read_long_long()
	{
		if (current_datatype != LONG_LONG) {
			current_datatype = LONG_LONG;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		long long x;
		memcpy(&x, data + read_pos, sizeof(long long));
		read_pos += sizeof(long long);

		return x;
	}

	const long long read_long_long(uint32_t & i)
	{
		long long x;
		memcpy(&x, data + i, sizeof(long long));
		i + sizeof(long long);

		return x;
	}

	const long long read_long_long_int()
	{
		return read_long_long();
	}

	const long long read_long_long_int(uint32_t & i)
	{
		return read_long_long(i);
	}

	const unsigned long long read_unsigned_long_long()
	{
		if (current_datatype != UNSIGNED_LONG_LONG) {
			current_datatype = UNSIGNED_LONG_LONG;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		unsigned long long x;
		memcpy(&x, data + read_pos, sizeof(unsigned long long));
		read_pos += sizeof(unsigned long long);

		return x;
	}

	const unsigned long long read_unsigned_long_long(uint32_t & i)
	{
		unsigned long long x;
		memcpy(&x, data + i, sizeof(unsigned long long));
		i + sizeof(unsigned long long);

		return x;
	}

	const int8_t read_integer1(uint32_t & i)
	{
		int8_t x;
		read_atom(i, &x, 1);

		return x;
	}

	const void read_integer1(int8_t * x)
	{
		uint32_t data_length;

		read_pos += sizeof(datatype);
		memcpy(&data_length, &data[read_pos], 4);
		data_length = be32toh(data_length);
		read_pos += 4;

		memcpy(x, &data[read_pos], data_length);

		read_pos += data_length;
	}

	const int8_t read_integer1()
	{
		if (current_datatype != INTEGER1) {
			current_datatype = INTEGER1;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		int8_t x;

		memcpy(&x, &data[read_pos], 1);
		read_pos += 1;

		return x;
	}

	const int16_t read_integer2(uint32_t & i)
	{
		int16_t x;
		read_atom(i, &x, 2);

		return x;
	}

	const void read_integer2(int16_t * x)
	{
		uint32_t data_length;

		read_pos += sizeof(datatype);
		memcpy(&data_length, &data[read_pos], 4);
		data_length = be32toh(data_length);
		read_pos += 4;

		memcpy(x, &data[read_pos], 2*data_length);

		read_pos += 2*data_length;
	}

	const int16_t read_integer2()
	{
		if (current_datatype != INTEGER2) {
			current_datatype = INTEGER2;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		int16_t x;

		memcpy(&x, &data[read_pos], 2);
		read_pos += 2;

		return x;
	}

	const int32_t read_integer4(uint32_t & i)
	{
		int32_t x;
		read_atom(i, &x, 4);

		return x;
	}

	const void read_integer4(int32_t * x)
	{
		uint32_t data_length;

		read_pos += sizeof(datatype);
		memcpy(&data_length, &data[read_pos], 4);
		data_length = be32toh(data_length);
		read_pos += 4;

		memcpy(x, &data[read_pos], 4*data_length);

		read_pos += 4*data_length;
	}

	const int32_t read_integer4()
	{
		if (current_datatype != INTEGER4) {
			current_datatype = INTEGER4;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		int32_t x;

		memcpy(&x, &data[read_pos], 4);
		read_pos += 4;

		return x;
	}

	const int64_t read_integer8(uint32_t & i)
	{
		int64_t x;
		read_atom(i, &x, 8);

		return x;
	}

	const void read_integer8(int64_t * x)
	{
		uint32_t data_length;

		read_pos += sizeof(datatype);
		memcpy(&data_length, &data[read_pos], 4);
		data_length = be32toh(data_length);
		read_pos += 4;

		memcpy(x, &data[read_pos], 8*data_length);

		read_pos += 8*data_length;
	}

	const int64_t read_integer8()
	{
		if (current_datatype != INTEGER8) {
			current_datatype = INTEGER8;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		int64_t x;

		memcpy(&x, &data[read_pos], 8);
		read_pos += 8;

		return x;
	}

	const void read_int8(int8_t * x)
	{
		uint32_t data_length;

		read_pos += sizeof(datatype);
		memcpy(&data_length, &data[read_pos], 4);
		data_length = be32toh(data_length);
		read_pos += 4;

		memcpy(x, &data[read_pos], data_length);

		read_pos += data_length;
	}

	const int8_t read_int8()
	{
		if (current_datatype != INT8_T) {
			current_datatype = INT8_T;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		int8_t x;
		memcpy(&x, data + read_pos, 1);
		read_pos += 1;

		return x;
	}

	const int8_t read_int8(uint32_t & i)
	{
		int8_t x;
		memcpy(&x, data + i, 1);
		i += 1;

		return x;
	}

	const uint8_t read_uint8()
	{
		if (current_datatype != UINT8_T) {
			current_datatype = UINT8_T;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		uint8_t x;
		memcpy(&x, data + read_pos, 1);
		read_pos += 1;

		return x;
	}

	const uint8_t read_uint8(uint32_t & i)
	{
		uint8_t x;
		memcpy(&x, data + i, 1);
		i += 1;

		return x;
	}

	const int16_t read_int16()
	{
		if (current_datatype != INT16_T) {
			current_datatype = INT16_T;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_int16(read_pos);
	}

	const int16_t read_int16(uint32_t & i)
	{
		int16_t x;
		memcpy(&x, data + i, 2);
		x = be16toh(x);
		i += 2;

		return x;
	}

	const uint16_t read_uint16(uint32_t & i)
	{
		switch(cl) {
		case C:
		case CPP:
			{
				uint16_t x;
				memcpy(&x, data + i, 2);
				x = be16toh(x);
				i += 2;

				return x;
			}
		case SCALA:
		case JAVA:
			{
				int16_t x;
				memcpy(&x, data + i, 2);
				x = be16toh(x);
				i += 2;

				return (uint16_t) x;
			}
		case PYTHON:
		case JULIA:
			{
				int16_t x;
				memcpy(&x, data + i, 2);
				x = be16toh(x);
				i += 2;

				return (uint16_t) x;
			}
		default:
			{
				uint16_t x;
				memcpy(&x, data + i, 2);
				x = be16toh(x);
				i += 2;

				return x;
			}
		}
	}

	const uint16_t read_uint16()
	{
		if (current_datatype != UINT16_T) {
			current_datatype = UINT16_T;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_uint16(read_pos);
	}

	const int32_t read_int32()
	{
		if (current_datatype != INT32_T) {
			current_datatype = INT32_T;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_int32(read_pos);
	}

	const int32_t read_int32(uint32_t & i)
	{
		int32_t x;
		memcpy(&x, data + i, 4);
		x = be32toh(x);
		i += 4;

		return x;
	}

	const uint32_t read_uint32(uint32_t & i)
	{
		switch(cl) {
		case C:
		case CPP:
			{
				uint32_t x;
				memcpy(&x, data + i, 4);
				x = be32toh(x);
				i += 4;

				return x;
			}
		case SCALA:
		case JAVA:
			{
				int32_t x;
				memcpy(&x, data + i, 4);
				x = be32toh(x);
				i += 4;

				return (uint32_t) x;
			}
		case PYTHON:
		case JULIA:
			{
				int32_t x;
				memcpy(&x, data + i, 4);
				x = be32toh(x);
				i += 4;

				return (uint32_t) x;
			}
		default:
			{
				uint32_t x;
				memcpy(&x, data + i, 4);
				x = be32toh(x);
				i += 4;

				return x;
			}
		}
	}

	const uint32_t read_uint32()
	{
		if (current_datatype != UINT32_T) {
			current_datatype = UINT32_T;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_uint32(read_pos);
	}

	const int64_t read_int64()
	{
		if (current_datatype != INT64_T) {
			current_datatype = INT64_T;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_int64(read_pos);
	}

	const int64_t read_int64(uint32_t & i)
	{
		int64_t x;
		memcpy(&x, data + i, 8);
		x = be64toh(x);
		i += 8;

		return x;
	}

	const uint64_t read_uint64(uint32_t & i)
	{
		switch(cl) {
		case C:
		case CPP:
			{
				uint64_t x;
				memcpy(&x, data + i, 8);
				x = be64toh(x);
				i += 8;

				return x;
			}
		case SCALA:
		case JAVA:
			{
				int64_t x;
				memcpy(&x, data + i, 8);
				x = be64toh(x);
				i += 8;

				return (uint64_t) x;
			}
		case PYTHON:
		case JULIA:
			{
				int64_t x;
				memcpy(&x, data + i, 8);
				x = be64toh(x);
				i += 8;

				return (uint64_t) x;
			}
		default:
			{
				uint64_t x;
				memcpy(&x, data + i, 8);
				x = be64toh(x);
				i += 8;

				return x;
			}
		}
	}

	const uint64_t read_uint64()
	{
		if (current_datatype != UINT64_T) {
			current_datatype = UINT64_T;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_uint64(read_pos);
	}

	const float read_real4()
	{
		return read_float();
	}

	const float read_real4(uint32_t & i)
	{
		return read_float(i);
	}

	void reverse_float(float * x)
	{
		char * x_char = (char *) x;
		char temp;
		temp = x[0]; x[0] = x[3]; x[3] = x[0];
		temp = x[1]; x[1] = x[2]; x[2] = x[1];
	}

	const float read_float()
	{
		if (current_datatype != FLOAT) {
			current_datatype = FLOAT;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		float x;
		memcpy(&x, data + read_pos, 4);
		read_pos += 4;

		if (!big_endian) reverse_float(&x);

		return x;
	}

	void read_float(float * x)
	{
		if (current_datatype != FLOAT) {
			current_datatype = FLOAT;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		memcpy(x, data + read_pos, 4);
		read_pos += 4;

		if (!big_endian) reverse_float(x);
	}

	const float read_float(uint32_t & i)
	{
		float x;
		memcpy(&x, data + i, 4);
		i += 4;

		if (!big_endian) reverse_float(&x);

		return x;
	}

	void read_float(uint32_t & i, float * x)
	{
		memcpy(x, data + i, 4);
		i += 4;

		if (!big_endian) reverse_float(x);
	}

	void reverse_double(double * x)
	{
		char * x_char = (char *) x;
		char temp;
		temp = x[0]; x[0] = x[7]; x[7] = x[0];
		temp = x[1]; x[1] = x[6]; x[6] = x[1];
		temp = x[2]; x[2] = x[5]; x[5] = x[2];
		temp = x[3]; x[3] = x[4]; x[4] = x[3];
	}

	const double read_real()
	{
		return read_double();
	}

	const double read_real(uint32_t & i)
	{
		return read_double(i);
	}

	const double read_real8()
	{
		return read_double();
	}

	const double read_real8(uint32_t & i)
	{
		return read_double(i);
	}

	const double read_double_precision()
	{
		return read_double();
	}

	const double read_double_precision(uint32_t & i)
	{
		return read_double(i);
	}

	const double read_double()
	{
		if (current_datatype != DOUBLE) {
			current_datatype = DOUBLE;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		double x;
		memcpy(&x, data + read_pos, 8);
		read_pos += 8;

		if (!big_endian) reverse_double(&x);

		return x;
	}

	void read_double(double * x)
	{
		if (current_datatype != DOUBLE) {
			current_datatype = DOUBLE;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		memcpy(x, data + read_pos, 8);
		read_pos += 8;

		if (!big_endian) reverse_double(x);
	}

	const double read_double(uint32_t & i)
	{
		double x;
		memcpy(&x, data + i, 8);
		i += 8;

		if (!big_endian) reverse_double(&x);

		return x;
	}

	void read_double(uint32_t & i, double * x)
	{
		memcpy(&x, data + i, 8);
		i += 8;

		if (!big_endian) reverse_double(x);
	}

	const string read_string()
	{
		switch(cl) {
		case C:
		case CPP:
			return read_string_();
		case SCALA:
		case JAVA:
			return read_wstring();
		case PYTHON:
		case JULIA:
			return read_string_();
		default:
			return read_string_();
		}
	}

	const string read_string_()
	{
		if (current_datatype != STRING) {
			current_datatype = STRING;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_string(read_pos);
	}

	const string read_string(uint32_t & i)
	{
		switch(cl) {
		case C:
		case CPP:
			{
				uint32_t string_length;
				memcpy(&string_length, data + i, 4);
				i += 4;
				string_length = be32toh(string_length);
				char string_c[string_length+1];
				memcpy(string_c, data + i, string_length);
				i += string_length;
				string_c[string_length] = '\0';
				return string(string_c);
			}
			break;
		case SCALA:
		case JAVA:
			{
				int32_t string_length;
				memcpy(&string_length, data + i, 4);
				i += 4;
				string_length = be32toh(string_length);
				char string_c[string_length+1];
				memcpy(string_c, data + i, string_length);
				i += string_length;
				string_c[string_length] = '\0';
				return string(string_c);
			}
			break;
		case PYTHON:
		case JULIA:
			{
				int32_t string_length;
				memcpy(&string_length, data + i, 4);
				i += 4;
				string_length = be32toh(string_length);
				char string_c[string_length+1];
				memcpy(string_c, data + i, string_length);
				i += string_length;
				string_c[string_length] = '\0';
				return string(string_c);
			}
			break;
		}
		return string(" ");
	}

	const std::wstring s2ws(const string & str)
	{
	    using convert_type = std::codecvt_utf8<wchar_t>;
	    std::wstring_convert<convert_type, wchar_t> converter;

	    return converter.from_bytes(str);
	}

	const string ws2s(const std::wstring & wstr)
	{
	    using convert_type = std::codecvt_utf8<wchar_t>;
	    std::wstring_convert<convert_type, wchar_t> converter;

	    return converter.to_bytes(wstr);
	}

	const string read_wstring(uint32_t & i)
	{
		uint32_t string_length;
		memcpy(&string_length, data + i, 4);
		i += 4;
		string_length = be32toh(string_length);
		wchar_t string_c[string_length+1];
		memcpy(string_c, data + i, 2*string_length);
		i += 2*string_length;
		string_c[string_length] = L'\0';
		std::wstring wstr = std::wstring(string_c);

		return ws2s(wstr);
	}

	const string read_wstring()
	{
		if (current_datatype != WSTRING) {
			current_datatype = WSTRING;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_wstring(read_pos);
	}

	const Matrix_ID read_matrix_ID()
	{
		if (current_datatype != MATRIX_ID) {
			current_datatype = MATRIX_ID;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_matrix_ID(read_pos);
	}

	void read_parameter()
	{
		if (current_datatype != PARAMETER) {
			current_datatype = PARAMETER;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}
	}

	const Matrix_ID read_matrix_ID(uint32_t & i)
	{
		Matrix_ID matrix_ID;
		memcpy(&matrix_ID, &data[i], 2);
		matrix_ID = be16toh(matrix_ID);
		i += 2;

		return matrix_ID;
	}

	const Library_ID read_library_ID()
	{
		if (current_datatype != LIBRARY_ID) {
			current_datatype = LIBRARY_ID;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_library_ID(read_pos);
	}

	const Library_ID read_library_ID(uint32_t & i)
	{
		Library_ID library_ID;
		memcpy(&library_ID, &data[i], 1);
		i += 1;

		return library_ID;
	}

	const MatrixInfo_ptr read_matrix_info()
	{
		if (current_datatype != MATRIX_INFO) {
			current_datatype = MATRIX_INFO;

			read_pos += sizeof(datatype);
			read_pos += 4;
		}

		return read_matrix_info(read_pos);
	}

	const MatrixInfo_ptr read_matrix_info(uint32_t & i)
	{
		datatype dt;
		uint32_t data_length = 0;

		Matrix_ID matrix_ID;
		uint32_t name_length;
		string name;
		uint64_t num_rows, num_cols;
		bool sparse;
		uint8_t layout, num_partitions;

		memcpy(&matrix_ID, data + i, 2);
		matrix_ID = be16toh(matrix_ID);
		i += 2;
		memcpy(&name_length, data + i, 4);
		name_length = be32toh(name_length);
		i += 4;
		if (name_length > 0) {
			char name_c[name_length];
			memcpy(name_c, data + i, name_length);
			i += name_length;
			name = string(name_c);
		}
		else name = "";
		memcpy(&num_rows, data + i, 8);
		num_rows = be64toh(num_rows);
		i += 8;
		memcpy(&num_cols, data + i, 8);
		num_cols = be64toh(num_cols);
		i += 8;
		memcpy(&sparse, data + i, 1);
		i += 1;
		memcpy(&layout, data + i, 1);
		i += 1;
		memcpy(&num_partitions, &data[i], 1);
		i += 1;
		MatrixInfo_ptr matrix = std::make_shared<MatrixInfo>(matrix_ID, name, num_rows, num_cols, sparse, layout, num_partitions);
		memcpy(matrix->row_assignments, data + i, num_rows);
		i += num_rows;

		return matrix;
	}

	const void read_next(stringstream & ss, uint32_t & i, const datatype & dt)
	{
		switch (dt) {
			case BYTE:
				ss << (int16_t) read_byte(i);
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
			case WCHAR: {
					std::wstring str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(read_wchar(i));
					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

					ss << converter.to_bytes(str);
				}
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
				ss << (int16_t) read_int8(i);
				break;
			case UINT8_T:
				ss << (int16_t) read_uint8(i);
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
			case MATRIX_ID:
				ss << read_matrix_ID(i);
				break;
			case LIBRARY_ID:
				ss << (int16_t) read_library_ID(i);
				break;
			default:
				ss << "Invalid datatype";
				break;
			}
	}

	const bool eom() {
		return (read_pos >= body_length + header_length);
	}

	const string to_string()
	{
		stringstream ss;

		uint16_t _client_ID, _session_ID;
		client_command _cc;
		uint32_t _body_length;

		uint32_t i = 0;

		_client_ID = read_uint16(i);
		_session_ID = read_uint16(i);
		_cc = (client_command) read_uint8(i);
		_body_length = read_uint32(i);

		string space = "                                                  ";

		ss << std::endl;
		ss << space << "==============================================" << std::endl;
		ss << space << "Client ID:            " << _client_ID << std::endl;
		ss << space << "Session ID:           " << _session_ID << std::endl;
		ss << space << "Command code:         " << _cc << " (" << get_command_name(_cc) << ") " << std::endl;
		ss << space << "Message body length:  " << _body_length << std::endl;
		ss << space << "----------------------------------------------" << std::endl;
		ss << std::endl;

		uint32_t data_length;
		datatype dt;

		for (uint32_t i = header_length; i < header_length + _body_length; ) {
			memcpy(&dt, data + i, sizeof(datatype));
			i += sizeof(datatype);
			memcpy(&data_length, data + i, 4);
			data_length = be32toh(data_length);
			i += 4;
			if (data_length > _body_length) {
				ss << space << "INVALID MESSAGE FORMAT " << dt << " " << i << " " << data_length << std::endl << std::endl;
				break;
			}
			else {
				ss << space << "Datatype (length):    " << get_datatype_name(dt) << " (" << data_length << ")" << std::endl;
				ss << space << "Data:                 ";

				for (uint32_t j = 0; j < data_length; j++) {

					if (dt == STRING)
						ss << read_string(i) << "\n" << space;
					else if (dt == WSTRING)
						ss << read_wstring(i) << "\n" << space;
					else if (dt == PARAMETER)
						ss << " ";
					else if (dt == MATRIX_INFO)
						ss << read_matrix_info(i)->to_string(true);
					else {
						read_next(ss, i, dt);
						ss << " ";
					}
				}
				ss << std::endl << std::endl;
			}
		}
		ss << space << "==============================================" << std::endl;

		return ss.str();
	}


};

}

#endif // ALCHEMIST__MESSAGE_HPP
