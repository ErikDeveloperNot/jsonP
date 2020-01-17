#ifndef IPUSH_HANDLER_H
#define IPUSH_HANDLER_H

#include "element.h"
#include <string>
#include <vector>
#include "jsonP_json.h"

//remove
#include <iostream>
//end



class IPush_handler 
{
public:
	virtual ~IPush_handler() {}
	
	//callbacks
	virtual bool get_element(const char *path) = 0;

	// memory that holds data for *val is only guaranteed to exist as long as this callback, 
	//if needded longer needs to be copied
	virtual void element_parsed(const char * path, element_type, const void *val) = 0;
};



class test_push_handler : public IPush_handler
{
public:
	char match1[25] = {"/widget/embed_array/2\0"};
	char match2[25] = {"/widget/main_window\0"};
	char match3[25] = {"/dont_use/bool_true\0"};
	char match4[25] = {"/dont_use/bool_false\0"};
	char match5[20] = {"/dont_use/null_key\0"};
	char match6[20] = {"/dont_use/float\0"};
	char match7[20] = {"/widget/int_long\0"};
	char match8[25] = {"/widget/embed_array/4\0"};
	
	jsonP_json *long_live = nullptr;
	char *pretty;
	
	test_push_handler() = default;
	
	~test_push_handler()
	{
		std::cout << "test_push_handler destructor\n";
		delete long_live;
	}
	
	virtual bool get_element(const char *path)
	{
		std::cout << "get_element called for path: " << path << std::endl;

		if (strcmp(path, match1) == 0 || strcmp(path, match2) == 0 || strcmp(path, match3) == 0 || strcmp(path, match4) == 0 ||
			strcmp(path, match5) == 0 || strcmp(path, match6) == 0 || strcmp(path, match7) == 0 || strcmp(path, match8) == 0) {
			std::cout << "\nReturning true for: " << path << std::endl;
			return true;
		}

		return false;

	}
	
	virtual void element_parsed(const char * path, element_type type, const void *val)
	{
		std::cout << "element_parsed: " << path << "\n";
		std::cout << "value type: " << get_element_type_string(type) << std::endl;

		if (type == string) {
			std::cout << "string value: " << (char*)val << std::endl;
		} else if (type == numeric_long) {
			long l = *(long*)val;
			std::cout << "numeric long value: " << l << std::endl;
		} else if (type == numeric_double) {
			double d = *(double*)val;
			std::cout << "numeric double value: " << d << std::endl;
		} else if (type == object) {
			jsonP_json *j = (jsonP_json*)val;
			object_id root = j->get_doc_root();
			unsigned int k_cnt = j->get_elements_count(root);
			std::cout << "\nObject key count=" << k_cnt << std::endl;
			pretty = j->stringify_pretty();
			std::cout << pretty << "\n\n";
			free(pretty);
			
			//make a copy
			long_live = new jsonP_json{*j};
			pretty = long_live->stringify_pretty();
			std::cout << pretty << "\n\n";
			free(pretty);
			delete long_live;
			long_live = nullptr;
		} else if (type == bool_true) {
			std::cout << "bool value: true" << std::endl;
		} else if (type == bool_false) {
			std::cout << "bool value: false" << std::endl;
		} else if (type == null) {
			std::cout << "Null value: null" << std::endl;
		} else if (type == array) {
			jsonP_json *j = (jsonP_json*)val;
			object_id root = j->get_doc_root();
			unsigned int k_cnt = j->get_elements_count(root);
			std::cout << "\nArray member count=" << k_cnt << std::endl;
			pretty = j->stringify_pretty();
			std::cout << pretty << "\n\n";
			free(pretty);
		}
		
		std::cout << std::endl;
	}
};

#endif //IPUSH_HANDLER_H