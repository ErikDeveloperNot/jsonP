#include "modified_json_push_handler_impl.h"
#include "element_object.h"


modified_json_push_handler_impl::modified_json_push_handler_impl(element_type type)
{
	if (type == array)
		root = new element_array{string};
	else
		root = new element_object{};
}

modified_json_push_handler_impl::~modified_json_push_handler_impl()
{
}


bool modified_json_push_handler_impl::get_element(std::string& path, std::string& key)
{
	if (keys_map.count(path+key) > 0) {
		return true;
	} else {
		return false;
	}
}


void modified_json_push_handler_impl::element_parsed(std::string& path, std::string& key, element* e)
{
	keys_map[path+key].elements.push_back(e);
}


void modified_json_push_handler_impl::add_key(std::string path, std::string key)
{
	std::string p = path;
	
	if (p[0] != '/')
		p = '/' + p;
		
	if (p[p.length()-1] != '/')
		p += '/';
		
//	p += key;
	keys_map.emplace(p+key, element_entry{p, key});
}


element* modified_json_push_handler_impl::get_modified_json()
{
	if (root->get_type() == object) {
		// doc is an object
		element_object *obj;
		
		for (auto it = keys_map.begin(); it != keys_map.end(); it++) {
			obj = (element_object *)root;
			element_entry &entry = it->second;

			if (entry.elements.size() < 1)
				continue;
			
			create_object(entry, obj);
		}
		
		return root;
	} else {
		// doc is an array
		element_array *arr = (element_array *)root;
		
		for (auto it = keys_map.begin(); it != keys_map.end(); it++) {
//			obj = (element_object *)root;
			element_entry &entry = it->second;

			if (entry.elements.size() < 1)
				continue;
			
			if (entry.path.length() > 2) { 
				element_object *o = new element_object{};
				element_object *obj = o;
				create_object(entry, obj);
				arr->add_element(o);
			} else {
				for (element *e : entry.elements)
					arr->add_element(e);
			}
		}
		
		return root;
	}
}


void modified_json_push_handler_impl::create_object(element_entry & entry, element_object *& obj)
{
	int i = (entry.path[0] == '/') ? 1 : 0;
//			int e = s + 1;
	std::string p{""};
			
	while (i < entry.path.length()) {
		if (entry.path[i] == '/') {
					
			if (p.length() > 0) {
						
				if (!obj->contains_key(p)) {
					element_object *o = new element_object{};
					obj->add_element(p, o);
					obj = o;
				} else {
					obj = obj->get_as_object(p);
				}
						
				p = "";
			}
					
			i++;
		} else {
			p += entry.path[i++];
		}
	}
			
	if (entry.elements.size() > 1) {
		//its an array
		element_array *arr = new element_array{string};
				
		for (element *e : entry.elements)
			arr->add_element(e);
					
		obj->add_element(entry.key, arr);
	} else {
		//its a non array value
		obj->add_element(entry.key, entry.elements[0]);
	}
}


void modified_json_push_handler_impl::debug_key_map()
{
	std::cout << "Debug modified_json_push_handler_impl keys_map contents\n";
	
	for (auto it = keys_map.begin(); it != keys_map.end(); it++) {
		std::cout << it->first << std::endl;
	}
}

