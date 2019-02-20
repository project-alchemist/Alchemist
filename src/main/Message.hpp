#ifndef ALCHEMIST__MESSAGE_HPP
#define ALCHEMIST__MESSAGE_HPP

#include <codecvt>
#include <locale>
#include <sstream>
#include <iomanip>
#include "Alchemist.hpp"

namespace alchemist {

using std::string;
using std::stringstream;

class Message
{
public:
	enum { header_length = 10 };

	uint32_t max_body_length;

	bool big_endian;

	char * data;

	uint16_t clientID;
	uint16_t sessionID;
	client_command cc;
	alchemist_error_code ec;

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
	bool reverse_floats;
	bool signed_ints_only;

	Message() : Message(100000000) { }

	Message(uint32_t _max_body_length) : cc(WAIT), clientID(0), sessionID(0), body_length(0), cl(C), read_pos(header_length), current_datatype(NONE),
				current_datatype_count(0), current_datatype_count_max(0), max_body_length(_max_body_length), current_datatype_count_pos(header_length+1),
				write_pos(header_length), data_copied(false), reverse_floats(false), signed_ints_only(false) {

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
		memcpy(data + header_length, _body, _body_length);

		data_copied = true;
		body_length = _body_length;
		write_pos = body_length + header_length;
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
        put_body_length();

		return &data[0];
	}

	void decode_header()
	{
		clientID = get_ClientID();
		sessionID = get_SessionID();
		cc = get_client_command();
		ec = get_error_code();
		body_length = get_body_length();
	}

	void set_client_language(const client_language & _cl)
	{
		cl = _cl;
		switch(cl) {
		case SCALA:
		case JAVA:
		case PYTHON:
		case JULIA:
			signed_ints_only = true;
			break;
		default:
			signed_ints_only = false;
		}
	}

	const client_language get_client_language()
	{
		return cl;
	}

	void clear()
	{
		clientID = 0;
		sessionID = 0;
		cc = WAIT;
		body_length = 0;

		read_pos = header_length;
		write_pos = header_length;
	}

	void start(const ClientID & clientID, const SessionID & sessionID, const client_command & cc)
	{
		clear();
		put_ClientID(clientID);
		put_SessionID(sessionID);
		put_client_command(cc);
		put_error_code(ERR_NONE);
	}

	const bool eom() {
		return (read_pos >= body_length + header_length);
	}

	float reverse_float(float * x)
	{
		char * x_char = (char *) x;
		char temp;
		temp = x_char[0]; x_char[0] = x_char[3]; x_char[3] = temp;
		temp = x_char[1]; x_char[1] = x_char[2]; x_char[2] = temp;
		x = (float *) x_char;

		return *x;
	}

	double reverse_double(double * x)
	{
		char * x_char = (char *) x;
		char temp;
		temp = x_char[0]; x_char[0] = x_char[7]; x_char[7] = temp;
		temp = x_char[1]; x_char[1] = x_char[6]; x_char[6] = temp;
		temp = x_char[2]; x_char[2] = x_char[5]; x_char[5] = temp;
		temp = x_char[3]; x_char[3] = x_char[4]; x_char[4] = temp;
		x = (double *) x_char;

		return *x;
	}

	void put_datatype(const datatype dt)
	{
		memcpy(data + write_pos++, &dt, 1);
	}

	const datatype get_datatype()
	{
		datatype next_datatype;
		memcpy(&next_datatype, data + read_pos++, 1);

		return next_datatype;
	}

	const datatype preview_datatype()
	{
		datatype next_datatype;
		memcpy(&next_datatype, data + read_pos, 1);

		return next_datatype;
	}

	void check_datatype(const datatype & dt)
	{
		datatype next_datatype;
		memcpy(&next_datatype, data + read_pos, 1);

		if (next_datatype != dt) {
			string error_message = "Inconsistent datatypes: Expected ";
			error_message += get_datatype_name(dt);
			error_message += ", got ";
			error_message += (uint16_t) (next_datatype);
			error_message += get_datatype_name(next_datatype);
			std::cerr << "ERROR: " << error_message << std::endl;
			throw std::exception();
		}
		else ++read_pos;
	}

	void finish()
	{
		put_body_length();
	}

	// ========================================================================================================================================================

	void put_ClientID(const ClientID & x)
	{
		if (signed_ints_only) {
			int16_t temp = htobe16((int16_t) x);
			memcpy(data, &temp, 2);
		}
		else {
			uint16_t temp = htobe16(x);
			memcpy(data, &temp, 2);
		}
	}

	void put_SessionID(const SessionID & x)
	{
		if (signed_ints_only) {
			int16_t temp = htobe16((int16_t) x);
			memcpy(data + 2, &temp, 2);
		}
		else {
			uint16_t temp = htobe16(x);
			memcpy(data + 2, &temp, 2);
		}
	}

	void put_client_command(const client_command & x)
	{
		memcpy(data + 4, &x, 1);
	}

	void put_error_code(const alchemist_error_code & x)
	{
		memcpy(data + 5, &x, 1);
	}

	void put_body_length()
	{
		body_length = write_pos - header_length;
		if (signed_ints_only) {
			int32_t temp = htobe32((int32_t) body_length);
			memcpy(data + 6, &temp, 4);
		}
		else {
			uint32_t temp = htobe32(body_length);
			memcpy(data + 6, &temp, 4);
		}
	}

	void put_char(const char & x)
	{
		memcpy(data + write_pos, &x, 1);
		write_pos += 1;
	}

	void put_int8(const int8_t & x)
	{
		memcpy(data + write_pos, &x, 1);
		write_pos += 1;
	}

	void put_int16(const int16_t & x)
	{
		int16_t temp = htobe16(x);
		memcpy(data + write_pos, &temp, 2);
		write_pos += 2;
	}

	void put_int32(const int32_t & x)
	{
		int32_t temp = htobe32(x);
		memcpy(data + write_pos, &temp, 4);
		write_pos += 4;
	}

	void put_int64(const int64_t & x)
	{
		int64_t temp = htobe64(x);
		memcpy(data + write_pos, &temp, 8);
		write_pos += 8;
	}

	void put_uint8(const uint8_t & x)
	{
		memcpy(data + write_pos, &x, 1);
		write_pos += 1;
	}

	void put_uint16(const uint16_t & x)
	{
		uint16_t temp = htobe16(x);
		memcpy(data + write_pos, &temp, 2);
		write_pos += 2;
	}

	void put_uint32(const uint32_t & x)
	{
		uint32_t temp = htobe32(x);
		memcpy(data + write_pos, &temp, 4);
		write_pos += 4;
	}

	void put_uint64(const uint64_t & x)
	{
		uint64_t temp = htobe64(x);
		memcpy(data + write_pos, &temp, 8);
		write_pos += 8;
	}

	void put_float(const float & x)
	{
		float temp = x;
		if (reverse_floats) reverse_float(&temp);
		memcpy(data + write_pos, &temp, 4);
		write_pos += 4;
	}

	void put_double(const double & x)
	{
		double temp = x;
		if (reverse_floats) reverse_double(&temp);
		memcpy(data + write_pos, &temp, 8);
		write_pos += 8;
	}

	void put_string(const string & x)
	{
		uint16_t string_length = (uint16_t) x.length();
		signed_ints_only ? put_int16((int16_t) string_length) : put_uint16(string_length);
		auto cdata = x.c_str();
		memcpy(data + write_pos, cdata, string_length);
		write_pos += (uint32_t) string_length;
	}

	void put_LibraryID(const LibraryID & x)
	{
		signed_ints_only ? put_int8((int8_t) x) : put_uint8(x);
	}

	void put_GroupID(const GroupID & x)
	{
		signed_ints_only ? put_int16((int16_t) x) : put_uint16(x);
	}

	void put_TaskID(const TaskID & x)
	{
		signed_ints_only ? put_int16((int16_t) x) : put_uint16(x);
	}

	void put_WorkerID(const WorkerID & x)
	{
		signed_ints_only ? put_int16((int16_t) x) : put_uint16(x);
	}

	void put_WorkerInfo(const WorkerInfo_ptr & x)
	{
		put_WorkerID(x->ID);
		put_string(x->hostname);
		put_string(x->address);
		signed_ints_only ? put_int16((int16_t) x->port) : put_uint16(x->port);
		put_GroupID(x->groupID);
	}

	void put_ArrayID(const ArrayID & x)
	{
		signed_ints_only ? put_int16((int16_t) x) : put_uint16(x);
	}

