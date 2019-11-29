#ifndef JSONP_PUSH_PARSER_H
#define JSONP_PUSH_PARSER_H

#include "jsonP_buffer_parser.h"
#include "IPush_handler.h"

#include <string>
#include <vector>


class jsonP_push_parser : public jsonP_buffer_parser
{
private:
	IPush_handler *handler;
	
	std::vector<std::string> path_tokens;
	std::string cur_path;
	std::string cur_key;
	std::string par_path;
	std::string par_key;
//	element_type type;
	bool parsing;
	bool parsing_started;
	bool process_element;
	
	inline void set_cur_path() {
		cur_path = "";
		for (std::string &s : path_tokens) cur_path += s;
	}
	
	//overrides
	virtual void parse_key(std::string &) override;
	virtual void parse_bool(bool &) override;
//	virtual element_type parse_numeric(std::string &) override;
virtual element_type parse_numeric() override;
	virtual unsigned int parse_array(element_array *&) override;
	virtual unsigned int parse_object(element_object *&) override;
	virtual void parse_value(element *&) override;
	
	virtual inline element_string * create_string_element(std::string &str) override { 
		return ((parsing) ? new element_string{str} : nullptr); 
	}
		
	virtual inline element_boolean * create_boolean_element(bool b) override { 
		return ((parsing) ? new element_boolean{b} : nullptr);
	}
	
//	virtual inline element_numeric * create_int_element(std::string &str) override { 
//		return ((parsing) ? new element_numeric{atoi(str.c_str())} : nullptr);
//	}
	virtual inline element_numeric * create_int_element(char *ptr) override { 
		return ((parsing) ? new element_numeric{ptr, numeric_int} : nullptr);
	}	
	
//	virtual inline element_numeric * create_long_element(std::string &str) override { 
//		return ((parsing) ? new element_numeric{atol(str.c_str())} : nullptr); 
//	}
	virtual inline element_numeric * create_long_element(char *ptr) override { 
		return ((parsing) ? new element_numeric{ptr, numeric_long} : nullptr);
	}	

//	virtual inline element_numeric * create_float_element(std::string &str) override { 
//		return ((parsing) ? new element_numeric{atof(str.c_str())} : nullptr); 
//	}
	virtual inline element_numeric * create_float_element(char *ptr) override { 
		return ((parsing) ? new element_numeric{ptr, numeric_double} : nullptr);
	}	

	virtual inline element_null * create_null_element() override { 
		return ((parsing) ? new element_null{} : nullptr); 
	}
	
	virtual inline element_array * create_element_array(element_type t) override { 
		return ((parsing) ? new element_array{t} : nullptr); 
	}
	
	virtual inline void add_array_element(element_array *a, element *e) override { 
		if (a && e) a->add_element(e); 
	}
	
	virtual inline element_object * create_element_object() override { 
		return ((parsing) ? new element_object{} : nullptr);
	}
	
	virtual inline void add_object_element(element_object *o, element *e, std::string &k) override { 
		if (o && e) o->add_element(k, e); 
	}
	
public:
//	jsonP_push_parser(std::string json);
	jsonP_push_parser(std::string file_name, IPush_handler *handler, int buf_sz = 8192);
	jsonP_push_parser(IChunk_reader *reader, IPush_handler *handler, int buf_sz = 8192);
	
	~jsonP_push_parser();

	void parse();
};

#endif // JSONP_PUSH_PARSER_H
