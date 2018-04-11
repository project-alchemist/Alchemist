#ifndef ALCHEMIST__PARAMETERS_HPP
#define ALCHEMIST__PARAMETERS_HPP

#include <El.hpp>
#include <string>
#include <map>
#include <memory>
#include <sstream>

namespace alchemist {

//typedef El::AbstractDistMatrix<double> DistMatrix;

using std::string;

struct Parameter {
public:
	Parameter(string _name, string _type) :
		name(_name), type(_type) {}

	virtual ~Parameter() {}

	string get_name() const {
		return name;
	}

	string get_type() const {
		return type;
	}

	virtual string to_string() const = 0;
protected:
	string name;
	string type;
};

struct IntParameter : Parameter {
public:

	IntParameter(string _name, int _value) :
		Parameter(_name, "i"), value(_value) {}

	~IntParameter() {}

	int get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	int value;
};

struct LongParameter : Parameter {
public:

	LongParameter(string _name, long _value) :
		Parameter(_name, "l"), value(_value) {}

	~LongParameter() {}

	long get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	long value;
};

struct LongLongParameter : Parameter {
public:

	LongLongParameter(string _name, long long _value) :
		Parameter(_name, "ll"), value(_value) {}

	~LongLongParameter() {}

	long long get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	long long value;
};

struct UnsignedParameter : Parameter {
public:

	UnsignedParameter(string _name, unsigned _value) :
		Parameter(_name, "u"), value(_value) {}

	~UnsignedParameter() {}

	unsigned get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	unsigned value;
};

struct UnsignedLongParameter : Parameter {
public:

	UnsignedLongParameter(string _name, unsigned long _value) :
		Parameter(_name, "ul"), value(_value) {}

	~UnsignedLongParameter() {}

	unsigned long get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	unsigned long value;
};

struct UnsignedLongLongParameter : Parameter {
public:

	UnsignedLongLongParameter(string _name, unsigned long long _value) :
		Parameter(_name, "ull"), value(_value) {}

	~UnsignedLongLongParameter() {}

	unsigned long long get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	unsigned long long value;
};

struct FloatParameter : Parameter {
public:

	FloatParameter(string _name, float _value) :
		Parameter(_name, "f"), value(_value) {}

	~FloatParameter() {}

	float get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	float value;
};

struct DoubleParameter : Parameter {
public:

	DoubleParameter(string _name, double _value) :
		Parameter(_name, "d"), value(_value) {}

	~DoubleParameter() {}

	double get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	double value;
};

struct LongDoubleParameter : Parameter {
public:

	LongDoubleParameter(string _name, long double _value) :
		Parameter(_name, "ld"), value(_value) {}

	~LongDoubleParameter() {}

	long double get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	long double value;
};

struct CharParameter : Parameter {
public:

	CharParameter(string _name, char _value) :
		Parameter(_name, "c"), value(_value) {}

	~CharParameter() {}

	char get_value() const {
		return value;
	}

	string to_string() const {
		return string(1, value);
	}

protected:
	char value;
};

struct BoolParameter : Parameter {
public:

	BoolParameter(string _name, bool _value) :
		Parameter(_name, "b"), value(_value) {}

	~BoolParameter() {}

	bool get_value() const {
		return value;
	}

	string to_string() const {
		return value ? "t" : "f";
	}

protected:
	bool value;
};

struct StringParameter : Parameter {
public:

	StringParameter(string _name, string _value) :
		Parameter(_name, "s"), value(_value) {}

	~StringParameter() {}

	string get_value() const {
		return value;
	}

	string to_string() const {
		return value;
	}

protected:
	string value;
};

struct MatrixHandleParameter : Parameter {
public:

	MatrixHandleParameter(string _name, uint32_t _value) :
		Parameter(_name, "mh"), value(_value) {}

	~MatrixHandleParameter() {}

	uint32_t get_value() const {
		return value;
	}

	string to_string() const {
		return std::to_string(value);
	}

protected:
	uint32_t value;
};

//struct DistMatrixParameter : Parameter {
//public:
//
//	DistMatrixParameter(string _name, DistMatrix * _value) :
//		Parameter(_name, "dm"), value(_value) {}
//
//	~DistMatrixParameter() {}
//
//	DistMatrix * get_value() const {
//		return value;
//	}
//
//	string to_string() const {
//		std::stringstream ss;
//		ss << value;
//		return ss.str();
//	}
//
//protected:
//	DistMatrix * value;
//};

struct PointerParameter : Parameter {
public:

	PointerParameter(string _name, void * _value) :
		Parameter(_name, "p"), value(_value) {}

	~PointerParameter() {}

	void * get_value() const {
		return value;
	}

	string to_string() const {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

protected:
	void * value;
};

struct Parameters {
public:
	Parameters() {}

	~Parameters() {}

	int num() const {
		return parameters.size();
	}

//	size_t num_distmatrices() const {
//		return distmatrix_parameters.size();
//	}

	size_t num_matrixhandles() const {
		return matrixhandle_parameters.size();
	}

	void add(Parameter * p) {
		parameters.insert(std::make_pair(p->get_name(), p));
	}

	std::shared_ptr<Parameter> get(string name) const {
		return parameters.find(name)->second;
	}

	void add_int(string name, int value) {
		parameters.insert(std::make_pair(name, new IntParameter(name, value)));
	}