	void put_ArrayInfo(const ArrayInfo_ptr & x)
	{
		put_ArrayID(x->ID);
		put_string(x->name);
		signed_ints_only ? put_int64((int64_t) x->num_rows) : put_uint64(x->num_rows);
		signed_ints_only ? put_int64((int64_t) x->num_cols) : put_uint64(x->num_cols);
		signed_ints_only ? put_int8((int8_t) x->sparse) : put_uint8(x->sparse);
		signed_ints_only ? put_int8((int8_t) x->layout) : put_uint8(x->layout);
		signed_ints_only ? put_int8((int8_t) x->num_partitions) : put_uint8(x->num_partitions);
		for (auto i = 0; i < x->num_partitions; i++)
			signed_ints_only ? put_int8((int8_t) x->worker_assignments[i]) : put_uint8(x->worker_assignments[i]);
	}

	void put_FloatArrayBlock(const FloatArrayBlock_ptr & x)
	{
		uint8_t ndims = x->ndims;
		signed_ints_only ? put_int8((int8_t) x->ndims) : put_uint8(x->ndims);
		signed_ints_only ? put_int64((int64_t) x->size) : put_uint64(x->size);
		for (uint8_t i = 0; i < 3; i++)
			for (uint8_t j = 0; j < ndims; j++)
				signed_ints_only ? put_int64((int64_t) x->dims[i][j]) : put_uint64(x->dims[i][j]);
		x->start = data + write_pos;
		write_pos += 4*x->size;
	}

	void put_DoubleArrayBlock(const DoubleArrayBlock_ptr & x)
	{
		uint8_t ndims = x->ndims;
		signed_ints_only ? put_int8((int8_t) x->ndims) : put_uint8(x->ndims);
		signed_ints_only ? put_int64((int64_t) x->size) : put_uint64(x->size);
		for (uint8_t i = 0; i < 3; i++)
			for (uint8_t j = 0; j < ndims; j++)
				signed_ints_only ? put_int64((int64_t) x->dims[i][j]) : put_uint64(x->dims[i][j]);
		x->start = data + write_pos;
		write_pos += 8*x->size;
	}

	// ========================================================================================================================================================


	void write_error_code(const alchemist_error_code & x)
	{
		put_error_code(x);
	}

	void write_byte(const int8_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(BYTE);
		put_int8(x);
	}

	void write_char(const char & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(CHAR);
		put_char(x);
	}

	void write_int8(const int8_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(INT8);
		put_int8(x);
	}

	void write_int16(const int16_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(INT16);
		put_int16(x);
	}

	void write_int32(const int32_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(INT32);
		put_int32(x);
	}
	void write_int64(const int64_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(INT64);
		put_int64(x);
	}

	void write_uint8(const uint8_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		if (signed_ints_only)
			write_int8((uint8_t) x);
		else {
			put_datatype(UINT8);
			put_uint8(x);
		}
	}

	void write_uint16(const uint16_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		if (signed_ints_only)
			write_int16((uint16_t) x);
		else {
			put_datatype(UINT16);
			put_uint16(x);
		}
	}

	void write_uint32(const uint32_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		if (signed_ints_only)
			write_int32((uint32_t) x);
		else {
			put_datatype(UINT32);
			put_uint32(x);
		}
	}

	void write_uint64(const uint64_t & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		if (signed_ints_only)
			write_int64((int64_t) x);
		else {
			put_datatype(UINT64);
			put_uint64(x);
		}
	}

	void write_float(const float & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(FLOAT);
		put_float(x);
	}

	void write_double(const double & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(DOUBLE);
		put_double(x);
	}

	void write_string(const string & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(STRING);
		put_string(x);
	}

	void write_LibraryID(const LibraryID & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(LIBRARY_ID);
		put_LibraryID(x);
	}

	void write_GroupID(const GroupID & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(GROUP_ID);
		put_GroupID(x);
	}

	void write_WorkerID(const WorkerID & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(WORKER_ID);
		put_WorkerID(x);
	}

	void write_WorkerInfo(const WorkerInfo_ptr & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(WORKER_INFO);
		put_WorkerInfo(x);
	}

	void write_ArrayID(const ArrayID & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(ARRAY_ID);
		put_ArrayID(x);
	}

	void write_ArrayInfo(const ArrayInfo_ptr & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(ARRAY_INFO);
		put_ArrayInfo(x);
	}

	void write_FloatArrayBlock(const FloatArrayBlock_ptr & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(ARRAY_BLOCK_FLOAT);
		put_FloatArrayBlock(x);
	}

	void write_DoubleArrayBlock(const DoubleArrayBlock_ptr & x, bool is_parameter = false)
	{
		if (is_parameter) put_datatype(PARAMETER);
		put_datatype(ARRAY_BLOCK_DOUBLE);
		put_DoubleArrayBlock(x);
	}

	// ========================================================================================================================================================

	const ClientID get_ClientID()
	{
		if (signed_ints_only) {
			int16_t x;
			memcpy(&x, data, 2);
			return (ClientID) be16toh(x);
		}
		else {
			uint16_t x;
			memcpy(&x, data, 2);
			return (ClientID) be16toh(x);
		}
	}

	const SessionID get_SessionID()
	{
		if (signed_ints_only) {
			int16_t x;
			memcpy(&x, data + 2, 2);
			return (SessionID) be16toh(x);
		}
		else {
			uint16_t x;
			memcpy(&x, data + 2, 2);
			return (SessionID) be16toh(x);
		}
	}

	const client_command get_client_command()
	{
		client_command cc;
		memcpy(&cc, data + 4, 1);

		return cc;
	}

	const alchemist_error_code get_error_code()
	{
		alchemist_error_code ec;
		memcpy(&ec, data + 5, 1);

		return ec;
	}

	const uint32_t get_body_length()
	{
		if (signed_ints_only) {
			int32_t x;
			memcpy(&x, data + 6, 4);
			return (uint32_t) be32toh(x);
		}
		else {
			uint32_t x;
			memcpy(&x, data + 6, 4);
			return be32toh(x);
		}
	}

	const uint8_t get_byte()
	{
		uint8_t x;

		memcpy(&x, data + read_pos, 1);
		read_pos += 1;

		return x;
	}

	const char get_char()
	{
		char x;

		memcpy(&x, data + read_pos, 1);
		read_pos += 1;

		return x;
	}

	const int8_t get_int8()
	{
		int8_t x;

		memcpy(&x, data + read_pos, 1);
		read_pos += 1;

		return x;
	}

	const int16_t get_int16()
	{
		int16_t x;

		memcpy(&x, data + read_pos, 2);
		read_pos += 2;

		return be16toh(x);
	}

	const int32_t get_int32()
	{
		int32_t x;

		memcpy(&x, data + read_pos, 4);
		read_pos += 4;

		return be32toh(x);
	}

	const int64_t get_int64()
	{
		int64_t x;

		memcpy(&x, data + read_pos, 8);
		read_pos += 8;

		return be64toh(x);
	}

	const uint8_t get_uint8()
	{
		uint8_t x;

		memcpy(&x, data + read_pos, 1);
		read_pos += 1;

		return x;
	}

	const uint16_t get_uint16()
	{
		uint16_t x;

		memcpy(&x, data + read_pos, 2);
		read_pos += 2;

		return be16toh(x);
	}

	const uint32_t get_uint32()
	{
		uint32_t x;

		memcpy(&x, data + read_pos, 4);
		read_pos += 4;

		return be32toh(x);
	}

	const uint64_t get_uint64()
	{
		uint64_t x;

		memcpy(&x, data + read_pos, 8);
		read_pos += 8;

		return be64toh(x);
	}

	const float get_float()
	{
		float x;

		memcpy(&x, data + read_pos, 4);
		if (reverse_floats) reverse_float(&x);
		read_pos += 4;

		return x;
	}

	const double get_double()
	{
		double x;

		memcpy(&x, data + read_pos, 8);
		if (reverse_floats) reverse_double(&x);
		read_pos += 8;

		return x;
	}

	const string get_string()
	{
		uint16_t string_length = signed_ints_only ? (uint16_t) get_int16() : get_uint16();
		char string_c[string_length+1];
		memcpy(string_c, data + read_pos, string_length);
		read_pos += string_length;
		string_c[string_length] = '\0';

		return string(string_c);
	}

