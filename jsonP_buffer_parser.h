#ifndef JSONP_BUFFER_PARSER_H
#define JSONP_BUFFER_PARSER_H

#include "jsonP_parser.h"
#include "file_chunk_impl.h"


#define need_more_bytes		300
#define min_buffer_size		1024


class jsonP_buffer_parser : public jsonP_parser
{
private:
	int buffer_size;
	char *buffer;
	bool more_chunks;
	int read;
	IChunk_reader *reader = nullptr;
	
	int read_next_chunk();
	
	//overrides
	virtual inline void eat_whitespace() override 
	{
		check_buffer();
	
		try {
			jsonP_parser::eat_whitespace();
		} catch (jsonP_exception &ex) {
			if (index >= json_length) {
				if (check_buffer())
					eat_whitespace();
				else
					throw ex;
			} else {
				throw ex;
			}
		}
		
		check_buffer();
	}
	
	
	virtual bool check_buffer() //bool &more_chunks, unsigned int &json_length, unsigned int &index)
	{ 
		if (!more_chunks)
			return false;
			
		if (json_length - index <= need_more_bytes) {
			int read = read_next_chunk();

//			std::cout << "json_length: " << json_length << ", index: " << index << 
//				", need_more_bytes: " << need_more_bytes << ", read: " << read <<std::endl;
		
			if (read == 0) {
				return false;
			} else {
				return true;
			}
		}
			
		return false;
	}
	
	
protected:
	//overrides
	virtual void parse_key() override;
	virtual element_type parse_numeric() override;

	
public:
	jsonP_buffer_parser(std::string file_name, int buf_sz = 8192, unsigned short options_ = 0);
	jsonP_buffer_parser(IChunk_reader *reader, int buf_sz = 8192, unsigned short options_ = 0);
	~jsonP_buffer_parser();

	jsonP_json * parse();
};

#endif // JSONP_BUFFER_PARSER_H
