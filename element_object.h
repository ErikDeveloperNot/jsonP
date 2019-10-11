#ifndef _ELEMENT_OBJECT_H_
#define _ELEMENT_OBJECT_H_

#include "elements.h"

#include <map>


class element_object : public element
{
private:
	std::map<std::string, element*> elements;
	
public:
	element_object();
	~element_object();

	void add_element(std::string key, element *e);
	element_object * get_object_value() override;
	
	std::string & get_as_string(std::string key) override;
	bool get_as_boolean(std::string key) override;
	int get_as_numeric_int(std::string key) override;
	long get_as_numeric_long(std::string key) override;
	double get_as_numeric_double(std::string key) override;
	std::vector<element *> & get_as_array(std::string key) override;
	element_object * get_as_object(std::string key) override;
	
	void stringify(std::string &) override;
//	void stringify_pretty(std::string &);
};

//typedef element_object jsonP_doc;

class jsonP_doc
{
private:
	element_type type;
	element_object *obj = nullptr;
	element_array *arr = nullptr;
	
public:
	jsonP_doc() : type{object} {}								//defaut to type of object
	jsonP_doc(element_object * o) : type{object}, obj{o} {}
	jsonP_doc(element_array * a) : type{array}, arr{a} {}
	~jsonP_doc() = default;
	
	void set_object(element_object * o) 
	{ 
		obj = o; 
		type = object;
	}
	
	void set_array(element_array * a) 
	{ 
		arr = a; 
		type = array;
	}
	
	element_object * get_object() 
	{
		if (type == object) {
			return obj;
		} else {
			throw jsonP_exception{"This json document is of type array"};
		}
	}
	
	element_array * get_array() 
	{
		if (type == array) {
			return arr;
		} else {
			throw jsonP_exception{"This json document is of type object"};
		}
	}
	
	element_type get_type() { return type; }
};


#endif // _ELEMENT_OBJECT_H_
