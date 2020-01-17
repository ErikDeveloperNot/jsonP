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
	options = PRESERVE_JSON | SHRINK_BUFS;
	}


jsonP_buffer_parser::jsonP_buffer_parser(IChunk_reader *reader, int buf_sz) : buffer_size{buf_sz}, reader{reader}
{	
	if (buffer_size < min_buffer_size) {
		std::cerr << "jsonP_buffer_parser initiated with buffer size too small: " << buffer_size <<
						", defaulting to: " << min_buffer_size << std::endl;
		buffer_size = min_buffer_size;
	}
	
	buffer = (char*) malloc(sizeof(char) * (buffer_size + 1));
	options = PRESERVE_JSON | SHRINK_BUFS;
}


jsonP_buffer_parser::~jsonP_buffer_parser()
{
//	std::cout << "jsonP_buffer_parser destructor\n";
	free(buffer);
//	delete ((file_chunk_impl*)reader);
	delete reader;
}



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
	
	if (i == 0)
		json_length = read;
	else
		json_length = read + i;
	
	buffer[json_length] = '\0';
	index = 0;
	json = buffer;

//	std::cout << "json_length: " << json_length << ", read: " << read << ", i: " << i << ", json:\n" << json << std::endl;

	if (read == 0) {
		more_chunks = false;
	}

	return read;
}


jsonP_json* jsonP_buffer_parser::parse()
{
	int r{0};
	json_length = 0;
	index = 0;
	read = 0;
	more_chunks = true;
	jsonP_json *doc = nullptr;
	
	//new stuff
	use_json = (options & PRESERVE_JSON) ? false : true;
	shrink_buffers = (options & SHRINK_BUFS) ? true : false;
	dont_sort_keys = (options & DONT_SORT_KEYS) ? true : false;
	stack_buf_sz = 1024;
	stack_buf = (byte*) malloc(stack_buf_sz);
	data_sz = buffer_size;
	data = (byte*) malloc(data_sz);
	look_for_key = false;
	stack_i = 0;
	data_i = 0;
	
	//get first chunk and start parsing
	if (read_next_chunk() > 2) {
		doc = jsonP_parser::parse(buffer, json_length);
	} else {
		//throw
		throw jsonP_exception{"Error parsing json text, error reading first chunk"};
	}
	
	return doc;
}



element_type jsonP_buffer_parser::parse_numeric()
{
	check_buffer();
	element_type t;

	try {
		t = jsonP_parser::parse_numeric();
	} catch (jsonP_exception &ex) {
//		std::cerr << "jsonP_buffer_parser error caught paring numeric, index: " << index << ", json_length: " << json_length << std::endl;
//		std::cerr << json << "\n\n";

		if (index >= json_length) {
			if (check_buffer())
				parse_numeric();
			else
				throw ex;
		} else {
			throw ex;
		}
	}
	
	check_buffer();
	
	return t;
}


void jsonP_buffer_parser::parse_key()
{
	check_buffer();
	
	try {
		jsonP_parser::parse_key();
	} catch (jsonP_exception &ex) {
		if (index >= json_length) {
			if (check_buffer())
				parse_key();
			else
				throw ex;
		} else {
			throw ex;
		}
	}
	
	check_buffer();
}


