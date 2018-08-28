#ifndef ALCHEMIST__DATATYPE_HPP
#define ALCHEMIST__DATATYPE_HPP

#include <cstdlib>

namespace alchemist {

typedef enum _datatype : uint8_t {
	NONE = 0,
	CHAR,
	UNSIGNED_CHAR,
	SHORT,
	UNSIGNED_SHORT,
	INT,
	UNSIGNED,
	LONG,
	UNSIGNED_LONG,
	LONG_LONG_INT,
	LONG_LONG,
	UNSIGNED_LONG_LONG,
	FLOAT,
	DOUBLE,
	LONG_DOUBLE,
	BYTE,
	WCHAR,
	BOOL,
	SIGNED_CHAR,
	CHARACTER,
	INTEGER,
	REAL,
	LOGICAL,
	COMPLEX,
	DOUBLE_PRECISION,
	REAL4,
	COMPLEX8,
	REAL8,
	COMPLEX16,
	INTEGER1,
	INTEGER2,
	INTEGER4,
	INTEGER8,
	INT8_T,
	INT16_T,
	INT32_T,
	INT64_T,
	UINT8_T,
	UINT16_T,
	UINT32_T,
	UINT64_T,
	FLOAT_INT,
	DOUBLE_INT,
	LONG_INT,
	SHORT_INT,
	LONG_DOUBLE_INT,
	STRING,
	COMMAND_CODE
} datatype;


inline const uint8_t get_datatype_length(const datatype & dt)
{
	switch (dt) {
		case BYTE:
			return 1;
		case CHAR:
			return 1;
		case UNSIGNED_CHAR:
			return 1;
		case SIGNED_CHAR:
			return 1;
		case CHARACTER:
			return 1;
		case BOOL:
			return 1;
		case LOGICAL:
			return 1;
		case SHORT:
			return 2;
		case UNSIGNED_SHORT:
			return 2;
		case LONG:
			return 4;
		case UNSIGNED_LONG:
			return 4;
		case INTEGER1:
			return 1;
		case INT8_T:
			return 1;
		case UINT8_T:
			return 1;
		case INTEGER2:
			return 2;
		case INT16_T:
			return 2;
		case UINT16_T:
			return 2;
		case INTEGER4:
			return 4;
		case INT32_T:
			return 4;
		case UINT32_T:
			return 4;
		case INTEGER8:
			return 8;
		case INT64_T:
			return 8;
		case UINT64_T:
			return 8;
		case FLOAT:
			return 4;
		case DOUBLE:
			return 8;
		case REAL:
			return 8;
		default:
			return 1;
		}
}

inline const std::string get_datatype_name(const datatype & dt)
{
	switch (dt) {
		case BYTE:
			return "BYTE";
		case CHAR:
			return "CHAR";
		case UNSIGNED_CHAR:
			return "UNSIGNED CHAR";
		case SIGNED_CHAR:
			return "SIGNED CHAR";
		case CHARACTER:
			return "CHARACTER";
		case BOOL:
			return "BOOL";
		case LOGICAL:
			return "LOGICAL";
		case SHORT:
			return "BYTE";
		case UNSIGNED_SHORT:
			return "UNSIGNED SHORT";
		case LONG:
			return "LONG";
		case UNSIGNED_LONG:
			return "UNSIGNED LONG";
		case INTEGER1:
			return "INTEGER1";
		case INT8_T:
			return "INT8";
		case UINT8_T:
			return "UINT8";
		case INTEGER2:
			return "INTEGER2";
		case INT16_T:
			return "INT16";
		case UINT16_T:
			return "UINT16";
		case INTEGER4:
			return "INTEGER4";
		case INT32_T:
			return "INT32";
		case UINT32_T:
			return "UINT32";
		case INTEGER8:
			return "INTEGER8";
		case INT64_T:
			return "INT64";
		case UINT64_T:
			return "UINT64";
		case FLOAT:
			return "FLOAT";
		case DOUBLE:
			return "DOUBLE";
		case REAL:
			return "REAL";
		case STRING:
			return "STRING";
		default:
			return "INVALID DATATYPE";
		}
}

}

#endif
