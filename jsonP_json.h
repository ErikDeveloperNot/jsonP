#ifndef JSONP_JSON_H
#define JSONP_JSON_H

#include "element.h"


//typedef char byte;



class jsonP_json
{
private:
	byte *data;
	byte *meta_data;
	unsigned long data_length;
	unsigned int doc_root;
	
	void parse_object(unsigned int &, unsigned int &, unsigned int &, char *&);
	void parse_array(unsigned int &, unsigned int &, unsigned int &, char *&);

public:
	jsonP_json(byte *, byte *, unsigned long, unsigned int);
	jsonP_json() = delete;
	~jsonP_json();
	
	char * stringify();
	char * stringify_pretty();
	
	static inline void increase_buffer(unsigned int needed, unsigned int &sz, unsigned int indx, char *& txt) 
	{
		if (needed + 20 > sz - indx) {
			//sz += needed + 200;
			sz += needed + (unsigned int)(sz * 0.25);
			txt = (char*) realloc(txt, sz);
std::cout << "stringify realloc needed" << std::endl;
		}
	}
};

#endif // JSONP_JSON_H
