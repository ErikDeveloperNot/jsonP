#include "jsonP_buffer_parser.h"



jsonP_buffer_parser::jsonP_buffer_parser(std::string file_name, int buf_sz) : buffer_size{buf_sz}
{
	if (buffer_size < min_buffer_size) {
		std::cerr << "jsonP_buffer_parser initiated with buffer size too small: " << buffer_size <<
						", defaulting to: " << min_buffer_size << std::endl;
		buffer_size = min_buffer_size;
	}
	
	reader = new file_chunk_impl{file_name, buffer_size};
	buffer = (char*) malloc(sizeof(char) * (buffer_size + 1));
}


jsonP_buffer_parser::jsonP_buffer_parser(IChunk_reader *reader, int buf_sz) : buffer_size{buf_sz}, reader{reader}
{	
	if (buffer_size < min_buffer_size) {
		std::cerr << "jsonP_buffer_parser initiated with buffer size too small: " << buffer_size <<
						", defaulting to: " << min_buffer_size << std::endl;
		buffer_size = min_buffer_size;
	}
	
	buffer = (char*) malloc(sizeof(char) * (buffer_size + 1));
}


jsonP_buffer_parser::~jsonP_buffer_parser()
{
//	std::cout << "jsonP_buffer_parser destructor\n";
	free(buffer);
//	delete ((file_chunk_impl*)reader);
	delete reader;
}


// std::string version
//int jsonP_buffer_parser::read_next_chunk()
//{
////	std::cout << "reader->get_next about to be called\n";
//	char left_over[json_length - index + 1];
//	int j{0};
//	
//	for (int i{index}; i < json_length; i++, j++)
//		left_over[j] = json[i];
//		
//	left_over[j] = '\0';
//	json = left_over;
//	
//	int read = reader->get_next(buffer, buffer_size);
//	
//	if (read < 0)
//		throw jsonP_exception{"JsonP_buffer_parser error in getting more chunks from implemented IChunk_reader"};
//	
////	std::cout << "j: " << read << "\n";
//	buffer[read] = '\0';
//	json += buffer;
//	index = 0;
//	json_length = json.length();
////std::cout << "read: " << read << ", json:\n" << json << std::endl;
//
//	if (read == 0) {
//		more_chunks = false;
//	}
//
//	return read;
//}


int jsonP_buffer_parser::read_next_chunk()
{
//	std::cout << "reader->get_next about to be called\n";
	if (!more_chunks)
		return 0;

	int i{0};
	for ( ; index < json_length; i++, index++)
		buffer[i] = buffer[index];

	read = reader->get_next(&buffer[i], buffer_size - i);
	
	if (read < 0)
		throw jsonP_exception{"JsonP_buffer_parser error in getting more chunks from implemented IChunk_reader"};
	
//std::cout << "read: " << read << ", i: " << i << "\n";

	if (i == 0)
		json_length = read;
	else
		json_length = read + i;
	
	buffer[json_length] = '\0';
	index = 0;
	json = buffer;

//std::cout << "json_length: " << json_length << ", read: " << read << ", json:\n" << json << std::endl;

	if (read == 0) {
		more_chunks = false;
	}

	return read;
}


bool jsonP_buffer_parser::check_buffer()
{
	if (!more_chunks)
		return false;
		
	if (json_length - index <= need_more_bytes) {
		int read = read_next_chunk();
		
		if (read == 0) {
			return false;
		} else {
			return true;
		}
	}
		
	return false;
}


jsonP_doc* jsonP_buffer_parser::parse()
{
	int r{0};
//	json = "";
	json_length = 0;
	index = 0;
	read = 0;
//	index = buffer_size;
	more_chunks = true;
	jsonP_doc *doc = nullptr;
	
	//get first chunk and start parsing
	if (read_next_chunk() > 2) {
		eat_whitespace(index);

		while (index >= json_length) {
			r = read_next_chunk();
			
			if (r > 0) {
				eat_whitespace(index);
			} else {
				//throw end of file reached and never found json
			}
		}

		if (json[index] == '{') {
			element_object *obj = new element_object{};
			parse_object(obj);
			doc = new jsonP_doc{obj};
		} else if (json[index] == '[') {
			element_array *arr = nullptr;
			parse_array(arr);
			doc = new jsonP_doc{arr};
		} else {
			throw jsonP_exception{"Error parsing json text, does not appear to be an object or an array"};
		}
		
	} else {
		//throw
		throw jsonP_exception{"Error parsing json text, error reading first chunk"};
	}
	
	
//std::cout << "4\n";
	
	return doc;
}


element_type jsonP_buffer_parser::parse_numeric(std::string & value)
{
	check_buffer();
	element_type t;
	
	try {
		t = jsonP_parser::parse_numeric(value);
	} catch (jsonP_exception &ex) {
//std::cerr << "jsonP_buffer_parser error caught paring numeric, index: " << index << ", json_length: " << json_length << std::endl;
//std::cerr << json << "\n\n";
		if (index >= json_length) {
			if (check_buffer())
				parse_numeric(value);
			else
				throw ex;
		} else {
			throw ex;
		}
	}
	
	check_buffer();
	
	return t;
}


void jsonP_buffer_parser::parse_bool(bool & value)
{
	check_buffer();
	
	try {
		jsonP_parser::parse_bool(value);
	} catch (jsonP_exception &ex) {
		if (index >= json_length) {
			if (check_buffer())
				parse_bool(value);
			else
				throw ex;
		} else {
			throw ex;
		}
	}
	
	check_buffer();
}


void jsonP_buffer_parser::parse_key(std::string & value)
{
	check_buffer();
	
	try {
		jsonP_parser::parse_key(value);
	} catch (jsonP_exception &ex) {
		if (index >= json_length) {
			if (check_buffer())
				parse_key(value);
			else
				throw ex;
		} else {
			throw ex;
		}
	}
	
	check_buffer();
}


void jsonP_buffer_parser::eat_whitespace(int idx)
{
	check_buffer();
	
	try {
		jsonP_parser::eat_whitespace(idx);
	} catch (jsonP_exception &ex) {
		if (index >= json_length) {
			if (check_buffer())
				eat_whitespace(idx);
			else
				throw ex;
		} else {
			throw ex;
		}
	}
	
	check_buffer();
}