	const LibraryID get_LibraryID()
	{
		return (LibraryID) (signed_ints_only ? get_int8() : get_uint8());
	}

	const GroupID get_GroupID()
	{
		return (GroupID) (signed_ints_only ? get_int16() : get_uint16());
	}

	const WorkerID get_WorkerID()
	{
		return (WorkerID) (signed_ints_only ? get_int16() : get_uint16());
	}

	const WorkerInfo_ptr get_WorkerInfo()
	{
		WorkerID ID = get_WorkerID();
		string hostname = get_string();
		string address = get_string();
		uint16_t port = (uint16_t) (signed_ints_only ? get_int16() : get_uint16());
		GroupID groupID = get_GroupID();

		return std::make_shared<WorkerInfo>(ID, hostname, address, port, groupID);
	}

	const ArrayID get_ArrayID()
	{
		return (ArrayID) (signed_ints_only ? get_int16() : get_uint16());
	}

	const ArrayInfo_ptr get_ArrayInfo()
	{
		ArrayID ID = get_ArrayID();
		string name = get_string();
		uint64_t num_rows = (uint64_t) (signed_ints_only ? get_int64() : get_uint64());
		uint64_t num_cols = (uint64_t) (signed_ints_only ? get_int64() : get_uint64());
		uint8_t sparse = (uint8_t) (signed_ints_only ? get_int8() : get_uint8());
		uint8_t layout = (uint8_t) (signed_ints_only ? get_int8() : get_uint8());
		uint8_t num_partitions = (uint8_t) (signed_ints_only ? get_int8() : get_uint8());

		ArrayInfo_ptr x = std::make_shared<ArrayInfo>(ID, name, num_rows, num_cols, sparse, layout, num_partitions);

		for (auto i = 0; i < num_partitions; i++)
			x->worker_assignments[i] = (uint8_t) (signed_ints_only ? get_int8() : get_uint8());

		return x;
	}

	const FloatArrayBlock_ptr get_FloatArrayBlock()
	{
		uint8_t ndims = (uint8_t) (signed_ints_only ? get_int8() : get_uint8());
		FloatArrayBlock_ptr block = std::make_shared<ArrayBlock<float>>(ndims);
		block->size = (uint64_t) (signed_ints_only ? get_int64() : get_uint64());
		for (uint8_t j = 0; j < ndims; j++)
			for (uint8_t k = 0; k < 3; k++)
				block->dims[k][j] = (uint64_t) (signed_ints_only ? get_int64() : get_uint64());
		block->start = data + read_pos;
		read_pos += 4*block->size;

		return block;
	}

	const DoubleArrayBlock_ptr get_DoubleArrayBlock()
	{
		uint8_t ndims = (uint8_t) (signed_ints_only ? get_int8() : get_uint8());
		DoubleArrayBlock_ptr block = std::make_shared<ArrayBlock<double>>(ndims);
		block->size = (uint64_t) (signed_ints_only ? get_int64() : get_uint64());
		for (uint8_t j = 0; j < ndims; j++)
			for (uint8_t k = 0; k < 3; k++)
				block->dims[k][j] = (uint64_t) (signed_ints_only ? get_int64() : get_uint64());
		block->start = data + read_pos;
		read_pos += 8*block->size;

		return block;
	}

	// ========================================================================================================================================================

	const client_language read_client_language()
	{
		client_language cl;
		datatype dt = preview_datatype();
		if (dt == INT8)
			cl = (client_language) read_int8();
		else if (dt == UINT8)
			cl = (client_language) read_uint8();

		set_client_language(cl);

		return cl;
	}

	const uint8_t read_byte()
	{
		check_datatype(BYTE);

		return get_byte();
	}

	const char read_char()
	{
		check_datatype(CHAR);

		return get_char();
	}

	const int8_t read_int8()
	{
		check_datatype(INT8);

		return get_int8();
	}

	const int16_t read_int16()
	{
		check_datatype(INT16);

		return get_int16();
	}

	const int32_t read_int32()
	{
		check_datatype(INT32);

		return get_int32();
	}

	const int64_t read_int64()
	{
		check_datatype(INT64);

		return get_int64();
	}

	const uint8_t read_uint8()
	{
		if (signed_ints_only) {
			check_datatype(INT8);
			return (uint8_t) get_int8();
		}
		else {
			check_datatype(UINT8);
			return get_uint8();
		}
	}

	const uint16_t read_uint16()
	{
		if (signed_ints_only) {
			check_datatype(INT16);
			return (uint16_t) get_int16();
		}
		else {
			check_datatype(UINT16);
			return get_uint16();
		}
	}

	const uint32_t read_uint32()
	{
		if (signed_ints_only) {
			check_datatype(INT32);
			return (uint32_t) get_int32();
		}
		else {
			check_datatype(UINT32);
			return get_uint32();
		}
	}

	const uint64_t read_uint64()
	{
		if (signed_ints_only) {
			check_datatype(INT64);
			return (uint64_t) get_int64();
		}
		else {
			check_datatype(UINT64);
			return get_uint64();
		}
	}

	const float read_float()
	{
		check_datatype(FLOAT);

		return get_float();
	}

	const double read_double()
	{
		check_datatype(DOUBLE);

		return get_double();
	}

	const string read_string()
	{
		check_datatype(STRING);

		return get_string();
	}

	const LibraryID read_LibraryID()
	{
		check_datatype(LIBRARY_ID);

		return get_LibraryID();
	}

	const GroupID read_GroupID()
	{
		check_datatype(GROUP_ID);

		return get_GroupID();
	}

	const WorkerID read_WorkerID()
	{
		check_datatype(WORKER_ID);

		return get_WorkerID();
	}

	const WorkerInfo_ptr read_WorkerInfo()
	{
		check_datatype(WORKER_INFO);

		return get_WorkerInfo();
	}

	const ArrayID read_ArrayID()
	{
		check_datatype(ARRAY_ID);

		return get_ArrayID();
	}

	const ArrayInfo_ptr read_ArrayInfo()
	{
		check_datatype(ARRAY_INFO);

		return get_ArrayInfo();
	}

	const FloatArrayBlock_ptr read_FloatArrayBlock()
	{
		check_datatype(ARRAY_BLOCK_FLOAT);

		return get_FloatArrayBlock();
	}

	const DoubleArrayBlock_ptr read_DoubleArrayBlock()
	{
		check_datatype(ARRAY_BLOCK_DOUBLE);

		return get_DoubleArrayBlock();
	}

	// ========================================================================================================================================================

	bool compare_array_block(DoubleArrayBlock_ptr block, double * temp)
	{
		double local;
		for (uint64_t i = 0; i < block->size; i++) {
			memcpy(&local, block->start + 8*i, 8);
			if (local != temp[i]) {
				if (reverse_floats) reverse_double(&local);
				if (local != temp[i]) return false;
			}
		}
		return true;
	}

	const string to_string()
	{
		decode_header();

		stringstream ss;
		datatype dt;
		string dt_name, space = "                                                  ";

		ss << std::endl;
		ss << space << "=========================================================" << std::endl;
		ss << space << "Client ID:                 " << clientID << std::endl;
		ss << space << "Session ID:                " << sessionID << std::endl;
		ss << space << "Command code:              " << cc << " (" << get_command_name(cc) << ") " << std::endl;
		ss << space << "Error code:                " << ec << " (" << get_error_name(ec) << ") " << std::endl;
		ss << space << "Message body length:       " << body_length << std::endl;
		ss << space << "---------------------------------------------------------" << std::endl;
		ss << std::endl;

		while (!eom()) {
			dt = get_datatype();
			dt_name = get_datatype_name(dt);
			ss << space << dt_name.append(27 - dt_name.length(), ' ');

			switch (dt) {
			case BYTE:
				ss << (int16_t) get_byte();
				break;
			case CHAR:
				ss << get_char();
				break;
			case INT8:
				ss << (int16_t) get_int8();
				break;
			case INT16:
				ss << get_int16();
				break;
			case INT32:
				ss << get_int32();
				break;
			case INT64:
				ss << get_int64();
				break;
			case UINT8:
				ss << (int16_t) get_uint8();
				break;
			case UINT16:
				ss << get_uint16();
				break;
			case UINT32:
				ss << get_uint32();
				break;
			case UINT64:
				ss << get_uint64();
				break;
			case FLOAT:
				ss << get_float();
				break;
			case DOUBLE:
				ss << get_double();
				break;
			case STRING:
				ss << get_string();
				break;
			case WORKER_ID:
				ss << get_WorkerID();
				break;
			case WORKER_INFO:
				ss << get_WorkerInfo()->to_string();
				break;
			case ARRAY_ID:
				ss << get_ArrayID();
				break;
			case ARRAY_INFO:
				ss << get_ArrayInfo()->to_string();
				break;
			case ARRAY_BLOCK_DOUBLE:
				ss << get_DoubleArrayBlock()->to_string();
				break;
			case ARRAY_BLOCK_FLOAT:
				ss << get_FloatArrayBlock()->to_string();
				break;
			case LIBRARY_ID:
				ss << (int16_t) get_LibraryID();
				break;
			default:
				ss << "Invalid datatype";
				break;
			}
			ss << "\n";
		}
		ss << space << "=========================================================" << std::endl;

		read_pos = header_length;

		return ss.str();
	}


};

}

