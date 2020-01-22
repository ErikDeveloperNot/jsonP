#include "jsonP_parser.h"
//#include <chrono>



jsonP_parser::jsonP_parser(std::string & json_, unsigned short options) : 
json_str{json_}, 
look_for_key{false}, 
stack_i{0}, 
data_i{0},
options{options}
{
	use_json = (options & PRESERVE_JSON) ? false : true;
	shrink_buffers = (options & SHRINK_BUFS) ? true : false;
	dont_sort_keys = (options & DONT_SORT_KEYS) ? true : false;

	// For now just make initial 100kb, will be configurable
	stack_buf_sz = 1024;// * 10 * 10;
	stack_buf = (byte*) malloc(stack_buf_sz);

	if (use_json)
		data_sz = json_str.length() / 4;
	else
		data_sz = json_str.length();
	
	data = (byte*) malloc(data_sz); 
}


jsonP_parser::jsonP_parser(char * json_, unsigned long length, unsigned short options) : 
json{json_}, 
json_length{length}, 
look_for_key{false},
stack_i{0}, 
data_i{0},
options{options}
{
	use_json = (options & PRESERVE_JSON) ? false : true;
	shrink_buffers = (options & SHRINK_BUFS) ? true : false;
	dont_sort_keys = (options & DONT_SORT_KEYS) ? true : false;

//std::cout << "OPTIONS: " << options << ", (options & DONT_SORT_KEYS)=" << (options & DONT_SORT_KEYS) << std::endl;

	stack_buf_sz = 1024;// * 10 * 10;
	stack_buf = (byte*) malloc(stack_buf_sz);

	if (use_json)
		data_sz = length / 4;
	else
		data_sz = length;
		
	data = (byte*) malloc(data_sz);   
//	std::cout << "use_json:" << use_json << ", shrink_buffers:" << shrink_buffers << std::endl;
}


jsonP_parser::~jsonP_parser()
{
	free(stack_buf);
	
	std::cout << "End jsonP_parser destructor" << std::endl;
}


jsonP_json * jsonP_parser::parse()
{
	if (json != nullptr)
		return parse(json, json_length);
	else
		return parse((char*)json_str.c_str(), json_str.length());
}


jsonP_json * jsonP_parser::parse(std::string & json_)
{
	return parse((char*)json_.c_str(), json_.length());
}


jsonP_json * jsonP_parser::parse(char * json_, unsigned long length)
{
	json = json_;
	json_length = length;
	jsonP_json *jsonPjson = nullptr;
	
	index = 0;
	
	if (json_length < 2)
		throw jsonP_exception{"Error parsing into a json doc"};

	eat_whitespace();
	
	if (json[index] == '{') {
		stack_i += obj_member_sz;
//		*(unsigned int*)&stack_buf[obj_member_key_offx] = 0;
		set_key_offx_value(stack_buf, 0, 0);
//		*(element_type*)&stack_buf[0] = object;
		set_element_type(stack_buf, 0, object);
		data[data_i+4] = '\0';
		data_i = 5;

		unsigned long i = parse_object();
		
		if (shrink_buffers) {
//			std::cout << "shrinking data from: " << data_sz << ", to: " << data_i << std::endl;
			data = (byte*)realloc(data, data_i);
			
			if (!use_json)
				length = data_i;
		}
		
		if (use_json)
			jsonPjson = new jsonP_json{json, data, length, data_i, i, options};//i+5};
		else
			jsonPjson = new jsonP_json{data, data, length, data_i, i, options};//i+5};
			
//		std::cout << "stack_i = " << stack_i << "\ndata_i = " << data_i << ", i = " << i << std::endl;
//		std::cout << "root type: " << *((element_type*)&stack_buf[0]) << ", key: " << (unsigned int)data[*(unsigned int*)&stack_buf[1]] << std::endl;
//		std::cout << "root2 type: " << *(element_type*)&data[i] << ", Key: " << *(unsigned int*)&data[i+1] << std::endl;
//		std::cout << "length: " << length << std::endl;

//test_parse_object(i+5);

	} else if (json[index] == '[') {
		stack_i += arry_member_sz;
//		*(unsigned int*)&stack_buf[arry_member_val_offx] = 0;
		set_key_offx_value(stack_buf, 0, 0);
//		*(element_type*)&stack_buf[0] = array;
		set_element_type(stack_buf, 0, array);
		data[data_i+4] = '\0';
		data_i = 5;

		unsigned long i = parse_array();
	
		if (shrink_buffers) {
//			std::cout << "shrinking data from: " << data_sz << ", to: " << data_i << std::endl;
			data = (byte*)realloc(data, data_i);
			
			if (!use_json)
				length = data_i;
		}

		if (use_json)
			jsonPjson = new jsonP_json{json, data, length, data_i, i};//i+5};
		else
			jsonPjson = new jsonP_json{data, data, length, data_i, i};//i+5};
		
//test_parse_array(i+5);

	} else {
		set_error("Error parsing json text, does not appear to be an object or an array");
		throw jsonP_exception{"Error parsing json text, does not appear to be an object or an array"};
	}
		
	return jsonPjson;
}


