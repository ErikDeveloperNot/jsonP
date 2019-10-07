#include "element_object.h"

#include <iostream>



element_object::element_object() : element{object}
{
}


element_object::~element_object()
{
	for (auto it = elements.begin(); it != elements.end(); it++) {
		if (it->second->get_count() < 1) {
			delete it->second;
		}
	}
}


void element_object::add_element(std::string key, element* e)
{
	elements[key] = e;
	e->incr_count();
}


element_object* element_object::get_object_value() 
{
	return this;
}


element_object* element_object::get_as_object(std::string key) 
{
	if (elements.count(key) > 0)
		return elements[key]->get_object_value();
	else
		throw jsonP_exception{"Key not found"};
}


std::vector<element*>& element_object::get_as_array(std::string key) 
{
	if (elements.count(key) > 0)
		return elements[key]->get_array_value();
	else
		throw jsonP_exception{"Key not found"};
}


int element_object::get_as_numeric_int(std::string key) 
{
	if (elements.count(key) > 0)
		return elements[key]->get_int_value();
	else
		throw jsonP_exception{"Key not found"};
}


long element_object::get_as_numeric_long(std::string key) 
{
	if (elements.count(key) > 0)
		return elements[key]->get_long_value();
	else
		throw jsonP_exception{"Key not found"};
}


double element_object::get_as_numeric_double(std::string key) 
{
	if (elements.count(key) > 0)
		return elements[key]->get_double_value();
	else
		throw jsonP_exception{"Key not found"};
}


bool element_object::get_as_boolean(std::string key) 
{
	if (elements.count(key) > 0)
		return elements[key]->get_boolean_value();
	else
		throw jsonP_exception{"Key not found"};
}


std::string& element_object::get_as_string(std::string key) 
{
	if (elements.count(key) > 0)
		return elements[key]->get_string_value();
	else
		throw jsonP_exception{"Key not found"};
}


/*
 * no line return or indents
 */
void element_object::stringify(std::string &doc)
{
	doc += "{";
	
	for (auto it = elements.begin(); it != elements.end(); ++it) {
		doc += quote;
		doc += it->first;
		doc += quote_colon;
		it->second->stringify(doc);
		doc += comma;
	}

	if (doc[doc.length()-1] == ',')
		doc[doc.length()-1] = '}';
	else
		doc += "}";
}


void element_object::stringify_pretty(std::string &s)
{
	std::string raw;
	stringify(raw);
	size_t l = raw.length() * 3;
	char buf[l];
	int index{0};
	bool indent{false};
	size_t indent_l{0};
	
	for (size_t i{0}; i<raw.length(); i++) {
		if (index == l) {
			//need to reallocate buf
		}
		
		if (raw[i] == '{' || raw[i] == '[') {
			indent = true;
			indent_l += 2;
			buf[index++] = raw[i];
			buf[index++] = '\n';
			
			for (size_t t{0}; t<indent_l; t++)
				buf[index++] = ' ';
			
		} else if (raw[i] == ']' || raw[i] == '}') {
			indent_l -= 2;
			indent = (indent_l > 0) ? true : false;
			buf[index++] = '\n';
			
			if (indent)
				for (size_t t{0}; t<indent_l; t++)
					buf[index++] = ' ';
					
			buf[index++] = raw[i];
		} else if (raw[i] == ',') {
			buf[index++] = raw[i];
			buf[index++] = '\n';
			
			if (indent)
				for (size_t t{0}; t<indent_l; t++)
					buf[index++] = ' ';
					
		} else if (raw[i] == ':') {
			buf[index++] = raw[i];
			buf[index++] = ' ';
		} else {
			buf[index++] = raw[i];
		}
	}
	
	buf[index] = '\0';
	s = buf;
}