#endif // ALCHEMIST__MESSAGE_HPP



















//const void read_next(stringstream & ss, uint32_t & i, const datatype & dt)
//{
//	switch (dt) {
//		case BYTE:
//			ss << (int16_t) read_byte(i);
//			break;
//		case CHAR:
//			ss << read_signed_char(i);
//			break;
//		case UNSIGNED_CHAR:
//			ss << read_unsigned_char(i);
//			break;
//		case SIGNED_CHAR:
//			ss << read_signed_char(i);
//			break;
//		case CHARACTER:
//			ss << read_character(i);
//			break;
////			case WCHAR: {
////					std::wstring str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(read_wchar(i));
////					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
////
////					ss << converter.to_bytes(str);
////				}
////				break;
//		case BOOL:
//			ss << read_bool(i);
//			break;
//		case LOGICAL:
//			ss << read_logical(i);
//			break;
//		case SHORT:
//			ss << read_short(i);
//			break;
//		case UNSIGNED_SHORT:
//			ss << read_unsigned_short(i);
//			break;
//		case LONG:
//			ss << read_long(i);
//			break;
//		case UNSIGNED_LONG:
//			ss << read_unsigned_long(i);
//			break;
//		case INTEGER1:
//			ss << read_integer1(i);
//			break;
//		case INT8_T:
//			ss << (int16_t) read_int8(i);
//			break;
//		case UINT8_T:
//			ss << (int16_t) read_uint8(i);
//			break;
//		case INTEGER2:
//			ss << read_integer2(i);
//			break;
//		case INT16_T:
//			ss << read_int16(i);
//			break;
//		case UINT16_T:
//			ss << read_uint16(i);
//			break;
//		case INTEGER4:
//			ss << read_integer4(i);
//			break;
//		case INT32_T:
//			ss << read_int32(i);
//			break;
//		case UINT32_T:
//			ss << read_uint32(i);
//			break;
//		case INTEGER8:
//			ss << read_integer8(i);
//			break;
//		case INT64_T:
//			ss << read_int64(i);
//			break;
//		case UINT64_T:
//			ss << read_uint64(i);
//			break;
//		case FLOAT:
//			ss << read_float(i);
//			break;
//		case DOUBLE:
//			ss << read_double(i);
//			break;
//		case REAL:
//			ss << read_real(i);
//			break;
//		case MATRIXID:
//			ss << read_matrixID(i);
//			break;
//		case MATRIX_BLOCK:
//			ss << read_array_block(i);
//			break;
//		case LIBRARYID:
//			ss << (int16_t) read_libraryID(i);
//			break;
//		default:
//			ss << "Invalid datatype";
//			break;
//		}
//}