void jsonP_parser::parse_key()
{
	index++;
unsigned long start = index;	

	while (true) {
//		if (json[index] == '\\') {
		if ((int)json[index] == bk_slsh) {
//			switch (json[index+1])
			switch ((int)json[index+1])
			{
//				case '\\' :
//				case '"' :
//				case '/':
				case bk_slsh :
				case quote_int :
				case fwd_slsh :
				{
//					key += json[++index];
//					index++;
					index += 2;
					break;
				}
//				case 'b' :
//				case 'f' :
//				case 'r' :
//				case 'n' :
//				case 't' :
//				case 'u' :			//treat the same as control chars for now
				case ltr_b :
				case ltr_f :
				case ltr_r :
				case ltr_n :
				case ltr_t :
				case ltr_u :			//treat the same as control chars for now
				{
//					index++;
//					key += json[index++];
					index++;
					break;
				}
				default :
				{
					std::string err = "parse Error, invalid escape found at index: " + std::to_string(index);
					set_error(err);
					throw jsonP_exception{err.c_str()};
				}
			}
//		}	else if (json[index] == '"') {
		}	else if ((int)json[index] == quote_int) {
			if (use_json)
				json[index] = '\0';
			
			break;
		} else {
			index++;
		}
	}

	index++;

	if (!look_for_key) {
		if (use_json) {
			while (start < index)
				json[value_start++] = json[start++];
		} else {
			if (increase_data_buffer(index - start + 5)) {
//				std::cout << "increasing the data buffer" << std::endl;
				data_sz = (unsigned long)data_sz * 1.2 + index - start;
				data = (byte*) realloc(data, data_sz);
				stats.data_increases++;
			}

			while (start < index)
				data[data_i++] = json[start++];
				
			data[data_i-1] = '\0';
		}
	} else if (!use_json) {
		if (increase_data_buffer(index - start + 5)) {
//			std::cout << "increasing the data buffer" << std::endl;
			data_sz = (unsigned long)data_sz * 1.2 + index - start;
			data = (byte*) realloc(data, data_sz);
			stats.data_increases++;
		}
			
		while (start < index)
			data[data_i++] = json[start++];
			
		data[data_i-1] = '\0';
	}
}


element_type jsonP_parser::parse_numeric()
{
	bool is_long{true};
	int c = (int)json[index];
	bool sign {true};
	bool exp{false};
	bool exp_sign{false};
	bool dec{true};

	unsigned long s = index-1;

//	while (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ',' && c != ']' && c != '}') {
	while (c != space && c != tab && c != new_line && c != car_return && c != comma_int && c != rt_brac && c != rt_curly) {
//		if (c >= '0' && c <= '9') {
		if (c >= zero && c <= nine) {
//			zer0 = true;
			exp_sign = false;
		/*} else if (c == '0' && zer0) {
			exp_sign = false;*/
//		} else if ((c == 'e' || c == 'E') /*&& zer0*/ && !exp) {
		} else if ((c == ltr_e || c == ltr_E) /*&& zer0*/ && !exp) {
			exp = true;
			exp_sign = true;
			dec = false;
			is_long = false;
//		} else if (c == '.' && dec) {
		} else if (c == period && dec) {
			exp = false;
			is_long = false;
//		} else if ((c == '-' || c == '+') && (sign || exp_sign)) {
		} else if ((c == minus || c == plus) && (sign || exp_sign)) {
			exp_sign = false;
		} else {
			std::string err = "parse error, trying to parse numeric value at index: " + std::to_string(index);
			set_error(err);
			throw jsonP_exception{err.c_str()};
		}
		
		sign = false;
//		c = json[++index];
		c = (int)json[++index];
	}
	
	if (use_json) {
		while (s < index-1) {
			json[value_start++] = json[s+1];
			s++;
		}
		
		json[value_start] = '\0';
	} else {
		if (index+1 - s >= data_sz - data_i) {
			data_sz = (unsigned long)data_sz * 1.2 + index - s;
			data = (byte*) realloc(data, data_sz);
			stats.data_increases++;
		}
		
		while (s < index-1) {
			data[data_i++] = json[s+1];
			s++;
		}
		
		data[data_i++] = '\0';
	}

	if (is_long)
		return numeric_long;
	else
		return numeric_double;

}


