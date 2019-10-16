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
	
//	bool buffer_parser;
	
	virtual void eat_whitespace(int);
	virtual void parse_key(std::string &);
	virtual void parse_bool(bool &);
	virtual element_type parse_numeric(std::string &);
	
	void parse_value(element *&);
	void parse_array(element_array *&);
	void parse_object(element_object *&);
	void parse_numeric(long &);
	void parse_numeric_double(double &);
	void parse_numeric_int(int &);
	element_type parse_numeric(int & start, int & end);	//dont use
	
//	long numeric_total{0};
//	long bool_total{0};
//	long key_total{0};

	
public:
	jsonP_parser() = default;
	jsonP_parser(std::string json);
	
	~jsonP_parser() = default;

	jsonP_doc * parse();
	jsonP_doc * parse(std::string & json);
};

#endif // _JSONP_PARSER_H_
