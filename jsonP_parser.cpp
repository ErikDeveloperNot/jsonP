#include "jsonP_parser.h"

//#include <chrono>



jsonP_parser::jsonP_parser(std::string & json_) : json_str{json_}, look_for_key{false}, stack_i{0}, data_i{0}
{
// For now just make initial 1mb, will be configurable
stack_buf = (byte*) malloc(1024 * 10 * 10);
data = (byte*) malloc(json_str.length() / 2);  //************ THIS NEEDS TO CHANGE ******************
}


jsonP_parser::jsonP_parser(char * json_, unsigned long length) : 
json{json_}, 
json_length{length}, 
look_for_key{false},
stack_i{0}, 
data_i{0}
{
// For now just make initial 1mb, will be configurable
stack_buf = (byte*) malloc(1024 * 10 * 10);
data = (byte*) malloc(length * 3);   //************ THIS NEEDS TO CHANGE ******************
}


jsonP_parser::~jsonP_parser()
{
//	free(stack_buf);
//	free(data);
	
	std::cout << "End jsonP destructor" << std::endl;
}


jsonP_doc * jsonP_parser::parse()
{
	if (json != nullptr)
		return parse(json, json_length);
	else
		return parse((char*)json_str.c_str(), json_str.length());
}


jsonP_doc * jsonP_parser::parse(std::string & json_)
{
	return parse((char*)json_.c_str(), json_.length());
}


jsonP_doc * jsonP_parser::parse(char * json_, unsigned long length)
{
//	json = json_;
	json = json_;
//	json_length = json.length();
json_length = length;
	jsonP_doc *doc;
	index = 0;
	
	if (json_length < 2)
		throw jsonP_exception{"Error parsing into a json doc"};

//	eat_whitespace(index);
	eat_whitespace();
	
	if (json[index] == '{') {
		element_object *obj = nullptr;
stack_i += obj_member_sz;
//*((char**)&stack_buf[obj_member_key_offx]) = root_ptr;
*(unsigned int*)&stack_buf[obj_member_key_offx] = 0;
*(element_type*)&stack_buf[0] = object;
//std::cout << "TTTTTTT: " << *(unsigned int*)&stack_buf[1] << std::endl;
memcpy(&data[data_i], &root, 4);
data[data_i+4] = '\0';
data_i = 5;

unsigned int i = parse_object(obj);
		doc = new jsonP_doc{obj};

std::cout << "stack_i = " << stack_i << "\ndata_i = " << data_i << ", i = " << i << std::endl;
//std::cout << "root type: " << *((element_type*)&stack_buf[0]) << ", key: " << (unsigned int)data[*(unsigned int*)&stack_buf[1]] << std::endl;
//std::cout << "root2 type: " << *(element_type*)&data[i] << ", Key: " << *(unsigned int*)&data[i+1] << std::endl;

//test_parse_object(i+5);
		
	} else if (json[index] == '[') {
		element_array *arr = nullptr;
		parse_array(arr);
		doc = new jsonP_doc{arr};
	} else {
		set_error("Error parsing json text, does not appear to be an object or an array");
		throw jsonP_exception{"Error parsing json text, does not appear to be an object or an array"};
	}
	
	return doc;
}