void jsonP_parser::parse_value()
{
	//figure out what type of data type this is
//	if (json[index] == '{') {
	if ((int)json[index] == lft_curly) {
		//object
//		*((element_type*)&stack_buf[stack_i]) = object;
		set_element_type(stack_buf, stack_i, object);
		stack_i += obj_member_sz;
		parse_object();
//	} else if (json[index] == '[') {
	} else if ((int)json[index] == lft_brac) {
		//array
//		*((element_type*)&stack_buf[stack_i]) = array_ptr;
		set_element_type(stack_buf, stack_i, array_ptr);
		stack_i += arry_member_sz;
		parse_array();
//	} else if (json[index] == '"') {
	} else if ((int)json[index] == quote_int) {
		//string
//		*((element_type*)&stack_buf[stack_i]) = string;
		set_element_type(stack_buf, stack_i, string);
		parse_key();
	} else if (json[index] == 't') {
		if (json[++index] == 'r' && json[++index] == 'u' && json[++index] == 'e') {
			set_element_type(stack_buf, stack_i, bool_true);
			++index;
		} else {
			std::string err = "Invalid bool value found at index: " + std::to_string(index);
			set_error(err);
			throw jsonP_exception{err.c_str()};
		}
	} else if (json[index] == 'f') {
		if (json[++index] == 'a' && json[++index] == 'l' && json[++index] == 's' && json[++index] == 'e') {
			set_element_type(stack_buf, stack_i, bool_false);
			++index;
		} else {
			std::string err = "Invalid bool value found at index: " + std::to_string(index);
			set_error(err);
			throw jsonP_exception{err.c_str()};
		}

//	} else if ((json[index] >= '0' && json[index] <= '9') || json[index] == '-' || json[index] == '+') {
	} else if (((int)json[index] >= zero && (int)json[index] <= nine) || (int)json[index] == minus || (int)json[index] == plus) {
//		std::string number;
		int s = index;
		
//		switch (parse_numeric(number))//start, end)) 
		switch (parse_numeric())
		{
			case numeric_int :
			{
//				*((element_type*)&stack_buf[stack_i]) = numeric_int;
				set_element_type(stack_buf, stack_i, numeric_int);
				break;
			}
			case numeric_long :
			{
//				*((element_type*)&stack_buf[stack_i]) = numeric_long;
				set_element_type(stack_buf, stack_i, numeric_long);
				break;
			}
			case numeric_double :
			{
//				*((element_type*)&stack_buf[stack_i]) = numeric_double;
				set_element_type(stack_buf, stack_i, numeric_double);
				break;
			}
			default :
				std::string err = "parse error, invalid return type from parse_numeric at index: " + std::to_string(index);
				set_error(err);
				throw jsonP_exception{err.c_str()};
		}

//	} else if (json[index] == 'n') { 
	} else if ((int)json[index] == ltr_n) { 
		if (json[++index] == 'u' && json[++index] == 'l' && json[++index] == 'l') {
//			*((element_type*)&stack_buf[stack_i]) = null;
			set_element_type(stack_buf, stack_i, null);
			index++;
		} else {
			std::string err = "parse error, trying to get null value at index: " + std::to_string(index);
			set_error(err);
			throw jsonP_exception{err.c_str()};
		}
	} else {
		std::string err = "parse error, trying to get value at index: " + std::to_string(index);
		set_error(err);
		throw jsonP_exception{err.c_str()};
	}
}



