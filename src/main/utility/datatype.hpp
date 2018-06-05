#ifndef ALCHEMIST__DATATYPE_HPP
#define ALCHEMIST__DATATYPE_HPP

#include <cstdlib>

namespace alchemist {

typedef enum _datatype : uint8_t {
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
	SHORT_INT              = 43,
	LONG_DOUBLE_INT        = 44,
	STRING                 = 45,
	COMMAND_CODE           = 46
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
