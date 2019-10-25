#ifndef MODIFIED_JSON_PUSH_HANDLER_IMPL_H
#define MODIFIED_JSON_PUSH_HANDLER_IMPL_H

#include "IPush_handler.h"

#include <map>


struct element_entry
{
	std::string path;
	std::string key;
	std::vector<element *> elements;
	
	element_entry() = default;
	element_entry(std::string p, std::string k) : path{p}, key{k} {}
	element_entry(const element_entry & other) : path{other.path}, key{other.key} {}
	element_entry(const element_entry && other) : path{other.path}, key{other.key} {}
};


class modified_json_push_handler_impl : public IPush_handler
{
private:
	element *root = nullptr;
	std::map<std::string, element_entry> keys_map;
	
	void create_object(element_entry &, element_object *&);
	
public:
	modified_json_push_handler_impl(element_type);
	~modified_json_push_handler_impl();
	
	void add_key(std::string, std::string);
	element * get_modified_json();
	
	void debug_key_map();
	
	//overrides
	virtual bool get_element(std::string &path, std::string &key) override;
	virtual void element_parsed(std::string &path, std::string &key, element *) override;

};

#endif // MODIFIED_JSON_PUSH_HANDLER_IMPL_H