unsigned long jsonP_parser::parse_array()
{
	// create local index for this obj record and advace global index
	unsigned long loc_stack_i = stack_i; // - obj_member_sz;
	unsigned long num_elements = 0;
	unsigned long to_return;	//most likely will need
	//std::cout << "Addess: " << &*((char**)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx]) << std::endl;
//	*((element_type*)&stack_buf[loc_stack_i]) = array;
	set_element_type(stack_buf, loc_stack_i, array);
//	*(unsigned int*)&stack_buf[loc_stack_i + arry_member_val_offx] = *(unsigned int*)&stack_buf[loc_stack_i - arry_member_sz + arry_member_val_offx];
	set_key_offx_value(stack_buf, loc_stack_i, get_val_location(stack_buf, loc_stack_i - arry_member_sz));

	stack_i += (arry_member_sz + arry_root_sz);
	value_start = index;
	
//	*(unsigned int*)&stack_buf[stack_i + arry_member_val_offx] = (use_json) ? index : data_i;
	set_key_offx_value(stack_buf, stack_i, (use_json) ? index : data_i);
		
	++index;
	eat_whitespace();
	
	//make sure array is empty
//	if (json[index] == ']') {
	if ((int)json[index] == rt_brac) {
//		*((unsigned int*)&stack_buf[loc_stack_i + arry_member_sz]) = num_elements;
		set_uint_a_indx(stack_buf, loc_stack_i + arry_member_sz, num_elements);

//		std::cout << "Arry number keys:" << *((unsigned int*)&stack_buf[loc_stack_i + obj_member_sz]) << ", loc_stack_i: " << loc_stack_i <<
//			", for key index: " << *((int*)&stack_buf[loc_stack_i + obj_member_sz]) << std::endl; 

//		*((element_type*)&stack_buf[stack_i]) = extended;
		set_element_type(stack_buf, stack_i, extended);
//		*((unsigned int*)&stack_buf[stack_i + arry_member_val_offx]) = 0; 
		set_key_offx_value(stack_buf, stack_i, 0);
		stack_i += arry_member_sz;

		if (increase_data_buffer(stack_i - loc_stack_i + arry_member_sz)) {
//			std::cout << "increasing the data buffer" << std::endl;
			data_sz = (unsigned long)data_sz * 1.2 + stack_i - loc_stack_i;
			data = (byte*) realloc(data, data_sz);
		}

		memcpy(&data[data_i], &stack_buf[loc_stack_i], stack_i - loc_stack_i);

//		*(unsigned int*)&stack_buf[loc_stack_i - arry_member_sz + arry_member_val_offx] = data_i + arry_member_val_offx;
		set_key_offx_value(stack_buf, loc_stack_i - arry_member_sz, data_i + arry_member_val_offx);
//		*(element_type*)&stack_buf[loc_stack_i - obj_member_sz] = array_ptr;
		set_element_type(stack_buf, loc_stack_i - obj_member_sz, array_ptr);

		to_return = data_i;
		data_i += (stack_i - loc_stack_i);
		stack_i = loc_stack_i - arry_member_sz;
		
		index++;
		return to_return;
	}
	
	parse_value();
	num_elements++;
	stack_i += arry_member_sz;

	bool look_for_value{false};
	
	while (true) {

		if (increase_stack_buffer()) {
//			std::cout << "old stack_buf_sz: " << stack_buf_sz;
			stack_buf_sz = (unsigned long) stack_buf_sz * 1.2;
//			std::cout << ", new stack_buf_sz: " << stack_buf_sz << std::endl;
			stack_buf = (byte*) realloc(stack_buf, stack_buf_sz);
			stats.stack_buf_increases++;
		}

		eat_whitespace();
		
//		if (json[index] == ']') {
		if ((int)json[index] == rt_brac) {
			// end of array
			if (look_for_value) { 
				std::string err = "parse error, in array, looking for another value at index: " + std::to_string(index);
				set_error(err);
				throw jsonP_exception{err.c_str()};
			} else {
				index++;
				break;
			}
//		} else if (json[index] == ',') {
		} else if ((int)json[index] == comma_int) {
			// more elements
			if (look_for_value) { 
				std::string err = "parse error, in array, looking for another value at index: " + std::to_string(index);
				set_error(err);
				throw jsonP_exception{err.c_str()};
			} else {
				value_start = index;
//				*(unsigned int*)&stack_buf[stack_i + arry_member_val_offx] = (use_json) ? index : data_i;
				set_key_offx_value(stack_buf, stack_i, (use_json) ? index : data_i);

				index++;
				look_for_value = true;
			}
		} else {
			look_for_value = false;
			parse_value();
			num_elements++;
			stack_i += arry_member_sz;
		}
	}
	
//	*((unsigned int*)&stack_buf[loc_stack_i + arry_member_sz]) = num_elements;
	set_uint_a_indx(stack_buf, loc_stack_i + arry_member_sz, num_elements);
	
//	*((element_type*)&stack_buf[stack_i]) = extended;
	set_element_type(stack_buf, stack_i, extended);
//	*((unsigned int*)&stack_buf[stack_i + arry_member_val_offx]) = 0; 
	set_key_offx_value(stack_buf, stack_i, 0);
	stack_i += arry_member_sz;

	if (increase_data_buffer(stack_i - loc_stack_i + arry_member_sz)) { 
//		std::cout << "increasing the data buffer" << std::endl;
		data_sz = (unsigned long) data_sz * 1.2 + stack_i - loc_stack_i;
		data = (byte*) realloc(data, data_sz);
		stats.data_increases++;
	}

	memcpy(&data[data_i], &stack_buf[loc_stack_i], stack_i - loc_stack_i);

//	*(unsigned int*)&stack_buf[loc_stack_i - arry_member_sz + arry_member_val_offx] = data_i + arry_member_val_offx;
	set_key_offx_value(stack_buf, loc_stack_i - arry_member_sz, data_i + arry_member_val_offx);
//	*(element_type*)&stack_buf[loc_stack_i - obj_member_sz] = array_ptr;
	set_element_type(stack_buf, loc_stack_i - obj_member_sz, array_ptr);

	to_return = data_i;
	data_i += (stack_i - loc_stack_i);
	stack_i = loc_stack_i - arry_member_sz;
	
	return to_return;
}


