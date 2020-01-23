#ifndef JSONP_PUSH_PARSER_H
#define JSONP_PUSH_PARSER_H

#include "jsonP_buffer_parser.h"
#include "IPush_handler.h"



class jsonP_push_parser : public jsonP_buffer_parser
{
private:
	IPush_handler *handler;
	
	
	char *cur_path;
	unsigned int cur_path_sz;
	unsigned int cur_path_l;
	unsigned int cur_key_i;
	char slash[3] = {"/\0"};
	char sprintf_buf[30];

	bool parsing;
	bool parsing_started;
	
	bool parsing_array;
	unsigned int array_i;
	
	
	inline void path_add(const char *key)
	{
		unsigned int len = strlen(key);
		cur_path_l += len + 1;
		
		if (cur_path_l + 5 > cur_path_sz) {
			//realloc
			cur_path_sz = (cur_path_sz + len) * 1.2;
			cur_path = (char*) realloc(cur_path, cur_path_sz);
		}

		cur_path = strcat(cur_path, slash);
		cur_path = strcat(cur_path, key);
	}
	
	inline void path_remove(unsigned int idx)
	{
		cur_path_l = idx;
		cur_path[cur_path_l] = '\0';
	}
	
	//overrides
	virtual void parse_key() override;
	virtual unsigned int parse_array() override;
	virtual unsigned int parse_object() override;
	virtual void parse_value() override;
	
	
public:
	jsonP_push_parser(std::string file_name, IPush_handler *handler, int buf_sz = 8192);
	jsonP_push_parser(IChunk_reader *reader, IPush_handler *handler, int buf_sz = 8192);
	
	~jsonP_push_parser();

	void parse();
};

#endif // JSONP_PUSH_PARSER_H