void jsonP_parser::test_parse_object(unsigned int i)
{
	unsigned int k_cnt = get_key_count(data,i);
	
	std::cout << "key count: " << k_cnt << std::endl;
	
	i += obj_root_sz;
	
	for (int j=0; j<k_cnt; j++) {
		element_type type = get_element_type(data, i);
		std::cout << "	element type: " << type << ", at index: " << i;
		unsigned int loc = get_key_location(data,i+obj_member_key_offx);
		i += obj_member_sz;
		
		if (type == string) {
			std::cout << ", key: " << json+loc; // << std::endl;
			std::cout << ", value: " <<  json+loc+strlen(json+loc)+1 << std::endl;  //<-- pain deref to char* add offset from key length
		} else if (type == numeric_long) {
			std::cout << ", key: " << json+loc; // << std::endl;
			std::cout << ", value: " <<  atol(json+loc+2+strlen(json+loc+1)) << std::endl;;
		} else if (type == numeric_double) {
			std::cout << ", key: " << json+loc; // << std::endl;
			std::cout << ", value: " <<  atof(json+loc+2+strlen(json+loc+1)) << std::endl;
		} else if (type == boolean) {
			std::cout << ", key: " << json+loc; // << std::endl;
			std::cout << ", value: " << ((json[loc+2+strlen(json+loc+1)] == '1') ? "true" : "false")  /*(bool*)(json+loc+2+strlen(json+loc+1))*/ << std::endl;
		} else if (type == object_ptr) {
			std::cout << ", key: " << json+get_key_location(data,loc) << std::endl;
			//loc += 4;
			test_parse_object(loc + 4);
			std::cout << "test_parse returned\n\n";
		} else if (type == object) {
			std::cout << ", key: " << json+loc << std::endl;
		} else if (type == array_ptr) {
			std::cout << ", Array key: " << json+get_key_location(data,loc) << std::endl;
//std::cout << json+4  << " - " << json+10 << std::endl;

			test_parse_array(loc+4);
		} else if (type == null) {
			std::cout << ", key: " << json+loc; // << std::endl;
			std::cout << ", value: " <<  json+loc+strlen(json+loc)+1 << std::endl;
		}

		
//		b_ptr += obj_member_sz;
	}

}


void jsonP_parser::test_parse_array(unsigned int i)
{
	unsigned int num_elements = get_key_count(data, i); 
	std::cout << "test_parse_array called, starting index: " << i << 
		", number of elements: " << num_elements << std::endl;
	i += arry_root_sz;
		
	for (int k=0; k<num_elements; k++) {
		element_type type = get_element_type(data, i);
		unsigned int loc = get_key_location(data, i+arry_member_val_offx);
		i+=arry_member_sz;
		
		if (type == string) {
			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
			std::cout << ", value: " << json+loc << std::endl;
//			std::cout << ", value: " <<  json+loc+strlen(json+loc)+1 << std::endl;  //<-- pain deref to char* add offset from key length
		} else if (type == numeric_long) {
			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
			std::cout << ", val: " << atol(json+loc) << std::endl;
//			std::cout << ", value: " <<  atol(json+loc+2+strlen(json+loc+1)) << std::endl;;
		} else if (type == numeric_double) {
			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
			std::cout << ", value: " << atof(json+loc) << std::endl;
//			std::cout << ", value: " <<  atof(json+loc+2+strlen(json+loc+1)) << std::endl;
		} else if (type == boolean) {
			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
			std::cout << ", value: " << ((json[loc] == '1') ? "true" : "false") << std::endl;
//			std::cout << ", value: " <<  *(bool*)(json+loc+2+strlen(json+loc+1)) << std::endl;
		} else if (type == object_ptr) {
// dont try to get key for embedded object			std::cout << ", key: " << json+get_key_location(data,loc) << std::endl;
			//loc += 4;
			test_parse_object(loc + 4);
			std::cout << "test_parse returned\n\n";
		} else if (type == object) {
			std::cout << ", key: " << json+loc << std::endl;
		} else if (type == array_ptr) {
// dont try to get key for embedded array			std::cout << ", key: " << json+get_key_location(data,loc) << std::endl;
//std::cout << json+4  << " - " << json+10 << std::endl;

			test_parse_array(loc+4);
		} else if (type == null) {
			std::cout << "array val type: " << type << ", indx: " << i-arry_root_sz << ", -" << (int)data[i-arry_root_sz];
			std::cout << ", value: " << json+loc << std::endl;
		}
	}
	
	std::cout << "test_parse_array returning\n";
}

//void jsonP_parser::eat_whitespace(int idx)
//{
////	while ((json[index] == ' ' || json[index] == '\t' || json[index] == '\n' || json[index] == '\r') && json_length > index) 
////		index++;
////	while (((int)json[index] == space || (int)json[index] == tab || (int)json[index] == new_line || 
////			(int)json[index] == car_return) && json_length > index) 
////		index++;
//		
//	while ((json[index] == space || json[index] == tab || json[index] == new_line || 
//			json[index] == car_return))// && json_length > index) 
//		index++;
//}