//void add_string(const string & str)
//{
//	uint32_t string_length = (uint32_t) str.length();
//	write_uint32(string_length);
//	auto cdata = str.c_str();
//	memcpy(data + write_pos, cdata, string_length);
//	write_pos += string_length;
//}
//
//void put_string(const string & str)
//{
//	uint32_t string_length = (uint32_t) str.length();
//	put_uint32(string_length);
//	auto cdata = str.c_str();
//	memcpy(data + write_pos, cdata, string_length);
//	write_pos += string_length;
//}
//
//void write_string(const string & str)
//{
//	check_datatype(STRING);
//
//	put_string(str);
//
//	current_datatype_count += 1;
//}
//
//void add_byte(const unsigned char * _data, uint32_t length)
//{
//	check_datatype(BYTE);
//
//	unsigned char temp_byte;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_byte = _data[i];
//		memcpy(data + write_pos, &temp_byte, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_byte(const unsigned char _data)
//{
//	check_datatype(BYTE);
//
//	unsigned char temp_byte = _data;
//	memcpy(data + write_pos, &temp_byte, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_bool(const bool * _data, uint32_t length)
//{
//	check_datatype(BOOL);
//
//	bool temp_bool;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_bool = _data[i];
//		memcpy(data + write_pos, &temp_bool, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_bool(const unsigned char _data)
//{
//	check_datatype(BOOL);
//
//	bool temp_bool = _data;
//	memcpy(data + write_pos, &temp_bool, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_logical(const bool * _data, uint32_t length)
//{
//	check_datatype(LOGICAL);
//
//	bool temp_bool;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_bool = _data[i];
//		memcpy(data + write_pos, &temp_bool, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_logical(const unsigned char _data)
//{
//	check_datatype(LOGICAL);
//
//	bool temp_bool = _data;
//	memcpy(data + write_pos, &temp_bool, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_char(const char * _data, uint32_t length)
//{
//	check_datatype(CHAR);
//
//	unsigned char temp_char;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_char = _data[i];
//		memcpy(data + write_pos, &temp_char, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_char(const char _data)
//{
//	check_datatype(CHAR);
//
//	unsigned char temp_char = _data;
//	memcpy(data + write_pos, &temp_char, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_unsigned_char(const unsigned char & _data)
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		add_unsigned_char_(_data);
//		break;
//	case SCALA:
//	case JAVA:
//		add_char((char) _data);
//		break;
//	case PYTHON:
//	case JULIA:
//		add_char((char) _data);
//		break;
//	}
//}
//
//void add_unsigned_char(const unsigned char * _data, uint32_t length)
//{
//	check_datatype(UNSIGNED_CHAR);
//
//	unsigned char temp_uchar;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_uchar = _data[i];
//		memcpy(data + write_pos, &temp_uchar, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_unsigned_char_(const unsigned char _data)
//{
//	check_datatype(UNSIGNED_CHAR);
//
//	unsigned char temp_uchar = _data;
//	memcpy(data + write_pos, &temp_uchar, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_wchar(const wchar_t * _data, uint32_t length)
//{
//	check_datatype(WCHAR);
//
//	wchar_t temp_wchar;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_wchar = _data[i];
//		temp_wchar = htobe16(temp_wchar);
//		memcpy(data + write_pos, &temp_wchar, 2);
//		write_pos += 2;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_wchar(const wchar_t _data)
//{
//	check_datatype(WCHAR);
//
//	wchar_t temp_wchar = _data;
//	temp_wchar = htobe16(temp_wchar);
//	memcpy(data + write_pos, &temp_wchar, 2);
//	write_pos += 2;
//
//	current_datatype_count += 1;
//}
//
//void add_int(const int * _data, uint32_t length)
//{
//	check_datatype(INT);
//
//	int temp_int;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int = _data[i];
//		memcpy(data + write_pos, &temp_int, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_int(const int _data)
//{
//	check_datatype(INT);
//
//	int temp_int = _data;
//	memcpy(data + write_pos, &temp_int, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_integer1(const int8_t * _data, uint32_t length)
//{
//	check_datatype(INTEGER1);
//
//	int8_t temp_int8;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int8 = _data[i];
//		memcpy(data + write_pos, &temp_int8, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_integer1(const int8_t _data)
//{
//	check_datatype(INTEGER1);
//
//	int8_t temp_int8 = _data;
//	memcpy(data + write_pos, &temp_int8, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_integer2(const int16_t * _data, uint32_t length)
//{
//	check_datatype(INTEGER2);
//
//	int16_t temp_int16;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int16 = _data[i];
//		memcpy(data + write_pos, &temp_int16, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_integer2(const int16_t _data)
//{
//	check_datatype(INTEGER2);
//
//	int16_t temp_int16 = _data;
//	memcpy(data + write_pos, &temp_int16, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_integer4(const int32_t * _data, uint32_t length)
//{
//	check_datatype(INTEGER4);
//
//	int32_t temp_int32;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int32 = _data[i];
//		memcpy(data + write_pos, &temp_int32, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_integer4(const int32_t _data)
//{
//	check_datatype(INTEGER4);
//
//	int16_t temp_int32 = _data;
//	memcpy(data + write_pos, &temp_int32, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_integer8(const int64_t * _data, uint32_t length)
//{
//	check_datatype(INTEGER8);
//
//	int64_t temp_int64;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int64 = _data[i];
//		memcpy(data + write_pos, &temp_int64, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_integer8(const int64_t _data)
//{
//	check_datatype(INTEGER8);
//
//	int16_t temp_int64 = _data;
//	memcpy(data + write_pos, &temp_int64, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_int8(const int8_t * _data, uint32_t length)
//{
//	check_datatype(INT8_T);
//
//	int8_t temp_int8;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int8 = _data[i];
//		memcpy(data + write_pos, &temp_int8, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_int8(const int8_t & _data)
//{
//	check_datatype(INT8_T);
//
//	memcpy(data + write_pos, &_data, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_uint8(const uint8_t & _data) {
//
//	switch(cl) {
//	case C:
//	case CPP:
//		add_uint8_(_data);
//		break;
//	case SCALA:
//	case JAVA:
//		add_int8((int8_t) _data);
//		break;
//	case PYTHON:
//	case JULIA:
//		add_int8((int8_t) _data);
//		break;
//	}
//}
//
//void add_uint8(const uint8_t * _data, uint32_t length)
//{
//	check_datatype(UINT8_T);
//
//	uint8_t temp_uint8;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_uint8 = _data[i];
//		memcpy(data + write_pos, &temp_uint8, 1);
//		write_pos += 1;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_uint8_(const uint8_t _data)
//{
//	check_datatype(UINT8_T);
//
//	uint8_t temp_uint8 = _data;
//	memcpy(data + write_pos, &temp_uint8, 1);
//	write_pos += 1;
//
//	current_datatype_count += 1;
//}
//
//void add_int16(const int16_t * _data, uint32_t length)
//{
//	check_datatype(INT16_T);
//
//	int16_t temp_int16;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int16 = _data[i];
//		temp_int16 = htobe16(temp_int16);
//		memcpy(data + write_pos, &temp_int16, 2);
//		write_pos += 2;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_int16(const int16_t _data)
//{
//	check_datatype(INT16_T);
//
//	int16_t temp_int16 = htobe16(_data);
//	memcpy(data + write_pos, &temp_int16, 2);
//	write_pos += 2;
//
//	current_datatype_count += 1;
//}
//
//void add_uint16(const uint16_t & _data) {
//
//	switch(cl) {
//	case C:
//	case CPP:
//		add_uint16_(_data);
//		break;
//	case SCALA:
//	case JAVA:
//		add_int16((int16_t) _data);
//		break;
//	case PYTHON:
//	case JULIA:
//		add_int16((int16_t) _data);
//		break;
//	}
//}
//
//void add_uint16(const uint16_t * _data, uint32_t length)
//{
//	check_datatype(UINT16_T);
//
//	uint16_t temp_uint16;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_uint16 = _data[i];
//		temp_uint16 = htobe16(temp_uint16);
//		memcpy(data + write_pos, &temp_uint16, 2);
//		write_pos += 2;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_uint16_(const uint16_t _data)
//{
//	check_datatype(UINT16_T);
//
//	uint16_t temp_uint16 = htobe16(_data);
//	memcpy(data + write_pos, &temp_uint16, 2);
//	write_pos += 2;
//
//	current_datatype_count += 1;
//}
//
//void add_int32(const int32_t * _data, uint32_t length)
//{
//	check_datatype(INT32_T);
//
//	int32_t temp_int32;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int32 = _data[i];
//		temp_int32 = htobe32(temp_int32);
//		memcpy(data + write_pos, &temp_int32, 4);
//		write_pos += 4;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_int32(const int32_t _data)
//{
//
//	check_datatype(INT32_T);
//	int32_t temp_int32 = _data;
//	temp_int32 = htobe32(temp_int32);
//	memcpy(data + write_pos, &temp_int32, 4);
//	write_pos += 4;
//
//	current_datatype_count += 1;
//}
//
//void write_uint32(const uint32_t & _data)
//{
//	switch(cl) {
//	case SCALA:
//	case JAVA:
//	case PYTHON:
//	case JULIA:
//		write_int32((int32_t) _data);
//		break;
//	default:
//		write_uint32_(_data);
//		break;
//	}
//}
//
//void write_uint32_(const uint32_t & _data)
//{
//	check_datatype(UINT32_T);
//
//	put_uint32(_data);
//
//	current_datatype_count += 1;
//}
//
//void put_uint32(const uint32_t & _data)
//{
//	uint32_t temp_uint32 = htobe32(_data);
//	memcpy(data + write_pos, &temp_uint32, 4);
//	write_pos += 4;
//}
//
//void write_int32(const int32_t & _data)
//{
//	check_datatype(INT32_T);
//
//	put_uint32(_data);
//
//	current_datatype_count += 1;
//}
//
//void put_int32(const int32_t & _data)
//{
//	int32_t temp_int32 = htobe32(_data);
//	memcpy(data + write_pos, &temp_int32, 4);
//	write_pos += 4;
//}
//
//void add_uint32(const uint32_t * _data, uint32_t length)
//{
//	check_datatype(UINT32_T);
//
//	uint32_t temp_uint32;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_uint32 = _data[i];
//		temp_uint32 = htobe32(temp_uint32);
//		memcpy(data + write_pos, &temp_uint32, 4);
//		write_pos += 4;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_int64(const int64_t * _data, uint32_t length)
//{
//	check_datatype(INT64_T);
//
//	uint64_t temp_int64;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_int64 = _data[i];
//		temp_int64 = htobe64(temp_int64);
//		memcpy(data + write_pos, &temp_int64, 8);
//		write_pos += 8;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_int64(const int64_t _data)
//{
//	check_datatype(INT64_T);
//
//	int64_t temp_int64 = _data;
//	temp_int64 = htobe64(temp_int64);
//	memcpy(data + write_pos, &temp_int64, 8);
//	write_pos += 8;
//
//	current_datatype_count += 1;
//}
//
//void add_uint64(const uint64_t & _data) {
//
//	switch(cl) {
//	case C:
//	case CPP:
//		add_uint64_(_data);
//		break;
//	case SCALA:
//	case JAVA:
//		add_int64((int64_t) _data);
//		break;
//	case PYTHON:
//	case JULIA:
//		add_int64((int64_t) _data);
//		break;
//	}
//}
//
//void add_uint64(const uint64_t * _data, uint32_t length)
//{
//	check_datatype(UINT64_T);
//
//	uint64_t temp_uint64;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_uint64 = _data[i];
//		temp_uint64 = htobe64(temp_uint64);
//		memcpy(data + write_pos, &temp_uint64, 8);
//		write_pos += 8;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_uint64_(const uint64_t & _data)
//{
//	check_datatype(UINT64_T);
//
//	uint64_t temp_uint64 = _data;
//	temp_uint64 = htobe64(temp_uint64);
//	memcpy(data + write_pos, &temp_uint64, 8);
//	write_pos += 8;
//
//	current_datatype_count += 1;
//}
//
//void add_float(const float * _data, uint32_t length)
//{
//	check_datatype(FLOAT);
//
//	float temp_float;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_float = _data[i];
//		reverse_float(&temp_float);
//		memcpy(data + write_pos, &temp_float, 4);
//		write_pos += 4;
//	}
//
//	current_datatype_count += length;
//}
//
//void add_float(const float _data)
//{
//	check_datatype(FLOAT);
//
//	float temp_float = _data;
//	reverse_float(&temp_float);
//	memcpy(data + write_pos, &temp_float, 4);
//	write_pos += 4;
//
//	current_datatype_count += 1;
//}
//
//void add_double(const double * _data, uint32_t length)
//{
//	check_datatype(DOUBLE);
//
//	double temp_double;
//
//	for (uint32_t i = 0; i < length; i++) {
//		temp_double = _data[i];
//		if (reverse_floats) reverse_double(&temp_double);
//		memcpy(data + write_pos, &temp_double, 8);
//		write_pos += 8;
//	}
//
//	current_datatype_count += length;
//}
//
//void put_double(const double & _data)
//{
//	double temp_double = _data;
//	if (reverse_floats) reverse_double(&temp_double);
//	memcpy(data + write_pos, &temp_double, 8);
//	write_pos += 8;
//}
//
//void add_double(const double & _data)
//{
//	check_datatype(DOUBLE);
//
//	double temp_double = _data;
//	if (reverse_floats) reverse_double(&temp_double);
//	memcpy(data + write_pos, &temp_double, 8);
//	write_pos += 8;
//
//	current_datatype_count += 1;
//}
//
//void add_parameter()
//{
//	check_datatype(PARAMETER);
//}









