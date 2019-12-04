#include "jsonP_push_parser.h"



//
//jsonP_push_parser::jsonP_push_parser(std::string file_name, IPush_handler *handler, int buf_sz) : 
//jsonP_buffer_parser(file_name, buf_sz),
//handler{handler},
//cur_path{""},
//cur_key{""},
//par_path{""},
//par_key{""},
//parsing{false},
//parsing_started{false},
//process_element{false}
//{
//}
//
//
//jsonP_push_parser::jsonP_push_parser(IChunk_reader* reader, IPush_handler *handler, int buf_sz) : 
//jsonP_buffer_parser(reader, buf_sz),
//handler{handler},
//cur_path{""},
//cur_key{""},
//par_path{""},
//par_key{""},
//parsing{false},
//parsing_started{false},
//process_element{false}
//{
//}
//
//
//jsonP_push_parser::~jsonP_push_parser()
//{
////	std::cout << "jsonP_push_parser destructor\n";
//}
//
//
//void jsonP_push_parser::parse()
//{
//	path_tokens.push_back("");
//	set_cur_path();
//	jsonP_doc *doc = jsonP_buffer_parser::parse();
//	delete doc;
//}
//
//
///*
// * overrides
// */
//void jsonP_push_parser::parse_key(std::string & value)
//{
////std::cout << "  parse_key: " << value << ", look_for_key: " << look_for_key << "\n";	
//	// verify not parsing a string value, but rather a key
//	if (look_for_key) {
//		jsonP_buffer_parser::parse_key(value);
//		cur_key = value;
//		
//		if (!parsing && handler->get_element(cur_path, cur_key)) {
//			par_key = value;
//			par_path = cur_path;
//			parsing = true;
//			parsing_started = false;
//		}
//	} else {
//		std::string key{cur_key};
//		bool root_parse{false};
//		
//		if (parsing && !parsing_started) {
//			root_parse = true;
//			parsing_started = true;
//		}
//			
//		jsonP_buffer_parser::parse_key(value);
//		
//		if (root_parse)
//			process_element = true;
//	}
//}
//
//
//unsigned int jsonP_push_parser::parse_array(element_array *& value)
//{
//	std::string key{cur_key};
//	bool root_parse{false};
//	
//	if (parsing && !parsing_started) {
//		root_parse = true;
//		parsing_started = true;
//	}
//		
//	unsigned int to_return = jsonP_buffer_parser::parse_array(value);
//	
//	if (root_parse)
//		process_element = true;
//		
//	return to_return;
//}
//
//
//unsigned int jsonP_push_parser::parse_object(element_object *& value)
//{
//	std::string key{cur_key};
//	bool root_parse{false};
//	
//	if (parsing && !parsing_started) {
//		root_parse = true;
//		parsing_started = true;
//	}
//	
//	path_tokens.push_back(key + "/");
//	set_cur_path();
//	
//	unsigned int to_return = jsonP_buffer_parser::parse_object(value);
//	
//	if (root_parse)
//		process_element = true;
//		
//	path_tokens.pop_back();
//	set_cur_path();
//	
//	return to_return;
//}
//
//
////element_type jsonP_push_parser::parse_numeric(std::string & value)
//element_type jsonP_push_parser::parse_numeric()
//{
//	std::string key{cur_key};
//	bool root_parse{false};
//	
//	if (parsing && !parsing_started) {
//		root_parse = true;
//		parsing_started = true;
//	}
//	
////	element_type type = jsonP_buffer_parser::parse_numeric(value);
//element_type type = jsonP_buffer_parser::parse_numeric();
//	
//	if (root_parse)
//		process_element = true;
//		
//	return type;
//}
//
//
//void jsonP_push_parser::parse_bool(bool & value)
//{
//	std::string key{cur_key};
//	bool root_parse{false};
//	
//	if (parsing && !parsing_started) {
//		root_parse = true;
//		parsing_started = true;
//	}
//	
//	jsonP_buffer_parser::parse_bool(value);
//	
//	if (root_parse)
//		process_element = true;
//}
//
//
//void jsonP_push_parser::parse_value(element *& value)
//{
//	std::string key{cur_key};
//	jsonP_buffer_parser::parse_value(value);
//
//	if (process_element) {
////		std::cout << "\nparse_value has value that needs to be returned\n";
//		handler->element_parsed(cur_path, par_key, value);
//		parsing = false;
//		parsing_started = false;
//		par_key = "";
//		par_path = "";
//		process_element = false;
//	}
//	
//	cur_key = key;
//}