void jsonP_parser::parse_key(std::string & key)
{
	index++;
unsigned int start = index;	

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
					key += json[++index];
					index++;
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
					key += json[index++];
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
json[index] = '\0';
			break;
		} else {
			key += json[index++];
		}
	}
	

	index++;

if (!look_for_key) {
//std::cout << "start: " << start << ", index: " << index << ", value_start: " << value_start << std::endl;
	while (start < index) {
		json[value_start++] = json[start++];
//std::cout << 	json[value_start-1] << " : " << json[start-1] << std::endl; 
	}
//std::cout << json[value_start] << std::endl;
}


//	std::cout << "parse_key returning: " << key << ", next index: " << index << ", val: " << json[index] << std::endl;
}


// should only allow lowercase but accept either
void jsonP_parser::parse_bool(bool& value)
{
//	if (json[index] == 't' || json[index] == 'T') {
//	if ((int)json[index] == ltr_t || (int)json[index] == ltr_T) {
//		std::string t{"rue"};
//		
//		for (char &c : t) {
//			index++;
//			
//			if (c != json[index] && (c-32) != json[index]) {
//				std::string err = "Invalid bool value found at index: " + std::to_string(index);
//				set_error(err);
//				throw jsonP_exception{err.c_str()};
//			}
//		}
//		
//		value = true;
//	} else {
//		std::string t{"alse"};
//		
//		for (char &c : t) {
//			index++;
//			
//			if (c != json[index] && (c-32) != json[index]) {
//				std::string err = "Invalid bool value found at index: " + std::to_string(index);
//				set_error(err);
//				throw jsonP_exception{err.c_str()};
//			}
//		}
//		
//		value = false;
//	}
	
	if (json[index] == 't') {
		if (json[++index] == 'r' && json[++index] == 'u' && json[++index] == 'e') {
			value = true;
			
//((obj_member*)&stack_buf[stack_i])->value = &boolean_true;

//memcpy(&stack_buf[stack_i + obj_member_val_offx], &boolean_true, sizeof(byte*));
//memcpy(&json[value_start], &boolean_true, sizeof(byte*));
//json[value_start] = boolean_true;			
json[value_start] ='1';

		} else {
			std::string err = "Invalid bool value found at index: " + std::to_string(index);
			set_error(err);
			throw jsonP_exception{err.c_str()};
		}
	} else {
		if (json[++index] == 'a' && json[++index] == 'l' && json[++index] == 's' && json[++index] == 'e') {
			value = false;
			
//((obj_member*)&stack_buf[stack_i])->value = &boolean_false;

//memcpy(&stack_buf[stack_i + obj_member_val_offx], &boolean_false, sizeof(byte*));
//memcpy(&json[value_start], &boolean_false, sizeof(byte*));
//json[value_start] = boolean_false;
json[value_start] ='0';

		} else {
			std::string err = "Invalid bool value found at index: " + std::to_string(index);
			set_error(err);
			throw jsonP_exception{err.c_str()};
		}
	}

json[value_start+1] = '\0';
	
	index++;
}


//element_type jsonP_parser::parse_numeric(std::string & number)
element_type jsonP_parser::parse_numeric()
{
	bool is_long{true};
//	char c = json[index];
	int c = (int)json[index];
	bool sign {true};
	bool exp{false};
	bool exp_sign{false};
	bool dec{true};

int s = index-1;

//	while (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ',' && c != ']' && c != '}') {
	while (c != space && c != tab && c != new_line && c != car_return && c != comma_int && c != rt_brac && c != rt_curly) {
//		number += json[index];
	
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
			std::string err = "parse error, trying to numeric value at index: " + std::to_string(index);
			set_error(err);
			throw jsonP_exception{err.c_str()};
		}
		
		sign = false;
//		c = json[++index];
		c = (int)json[++index];
	}
	
while (s < index-1) {
//	json[s] = json[s+1];
	json[value_start++] = json[s+1];
	s++;
}
//json[s] = '\0';
json[value_start] = '\0';
	
	if (is_long)
		return numeric_long;
	else
		return numeric_double;

}


