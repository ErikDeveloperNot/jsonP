#ifndef JSONP_JSON_H
#define JSONP_JSON_H

#include "element.h"


//typedef char byte;
typedef unsigned int object_id;
typedef unsigned int array_id;


enum index_type : u_int8_t { object_key=0, array_indx=1 }; 

enum error : u_int8_t { none=0, is_null=1, not_string=2, not_long=3, not_double=4, not_bool=5, not_found=6,
						invalid_container=7, invalid_id=8 };

struct search_path_element 
{
	index_type type;
	char * key;
};

struct object_key
{
	 char *key;
	element_type type;
};



class jsonP_json
{
private:
	byte *data;
	unsigned int data_length;
	unsigned int data_i;
	bool data_eq_meta;
	byte *meta_data;
	unsigned int meta_length;
	unsigned int *meta_length_ptr;
	unsigned int meta_i;
	unsigned int *meta_i_ptr;
	unsigned int doc_root;
	
	void *get_next_array_buf;
	unsigned int get_next_array_indx;
	unsigned int get_next_array_mem_cnt;
	object_id get_next_array_id;
	object_id get_next_array_ext_next;
	
	//options
	bool dont_sort_keys;
	bool weak_ref;
	bool convert_numerics;
		
	object_id get_object_id(search_path_element *, unsigned int, bool);
	object_id get_object_id(char *path, const char *delim, bool ret_ptr, object_id *ret_parent_container = NULL);
	
	void parse_object(unsigned int &, unsigned int &, unsigned int &, char *&, const char*);
	void parse_array(unsigned int &, unsigned int &, unsigned int &, char *&, const char*);
	
	unsigned int get_meta_slot(unsigned int, bool &needs_sort, element_type);

	element_type get_value(object_id, index_type, const char *&);
	
	long parse_long(const char*);
	double parse_float(const char*);

public:
	jsonP_json(byte *, byte *, unsigned int, unsigned int, unsigned int, unsigned short options = 0);
	jsonP_json(element_type, unsigned int, unsigned int buf_sz = 102400, unsigned short options = 0);
	jsonP_json(const jsonP_json &);
//	jsonP_json();

	~jsonP_json();
	
	// manipulate methods
	object_id add_container(const char *, unsigned int, object_id, element_type);
	int add_value_type(element_type, object_id, const char *, void * = NULL);
	
	int update_value(object_id, index_type, element_type, void *);
	int update_value(search_path_element *, unsigned int, element_type, void *);
	int update_value(const char *, const char *, element_type, void *);

	//for delete value it makes it way easier to have full path so no option for object_id for now
	int delete_value(search_path_element *, unsigned int, char *, error*);
	int delete_value(const char *path, const char *delim, char *key, error *);
	int delete_value(object_id id, object_id parent, char *key, error *);
	
	// access methods
	object_id get_doc_root() { return doc_root; }
	object_id get_object_id(search_path_element *, unsigned int);
	object_id get_object_id(const char *path, const char *delim);

	unsigned int get_members_count(object_id);
	unsigned int get_members_count(search_path_element *, unsigned int);
	unsigned int get_members_count(const char *path, const char *delim);
	
	inline unsigned int get_elements_count(object_id id) { return get_members_count(id); }
	inline unsigned int get_elements_count(search_path_element *path, unsigned int path_count) { return get_members_count(path, path_count); }
	inline unsigned int get_elements_count(const char *path, const char *delim) { return get_members_count(path, delim); }
	
	//object_key* needs to be freed by user when done
	unsigned int get_keys(search_path_element *, unsigned int, struct object_key *&);
	unsigned int get_keys(const char *path, const char *delim, struct object_key *&);
	unsigned int get_keys(object_id, struct object_key *&);
	
	
	/*
	 * for the first call pass the object_id or search_path or char *path, for remaining calls pass NULL or 0 (object_id)
	 * when no more elements are avail and empty element_type will be returned.
	 * an internal buffer is used to hold the value returned. This value will be replaced
	 * with each call to next. The memoery alloc/dealloc of this buffer is handled by the parser
	 */
	element_type get_next_array_element(object_id, const void *&);
	element_type get_next_array_element(search_path_element *, unsigned int, const void *&);
	element_type get_next_array_element(const char *path, const char *delim, const void *&);
	
	double get_double_value(search_path_element *, unsigned int, error*);
	double get_double_value(const char *, const char *, error*);
	double get_double_value(object_id, index_type, error*);
	double get_double_value(const char *key, object_id parent, error*);
	long get_long_value(search_path_element *, unsigned int, error*);
	long get_long_value(const char *, const char *, error*);
	long get_long_value(object_id, index_type, error*);
	long get_long_value(const char *key, object_id parent, error*);
	bool get_bool_value(search_path_element *, unsigned int, error*);
	bool get_bool_value(const char *, const char *, error*);
	bool get_bool_value(object_id, index_type, error*);
	bool get_bool_value(const char *key, object_id parent, error*);
	const char* get_string_value(search_path_element *, unsigned int, error*);
	const char* get_string_value(const char *, const char *, error*);
	const char* get_string_value(object_id, index_type, error*);
	const char* get_string_value(const char *key, object_id parent, error*);
	
	// create json txt representation methods
	char * stringify(int precision = 6);
	char * stringify_pretty(int precision = 6);
	
	// *paths should be passed in allocated with enought space for the number of path elements
	int build_search_path(char *, const char *, search_path_element *);

	inline element_type get_elements_type(object_id id)
	{
		return get_element_type(meta_data, id);
	}
	
	inline element_type get_elements_type(search_path_element *path, unsigned int path_count)
	{
		return get_elements_type(get_object_id(path, path_count));
	}
	
	inline unsigned int increase_txt_buffer(unsigned int needed, unsigned int sz, unsigned int indx, char *& txt) 
	{
		//add check if indx is greater then sz,
		if (needed + 20 > sz - indx || indx >= sz) {
//			std::cout << "Allocating, current sz: " << sz << ", needed: " << needed << ", indx: " << indx << std::endl;
			sz += (needed + (unsigned int)(sz * 0.25));
			txt = (char*) realloc(txt, sz);
//			std::cout << "stringify realloc needed, new size: " << sz << std::endl;
		}
		
		return sz;
	}
	
	
	inline unsigned int increase_meta_buffer(unsigned int needed, unsigned int sz, unsigned int indx)
	{
		sz = increase_txt_buffer(needed, sz, indx, meta_data);
		
		if (data_eq_meta) 
			data = meta_data;
			
		return sz;
	}
	
	
	inline unsigned int increase_dat_buffer(unsigned int needed, unsigned int sz, unsigned int indx)
	{
		sz = increase_txt_buffer(needed, sz, indx, data);
		
		if (data_eq_meta) 
			meta_data = data;
			
		return sz;
	}
};

#endif // JSONP_JSON_H