//void update_datatype_count()
//{
//	if (!data_copied) {
//		int32_t _current_datatype_count = htobe32(current_datatype_count);
//		memcpy(data + current_datatype_count_pos, &_current_datatype_count, 4);
//	}
//	data_copied = false;
//	}
//

//	void check_datatype(const datatype new_datatype)
//	{
//		if (current_datatype != new_datatype) {
//			current_datatype = new_datatype;
//
//			update_datatype_count();
//
//			memcpy(data + write_pos, &current_datatype, 1);
//
//			current_datatype_count = 0;
//			current_datatype_count_pos = write_pos + 1;
//			write_pos += 5;
//		}
////		else current_datatype_count += 1;
//	}


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

//const uint32_t next_data_length()
//{
//	uint32_t data_length;
//
//	memcpy(&data_length, data + read_pos + sizeof(datatype), 4);
//
//	return be32toh(data_length);
//}
//
//inline void read_atom(uint32_t & i, void * a, int psize)
//{
//	memcpy(a, data + i, psize);
//}
//
//const char read_char()
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		return read_char_();
//	case SCALA:
//	case JAVA:
//		return (char) read_char_();
//	case PYTHON:
//	case JULIA:
//		return (char) read_char_();
//	default:
//		return (char) '0';
//	}
//}
//
//const char read_char(uint32_t & i)
//{
//	char x;
//	read_atom(i, &x, 1);
//
//	return x;
//}
//
//const char read_char_()
//{
//	if (current_datatype != CHAR) {
//		current_datatype = CHAR;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	signed char x;
//
//	memcpy(&x, &data[read_pos], 1);
//	read_pos += 1;
//
//	return x;
//}
//
//const unsigned char read_signed_char()
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		return read_signed_char_();
//	case SCALA:
//	case JAVA:
//		return (signed char) read_signed_char_();
//	case PYTHON:
//	case JULIA:
//		return (signed char) read_signed_char_();
//	default:
//		return (signed char) '0';
//	}
//}
//
//const signed char read_signed_char(uint32_t & i)
//{
//	signed char x;
//	read_atom(i, &x, 1);
//
//	return x;
//}
//
//const signed char read_signed_char_()
//{
//	if (current_datatype != SIGNED_CHAR) {
//		current_datatype = SIGNED_CHAR;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	signed char x;
//
//	memcpy(&x, &data[read_pos], 1);
//	read_pos += 1;
//
//	return x;
//}
//
//const unsigned char read_unsigned_char()
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		return read_unsigned_char_();
//	case SCALA:
//	case JAVA:
//		return (unsigned char) read_unsigned_char_();
//	case PYTHON:
//	case JULIA:
//		return (unsigned char) read_unsigned_char_();
//	default:
//		return (unsigned char) '0';
//	}
//}
//
//const unsigned char read_unsigned_char(uint32_t & i)
//{
//	unsigned char x;
//	read_atom(i, &x, 1);
//
//	return x;
//}
//
//const unsigned char read_unsigned_char_()
//{
//	if (current_datatype != UNSIGNED_CHAR) {
//		current_datatype = UNSIGNED_CHAR;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	unsigned char x;
//
//	memcpy(&x, &data[read_pos], 1);
//	read_pos += 1;
//
//	return x;
//}
//
//const char read_character()
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		return read_character_();
//	case SCALA:
//	case JAVA:
//		return (char) read_character_();
//	case PYTHON:
//	case JULIA:
//		return (char) read_character_();
//	default:
//		return (char) '0';
//	}
//}
//
//const char read_character(uint32_t & i)
//{
//	char x;
//	read_atom(i, &x, 1);
//
//	return x;
//}
//
//const char read_character_()
//{
//	if (current_datatype != CHARACTER) {
//		current_datatype = CHARACTER;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	char x;
//
//	memcpy(&x, &data[read_pos], 1);
//	read_pos += 1;
//
//	return x;
//}
//
//const wchar_t read_wchar()
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		return read_character_();
//	case SCALA:
//	case JAVA:
//		return read_character_();
//	case PYTHON:
//	case JULIA:
//		return read_character_();
//	default:
//		return (wchar_t) '0';
//	}
//}
//
//const wchar_t read_wchar(uint32_t & i)
//{
//	wchar_t x;
//	read_atom(i, &x, 2);
//	x = (wchar_t) be16toh(x);
//
//	return x;
//}
//
//const wchar_t read_wchar_()
//{
//	if (current_datatype != WCHAR) {
//		current_datatype = WCHAR;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	wchar_t x;
//
//	memcpy(&x, &data[read_pos], 2);
//	read_pos += 2;
//
//	return (wchar_t) x;
//}
//
//const uint8_t read_byte()
//{
//	if (current_datatype != BYTE) {
//		current_datatype = BYTE;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_byte(read_pos);
//}
//
//const uint8_t read_byte(uint32_t & i)
//{
//	uint8_t x;
//	memcpy(&x, data + i, 1);
//	i += 1;
//
//	return x;
//}
//
//const bool read_bool()
//{
//	if (current_datatype != BOOL) {
//		current_datatype = BOOL;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	bool x;
//	memcpy(&x, data + read_pos, 1);
//	read_pos += 1;
//
//	return x;
//}
//
//const bool read_bool(uint32_t & i)
//{
//	bool x;
//	memcpy(&x, data + i, 1);
//	i += 1;
//
//	return x;
//}
//
//const bool read_logical()
//{
//	return read_bool();
//}
//
//const bool read_logical(uint32_t & i)
//{
//	return read_bool(i);
//}
//
//const short read_short()
//{
//	return (short) read_int16();
//}
//
//const short read_short(uint32_t & i)
//{
//	return (short) read_int16(i);
//}
//
//const unsigned short read_unsigned_short()
//{
//	return (unsigned short) read_uint16();
//}
//
//const unsigned short read_unsigned_short(uint32_t & i)
//{
//	return (unsigned short) read_uint16(i);
//}
//
//const int read_integer()
//{
//	return (int) read_int32();
//}
//
//const int read_integer(uint32_t & i)
//{
//	return (int) read_int32(i);
//}
//
//const int read_int()
//{
//	return (int) read_int32();
//}
//
//const int read_int(uint32_t & i)
//{
//	return (int) read_int32(i);
//}
//
//const unsigned int read_unsigned()
//{
//	return (unsigned int) read_uint32();
//}
//
//const unsigned int read_unsigned(uint32_t & i)
//{
//	return (unsigned int) read_uint32(i);
//}
//
//const unsigned int read_unsigned_int()
//{
//	return (unsigned int) read_uint32();
//}
//
//const unsigned int read_unsigned_int(uint32_t & i)
//{
//	return (unsigned int) read_uint32(i);
//}
//
//const long read_long()
//{
//	return (long) read_int64();
//}
//
//const long read_long(uint32_t & i)
//{
//	return (long) read_int64(i);
//}
//
//const unsigned long read_unsigned_long()
//{
//	return (unsigned long) read_uint64();
//}
//
//const unsigned long read_unsigned_long(uint32_t & i)
//{
//	return (unsigned long) read_uint64(i);
//}
//
//const long long read_long_long()
//{
//	if (current_datatype != LONG_LONG) {
//		current_datatype = LONG_LONG;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	long long x;
//	memcpy(&x, data + read_pos, sizeof(long long));
//	read_pos += sizeof(long long);
//
//	return x;
//}
//
//const long long read_long_long(uint32_t & i)
//{
//	long long x;
//	memcpy(&x, data + i, sizeof(long long));
//	i + sizeof(long long);
//
//	return x;
//}
//
//const long long read_long_long_int()
//{
//	return read_long_long();
//}
//
//const long long read_long_long_int(uint32_t & i)
//{
//	return read_long_long(i);
//}
//
//const unsigned long long read_unsigned_long_long()
//{
//	if (current_datatype != UNSIGNED_LONG_LONG) {
//		current_datatype = UNSIGNED_LONG_LONG;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	unsigned long long x;
//	memcpy(&x, data + read_pos, sizeof(unsigned long long));
//	read_pos += sizeof(unsigned long long);
//
//	return x;
//}
//
//const unsigned long long read_unsigned_long_long(uint32_t & i)
//{
//	unsigned long long x;
//	memcpy(&x, data + i, sizeof(unsigned long long));
//	i + sizeof(unsigned long long);
//
//	return x;
//}
//
//const int8_t read_integer1(uint32_t & i)
//{
//	int8_t x;
//	read_atom(i, &x, 1);
//
//	return x;
//}
//
//const void read_integer1(int8_t * x)
//{
//	uint32_t data_length;
//
//	read_pos += sizeof(datatype);
//	memcpy(&data_length, &data[read_pos], 4);
//	data_length = be32toh(data_length);
//	read_pos += 4;
//
//	memcpy(x, &data[read_pos], data_length);
//
//	read_pos += data_length;
//}
//
//const int8_t read_integer1()
//{
//	if (current_datatype != INTEGER1) {
//		current_datatype = INTEGER1;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	int8_t x;
//
//	memcpy(&x, &data[read_pos], 1);
//	read_pos += 1;
//
//	return x;
//}
//
//const int16_t read_integer2(uint32_t & i)
//{
//	int16_t x;
//	read_atom(i, &x, 2);
//
//	return x;
//}
//
//const void read_integer2(int16_t * x)
//{
//	uint32_t data_length;
//
//	read_pos += sizeof(datatype);
//	memcpy(&data_length, &data[read_pos], 4);
//	data_length = be32toh(data_length);
//	read_pos += 4;
//
//	memcpy(x, &data[read_pos], 2*data_length);
//
//	read_pos += 2*data_length;
//}
//
//const int16_t read_integer2()
//{
//	if (current_datatype != INTEGER2) {
//		current_datatype = INTEGER2;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	int16_t x;
//
//	memcpy(&x, &data[read_pos], 2);
//	read_pos += 2;
//
//	return x;
//}
//
//const int32_t read_integer4(uint32_t & i)
//{
//	int32_t x;
//	read_atom(i, &x, 4);
//
//	return x;
//}
//
//const void read_integer4(int32_t * x)
//{
//	uint32_t data_length;
//
//	read_pos += sizeof(datatype);
//	memcpy(&data_length, &data[read_pos], 4);
//	data_length = be32toh(data_length);
//	read_pos += 4;
//
//	memcpy(x, &data[read_pos], 4*data_length);
//
//	read_pos += 4*data_length;
//}
//
//const int32_t read_integer4()
//{
//	if (current_datatype != INTEGER4) {
//		current_datatype = INTEGER4;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	int32_t x;
//
//	memcpy(&x, &data[read_pos], 4);
//	read_pos += 4;
//
//	return x;
//}
//
//const int64_t read_integer8(uint32_t & i)
//{
//	int64_t x;
//	read_atom(i, &x, 8);
//
//	return x;
//}
//
//const void read_integer8(int64_t * x)
//{
//	uint32_t data_length;
//
//	read_pos += sizeof(datatype);
//	memcpy(&data_length, &data[read_pos], 4);
//	data_length = be32toh(data_length);
//	read_pos += 4;
//
//	memcpy(x, &data[read_pos], 8*data_length);
//
//	read_pos += 8*data_length;
//}
//
//const int64_t read_integer8()
//{
//	if (current_datatype != INTEGER8) {
//		current_datatype = INTEGER8;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	int64_t x;
//
//	memcpy(&x, &data[read_pos], 8);
//	read_pos += 8;
//
//	return x;
//}
//
//const void read_int8(int8_t * x)
//{
//	uint32_t data_length;
//
//	read_pos += sizeof(datatype);
//	memcpy(&data_length, &data[read_pos], 4);
//	data_length = be32toh(data_length);
//	read_pos += 4;
//
//	memcpy(x, &data[read_pos], data_length);
//
//	read_pos += data_length;
//}
//
//const int8_t read_int8()
//{
//	if (current_datatype != INT8_T) {
//		current_datatype = INT8_T;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_int8(read_pos);
//}
//
//const int8_t read_int8(uint32_t & i)
//{
//	int8_t x;
//	memcpy(&x, data + i, 1);
//	i += 1;
//
//	return x;
//}
//
//const uint8_t read_uint8()
//{
//	if (current_datatype != UINT8_T) {
//		current_datatype = UINT8_T;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_uint8(read_pos);
//}
//
//const uint8_t read_uint8(uint32_t & i)
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		{
//			uint8_t x;
//			memcpy(&x, data + i, 1);
//			i += 1;
//
//			return x;
//		}
//	case SCALA:
//	case JAVA:
//		{
//			int8_t x;
//			memcpy(&x, data + i, 1);
//			i += 1;
//
//			return (uint8_t) x;
//		}
//	case PYTHON:
//	case JULIA:
//		{
//			int8_t x;
//			memcpy(&x, data + i, 1);
//			i += 1;
//
//			return (uint8_t) x;
//		}
//	default:
//		{
//			uint8_t x;
//			memcpy(&x, data + i, 1);
//			i += 1;
//
//			return x;
//		}
//	}
//}
//
//const int16_t read_int16()
//{
//	if (current_datatype != INT16_T) {
//		current_datatype = INT16_T;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_int16(read_pos);
//}
//
//const int16_t read_int16(uint32_t & i)
//{
//	int16_t x;
//	memcpy(&x, data + i, 2);
//	x = be16toh(x);
//	i += 2;
//
//	return x;
//}
//
//const uint16_t read_uint16(uint32_t & i)
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		{
//			uint16_t x;
//			memcpy(&x, data + i, 2);
//			x = be16toh(x);
//			i += 2;
//
//			return x;
//		}
//	case SCALA:
//	case JAVA:
//		{
//			int16_t x;
//			memcpy(&x, data + i, 2);
//			x = be16toh(x);
//			i += 2;
//
//			return (uint16_t) x;
//		}
//	case PYTHON:
//	case JULIA:
//		{
//			int16_t x;
//			memcpy(&x, data + i, 2);
//			x = be16toh(x);
//			i += 2;
//
//			return (uint16_t) x;
//		}
//	default:
//		{
//			uint16_t x;
//			memcpy(&x, data + i, 2);
//			x = be16toh(x);
//			i += 2;
//
//			return x;
//		}
//	}
//}
//
//const uint16_t read_uint16()
//{
//	if (current_datatype != UINT16_T) {
//		current_datatype = UINT16_T;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_uint16(read_pos);
//}
//
//const int32_t read_int32()
//{
//	if (current_datatype != INT32_T) {
//		current_datatype = INT32_T;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_int32(read_pos);
//}
//
//const int32_t read_int32(uint32_t & i)
//{
//	int32_t x;
//	memcpy(&x, data + i, 4);
//	x = be32toh(x);
//	i += 4;
//
//	return x;
//}
//
//const uint32_t read_uint32(uint32_t & i)
//{
//	switch(cl) {
//	case C:
//	case CPP:
//		{
//			uint32_t x;
//			memcpy(&x, data + i, 4);
//			x = be32toh(x);
//			i += 4;
//
//			return x;
//		}
//	case SCALA:
//	case JAVA:
//		{
//			int32_t x;
//			memcpy(&x, data + i, 4);
//			x = be32toh(x);
//			i += 4;
//
//			return (uint32_t) x;
//		}
//	case PYTHON:
//	case JULIA:
//		{
//			int32_t x;
//			memcpy(&x, data + i, 4);
//			x = be32toh(x);
//			i += 4;
//
//			return (uint32_t) x;
//		}
//	default:
//		{
//			uint32_t x;
//			memcpy(&x, data + i, 4);
//			x = be32toh(x);
//			i += 4;
//
//			return x;
//		}
//	}
//}
//
//const uint32_t read_uint32()
//{
//	if (current_datatype != UINT32_T) {
//		current_datatype = UINT32_T;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_uint32(read_pos);
//}
//
//const int64_t read_int64()
//{
//	if (current_datatype != INT64_T) {
//		current_datatype = INT64_T;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_int64(read_pos);
//}
//
//const int64_t read_int64(uint32_t & i)
//{
//	int64_t x;
//	memcpy(&x, data + i, 8);
//	x = be64toh(x);
//	i += 8;
//
//	return x;
//}
//
//const uint64_t read_uint64(uint32_t & i)
//{
//	switch(cl) {
//	case SCALA:
//	case JAVA:
//	case PYTHON:
//	case JULIA:
//		{
//			int64_t x;
//			memcpy(&x, data + i, 8);
//			x = be64toh(x);
//			i += 8;
//
//			return (uint64_t) x;
//		}
//	default:
//		{
//			uint64_t x;
//			memcpy(&x, data + i, 8);
//			x = be64toh(x);
//			i += 8;
//
//			return x;
//		}
//	}
//}
//
//const uint64_t read_uint64()
//{
//	if (current_datatype != UINT64_T) {
//		current_datatype = UINT64_T;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_uint64(read_pos);
//}
//
//const float read_real4()
//{
//	return read_float();
//}
//
//const float read_real4(uint32_t & i)
//{
//	return read_float(i);
//}
//
//
//const float read_float()
//{
//	if (current_datatype != FLOAT) {
//		current_datatype = FLOAT;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	float x;
//	memcpy(&x, data + read_pos, 4);
//	read_pos += 4;
//
//	if (!big_endian) reverse_float(&x);
//
//	return x;
//}
//
//void read_float(float * x)
//{
//	if (current_datatype != FLOAT) {
//		current_datatype = FLOAT;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	memcpy(x, data + read_pos, 4);
//	read_pos += 4;
//
//	if (!big_endian) reverse_float(x);
//}
//
//const float read_float(uint32_t & i)
//{
//	float x;
//	memcpy(&x, data + i, 4);
//	i += 4;
//
//	if (!big_endian) reverse_float(&x);
//
//	return x;
//}
//
//void read_float(uint32_t & i, float * x)
//{
//	memcpy(x, data + i, 4);
//	i += 4;
//
//	if (!big_endian) reverse_float(x);
//}
//
//
//const double read_real()
//{
//	return read_double();
//}
//
//const double read_real(uint32_t & i)
//{
//	return read_double(i);
//}
//
//const double read_real8()
//{
//	return read_double();
//}
//
//const double read_real8(uint32_t & i)
//{
//	return read_double(i);
//}
//
//const double read_double_precision()
//{
//	return read_double();
//}
//
//const double read_double_precision(uint32_t & i)
//{
//	return read_double(i);
//}
//
//const double read_double()
//{
//	if (current_datatype != DOUBLE) {
//		current_datatype = DOUBLE;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_double(read_pos);
//}
//
//void read_double(double * x)
//{
//	if (current_datatype != DOUBLE) {
//		current_datatype = DOUBLE;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	memcpy(x, data + read_pos, 8);
//	read_pos += 8;
//
//	if (reverse_floats) reverse_double(x);
//}
//
//const double read_double(uint32_t & i)
//{
//	double x;
//	memcpy(&x, data + i, 8);
//	i += 8;
//
//	if (reverse_floats) reverse_double(&x);
//
//	return x;
//}
//
//void read_double(uint32_t & i, double * x)
//{
//	memcpy(&x, data + i, 8);
//	i += 8;
//
//	if (reverse_floats) reverse_double(x);
//}
//
//const string read_string()
//{
//	if (current_datatype != STRING) {
//		current_datatype = STRING;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_string(read_pos);
//}
//
//const string read_string(uint32_t & i)
//{
//	uint32_t string_length = read_uint32(i);
//	char string_c[string_length+1];
//	memcpy(string_c, data + i, string_length);
//	i += string_length;
//	string_c[string_length] = '\0';
//	return string(string_c);
//}
//
//const std::wstring s2ws(const string & str)
//{
//    using convert_type = std::codecvt_utf8<wchar_t>;
//    std::wstring_convert<convert_type, wchar_t> converter;
//
//    return converter.from_bytes(str);
//}
//
//const string ws2s(const std::wstring & wstr)
//{
//    using convert_type = std::codecvt_utf8<wchar_t>;
//    std::wstring_convert<convert_type, wchar_t> converter;
//
//    return converter.to_bytes(wstr);
//}
//
//const string read_wstring(uint32_t & i)
//{
//	uint32_t string_length = read_uint32(i);
//	wchar_t string_c[string_length+1];
//	memcpy(string_c, data + i, 2*string_length);
//	i += 2*string_length;
//	string_c[string_length] = L'\0';
//	std::wstring wstr = std::wstring(string_c);
//
//	return ws2s(wstr);
//}
//
//const string read_wstring()
//{
//	if (current_datatype != WSTRING) {
//		current_datatype = WSTRING;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_wstring(read_pos);
//}
//
//const MatrixID read_matrixID()
//{
//	if (current_datatype != MATRIXID) {
//		current_datatype = MATRIXID;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_matrixID(read_pos);
//}
//
//const MatrixID read_matrixID(uint32_t & i)
//{
//	MatrixID matrixID;
//	memcpy(&matrixID, &data[i], 2);
//	matrixID = be16toh(matrixID);
//	i += 2;
//
//	return matrixID;
//}
//
//bool compare_array_block(DoubleArrayBlock_ptr block, double * temp)
//{
//	double local;
//	for (uint64_t i = 0; i < block->size; i++) {
//		memcpy(&local, block->start + 8*i, 8);
//		if (local != temp[i]) {
//			if (reverse_floats) reverse_double(&local);
//			if (local != temp[i]) return false;
//		}
//	}
//	return true;
//}
//
//const DoubleArrayBlock_ptr read_array_block()
//{
//	if (current_datatype != MATRIX_BLOCK) {
//		current_datatype = MATRIX_BLOCK;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_array_block(read_pos);
//}
//
//const DoubleArrayBlock_ptr read_array_block(uint32_t & i)
//{
//	uint8_t ndims = read_uint8(i);
//	DoubleArrayBlock_ptr block = std::make_shared<ArrayBlock<double>>(ndims);
//	block->size = read_uint64(i);
//	for (uint8_t j = 0; j < ndims; j++)
//		for (uint8_t k = 0; k < 3; k++)
//			block->dims[k][j] = read_uint64(i);
//	block->start = data + i;
//	double tr = read_double(i);
//	double local;
//	memcpy(&local, block->start, 8);
//	if (!big_endian) reverse_double(&local);
//
//	i += 8*block->size;
//
//	return block;
//}
//
//void read_parameter()
//{
//	if (current_datatype != PARAMETER) {
//		current_datatype = PARAMETER;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//}
//
//const LibraryID read_libraryID()
//{
//	if (current_datatype != LIBRARYID) {
//		current_datatype = LIBRARYID;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_libraryID(read_pos);
//}
//
//const LibraryID read_libraryID(uint32_t & i)
//{
//	LibraryID libraryID;
//	memcpy(&libraryID, &data[i], 1);
//	i += 1;
//
//	return libraryID;
//}
//
//const MatrixInfo_ptr read_matrix_info()
//{
//	if (current_datatype != MATRIX_INFO) {
//		current_datatype = MATRIX_INFO;
//
//		read_pos += sizeof(datatype);
//		read_pos += 4;
//	}
//
//	return read_matrix_info(read_pos);
//}
//
//const MatrixInfo_ptr read_matrix_info(uint32_t & i)
//{
//	datatype dt;
//	uint32_t data_length = 0;
//
//	MatrixID matrixID;
//	uint32_t name_length;
//	string name;
//	uint64_t num_rows, num_cols;
//	bool sparse;
//	uint8_t layout, num_partitions;
//
//	memcpy(&matrixID, data + i, 2);
//	matrixID = be16toh(matrixID);
//	i += 2;
//	memcpy(&name_length, data + i, 4);
//	name_length = be32toh(name_length);
//	i += 4;
//	if (name_length > 0) {
//		char name_c[name_length];
//		memcpy(name_c, data + i, name_length);
//		i += name_length;
//		name = string(name_c);
//	}
//	else name = "";
//	memcpy(&num_rows, data + i, 8);
//	num_rows = be64toh(num_rows);
//	i += 8;
//	memcpy(&num_cols, data + i, 8);
//	num_cols = be64toh(num_cols);
//	i += 8;
//	memcpy(&sparse, data + i, 1);
//	i += 1;
//	memcpy(&layout, data + i, 1);
//	i += 1;
//	memcpy(&num_partitions, &data[i], 1);
//	i += 1;
//	MatrixInfo_ptr matrix = std::make_shared<MatrixInfo>(matrixID, name, num_rows, num_cols, sparse, layout, num_partitions);
//	memcpy(matrix->row_assignments, data + i, num_rows);
//	i += num_rows;
//
//	return matrix;
//}
