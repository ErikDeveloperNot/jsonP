#ifndef _JSONP_PARSER_H_
#define _JSONP_PARSER_H_

#include "element_object.h"
//#include "jsonP_exception.h"

#include <string>


class jsonP_parser
{
protected:
	std::string json;
	int json_length;
	int index;
	bool look_for_key;
	
//	bool buffer_parser;
	
	virtual void eat_whitespace(int);
	virtual void parse_key(std::string &);
	virtual void parse_bool(bool &);
	virtual element_type parse_numeric(std::string &);
	virtual void parse_array(element_array *&);
	virtual void parse_object(element_object *&);
	virtual void parse_value(element *&);
	
//	void parse_value(element *&);
//	void parse_array(element_array *&);
//	void parse_object(element_object *&);
	void parse_numeric(long &);
	void parse_numeric_double(double &);
	void parse_numeric_int(int &);
	element_type parse_numeric(int & start, int & end);	//dont use
	
//	long numeric_total{0};
//	long bool_total{0};
//	long key_total{0};

	virtual inline element_string * create_string_element(std::string &str) { return new element_string{str}; }
	virtual inline element_boolean * create_boolean_element(bool b) { return new element_boolean{b}; }
	virtual inline element_numeric * create_int_element(std::string &str) { return new element_numeric{atoi(str.c_str())}; }
	virtual inline element_numeric * create_long_element(std::string &str) { return new element_numeric{atol(str.c_str())}; }
	virtual inline element_numeric * create_float_element(std::string &str) { return new element_numeric{atof(str.c_str())}; }
	virtual inline element_null * create_null_element() { return new element_null{}; }
	
	virtual inline element_array * create_element_array(element_type t) { return new element_array{t}; }
	virtual inline void add_array_element(element_array *a, element *e) { a->add_element(e); }
	
	virtual inline element_object * create_element_object() { return new element_object{}; }
	virtual inline void add_object_element(element_object *o, element *e, std::string &k) { o->add_element(k, e); }
	
public:
	jsonP_parser() = default;
	jsonP_parser(std::string json);
	
	~jsonP_parser() = default;

	jsonP_doc * parse();
	jsonP_doc * parse(std::string & json);
};

#endif // _JSONP_PARSER_H_
