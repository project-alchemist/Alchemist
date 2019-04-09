#ifndef ALCHEMIST__DATATYPE_HPP
#define ALCHEMIST__DATATYPE_HPP

#include <cstdlib>

namespace alchemist {

// Defined data types for C/C++ clients
typedef enum _datatype : uint8_t {
	NONE = 0,
	CHAR,
	SIGNED_CHAR,
	UNSIGNED_CHAR,
	CHARACTER,
	WCHAR,
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
	BOOL,
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
	INT8,
	INT16,
	INT32,
	INT64,
	UINT8,
	UINT16,
	UINT32,
	UINT64,
	FLOAT_INT,
	DOUBLE_INT,
	LONG_INT,
	SHORT_INT,
	LONG_DOUBLE_INT,
	STRING,
	WSTRING,
	COMMAND_CODE,
	LIBRARY_ID,
	GROUP_ID,
	WORKER_ID,
	WORKER_INFO,
	MATRIX_ID,
	MATRIX_INFO,
	MATRIX_BLOCK,
	INDEXED_ROW,
	DISTMATRIX,
	ARRAY_ID,
	ARRAY_INFO,
	ARRAY_BLOCK_FLOAT,
	ARRAY_BLOCK_DOUBLE,
	VOID_POINTER,
	PARAMETER = 100
} datatype;

typedef enum _layout : uint8_t {
	MC_MR = 0,
	MC_STAR,
	MD_STAR,
	MR_MC,
	MR_STAR,
	STAR_MC,
	STAR_MD,
	STAR_MR,
	STAR_STAR,
	STAR_VC,
	STAR_VR,
	VC_STAR,
	VR_STAR,
	CIRC_CIRC
} layout;

//inline const uint8_t get_datatype_length(const datatype & dt)
//{
//	switch (dt) {
//		case BYTE:
//			return sizeof(unsigned char);
//		case CHAR:
//			return sizeof(char);
//		case SIGNED_CHAR:
//			return sizeof(signed char);
//		case UNSIGNED_CHAR:
//			return sizeof(unsigned char);
//		case CHARACTER:
//			return sizeof(char);
//		case WCHAR:
//			return 2; //sizeof(wchar_t);
//		case BOOL:
//			return sizeof(bool);
//		case LOGICAL:
//			return sizeof(bool);
//		case SHORT:
//			return sizeof(short);
//		case UNSIGNED_SHORT:
//			return sizeof(unsigned short);
//		case LONG:
//			return sizeof(long);
//		case UNSIGNED_LONG:
//			return sizeof(unsigned long);
//		case INTEGER1:
//			return sizeof(int8_t);
//		case INT8_T:
//			return sizeof(int8_t);
//		case UINT8_T:
//			return sizeof(int8_t);
//		case INTEGER2:
//			return sizeof(int16_t);
//		case INT16_T:
//			return sizeof(int16_t);
//		case UINT16_T:
//			return sizeof(int16_t);
//		case INTEGER4:
//			return sizeof(int32_t);
//		case INT32_T:
//			return sizeof(int32_t);
//		case UINT32_T:
//			return sizeof(int32_t);
//		case INTEGER8:
//			return sizeof(int64_t);
//		case INT64_T:
//			return sizeof(int64_t);
//		case UINT64_T:
//			return sizeof(int64_t);
//		case FLOAT:
//			return sizeof(float);
//		case DOUBLE:
//			return sizeof(double);
//		case REAL:
//			return sizeof(double);
//		case MATRIX_ID:
//			return sizeof(uint16_t);
//		case LIBRARY_ID:
//			return sizeof(uint16_t);
//		default:
//			return 1;
//		}
//}

inline const std::string get_layout(const layout & l)
{
	switch (l) {
	case MC_MR:
		return "MC MR";
	case MC_STAR:
		return "MC STAR";
	case MD_STAR:
		return "MD STAR";
	case MR_MC:
		return "MR MC";
	case MR_STAR:
		return "MR STAR";
	case STAR_MC:
		return "STAR MC";
	case STAR_MD:
		return "STAR MD";
	case STAR_MR:
		return "STAR MR";
	case STAR_STAR:
		return "STAR STAR";
	case STAR_VC:
		return "STAR VC";
	case STAR_VR:
		return "STAR VR";
	case VC_STAR:
		return "VC STAR";
	case VR_STAR:
		return "VR STAR";
	case CIRC_CIRC:
		return "CIRC CIRC";
	default:
		return "Unknown matrix layout";
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
		case WCHAR:
			return "WCHAR";
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
		case INT8:
			return "INT8";
		case UINT8:
			return "UINT8";
		case INTEGER2:
			return "INTEGER2";
		case INT16:
			return "INT16";
		case UINT16:
			return "UINT16";
		case INTEGER4:
			return "INTEGER4";
		case INT32:
			return "INT32";
		case UINT32:
			return "UINT32";
		case INTEGER8:
			return "INTEGER8";
		case INT64:
			return "INT64";
		case UINT64:
			return "UINT64";
		case FLOAT:
			return "FLOAT";
		case DOUBLE:
			return "DOUBLE";
		case REAL:
			return "REAL";
		case STRING:
			return "STRING";
		case WSTRING:
			return "WSTRING";
		case PARAMETER:
			return "PARAMETER";
		case LIBRARY_ID:
			return "LIBRARY ID";
		case MATRIX_ID:
			return "MATRIX ID";
		case MATRIX_INFO:
			return "MATRIX INFO";
		case MATRIX_BLOCK:
			return "MATRIX BLOCK";
		case ARRAY_ID:
			return "ARRAY ID";
		case ARRAY_INFO:
			return "ARRAY INFO";
		case ARRAY_BLOCK_FLOAT:
			return "ARRAY BLOCK FLOAT";
		case ARRAY_BLOCK_DOUBLE:
			return "ARRAY BLOCK DOUBLE";
		case INDEXED_ROW:
			return "INDEXED ROW";
		case DISTMATRIX:
			return "DISTMATRIX";
		case WORKER_INFO:
			return "WORKER INFO";
		case VOID_POINTER:
			return "VOID POINTER";
		default:
			return "INVALID DATATYPE";
		}
}

}

#endif
