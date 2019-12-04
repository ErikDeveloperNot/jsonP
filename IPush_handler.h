#ifndef IPUSH_HANDLER_H
#define IPUSH_HANDLER_H

#include "element.h"
#include <string>
#include <vector>

//remove
#include <iostream>
//end

//
//class IPush_handler 
//{
//public:
//	virtual ~IPush_handler() {}
//	
//	virtual bool get_element(std::string &path, std::string &key) = 0;
//	virtual void element_parsed(std::string &path, std::string &key, element *) = 0;
//};
//
//
//
//class test_push_handler : public IPush_handler
//{
//public:
//	test_push_handler() = default;// : el_vec{new std::vector<element *>{}}
//	
//	~test_push_handler()
//	{
////		std::cout << "test_push_handler destructor\n";
//	}
//	
//	virtual bool get_element(std::string &path, std::string &key)
//	{
//		std::cout << "get_element called for path: " << path << ", key: " << key << ",   ";
//		
//		if ( key == "GlossDef") {
//			std::cout << "returning true\n";
//			return true;
//		} else {
//			std::cout << "returning false\n";
//			return false;
//		}
//	}
//	
//	virtual void element_parsed(std::string &path, std::string &key, element *ele)
//	{
//		std::cout << "element_parsed called for path: "<< path << ", key: " << key << 
//			", element type: " << ele->get_type_string() << ", element:\n";
//		
//		std::string s;
//		ele->stringify_pretty(s);
//		std::cout << s << "\n";
//		delete ele;
//	}
//};

#endif //IPUSH_HANDLER_H