void jsonP_parser::parse_value(element *& value)
{
	//figure out what type of data type this is
//	if (json[index] == '{') {
	if ((int)json[index] == lft_curly) {
		//object
*((element_type*)&stack_buf[stack_i]) = object;
stack_i += obj_member_sz;
		element_object *obj;
		parse_object(obj);
		value = obj;
//	} else if (json[index] == '[') {
	} else if ((int)json[index] == lft_brac) {
		//array
*((element_type*)&stack_buf[stack_i]) = array_ptr;
stack_i += arry_member_sz;
		element_array *arr;
		parse_array(arr);
		value = arr;
//	} else if (json[index] == '"') {
	} else if ((int)json[index] == quote_int) {
		//string
		std::string str_val;
		

*((element_type*)&stack_buf[stack_i]) = string;
//*((char**)&stack_buf[stack_i + obj_member_val_offx]) = (char*)(&json[index + 1]);


//(char*)(stack_buf[stack_i + obj_member_val_offx]) = (char*)(&json[index + 1]);
//char *cptr = (char*)(stack_buf[stack_i + obj_member_val_offx]);
//cptr = (char*)(&json[index + 1]);
//memcpy(&stack_buf[stack_i + obj_member_val_offx], (&json + index + 1), sizeof(char*));

//std::cout <<  "josn address: " << (&json + index + 1) << ", buf address: " << (&stack_buf + stack_i + obj_member_val_offx);
//char *t = (char*)(&json[index + 1]);
//char *t = *((char**)&stack_buf[stack_i + obj_member_val_offx]);
		
		parse_key(str_val);


//std::cout << ", k1: " << str_val << ", k2: " << t << std::endl;
		
//		value = new element_string{str_val};
		value = create_string_element(str_val);
//	} else if (json[index] == 't' || json[index] == 'T' || json[index] == 'f' || json[index] == 'F') {
	} else if ((int)json[index] == ltr_t || (int)json[index] == ltr_T || (int)json[index] == ltr_f || 
			(int)json[index] == ltr_F) {
		//bool
		bool bool_val;
		parse_bool(bool_val);
//		value = new element_boolean{bool_val};
		value = create_boolean_element(bool_val);
		
//((obj_member*)&stack_buf[stack_i])->type = boolean;
*((element_type*)&stack_buf[stack_i]) = boolean;

		
//	} else if ((json[index] >= '0' && json[index] <= '9') || json[index] == '-' || json[index] == '+') {
	} else if (((int)json[index] >= zero && (int)json[index] <= nine) || (int)json[index] == minus || (int)json[index] == plus) {
//		std::string number;
int s = index;
		
//		switch (parse_numeric(number))//start, end)) 
		switch (parse_numeric())
		{
			case numeric_int :
//std::cout << "parse numeric returned int\n";
//				value = new element_numeric{atoi(json.substr(start, end).c_str())};
//				value = new element_numeric{atoi(number.c_str())};
//				value = create_int_element(number);
//value = new element_numeric{number, numeric_int};
{
//char *c = (char*) malloc(index-s+1);
//int i=0;
//while (s < index)
//	c[i++] = json[s++];
//c[i] = '\0';
//value = new element_numeric{c, numeric_int};
value = new element_numeric{&json[s-1], numeric_int};


//((obj_member*)&stack_buf[stack_i])->type = numeric_int;
//((obj_member*)&stack_buf[stack_i])->value = &json[s-1];

*((element_type*)&stack_buf[stack_i]) = numeric_int;
//*((char**)&stack_buf[stack_i + obj_member_val_offx]) = (char*)(&json[s-1]);

				break;
}
			case numeric_long :
//std::cout << "parse numeric returned long\n";
//				value = new element_numeric{atol(json.substr(start, end).c_str())};
//				value = new element_numeric{atol(number.c_str())};
//				value = create_long_element(number);
//value = new element_numeric{number, numeric_long};
{
//char *c = (char*) malloc(index-s+1);
//int i=0;
//while (s < index)
//	c[i++] = json[s++];
//c[i] = '\0';
//value = new element_numeric{c, numeric_long};
value = new element_numeric{&json[s-1], numeric_long};

//((obj_member*)&stack_buf[stack_i])->type = numeric_long;
//((obj_member*)&stack_buf[stack_i])->value = &json[s-1];

*((element_type*)&stack_buf[stack_i]) = numeric_long;
//*((char**)&stack_buf[stack_i + obj_member_val_offx]) = (char*)(&json[s-1]);

				break;
}
			case numeric_double :

//std::cout << "parse numeric returned double\n";
//				value = new element_numeric{atof(json.substr(start, end).c_str())};
//				value = new element_numeric{atof(number.c_str())};
//				value = create_float_element(number);
//value = new element_numeric{number, numeric_double};
{
//char *c = (char*) malloc(index-s+1);
//int i=0;
//while (s < index)
//	c[i++] = json[s++];
//c[i] = '\0';
//value = new element_numeric{c, numeric_double};
value = new element_numeric{&json[s-1], numeric_double};

//((obj_member*)&stack_buf[stack_i])->type = numeric_double;
//((obj_member*)&stack_buf[stack_i])->value = &json[s-1];

*((element_type*)&stack_buf[stack_i]) = numeric_double;
//*((char**)&stack_buf[stack_i + obj_member_val_offx]) = (char*)(&json[s-1]);


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
//			value = new element_null{};
			value = create_null_element();
		
//((obj_member*)&stack_buf[stack_i])->type = null;
*((element_type*)&stack_buf[stack_i]) = null;
json[value_start] = 'n';
json[++value_start] = 'u';
json[++value_start] = 'l';
json[++value_start] = 'l';
json[++value_start] = '\0';		
		
			index++;
		} else {
			std::string err = "parse error, trying to get null value at index: " + std::to_string(index);
			set_error(err);
			throw jsonP_exception{err.c_str()};
		}
	} else {
std::cout << "ERRROR: " << json[index] << "-" << (int)json[index] << std::endl;
		std::string err = "parse error, trying to get value at index: " + std::to_string(index);
		set_error(err);
		throw jsonP_exception{err.c_str()};
	}
}


unsigned int jsonP_parser::parse_array(element_array *& arr)
{
//std::cout << "parse_array start, index: " << stack_i << std::endl;

// create local index for this obj record and advace global index
unsigned int loc_stack_i = stack_i; // - obj_member_sz;
unsigned int num_elements = 0;
//byte *return_ptr;
unsigned int to_return;	//most likely will need
//std::cout << "Addess: " << &*((char**)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx]) << std::endl;
*((element_type*)&stack_buf[loc_stack_i]) = array;
*(unsigned int*)&stack_buf[loc_stack_i + arry_member_val_offx] = *(unsigned int*)&stack_buf[loc_stack_i - arry_member_sz + arry_member_val_offx];

stack_i += (arry_member_sz + arry_root_sz);

	//figure out array type based off of first element
//	eat_whitespace(++index);

value_start = index;
*(unsigned int*)&stack_buf[stack_i + arry_member_val_offx] = index;

	++index;
	eat_whitespace();
	
	//make sure array is empty
//	if (json[index] == ']') {
	if ((int)json[index] == rt_brac) {
		arr = create_element_array(string);
		
*((unsigned int*)&stack_buf[loc_stack_i + arry_member_sz]) = num_elements;
//std::cout << "Arry number keys:" << *((unsigned int*)&stack_buf[loc_stack_i + obj_member_sz]) << ", loc_stack_i: " << loc_stack_i <<
//	", for key index: " << *((int*)&stack_buf[loc_stack_i + obj_member_sz]) << std::endl; 

memcpy(&data[data_i], &stack_buf[loc_stack_i], stack_i - loc_stack_i);



*(unsigned int*)&stack_buf[loc_stack_i - arry_member_sz + arry_member_val_offx] = data_i + arry_member_val_offx;
*(element_type*)&stack_buf[loc_stack_i - obj_member_sz] = array_ptr;

to_return = data_i;
data_i += (stack_i - loc_stack_i);
stack_i = loc_stack_i - arry_member_sz;
		
		index++;
		return to_return;
	}
	
	element *val = nullptr;


	parse_value(val);
	
	if (val) {
		switch (val->get_type())
		{
			case string :
	//			arr = new element_array{string};
				arr = create_element_array(string);
				break;
			case boolean :
	//			arr = new element_array{boolean};
				arr = create_element_array(boolean);
				break;	
	//		case numeric :
	//			arr = new element_array{numeric};
	//			break;
			case numeric_int :
	//			arr = new element_array{numeric_int};
				arr = create_element_array(numeric_int);
				break;
			case numeric_long :
	//			arr = new element_array{numeric_long};
				arr = create_element_array(numeric_long);
				break;
			case numeric_double :
	//			arr = new element_array{numeric_double};
				arr = create_element_array(numeric_double);
				break;
			case object :
	//			arr = new element_array{object};
				arr = create_element_array(object);
				break;
			case array :
	//			arr = new element_array{array};
				arr = create_element_array(array);
			case null :
	//			arr = new element_array{null};
				arr = create_element_array(null);
		}
		
	//	arr->add_element(val);
		add_array_element(arr, val);

num_elements++;
stack_i += arry_member_sz;

	} else {
//		arr = new element_array{string};	//dont care about schemas so just make it string since it doesn't matter
		arr = create_element_array(string);
	}
	
	bool look_for_value{false};
	//keep adding elements, if different types of elements r present then the first type, they will fail
	while (true) {
//		eat_whitespace(index);
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
*(unsigned int*)&stack_buf[stack_i + arry_member_val_offx] = index;

				index++;
				look_for_value = true;
			}
		} else {
			look_for_value = false;


			parse_value(val);

			if (val) {

num_elements++;
stack_i += arry_member_sz;

				add_array_element(arr, val);
			}
		}
	}
	
