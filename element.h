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

//enum element_type {object=1, string=2, numeric=3, array=4, boolean=5};
enum element_type : u_int8_t {object_ptr=0, object=1, string=2, numeric_int=3, numeric_long=4, numeric_double=5, array_ptr=6, array=7, boolean=8, null=9};

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
			case numeric_int:
			return "This entity is a numeric";
			break;
			case numeric_long:
			return "This entity is a numeric long";
			break;
			case numeric_double:
			return "This entity is a numeric double";
			break;
			case array:
			return "This entity is an array";
			break;
			case object:
			case object_ptr:
			return "This entity is an object";
			break;
			case null:
			return "This entity contains a null value";
			break;
		}
		
		return "This entity is unknown";
	}
	
public:
	element() : ref_count{0} {}
	element(element_type t) : type{t}, ref_count{0} {};
//int ref_count;	
	virtual ~element() { /*std::cout << "element: " << type << " destructor" << std::endl;*/ }
	
	element_type get_type() { return type; }
	
	std::string get_type_string() 
	{
		switch (type)
		{
			case object:
				return "object"; break;
			case array:
				return "array"; break;
			case string:
				return "string"; break;
			case boolean:
				return "boolean"; break;
			case numeric_int:
				return "numeric int"; break;
			case numeric_double:
				return "numeric double"; break;
			case numeric_long:
				return "numeric long"; break;
			case null:
				return "null"; break;
			default :
			{
				std::cout << "Error: unknown elementtype of: %d, returning String" << std::endl;
				return "string";
			}	
		}
	}
	
	//methods used by the individual elements, if not overridden then throws
	virtual std::string & get_string_value() { throw jsonP_exception{get_error()}; }
	virtual bool get_boolean_value() { throw jsonP_exception{get_error()}; }
	virtual int get_int_value() { throw jsonP_exception{get_error()}; }
	virtual long get_long_value() { throw jsonP_exception{get_error()}; }
	virtual double get_double_value() { throw jsonP_exception{get_error()}; }
	virtual element_object * get_object_value() { throw jsonP_exception{get_error()}; }
	virtual std::vector<element *> & get_array_value() { throw jsonP_exception{get_error()}; }
	
	//methods only implmemented by object, other elements will throw 
	virtual std::string & get_as_string(std::string key) { throw jsonP_exception{get_error()}; }
	virtual bool get_as_boolean(std::string key) { throw jsonP_exception{get_error()}; }
	virtual int get_as_numeric_int(std::string key) { throw jsonP_exception{get_error()}; }
	virtual long get_as_numeric_long(std::string key) { throw jsonP_exception{get_error()}; }
	virtual double get_as_numeric_double(std::string key) { throw jsonP_exception{get_error()}; }
	virtual std::vector<element *> & get_as_array(std::string key) { throw jsonP_exception{get_error()}; }
	virtual element_object * get_as_object(std::string key) { throw jsonP_exception{get_error()}; }
//	virtual std::string & get_as_string(char *key) { throw jsonP_exception{get_error()}; }
//	virtual bool get_as_boolean(char *key) { throw jsonP_exception{get_error()}; }
//	virtual int get_as_numeric_int(std::string key) { throw jsonP_exception{get_error()}; }
//	virtual long get_as_numeric_long(std::string key) { throw jsonP_exception{get_error()}; }
//	virtual double get_as_numeric_double(std::string key) { throw jsonP_exception{get_error()}; }
//	virtual std::vector<element *> & get_as_array(std::string key) { throw jsonP_exception{get_error()}; }
//	virtual element_object * get_as_object(std::string key) { throw jsonP_exception{get_error()}; }
	
	void incr_count() { ref_count++; }
	int get_count() { return --ref_count; }
	
	virtual void stringify(std::string &) {};
	
	virtual void stringify_pretty(std::string & raw, std::string & pretty)
	{
		bool indent{false};
		bool parsing_value{false};
		size_t indent_l{0};
		
		for (size_t i{0}; i<raw.length(); i++) {
			
			if (raw[i] == '"') {
				
				if (parsing_value) {
					int j{1}, k{2};
					
					while (raw[i-j] == '\\') {
						j++;
						k++;
					}
						
					if (k%2 == 0)
						parsing_value = false;
					else
						parsing_value = true;
				} else {
					parsing_value = true;
				}
				
				pretty += raw[i];
			} else if (!parsing_value && (raw[i] == '{' || raw[i] == '[')) {
				indent = true;
				indent_l += 2;
				pretty += raw[i];
				pretty += '\n';
				
				for (size_t t{0}; t<indent_l; t++)
					pretty += ' ';
				
			} else if (!parsing_value && (raw[i] == ']' || raw[i] == '}')) {
				indent_l -= 2;
				indent = (indent_l > 0) ? true : false;
				pretty += '\n';
				
				if (indent)
					for (size_t t{0}; t<indent_l; t++)
						pretty += ' ';
						
				pretty += raw[i];
			} else if (!parsing_value && (raw[i] == ',')) {
				pretty += raw[i];
				pretty += '\n';
				
				if (indent)
					for (size_t t{0}; t<indent_l; t++)
						pretty += ' ';
						
			} else if (!parsing_value && (raw[i] == ':')) {
				pretty += raw[i];
				pretty += ' ';
			} else {
				pretty += raw[i];
			}
		}
	}

	
	virtual void stringify_pretty(std::string &s) {
		std::string raw;
		stringify(raw);
		stringify_pretty(raw, s);
	}
};



#endif // _ELEMENT_H_
