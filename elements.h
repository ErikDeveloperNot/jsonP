#include "element.h"

//#include <iostream>
//#include <chrono>

#ifndef _ELEMENTS_H_
#define _ELEMENTS_H_

//typedef std::chrono::high_resolution_clock clock2;
//typedef std::chrono::duration<float, std::milli> mil;

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
//		s += value;
//		for (char &c : value) {
		for (size_t i{0}; i < value.length(); i++) {
//			switch (c)
			switch (value[i])
			{
				case '"' :
				case '\\' :
				case '/' :
					s += '\\';
					s += value[i];
					break;
//				case '\\' :
//					if (i != value.length()-1) {
//						char next{value[i+1]};
//					
//						if (next == 'b' || next == 'n' || next == 't' || next == 'r' || next == 'f' || next == 'u') {
////							s += '\\';
//							s += value[i];
//						} else if (next == '\\') {
////							s += '\\';
//							s += next;
//							s += value[i];
//						} else {
//							s += '\\';
//							s += value[i];
//						}
//					} else {
//						s += '\\';
//						s += value[i];
//					}
//					break;
				default :
					s += value[i];
			}
		}

		s += quote;
	}
};



/*
 * JSON numeric element, all numerics will be store as double and parsed to the type retrieved
 */ 
class element_numeric : public element
{
private:
	double value;
	
public:
	element_numeric(double d) : element{numeric_double}, value{d} {}
	element_numeric(long l) : element{numeric_long}, value{double(l)} {}
	element_numeric(int i) : element{numeric_int}, value{double(i)} {}
	~element_numeric() = default;// { std::cout << "element_numeric destructor" << std::endl; }
	
	double get_double_value() override { return value; }
	int get_int_value() override { return int(value); }
	long get_long_value() override { return long(value); }
	
	void stringify(std::string &s) override {
		switch (get_type())
		{
			case numeric_int :
				s += std::to_string(int(value));
				break;
			case numeric_long :
				s += std::to_string(long(value));
				break;
			case numeric_double :
				s += std::to_string(value);
				break;
			default :
				s += std::to_string(value);
		}
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
 * JSON null element
 */
class element_null : public element
{
public:
	element_null() : element{null} {}
	~element_null() = default;
	
	void stringify(std::string &s) override {
		s += "null";
	}
};


/*
 * JSON array element
 */
class element_array : public element
{
private:
	std::vector<element *> elements;
//	element_type elements_types;
	
public:
	element_array(element_type t) : element{array} {} //, elements_types{t} {}
	
	element_array(std::vector<element *> v, element_type t) : element{array} {  //, elements_types{t} {
		for (element *e : v) {
//			if (e->get_type() == elements_types) {
				e->incr_count();
				elements.push_back(e);
//			}
		}
	}
	
	~element_array() {
//		std::cout << "element_array destructor" << std::endl;
		for (element *e : elements) {
			if (e->get_count() < 1) {
				delete e;
			}
		}
	}
	
	bool add_element(element *e) { 
		/*
		 * in the case the array is initialized as holding types of null (not sure if this is allowed)
		 * and a different type is passed in, then change the array type to that
		 * 
		 * update 10/19/2019 - i guess hetrogenous arrays are allowed, so add anytype
		 */
//		if (e->get_type() == elements_types) {
			elements.push_back(e); 
			e->incr_count();
			return true;
//		} else {
//			
//			if (elements_types == null) {
//				elements_types = e->get_type();
//				elements.clear();
//				elements.push_back(e); 
//				e->incr_count();
//				return true;
//			} else {
//				return false;
//			}
//		}
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