#ifndef _JSONP_PARSER_H_
#define _JSONP_PARSER_H_

#include "jsonP_json.h"

#include <string>


const int ltr_b = (int)'b';
const int space = (int)' ';
const int tab = (int)'\t';
const int new_line = (int)'\n';
const int car_return = (int)'\r';
const int quote_int = (int)'"';
const int bk_slsh = (int)'\\';
const int fwd_slsh = (int)'/';
const int comma_int = (int)',';
const int lft_curly = (int)'{';
const int rt_curly = (int)'}';
const int lft_brac = (int)'[';
const int rt_brac = (int)']';
const int colon = (int)':';
const int ltr_f = (int)'f';
const int ltr_r = (int)'r';
const int ltr_n = (int)'n';
const int ltr_t = (int)'t';
const int ltr_u = (int)'u';
const int ltr_e = (int)'e';
const int ltr_E = (int)'E';
const int ltr_T = (int)'T';
const int ltr_F = (int)'F';
const int zero = (int)'0';
const int nine = (int)'9';
const int plus = (int)'+';
const int minus = (int)'-';
const int period = (int)'.';



struct parse_stats
{
	unsigned short stack_buf_increases = 0;
	unsigned short data_increases = 0;
};
	
	
/*
 * borrowed from Chad Austin, sajson; who borrowed it from Rich Geldreich's Purple JSON parser
 */
const uint8_t parse_flags[256] = {
//  0    1    2    3    4    5    6    7      8    9    A    B    C    D    E    F
	0,   0,   0,   0,   0,   0,   0,   0,     0,   2,   2,   0,   0,   2,   0,   0, // 0
	0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 1
	3,   1,   0,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   0x11,1, // 2
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,  0x11,0x11,1,   1,   1,   1,   1,   1, // 3
	1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 4
	1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   0,   1,   1,   1, // 5
	1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 6
	1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 7

// 128-255
	0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0
};



class jsonP_parser
{
protected:
	std::string json_str;
	char * json = nullptr;
	unsigned int json_length;
	unsigned int index;
	bool look_for_key;
	unsigned int value_start;
	
	// STUFF for new modle to rid std::map and std::new for each element
	byte * stack_buf;
	unsigned int stack_buf_sz;
	unsigned int stack_i;
	byte * data;
	unsigned int data_sz;
	unsigned int data_i;

	//parse options
	unsigned short options;
	bool use_json;
	bool shrink_buffers;
	bool dont_sort_keys;

	parse_stats stats;
	// END STUFF for new modle to rid std::map and std::new for each element


	//error stuff
	std::string error_string;
	int error_index;
	std::string error_json_snip;
	
	//virtual parse commands
	virtual inline __attribute__((always_inline)) void eat_whitespace() {
//		while ((json[index] == space || json[index] == tab || json[index] == new_line || 
//			json[index] == car_return))// && json_length > index) 
//			index++;
		while (parse_flags[json[index]] & 2)
			index++;
	}
	

	virtual void parse_key();
	virtual element_type parse_numeric();
	virtual unsigned int parse_array();
	virtual unsigned int parse_object();
	virtual void parse_value();
	
	void set_error(std::string error);

void test_parse_object(unsigned int);
void test_parse_array(unsigned int);



public:
	jsonP_parser() = default;
	jsonP_parser(std::string & json, unsigned short options = 0);
	jsonP_parser(char * json, unsigned int, unsigned short options = 0);
	
	~jsonP_parser();

	jsonP_json * parse();
	jsonP_json * parse(std::string & json);
	jsonP_json * parse(char * json, unsigned int);

	std::string get_error_string() { return error_string; }
	int get_error_index() { return error_index; }
	std::string get_error_snip(int chars_before, int chars_after);
	
	parse_stats get_parse_stats() { return stats; }
};

#endif // _JSONP_PARSER_H_