	void add_long(string name, long value) {
		parameters.insert(std::make_pair(name, new LongParameter(name, value)));
	}

	void add_longlong(string name, long long value) {
		parameters.insert(std::make_pair(name, new LongLongParameter(name, value)));
	}

	void add_unsigned(string name, unsigned value) {
		parameters.insert(std::make_pair(name, new UnsignedParameter(name, value)));
	}

	void add_unsignedlong(string name, unsigned long value) {
		parameters.insert(std::make_pair(name, new UnsignedLongParameter(name, value)));
	}

	void add_unsignedlonglong(string name, unsigned long long value) {
		parameters.insert(std::make_pair(name, new UnsignedLongLongParameter(name, value)));
	}

	void add_float(string name, float value) {
		parameters.insert(std::make_pair(name, new FloatParameter(name, value)));
	}

	void add_double(string name, double value) {
		parameters.insert(std::make_pair(name, new DoubleParameter(name, value)));
	}

	void add_longdouble(string name, long double value) {
		parameters.insert(std::make_pair(name, new LongDoubleParameter(name, value)));
	}

	void add_string(string name, string value) {
		parameters.insert(std::make_pair(name, new StringParameter(name, value)));
	}

	void add_char(string name, char value) {
		parameters.insert(std::make_pair(name, new CharParameter(name, value)));
	}

	void add_bool(string name, bool value) {
		parameters.insert(std::make_pair(name, new BoolParameter(name, value)));
	}

	void add_matrixhandle(string name, uint32_t value) {
		matrixhandle_parameters.insert(std::make_pair(name, new MatrixHandleParameter(name, value)));
	}

//	void add_distmatrix(string name, DistMatrix * value) {
//		distmatrix_parameters.insert(std::make_pair(name, new DistMatrixParameter(name, value)));
//	}

	void add_ptr(string name, void * value) {
		pointer_parameters.insert(std::make_pair(name, new PointerParameter(name, value)));
	}

	int get_int(string name) const {
		return std::dynamic_pointer_cast<IntParameter> (parameters.find(name)->second)->get_value();
	}

	long get_long(string name) const {
		return std::dynamic_pointer_cast<LongParameter> (parameters.find(name)->second)->get_value();
	}

	long long get_longlong(string name) const {
		return std::dynamic_pointer_cast<LongLongParameter> (parameters.find(name)->second)->get_value();
	}

	unsigned get_unsigned(string name) const {
		return std::dynamic_pointer_cast<UnsignedParameter> (parameters.find(name)->second)->get_value();
	}

	unsigned long get_unsignedlong(string name) const {
		return std::dynamic_pointer_cast<UnsignedLongParameter> (parameters.find(name)->second)->get_value();
	}

	unsigned long long get_unsignedlonglong(string name) const {
		return std::dynamic_pointer_cast<UnsignedLongLongParameter> (parameters.find(name)->second)->get_value();
	}

	float get_float(string name) const {
		return std::dynamic_pointer_cast<FloatParameter> (parameters.find(name)->second)->get_value();
	}

	double get_double(string name) const {
		return std::dynamic_pointer_cast<DoubleParameter> (parameters.find(name)->second)->get_value();
	}

	long double get_longdouble(string name) const {
		return std::dynamic_pointer_cast<LongDoubleParameter> (parameters.find(name)->second)->get_value();
	}

	string get_string(string name) const {
		return std::dynamic_pointer_cast<StringParameter> (parameters.find(name)->second)->get_value();
	}

	char get_char(string name) const {
		return std::dynamic_pointer_cast<CharParameter> (parameters.find(name)->second)->get_value();
	}

	bool get_bool(string name) const {
		return std::dynamic_pointer_cast<BoolParameter> (parameters.find(name)->second)->get_value();
	}

	uint32_t get_matrixhandle(string name) const {
		return matrixhandle_parameters.find(name)->second->get_value();
	}

//	DistMatrix * get_distmatrix(string name) const {
//		return distmatrix_parameters.find(name)->second->get_value();
//	}

	void * get_ptr(string name) const {
		return pointer_parameters.find(name)->second->get_value();
	}

	std::map<string, std::shared_ptr<MatrixHandleParameter> > get_matrixhandles() const {
		return matrixhandle_parameters;
	}

//	std::map<string, std::shared_ptr<DistMatrixParameter> > get_distmatrices() const {
//		return distmatrix_parameters;
//	}

	std::map<string, std::shared_ptr<PointerParameter> > get_pointers() const {
		return pointer_parameters;
	}

	string to_string() const {
		string arg_list = "";

		for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
			arg_list.append(it->first);
			arg_list.append("(");
			arg_list.append(it->second->get_type());
			arg_list.append(")");
			arg_list.append(it->second->to_string());
			arg_list.append(" ");
		}

		for (auto it = matrixhandle_parameters.begin(); it != matrixhandle_parameters.end(); it++ ) {
			arg_list.append(it->first);
			arg_list.append("(mh)");
			arg_list.append(it->second->to_string());
			arg_list.append(" ");
		}

		return arg_list;
	}

private:
	std::map<string, std::shared_ptr<Parameter> > parameters;
	std::map<string, std::shared_ptr<MatrixHandleParameter> > matrixhandle_parameters;
//	std::map<string, std::shared_ptr<DistMatrixParameter> > distmatrix_parameters;
	std::map<string, std::shared_ptr<PointerParameter> > pointer_parameters;
};

}

#endif
