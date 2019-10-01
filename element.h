#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include "jsonP_exception.h"

//#include <map>
#include <vector>
#include <string>
#include <iostream>


const static std::string quote{"\""};
const static std::string quote_colon{"\":"};
const static std::string comma{","};

enum element_type {object=1, string=2, numeric=3, array=4, boolean=5};

class element_object;

class element
{
private:
	element_type type;
	int ref_count;
	
	const char* get_error() {
		switch (type)
		{
			case string:
			return "This entity is a string";
			break;
			case boolean:
			return "This entity is a bool";
			break;
			case numeric:
			return "This entity is a numeric";
			break;
			case array:
			return "This entity is an array";
			break;
			case object:
			return "This entity is an object";
			break;
		}
		
		return "This entity is unknown";
	}
	
public:
	element() : ref_count{0} {}
	element(element_type t) : type{t}, ref_count{0} {};
	
	virtual ~element() { /*std::cout << "element: " << type << " destructor" << std::endl;*/ }
	
	element_type get_type() { return type; }
	
	//methods used by the individual elements, if not overridden then throws
	virtual std::string & get_string_value() { throw jsonP_exception{get_error()}; }
	virtual bool get_boolean_value() { throw jsonP_exception{get_error()}; }
	virtual long get_long_value() { throw jsonP_exception{get_error()}; }
	virtual element_object * get_object_value() { throw jsonP_exception{get_error()}; }
	virtual std::vector<element *> & get_array_value() { throw jsonP_exception{get_error()}; }
	
	//methods only implmemented by object, other elements will throw 
	virtual std::string & get_as_string(std::string key) { throw jsonP_exception{get_error()}; }
	virtual bool get_as_boolean(std::string key) { throw jsonP_exception{get_error()}; }
	virtual long get_as_numeric(std::string key) { throw jsonP_exception{get_error()}; }
	virtual std::vector<element *> & get_as_array(std::string key) { throw jsonP_exception{get_error()}; }
	virtual element_object * get_as_object(std::string key) { throw jsonP_exception{get_error()}; }
	
	void incr_count() { ref_count++; }
	int get_count() { return --ref_count; }
	
	virtual void stringify(std::string &) {};
};



#endif // _ELEMENT_H_