*((unsigned int*)&stack_buf[loc_stack_i + arry_member_sz]) = num_elements;
//std::cout << "Arry number keys:" << *((unsigned int*)&stack_buf[loc_stack_i + obj_member_sz]) << ", loc_stack_i: " << loc_stack_i <<
//	", for key index: " << *((int*)&stack_buf[loc_stack_i + obj_member_sz]) << std::endl; 
	
//std::cout << "\n\nChecking json 1-";
//for (int k=0;k<38;k++)
//	std::cout << (int)json[k] << "-";
//std::cout << "\n\n";


memcpy(&data[data_i], &stack_buf[loc_stack_i], stack_i - loc_stack_i);
//for (size_t i=0; i<stack_i-loc_stack_i; i++)
//	data[data_i+i] = stack_buf[loc_stack_i+i];

//std::cout << "\n\nChecking json 2-";
//for (int k=0;k<38;k++)
//	std::cout << (int)json[k] << "-";
//std::cout << "\n\n";


*(unsigned int*)&stack_buf[loc_stack_i - arry_member_sz + arry_member_val_offx] = data_i + arry_member_val_offx;
*(element_type*)&stack_buf[loc_stack_i - obj_member_sz] = array_ptr;

to_return = data_i;
data_i += (stack_i - loc_stack_i);
stack_i = loc_stack_i - arry_member_sz;
return to_return;
	
}


