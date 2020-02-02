#include "jsonP_push_parser.h"



jsonP_push_parser::jsonP_push_parser(std::string file_name, IPush_handler *handler, int buf_sz) : 
jsonP_buffer_parser(file_name, buf_sz),
handler{handler},
cur_path_sz{1024},
cur_path_l{0},
cur_key_i{0},
parsing{false},
parsing_started{false},
parsing_array{false},
array_i{0}
{
	options = PRESERVE_JSON | DONT_SORT_KEYS;
	cur_path = (char*) malloc(1024);
}


jsonP_push_parser::jsonP_push_parser(IChunk_reader* reader, IPush_handler *handler, int buf_sz) : 
jsonP_buffer_parser(reader, buf_sz),
handler{handler},
cur_path_sz{1024},
cur_path_l{0},
cur_key_i{0},
parsing{false},
parsing_started{false},
parsing_array{false},
array_i{0}
{
	options = PRESERVE_JSON | DONT_SORT_KEYS;
	cur_path = (char*) malloc(1024);
}


jsonP_push_parser::~jsonP_push_parser()
{
//	std::cout << "jsonP_push_parser destructor\n";
	free(cur_path);
}


void jsonP_push_parser::parse()
{
	cur_path[0] = '\0';
	cur_path_l = 0;
	jsonP_json *doc = jsonP_buffer_parser::parse();
	delete doc;
}


/*
 * overrides
 */
void jsonP_push_parser::parse_key()
{
	// verify not parsing a string value, but rather a key
	if (look_for_key) {
		unsigned int start_stack_i = stack_i;
		jsonP_buffer_parser::parse_key();

		cur_key_i = cur_path_l;
		path_add(&data[get_key_location(stack_buf, start_stack_i)]);

		if (!parsing && handler->get_element(cur_path)) {
			parsing = true;
			parsing_started = false;
		}
	} else {
		jsonP_buffer_parser::parse_key();
	}
}


unsigned int jsonP_push_parser::parse_array()
{
	unsigned int start_key_i = cur_key_i;
	bool reset_index = parsing ? false : true;
	unsigned int start_stack_i = stack_i;
	
	bool start_parsing_array = parsing_array;
	unsigned int start_array_i = array_i;
	
	parsing_array = true;
	array_i = 0;
	
	unsigned int to_return = jsonP_buffer_parser::parse_array();
	
	if (reset_index) {
//std::cout << "resetting stack index from: " << stack_i << " to " << start_stack_i << std::endl;
		stack_i = start_stack_i;
	}
	
	parsing_array = start_parsing_array;
	array_i = start_array_i;
		
	return to_return;
}


unsigned int jsonP_push_parser::parse_object()
{
	unsigned int start_path_l = cur_path_l;
	unsigned int start_key_i = cur_key_i;

	bool reset_index = parsing ? false : true;
	unsigned int start_stack_i = stack_i;
	
	bool start_parsing_array = parsing_array;
	unsigned int start_array_i = array_i;
	
	parsing_array = false;
	
	unsigned int to_return = jsonP_buffer_parser::parse_object();
		
	if (!start_parsing_array) { 
		if (start_key_i < cur_path_l)
			path_remove(start_key_i);
	}
	
	if (reset_index) {
//std::cout << "resetting stack index from: " << stack_i << " to " << start_stack_i << std::endl;
		stack_i = start_stack_i;
	}
	
	parsing_array = start_parsing_array;
	array_i = start_array_i;
	
	return to_return;
}


void jsonP_push_parser::parse_value()
{
	unsigned int start_key_i = cur_key_i;
	unsigned int start_path_l = cur_path_l;
	unsigned int array_key_i;
	
	bool reset_index = parsing ? false : true;
	unsigned int start_data_i = data_i;
	unsigned int start_stack_i = stack_i;
	bool start_parsing_array = parsing_array;
	unsigned int start_array_i = array_i;
	
	bool root_parse{false};
	
	
	if (parsing && !parsing_started) {
		root_parse = true;
		parsing_started = true;
	} else if (parsing_array) {
		array_key_i = cur_path_l;		
		sprintf(sprintf_buf, "%u",array_i);
		path_add(sprintf_buf);
		
		if (!parsing && handler->get_element(cur_path)) {
			root_parse = true;
			parsing_started = true;
			parsing = true;
		}
	}
	
	
	jsonP_buffer_parser::parse_value();
	
	
	if (root_parse) {
		element_type type = get_element_type(stack_buf, start_stack_i);
	
		unsigned int k_loc = get_key_location(stack_buf, start_stack_i);
		size_t k_len = strlen(&data[k_loc]);
		
		if (type == string) {
			if (!start_parsing_array) 
				handler->element_parsed(cur_path, type, (const void*) &data[k_loc + k_len + 1]);
			else
				handler->element_parsed(cur_path, type, (const void*) &data[k_loc]);
		} else if (type == numeric_long) {
			int value;
			
			if (!start_parsing_array) {
				value = atol(&data[k_loc + k_len + 1]);
			} else {
				value = atol(&data[k_loc]);
			}
			
			handler->element_parsed(cur_path, type, &value);
		} else if (type == numeric_double) {
			double value;

			if (!start_parsing_array) {
				value = atof(&data[k_loc + k_len + 1]);
			} else {
				value = atof(&data[k_loc]);
			}
			
			handler->element_parsed(cur_path, type, &value);
		} else if (type == object_ptr) {
			jsonP_json json{data, data, data_sz, data_sz, k_loc-1, DONT_SORT_KEYS | WEAK_REF};
			handler->element_parsed(cur_path, object, &json);
		} else if (type == array_ptr) {
			jsonP_json json{data, data, data_sz, data_sz, k_loc-1, DONT_SORT_KEYS | WEAK_REF};
			handler->element_parsed(cur_path, array, &json);
		} else {
			handler->element_parsed(cur_path, type, NULL);
		}
		
		parsing = false;
		parsing_started = false;
		
		//always reset index after root parse
//std::cout << "resetting indexs from: " << data_i << ":" << stack_i << " to " << start_data_i << ":" << start_stack_i << std::endl;
		data_i = start_data_i;
		stack_i = start_stack_i;
	}

	if (start_parsing_array) {
		parsing_array = start_parsing_array;
		array_i = start_array_i + 1;
		path_remove(array_key_i);
	} else {
		if (start_key_i < cur_path_l) 
			path_remove(start_key_i);
	}

	if (reset_index) {
//std::cout << "resetting data index from: " << data_i << " to " << start_data_i << std::endl;
		data_i = start_data_i;
	}
	
	cur_key_i = start_key_i;

}
