#ifndef ALCHEMIST__MESSAGE_HPP
#define ALCHEMIST__MESSAGE_HPP

// Structure of Message:
//
//  Command code (4 bytes)
//  Header (4 bytes - contains length of data format section)
//  Format (Variable bytes - contains info on format of data contained in body)
//  Body (Variable bytes - contains data)

#include <cstdio>
#include <cstdlib>
#include <cstring>

typedef enum _Datatype {
	CHAR                   = 1,
	UNSIGNED_CHAR          = 2,
	SHORT                  = 3,
	UNSIGNED_SHORT         = 4,
	INT                    = 5,
	UNSIGNED               = 6,
	LONG                   = 7,
	UNSIGNED_LONG          = 8,
	LONG_LONG_INT          = 9,
	LONG_LONG              = LONG_LONG_INT,
	UNSIGNED_LONG_LONG     = 10,
	FLOAT                  = 11,
	DOUBLE                 = 12,
	LONG_DOUBLE            = 13,
	BYTE                   = 14,
	WCHAR                  = 15,
	BOOL                   = 16,
	SIGNED_CHAR            = 17,
	CHARACTER              = 18,
	INTEGER                = 19,
	REAL                   = 20,
	LOGICAL                = 21,
	COMPLEX                = 22,
	DOUBLE_PRECISION       = 23,
	REAL4                  = 24,
	COMPLEX8               = 25,
	REAL8                  = 26,
	COMPLEX16              = 27,
	INTEGER1               = 28,
	INTEGER2               = 29,
	INTEGER4               = 30,
	INTEGER8               = 31,
	INT8_T                 = 32,
	INT16_T                = 33,
	INT32_T                = 34,
	INT64_T                = 35,
	UINT8_T                = 36,
	UINT16_T               = 37,
	UINT32_T               = 38,
	UINT64_T               = 39,
	FLOAT_INT              = 40,
	DOUBLE_INT             = 41,
	LONG_INT               = 42,
	SHORT_INT				= 43,
	LONG_DOUBLE_INT			= 44,
	COMMAND_CODE				= 45
} Datatype;

class MessageOld
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 65536 };

	MessageOld() : body_length_(0) { }

	const char * data() const {
		return data_;
	}

	char * data() {
		return data_;
	}

	std::size_t length() const {
		return header_length + body_length_;
	}

	const char * body() const {
		return data_ + header_length;
	}

	char * body() {
		return data_ + header_length;
	}

	std::size_t body_length() const {
		return body_length_;
	}

	void body_length(std::size_t new_length) {
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
	}

	bool decode_header() {
		char header[header_length + 1] = "";
		std::strncat(header, data_, header_length);
		body_length_ = std::atoi(header);
		if (body_length_ > max_body_length) {
			body_length_ = 0;
			return false;
		}
		return true;
	}

	void encode_header()
	{
		char header[header_length + 1] = "";
		std::sprintf(header, "%4d", static_cast<int>(body_length_));
		std::memcpy(data_, header, header_length);
	}

private:
	char data_[header_length + max_body_length];
	std::size_t body_length_;
};

class Message
{
public:
	enum { header_length = 5 };
	enum { max_body_length = 65536 };

	Message() : command_code(0), body_length(0) { }

	unsigned char command_code;
	uint32_t body_length;

	char data[header_length + max_body_length];

	const uint32_t length() const
	{
		return body_length + header_length;
	}

	const uint32_t get_max_body_length() const { return max_body_length; }

	const char * header() const
	{
		return data;
	}

	char * header()
	{
		return data;
	}

	const char * body() const
	{
		return data + header_length;
	}

	char * body()
	{
		return data + header_length;
	}

	bool decode_header()
	{
		command_code = data[0];

		memcpy(&body_length, data+1, 4);
		if (body_length > max_body_length) {
			body_length = 0;
			return false;
		}

		return true;
	}

	void add_command_code(const unsigned char & cc)
	{
		command_code = cc;
		memcpy(data, &command_code, 1);
	}

	void add_string(const std::string & data)
	{
		add(data.c_str(), data.length(), CHAR);
	}

	void add_unsigned_char(const unsigned char & data)
	{
		add(reinterpret_cast<const char *>(&data), 1, CHAR);
	}

	void add_uint16(const uint16_t & data)
	{
		add(reinterpret_cast<const char *>(&data), 1, UINT16_T);
	}

	void add_uint32(const uint32_t & data)
	{
		add(reinterpret_cast<const char *>(&data), 1, UINT32_T);
	}

	void add(const char * _data, const uint32_t & length, const Datatype & dt)
	{
		memcpy(data + header_length + body_length, &length, 4);
		body_length += 4;
		memcpy(data + header_length + body_length, &dt, 1);
		body_length += 1;

		uint32_t data_length;

		switch (dt) {
		case BYTE:
			data_length = length;
			break;
		case CHAR:
			data_length = length;
			break;
		case UNSIGNED_CHAR:
			data_length = length;
			break;
		case SIGNED_CHAR:
			data_length = length;
			break;
		case CHARACTER:
			data_length = length;
			break;
		case BOOL:
			data_length = length;
			break;
		case LOGICAL:
			data_length = length;
			break;
		case SHORT:
			data_length = 2*length;
			break;
		case UNSIGNED_SHORT:
			data_length = 2*length;
			break;
		case LONG:
			data_length = 4*length;
			break;
		case UNSIGNED_LONG:
			data_length = 4*length;
			break;
		case INTEGER1:
			data_length = length;
			break;
		case INT8_T:
			data_length = length;
			break;
		case UINT8_T:
			data_length = length;
			break;
		case INTEGER2:
			data_length = 2*length;
			break;
		case INT16_T:
			data_length = 2*length;
			break;
		case UINT16_T:
			data_length = 2*length;
			break;
		case INTEGER4:
			data_length = 4*length;
			break;
		case INT32_T:
			data_length = 4*length;
			break;
		case UINT32_T:
			data_length = 4*length;
			break;
		case INTEGER8:
			data_length = 8*length;
			break;
		case INT64_T:
			data_length = 8*length;
			break;
		case UINT64_T:
			data_length = 8*length;
			break;
		case FLOAT:
			data_length = 4*length;
			break;
		case DOUBLE:
			data_length = 8*length;
			break;
		case REAL:
			data_length = 8*length;
			break;
		}

		memcpy(data + header_length + body_length, _data, data_length);
		body_length += data_length;

		memcpy(data + 1, &body_length, 4);
	}

	void clear()
	{
		body_length = 0;
		command_code = 0;
	}
};

#endif // ALCHEMIST__MESSAGE_HPP
