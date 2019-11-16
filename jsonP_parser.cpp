#include "jsonP_parser.h"

//#include <chrono>

jsonP_parser::jsonP_parser(std::string & json_) : json_str{json_}, look_for_key{false}
{
}


jsonP_parser::jsonP_parser(char * json_, unsigned long length) : json{json_}, json_length{length}, look_for_key{false}
{
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

	eat_whitespace(index);

	if (json[index] == '{') {
		element_object *obj = nullptr;
		parse_object(obj);
		doc = new jsonP_doc{obj};
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


void jsonP_parser::eat_whitespace(int idx)
{
//	while ((json[index] == ' ' || json[index] == '\t' || json[index] == '\n' || json[index] == '\r') && json_length > index) 
//		index++;
	while (((int)json[index] == space || (int)json[index] == tab || (int)json[index] == new_line || 
			(int)json[index] == car_return) && json_length > index) 
		index++;
}


void jsonP_parser::parse_key(std::string & key)
{
	index++;

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
			break;
		} else {
			key += json[index++];
		}
	}

	index++;
//	std::cout << "parse_key returning: " << key << ", next index: " << index << ", val: " << json[index] << std::endl;
}


// should only allow lowercase but accept either
void jsonP_parser::parse_bool(bool& value)
{
//	if (json[index] == 't' || json[index] == 'T') {
	if ((int)json[index] == ltr_t || (int)json[index] == ltr_T) {
		std::string t{"rue"};
		
		for (char &c : t) {
			index++;
			
			if (c != json[index] && (c-32) != json[index]) {
				std::string err = "Invalid bool value found at index: " + std::to_string(index);
				set_error(err);
				throw jsonP_exception{err.c_str()};
			}
		}
		
		value = true;
	} else {
		std::string t{"alse"};
		
		for (char &c : t) {
			index++;
			
			if (c != json[index] && (c-32) != json[index]) {
				std::string err = "Invalid bool value found at index: " + std::to_string(index);
				set_error(err);
				throw jsonP_exception{err.c_str()};
			}
		}
		
		value = false;
	}
	
	index++;
}


//void jsonP_parser::parse_numeric(long & value)
//{
//	bool negative{false};
//
//	if (json[index] == '-') {
//		negative = true;
//		index++;
//	}
//	
//	int start = index;
//	
//	while ((int)json[index] >= 48 && (int)json[index] <= 57)
//		index++;
//		
//	if ((index - start) > 0) {
//		value = std::atol(json.substr(start, index-start).c_str());
//
//		if (negative)
//			value *= -1;
//	} else {
//		set_error("parse error, numeric value of 0 length");
//		throw jsonP_exception{"parse error, numeric value of 0 length"};
////	}
//}


/*
 * uses string::substr() - method should not be used
 */
//element_type jsonP_parser::parse_numeric(int & start, int & end)
//{
//	start = index;
//	bool is_long{true};
//	char c = json[index];
//	
//	bool sign {true};
//	bool exp{false};
//	bool exp_sign{false};
//	bool dec{true};
//
//	while (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ',' && c != ']' && c != '}') {
////		if (c >= '1' && c <= '9') {
//		if (c >= '0' && c <= '9') {
////			zer0 = true;
//			exp_sign = false;
//		/*} else if (c == '0' && zer0) {
//			exp_sign = false;*/
//		} else if ((c == 'e' || c == 'E') /*&& zer0*/ && !exp) {
//			exp = true;
//			exp_sign = true;
//			dec = false;
//			is_long = false;
//		} else if (c == '.' && dec) {
//			exp = false;
//			is_long = false;
//		} else if ((c == '-' || c == '+') && (sign || exp_sign)) {
//			exp_sign = false;
//		} else {
//			std::string err = "parse error, trying to numeric value at index: " + std::to_string(index);
//			set_error(err);
//			throw jsonP_exception{err.c_str()};
//		}
//		
//		sign = false;
//		c = json[++index];
//	}
//	
//	end = index - 1;
//
////	if (is_int) 
////		return numeric_int;
//	if (is_long)
//		return numeric_long;
//	else
//		return numeric_double;
//}


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
	json[s] = json[s+1];
	s++;
}
json[s] = '\0';
	
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
		element_object *obj;
		parse_object(obj);
		value = obj;
//	} else if (json[index] == '[') {
	} else if ((int)json[index] == lft_brac) {
		//array
		element_array *arr;
		parse_array(arr);
		value = arr;
//	} else if (json[index] == '"') {
	} else if ((int)json[index] == quote_int) {
		//string
		std::string str_val;
		parse_key(str_val);
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


void jsonP_parser::parse_array(element_array *& arr)
{
	//figure out array type based off of first element
	eat_whitespace(++index);
	
	//make sure array is empty
//	if (json[index] == ']') {
	if ((int)json[index] == rt_brac) {
//		arr = new element_array{string};	//dont care about schemas so just make it string since it doesn't matter
		arr = create_element_array(string);
		index++;
		return;
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
	} else {
//		arr = new element_array{string};	//dont care about schemas so just make it string since it doesn't matter
		arr = create_element_array(string);
	}
	
	bool look_for_value{false};
	//keep adding elements, if different types of elements r present then the first type, they will fail
	while (true) {
		eat_whitespace(index);
		
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
				index++;
				look_for_value = true;
			}
		} else {
			look_for_value = false;
			parse_value(val);

			if (val) {
				add_array_element(arr, val);
			}
		}
	}
	
}


void jsonP_parser::parse_object(element_object *& obj)
{
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
			eat_whitespace(index);
			
//			std::cout << "CHECKING : " << json[index] << std::endl;

			//check for end object
//			if (json[index] == '}') {
			if ((int)json[index] == rt_curly) {
				index++;
				break;
//			} else if (json[index] == '"') {
			} else if ((int)json[index] == quote_int) {
				//check for key
				std::string key;
				look_for_key = true;
				parse_key(key);
				look_for_key = false;
				local_look_for_key = false;
				
				if (key.length() < 1) {
					std::string err = "parse error, blank key found at index: " + std::to_string(index);
					set_error(err);
					throw jsonP_exception{err.c_str()};
				}
				
				element *value = nullptr;
				eat_whitespace(index);

//				if (json[index] != ':') {
				if ((int)json[index] != colon) {
					std::string err = "parse error, expected ':' at index: " + std::to_string(index);
					set_error(err);
					throw jsonP_exception{err.c_str()};
				}
					
				eat_whitespace(++index);
				
				parse_value(value);
//				obj->add_element(key, value);
				add_object_element(obj, value, key);
				
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
}


std::string jsonP_parser::get_error_snip(int chars_before, int chars_after)
{
	chars_before = (error_index - chars_before >= 0) ? error_index - chars_before : 0;
	chars_after = (error_index + chars_after < json_length) ? error_index + chars_after : json_length - 1;
std::cout << "error index: " << error_index << "json_length: " << json_length << ", b4: " << chars_after << std::endl;
while (chars_before < chars_after)
	std::cout << json[chars_before++] << std::endl;

return "gg";
	
//	return (std::string(&json[chars_before], chars_after-chars_before));
}


void jsonP_parser::set_error(std::string error)
{
	error_index = index;
	error_string = error;
}
