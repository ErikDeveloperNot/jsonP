#include "jsonP_parser.h"



jsonP_parser::jsonP_parser(std::string json_) : json{json_}
{
}


jsonP_doc * jsonP_parser::parse()
{
	return parse(json);
}


jsonP_doc * jsonP_parser::parse(std::string & json_)
{
	json = json_;
	jsonP_doc *doc;
	index = 0;
	
	if (json.length() < 2)
		throw jsonP_exception{"Error parsing into a json doc"};
		
	eat_whitespace(index);
	parse_object(doc);
	
	return doc;
}


void jsonP_parser::eat_whitespace(int idx)
{
	while (json[index] == ' ' || json[index] == '\t' || json[index] == '\n' || json[index] == '\r')
		index++;
}


void jsonP_parser::parse_key(std::string & key)
{
	//ignore escapes for now
	size_t start = ++index;
	
	while (json[index] != '"')
		index++;
		
//	if ((index - start) < 1) {
//		std::string err = "parse Warning.... blank key found at index: " + std::to_string(index);
//		throw jsonP_exception{err.c_str()};
//	}
	
	key = json.substr(start, index-start);
	index++;
//std::cout << "parse_key returning: " << key << std::endl;
}


void jsonP_parser::parse_bool(bool& value)
{
	if (json[index] == 't' || json[index] == 'T') {
		std::string t{"rue"};
		
		for (char &c : t) {
			index++;
			
			if (c != json[index] && (c-32) != json[index]) {
				std::string err = "Invalid bool value found at index: " + std::to_string(index);
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
				throw jsonP_exception{err.c_str()};
			}
		}
		
		value = false;
	}
	
	index++;
}


void jsonP_parser::parse_numeric(long & value)
{
	bool negative{false};

	if (json[index] == '-') {
		negative = true;
		index++;
	}
	
	int start = index;
	
	while ((int)json[index] >= 48 && (int)json[index] <= 57)
		index++;
		
	if ((index - start) > 0) {
		value = std::atol(json.substr(start, index-start).c_str());

		if (negative)
			value *= -1;
	} else {
		throw jsonP_exception{"parse error, numeric value of 0 length"};
	}
}


void jsonP_parser::parse_value(element *& value)
{
	//figure out what type of data type this is
	if (json[index] == '{') {
		//object
		element_object *obj;
		parse_object(obj);
		value = obj;
	} else if (json[index] == '[') {
		//array
		element_array *arr;
		parse_array(arr);
		value = arr;
	} else if (json[index] == '"') {
		//string
		std::string str_val;
		parse_key(str_val);
		value = new element_string{str_val};
	} else if (json[index] == 't' || json[index] == 'T' || json[index] == 'f' || json[index] == 'F') {
		//bool
		bool bool_val;
		parse_bool(bool_val);
		value = new element_boolean{bool_val};
	} else if (((int)json[index] >= 48 && (int)json[index] <= 57) || json[index] == '-') {
		//number - no floating or scientific for now
		long long_val;
		parse_numeric(long_val);
		value = new element_numeric{long_val};
	} else {
		std::string err = "parse error, trying to get value at index: " + std::to_string(index);
		throw jsonP_exception{err.c_str()};
	}
}


void jsonP_parser::parse_array(element_array *& arr)
{
	//figure out array type based off of first element
	eat_whitespace(++index);
	
	//make sure array is empty
	if (json[index] == ']') {
		arr = new element_array{string};	//dont care about schemas so just make it string since it doesn't matter
		index++;
		return;
	}
	
	element *val;
	parse_value(val);
	
	switch (val->get_type())
	{
		case string :
			arr = new element_array{string};
			break;
		case boolean :
			arr = new element_array{boolean};
			break;	
		case numeric :
			arr = new element_array{numeric};
			break;
		case object :
			arr = new element_array{object};
			break;
		case array :
			arr = new element_array{array};
	}
	
	arr->add_element(val);
	
	bool look_for_value{false};
	//keep adding elements, if different types of elements r present then the first type, they will fail
	while (true) {
		eat_whitespace(index);
		
		if (json[index] == ']') {
			// end of array
			if (look_for_value) { 
				std::string err = "parse error, in array, looking for another value at index: " + std::to_string(index);
				throw jsonP_exception{err.c_str()};
			} else {
				index++;
				break;
			}
		} else if (json[index] == ',') {
			// more elements
			if (look_for_value) { 
				std::string err = "parse error, in array, looking for another value at index: " + std::to_string(index);
				throw jsonP_exception{err.c_str()};
			} else {
				index++;
				look_for_value = true;
			}
		} else {
			look_for_value = false;
			parse_value(val);
			
			if (!arr->add_element(val)) {
				std::cout << "Unable to add element type: " << val->get_type() << ", to array of type: " <<
								arr->get_type() << std::endl;
				delete val;
			}
		}
	}
	
}


void jsonP_parser::parse_object(element_object *& obj)
{
	obj = new element_object{};
	bool keep_going{true};
	bool look_for_key{true};
	
	if (json[index] != '{') {
		delete obj;
		std::string err = "parse error, expected '{' at index: " + std::to_string(index);
		throw jsonP_exception{err.c_str()};
	}
	
	++index;
	
	try {
		while (keep_going) {
			eat_whitespace(index);
			
//std::cout << "CHECKING : " << json[index] << std::endl;

			//check for end object
			if (json[index] == '}') {
				index++;
				break;
			} else if (json[index] == '"') {
				//check for key
				std::string key;
				look_for_key = false;
				
				parse_key(key);
				
				if (key.length() < 1) {
					std::string err = "parse error, blank key found at index: " + std::to_string(index);
					throw jsonP_exception{err.c_str()};
				}
				
				element *value;
				eat_whitespace(index);

				if (json[index] != ':') {
					std::string err = "parse error, expected ':' at index: " + std::to_string(index);
					throw jsonP_exception{err.c_str()};
				}
					
				eat_whitespace(++index);
				
				parse_value(value);
				obj->add_element(key, value);
				continue;
			} else if (json[index] == ',') {
				//check for comma
				if (look_for_key) {
					std::string err = "parse error, found ',' while looking for a key at index: " + std::to_string(index);
					throw jsonP_exception{err.c_str()};
				}
				
				look_for_key = true;
				index++;
			} else {
				std::string err = "parse error, in parse_object, found: " + std::string{json[index]} + ", at: " + std::to_string(index);
				throw jsonP_exception{err.c_str()};
			}
			
		}
	} catch (jsonP_exception &e) {
		delete obj;
		std::cout << e.what() << std::endl;
		throw e;
	}
}