unsigned long jsonP_parser::parse_object()
{
	// create local index for this obj record and advace global index
	unsigned long loc_stack_i = stack_i; // - obj_member_sz;
	unsigned long num_keys = 0;
	unsigned long to_return;
	//std::cout << "Addess: " << &*((char**)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx]) << std::endl;
//	*((element_type*)&stack_buf[loc_stack_i]) = object;
	set_element_type(stack_buf, loc_stack_i, object);
//	*(unsigned int*)&stack_buf[loc_stack_i + obj_member_key_offx] = *(unsigned int*)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx];
	set_key_offx_value(stack_buf, loc_stack_i, get_key_location(stack_buf, loc_stack_i - obj_member_sz));
	
	stack_i += (obj_root_sz + obj_member_sz);

	bool keep_going{true};
	bool local_look_for_key{true};
	
//	if (json[index] != '{') {
	if ((int)json[index] != lft_curly) {
		std::string err = "parse error, expected '{' at index: " + std::to_string(index);
		set_error(err);
		throw jsonP_exception{err.c_str()};
	}
	
	++index;
	
	try {
		while (keep_going) {
			if (increase_stack_buffer()) {
				stack_buf_sz += 512;
				stack_buf = (byte*) realloc(stack_buf, stack_buf_sz);
				stats.stack_buf_increases++;
			}
	
			eat_whitespace();
			//check for end object
//			if (json[index] == '}') {
			if ((int)json[index] == rt_curly) {
				index++;
//				*((unsigned int*)&stack_buf[loc_stack_i + obj_member_sz]) = num_keys;
				set_uint_a_indx(stack_buf, loc_stack_i + obj_member_sz, num_keys);
	
				if (!dont_sort_keys) {
					sort_keys(&stack_buf[loc_stack_i+obj_member_sz+obj_root_sz], 
						&stack_buf[loc_stack_i+obj_member_sz+obj_root_sz+(obj_member_sz*num_keys)],
						data, ((use_json) ? json : data));
				}
	
//				*((element_type*)&stack_buf[loc_stack_i+obj_member_sz+obj_root_sz+(obj_member_sz*num_keys)]) = extended;
				set_element_type(stack_buf, loc_stack_i+obj_member_sz+obj_root_sz+(obj_member_sz*num_keys), extended);
//				*((unsigned int*)&stack_buf[loc_stack_i+obj_member_sz+obj_root_sz+(obj_member_sz*num_keys)+obj_member_key_offx]) = 0;
				set_key_offx_value(stack_buf, loc_stack_i+obj_member_sz+obj_root_sz+(obj_member_sz*num_keys), 0);
				stack_i += obj_member_sz;

				if (increase_data_buffer(stack_i - loc_stack_i + obj_member_sz)) {
					data_sz = (unsigned long) data_sz * 1.2 + stack_i - loc_stack_i;
					data = (byte*) realloc(data, data_sz);
					stats.data_increases++;
				}

				memcpy(&data[data_i], &stack_buf[loc_stack_i], stack_i - loc_stack_i);

//				*(unsigned int*)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx] = data_i + obj_member_key_offx;
				set_key_offx_value(stack_buf, loc_stack_i - obj_member_sz, data_i + obj_member_key_offx);
//				*(element_type*)&stack_buf[loc_stack_i - obj_member_sz] = object_ptr;
				set_element_type(stack_buf, loc_stack_i - obj_member_sz, object_ptr);

//				std::cout << "set obj pointer to: " << *(unsigned int*)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx] <<
//					", for pointer type: " << *(element_type*)&stack_buf[loc_stack_i - obj_member_sz] << std::endl;

				to_return = data_i;
				data_i += (stack_i - loc_stack_i);
				stack_i = loc_stack_i - obj_member_sz;

				break;
//			} else if (json[index] == '"') {
			} else if ((int)json[index] == quote_int) {
				//check for key
				look_for_key = true;
				
				if (use_json)
//					*(unsigned int*)&stack_buf[stack_i + obj_member_key_offx] = index + 1;
					set_key_offx_value(stack_buf, stack_i, index + 1);
				else
//					*(unsigned int*)&stack_buf[stack_i + obj_member_key_offx] = data_i;
					set_key_offx_value(stack_buf, stack_i, data_i);
					
				num_keys ++;

				parse_key();
//				std::cout << "Key: " << *((char**)&stack_buf[stack_i + obj_member_key_offx]) << ", index =" << index << std::endl;

				look_for_key = false;
				local_look_for_key = false;
				value_start = index;

/************* TO DO, figure out a check for below *********************/				
//				if (key.length() < 1) {
//					std::string err = "parse error, blank key found at index: " + std::to_string(index);
//					set_error(err);
//					throw jsonP_exception{err.c_str()};
//				}
				
				eat_whitespace();

//				if (json[index] != ':') {
				if ((int)json[index] != colon) {
					std::string err = "parse error, expected ':' at index: " + std::to_string(index);
					set_error(err);
					throw jsonP_exception{err.c_str()};
				}
					
				index++;
				eat_whitespace();
				parse_value();
				stack_i += obj_member_sz;

				continue;
//			} else if (json[index] == ',') {
			} else if ((int)json[index] == comma_int) {
				//check for comma
				if (local_look_for_key) {
					std::string err = "parse error, found ',' while looking for a key at index: " + std::to_string(index);
					set_error(err);
					throw jsonP_exception{err.c_str()};
				}
				
				local_look_for_key = true;
				index++;
			} else {
				std::string err = "parse error, in parse_object, found: " + std::string{json[index]} + ", at: " + std::to_string(index);
				set_error(err);
				throw jsonP_exception{err.c_str()};
			}
			
		}
	} catch (jsonP_exception &e) {
		std::cout << e.what() << std::endl;
		throw e;
	}
	
	return to_return;
}