unsigned int jsonP_parser::parse_object(element_object *& obj)
{
//std::cout << "parse_object start, index: " << stack_i << std::endl;

// create local index for this obj record and advace global index
unsigned int loc_stack_i = stack_i; // - obj_member_sz;
unsigned int num_keys = 0;
//byte *return_ptr;
unsigned int to_return;
//std::cout << "Addess: " << &*((char**)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx]) << std::endl;
*((element_type*)&stack_buf[loc_stack_i]) = object;
*(unsigned int*)&stack_buf[loc_stack_i + obj_member_key_offx] = *(unsigned int*)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx];

//std::cout << "key index: " << *(unsigned int*)&stack_buf[loc_stack_i + obj_member_key_offx] << ", parse object key: " << &data[*(unsigned int*)&stack_buf[loc_stack_i + obj_member_key_offx]] << std::endl;


//stack_i += obj_root_sz;
stack_i += (obj_root_sz + obj_member_sz);

	
//	obj = new element_object{};
//if (obj == nullptr)
	obj = create_element_object();
	bool keep_going{true};
	bool local_look_for_key{true};
//	look_for_key = true;
	
//	if (json[index] != '{') {
	if ((int)json[index] != lft_curly) {
		delete obj;
		std::string err = "parse error, expected '{' at index: " + std::to_string(index);
		set_error(err);
		throw jsonP_exception{err.c_str()};
	}
	
	++index;
	
	try {
		while (keep_going) {
			eat_whitespace();
//			std::cout << "CHECKING : " << json[index] << std::endl;

			//check for end object
//			if (json[index] == '}') {
			if ((int)json[index] == rt_curly) {
				index++;

*((unsigned int*)&stack_buf[loc_stack_i + obj_member_sz]) = num_keys;
//std::cout << "num keyss --1:" << *((unsigned int*)&stack_buf[loc_stack_i + obj_member_sz]) << ", loc_stack_i: " << loc_stack_i <<
//	", for key index: " << *((int*)&stack_buf[loc_stack_i + obj_member_sz]) << std::endl; 

//std::cout << "\n\nChecking json 2-";
//for (int k=0;k<28;k++)
//	std::cout << (int)json[k] << "-";
//std::cout << "\n\n";


memcpy(&data[data_i], &stack_buf[loc_stack_i], stack_i - loc_stack_i);

//std::cout << "\n\nChecking json 3-";
//for (int k=0;k<28;k++)
//	std::cout << (int)json[k] << "-";
//std::cout << "\n\n";

//std::cout << "copied to data[" << data_i << "], from stack_buf[" << loc_stack_i << "], " << (stack_i-loc_stack_i) << " bytes\n";
//
//std::cout << "type data[" << data_i << "] = " << get_element_type(data, data_i) <<
//		", key location data[" << data_i+1 << "] = " << get_key_location(data, data_i+1) <<
//		", number of keys data[" << data_i+5 << "] = " << get_key_count(data, data_i+5) << 
//		", 1st key type data[" << data_i+9 << "] = " << *(element_type*)&data[data_i+9] <<
//		", 1st key location data[" << data_i+10 << "] = " << *(unsigned int*)&data[data_i+10] <<
//		std::endl;
		


*(unsigned int*)&stack_buf[loc_stack_i - obj_member_sz + obj_member_key_offx] = data_i + obj_member_key_offx;
*(element_type*)&stack_buf[loc_stack_i - obj_member_sz] = object_ptr;

//return_ptr = &data[data_i];
to_return = data_i;


data_i += (stack_i - loc_stack_i);
stack_i = loc_stack_i - obj_member_sz;

				break;
//			} else if (json[index] == '"') {
			} else if ((int)json[index] == quote_int) {
				//check for key
				std::string key;
				look_for_key = true;
				
*(unsigned int*)&stack_buf[stack_i + obj_member_key_offx] = index + 1;
num_keys ++;

				parse_key(key);
//std::cout << "Key: " << *((char**)&stack_buf[stack_i + obj_member_key_offx]) << ", index =" << index << std::endl;

				look_for_key = false;
				local_look_for_key = false;
value_start = index;
				
				if (key.length() < 1) {
					std::string err = "parse error, blank key found at index: " + std::to_string(index);
					set_error(err);
					throw jsonP_exception{err.c_str()};
				}
				
				element *value = nullptr;
//				eat_whitespace(index);
				eat_whitespace();

//				if (json[index] != ':') {
				if ((int)json[index] != colon) {
					std::string err = "parse error, expected ':' at index: " + std::to_string(index);
					set_error(err);
					throw jsonP_exception{err.c_str()};
				}
					
//				eat_whitespace(++index);
				index++;
				eat_whitespace();
//std::cout << "stack_i b4 parse_value: " << stack_i << std::endl;
				parse_value(value);
//std::cout << "stack_i after parse_value: " << stack_i << std::endl;
//				obj->add_element(key, value);
				add_object_element(obj, value, key);

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
		delete obj;
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
