#include "element.h"

//#include <iostream>

#ifndef _ELEMENTS_H_
#define _ELEMENTS_H_


/*
 * JSON string element
 */
class element_string : public element
{
private:
	std::string value;
	
public:
	element_string(std::string s) : element{string},  value{s} {}
	~element_string() = default;//{ std::cout << "element_string destructor" << std::endl; }
	
	std::string & get_string_value() override { return value; }

	void stringify(std::string &s) override {
		s += quote;
		s += value;
		s += quote;
	}
};


/*
 * JSON numeric element
 * numeric for now is only long, no float or ints since not needed for now
 */
 class element_numeric : public element
{
private:
	long value;
	
public:
	element_numeric(long l) : element{numeric}, value{l} {}
	~element_numeric() = default;// { std::cout << "element_numeric destructor" << std::endl; }
	
	long get_long_value() override { return value; }
	
	void stringify(std::string &s) override {
		s += std::to_string(value);
	}
};


/*
 * JSON bool element
 */
class element_boolean : public element
{
private:
	bool value;
	
public:
	element_boolean(bool b) : element{boolean}, value{b} {}
	~element_boolean() = default; //{ std::cout << "element_boolean destructor" << std::endl; }
	
	bool get_boolean_value() override { return value; }
	
	void stringify(std::string &s) override {
		if (value)
			s += "true";
		else
			s += "false";
	}
};


/*
 * JSON array element
 */
class element_array : public element
{
private:
	std::vector<element *> elements;
	element_type elements_types;
	
public:
	element_array(element_type t) : element{array}, elements_types{t} {}
	
	element_array(std::vector<element *> v, element_type t) : element{array}, elements_types{t} {
		for (element *e : v) {
			if (e->get_type() == elements_types) {
				e->incr_count();
				elements.push_back(e);
			}
		}
	}
	
	~element_array() {
//		std::cout << "element_array destructor" << std::endl;
		for (element *e : elements) {
//int cnt = e->get_count();
//std::cout << e->get_type() << " -- count: " << cnt << std::endl;
			if (e->get_count() < 1) {
//std::cout << "calling delete for " << e->get_type() << std::endl;
				delete e;
			}
		}
	}
	
	bool add_element(element *e) { 
		if (e->get_type() == elements_types) {
			elements.push_back(e); 
			e->incr_count();
			return true;
		} else {
			return false;
		}
	}
	
	std::vector<element *> & get_array_value() override { return elements; }
	
	void stringify(std::string &s) override {
		s += "[";
		
		for (element *e : elements) {
			e->stringify(s);
			s += comma;
		}

		if (s[s.length()-1] == ',')
			s[s.length()-1] = ']';
		else
			s += "]";
			
	}
};

#endif // _ELEMENTS_H_