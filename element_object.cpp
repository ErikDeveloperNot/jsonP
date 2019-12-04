#include "element_object.h"

#include <iostream>


//
//element_object::element_object() : element{object}
//{
//}
//
//
//element_object::~element_object()
//{
//	for (auto it = elements.begin(); it != elements.end(); it++) {
////std::cout << "1\n";
//		if (it->second->get_count() < 1) {
////std::cout << "deleting element with key: " << it->first << ", element type: " << it->second->get_type() << std::endl;
//			delete it->second;
//		}
//	}
//}
//
//
//void element_object::add_element(std::string key, element* e)
//{
////std::cout << "element_object::add_element key: " << key << ", " << e->get_type() << std::endl;
//	if (e != nullptr) {
////std::cout << "type: " << e->get_type() << std::endl;
////std::cout << "count: " << e->ref_count << std::endl;
////std::cout << "elements size: " << elements.size() << std::endl;
//		elements[key] = e;
//		e->incr_count();
//	}
//}
//
//
//bool element_object::contains_key(std::string & key)
//{
//	return (elements.count(key) > 0) ? true : false;
//}
//
//
//element_object* element_object::get_object_value() 
//{
//	return this;
//}
//
//
//element_object* element_object::get_as_object(std::string key) 
//{
//	if (elements.count(key) > 0)
//		return elements[key]->get_object_value();
//	else
//		throw jsonP_exception{"Key not found"};
//}
//
//
//std::vector<element*>& element_object::get_as_array(std::string key) 
//{
//	if (elements.count(key) > 0)
//		return elements[key]->get_array_value();
//	else
//		throw jsonP_exception{"Key not found"};
//}
//
//
//int element_object::get_as_numeric_int(std::string key) 
//{
//	if (elements.count(key) > 0)
//		return elements[key]->get_int_value();
//	else
//		throw jsonP_exception{"Key not found"};
//}
//
//
//long element_object::get_as_numeric_long(std::string key) 
//{
//	if (elements.count(key) > 0)
//		return elements[key]->get_long_value();
//	else
//		throw jsonP_exception{"Key not found"};
//}
//
//
//double element_object::get_as_numeric_double(std::string key) 
//{
//	if (elements.count(key) > 0)
//		return elements[key]->get_double_value();
//	else
//		throw jsonP_exception{"Key not found"};
//}
//
//
//bool element_object::get_as_boolean(std::string key) 
//{
//	if (elements.count(key) > 0)
//		return elements[key]->get_boolean_value();
//	else
//		throw jsonP_exception{"Key not found"};
//}
//
//
//std::string& element_object::get_as_string(std::string key) 
//{
//	if (elements.count(key) > 0)
//		return elements[key]->get_string_value();
//	else
//		throw jsonP_exception{"Key not found"};
//}
//
//
///*
// * no line return or indents
// */
//void element_object::stringify(std::string &doc)
//{
//	doc += "{";
//	
//	for (auto it = elements.begin(); it != elements.end(); ++it) {
////std::cout << "**" << std::endl;
////std::cout << "name: " << it->first << "\n";
////std::cout << "type: " << it->second->get_type() << "\n";
//		doc += quote;
//		doc += it->first;
//		doc += quote_colon;
//		if (it->second)
//			it->second->stringify(doc);
//		doc += comma;
////std::cout << doc << "\n\n";
//	}
//
//	if (doc[doc.length()-1] == ',')
//		doc[doc.length()-1] = '}';
//	else
//		doc += "}";
//}