std::string jsonP_parser::get_error_snip(int chars_before, int chars_after)
{
	chars_before = (error_index - chars_before >= 0) ? error_index - chars_before : 0;
	chars_after = (error_index + chars_after < json_length) ? error_index + chars_after : json_length - 1;
	return (std::string(&json[chars_before], chars_after-chars_before));
}


void jsonP_parser::set_error(std::string error)
{
	error_index = index;
	error_string = error;
}



/*
 * For Debugging
 */
//void jsonP_parser::test_parse_object(unsigned int i)
//{
//	unsigned int k_cnt = get_key_count(data,i);
//	
//	std::cout << "key count: " << k_cnt << std::endl;
//	
//	i += obj_root_sz;
//	
//	for (int j=0; j<k_cnt; j++) {
//		element_type type = get_element_type(data, i);
//		std::cout << "	element type: " << type << ", at index: " << i;
//		unsigned int loc = get_key_location(data,i+obj_member_key_offx);
//		i += obj_member_sz;
//		
//		if (type == string) {
//			std::cout << ", key: " << json+loc; // << std::endl;
//			std::cout << ", value: " <<  json+loc+strlen(json+loc)+1 << std::endl;  //<-- pain deref to char* add offset from key length
//		} else if (type == numeric_long) {
//			std::cout << ", key: " << json+loc; // << std::endl;
//			std::cout << ", value: " <<  atol(json+loc+2+strlen(json+loc+1)) << std::endl;;
//		} else if (type == numeric_double) {
//			std::cout << ", key: " << json+loc; // << std::endl;
//			std::cout << ", value: " <<  atof(json+loc+2+strlen(json+loc+1)) << std::endl;
//		} else if (type == boolean) {
//			std::cout << ", key: " << json+loc; // << std::endl;
//			std::cout << ", value: " << ((json[loc+2+strlen(json+loc+1)] == '1') ? "true" : "false")  /*(bool*)(json+loc+2+strlen(json+loc+1))*/ << std::endl;
//		} else if (type == object_ptr) {
//			std::cout << ", key: " << json+get_key_location(data,loc) << std::endl;
//			//loc += 4;
//			test_parse_object(loc + 4);
//			std::cout << "test_parse returned\n\n";
//		} else if (type == object) {
//			std::cout << ", key: " << json+loc << std::endl;
//		} else if (type == array_ptr) {
//			std::cout << ", Array key: " << json+get_key_location(data,loc) << std::endl;
//			test_parse_array(loc+4);
//		} else if (type == null) {
//			std::cout << ", key: " << json+loc; // << std::endl;
//			std::cout << ", value: " <<  json+loc+strlen(json+loc)+1 << std::endl;
//		}
//	}
//}
//
//
//void jsonP_parser::test_parse_array(unsigned int i)
//{
//	unsigned int num_elements = get_key_count(data, i); 
//	std::cout << "test_parse_array called, starting index: " << i << 
//		", number of elements: " << num_elements << std::endl;
//	i += arry_root_sz;
//		
//	for (int k=0; k<num_elements; k++) {
//		element_type type = get_element_type(data, i);
//		unsigned int loc = get_key_location(data, i+arry_member_val_offx);
//		i+=arry_member_sz;
//		
//		if (type == string) {
//			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
//			std::cout << ", value: " << json+loc << std::endl;
////			std::cout << ", value: " <<  json+loc+strlen(json+loc)+1 << std::endl;  //<-- pain deref to char* add offset from key length
//		} else if (type == numeric_long) {
//			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
//			std::cout << ", val: " << atol(json+loc) << std::endl;
////			std::cout << ", value: " <<  atol(json+loc+2+strlen(json+loc+1)) << std::endl;;
//		} else if (type == numeric_double) {
//			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
//			std::cout << ", value: " << atof(json+loc) << std::endl;
////			std::cout << ", value: " <<  atof(json+loc+2+strlen(json+loc+1)) << std::endl;
//		} else if (type == boolean) {
//			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
//			std::cout << ", value: " << ((json[loc] == '1') ? "true" : "false") << std::endl;
////			std::cout << ", value: " <<  *(bool*)(json+loc+2+strlen(json+loc+1)) << std::endl;
//		} else if (type == object_ptr) {
//// dont try to get key for embedded object			std::cout << ", key: " << json+get_key_location(data,loc) << std::endl;
//			//loc += 4;
//			test_parse_object(loc + 4);
//			std::cout << "test_parse returned\n\n";
//		} else if (type == object) {
//			std::cout << ", key: " << json+loc << std::endl;
//		} else if (type == array_ptr) {
//// dont try to get key for embedded array			std::cout << ", key: " << json+get_key_location(data,loc) << std::endl;
////std::cout << json+4  << " - " << json+10 << std::endl;
//
//			test_parse_array(loc+4);
//		} else if (type == null) {
//			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
//			std::cout << ", value: " << json+loc << std::endl;
//		}
//	}
//	
//	std::cout << "test_parse_array returning\n";
//}
