#ifndef _ELEMENT_OBJECT_H_
#define _ELEMENT_OBJECT_H_

#include "elements.h"

#include <map>


class element_object : public element
{
private:
	std::map<std::string, element*> elements;
	
public:
	element_object();
	~element_object();

	void add_element(std::string key, element *e);
	element_object * get_object_value() override;
	
	std::string & get_as_string(std::string key) override;
	bool get_as_boolean(std::string key) override;
	long get_as_numeric(std::string key) override;
	std::vector<element *> & get_as_array(std::string key) override;
	element_object * get_as_object(std::string key) override;
	
	void stringify(std::string &) override;
	void stringify_pretty(std::string &);
};

typedef element_object jsonP_doc;


#endif // _ELEMENT_OBJECT_H_
