#include "file_chunk_impl.h"
#include "jsonP_exception.h"

#include <iostream>


file_chunk_impl::file_chunk_impl(std::string file_name, int buf_sz) : 
//buffer_sz{buf_sz}, 
fp_position{0}, 
file_name{file_name},
done_parsing{false}
{}


file_chunk_impl::~file_chunk_impl()
{
	if (fp != NULL)
		fclose(fp);
}
	

int file_chunk_impl::get_next(char *buf, int cnt)
{
	if (done_parsing)
		return 0;
		
	if (fp == NULL) {
		fp = std::fopen(file_name.c_str(), "r");
			
		if (fp != NULL) {
			fseek(fp, fp_position, SEEK_SET);
		} else {
			perror("Error opening file: ");
			std::string error = "Error opening file " + file_name + ", ";
//				std::string error2 = strerror(errno);
//				error += error2;
			throw jsonP_exception{error.c_str()};
		}
	}

	int read = fread((void *)buf, sizeof(char), cnt, fp);
	fp_position = ftell(fp);
//std::cout << "3: " << read << ", position: " << fp_position << "\n";		
	if (ferror(fp)) {
		perror("Error reading file: ");
		throw jsonP_exception{"Error reading json file"};
	}
	
	if (feof(fp)) {
		fclose(fp);
		fp = NULL;
		done_parsing = true;
	}
		
	return read;
}

