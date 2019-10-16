#ifndef FILE_CHUNK_IMPL
#define FILE_CHUNK_IMPL

#include "IChunk_reader.h"

#include <stdio.h>
#include <string>

class file_chunk_impl : public IChunk_reader
{
private:
//	int buffer_sz;
	FILE *fp = NULL;
	long fp_position;
	std::string file_name;
	bool done_parsing;

public:
	file_chunk_impl(std::string file_name, int buf_sz);
	virtual ~file_chunk_impl();
	
	virtual int get_next(char *buf, int cnt) override;

};


#endif //FILE_CHUNK_IMPL