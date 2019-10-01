#ifndef _JSONP_EXCEPTION_H_
#define _JSONP_EXCEPTION_H_

#include <exception>
#include <cstring>
#include <cstdlib>


class jsonP_exception : public std::exception
{
public:
	jsonP_exception(char * msg) { copy_message(msg); }
	
	jsonP_exception(const char * msg) { copy_message((char*)msg); }
	
	jsonP_exception(const jsonP_exception & other) { copy_message(other.message); }
	
	jsonP_exception(const jsonP_exception && other) { copy_message(other.message); }
	
	~jsonP_exception() { free(message); }
	
	virtual const char* what() const noexcept
	{
		return message;
	}

private:
	char *message;
//	bool copyConstructed;
	
	void copy_message(char * msg) {
		message = (char*) malloc(strlen(msg)+1 * sizeof(char));
		message = strcpy(message, msg);
	}
};

#endif // _JSONP_EXCEPTION_H_
