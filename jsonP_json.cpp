#include "jsonP_json.h"
#include "element.h"
#include "jsonP_exception.h"

//#include <malloc/malloc.h>

jsonP_json::jsonP_json(byte *data_, byte *meta_data_, unsigned int data_length_, unsigned int meta_length_, 
						unsigned int doc_root_, unsigned short options) : 
data{data_},
meta_data{meta_data_}, 
data_length{data_length_},
data_i{data_length},
meta_length{meta_length_},
meta_i{meta_length_},
doc_root{doc_root_},
get_next_array_indx{0},
get_next_array_id{0}
{
	if (data == meta_data) {
//		std::cout << "SAME\n";
		meta_length_ptr = &data_length;
		meta_i_ptr = &data_i;
		data_eq_meta = true;
	} else {
//		std::cout << "DIFFERENT\n";
		meta_length_ptr = &meta_length;
		meta_i_ptr = &meta_i;
		data_eq_meta = false;
	}
	
	get_next_array_buf = (void*) malloc(sizeof(double) * 2);
	dont_sort_keys = (options & DONT_SORT_KEYS) ? true : false;
	weak_ref = (options & WEAK_REF) ? true : false;
	convert_numerics = (options & CONVERT_NUMERICS) ? true : false;
	
}


jsonP_json::jsonP_json(element_type type, unsigned int num_elements, unsigned int buf_sz, unsigned short options) :
get_next_array_indx{0},
get_next_array_id{0}
{
	if (num_elements < 1)
		num_elements = 1;
		
	if (buf_sz < (obj_member_sz*5 + obj_root_sz + num_elements*obj_member_sz))
		buf_sz = obj_member_sz*5 + obj_root_sz + num_elements*obj_member_sz;
		
	data_length = buf_sz;
	data = (byte*) malloc(data_length);
	data_i = 0;
	meta_data = data;
	meta_length_ptr = &data_length;
	meta_i_ptr = &data_i;
	data_eq_meta = true;
	
//std::cout << "========constructor\n";
//printf("&data: %p, data: %p\n", &data, data);
//printf("&meta_data: %p, meta_data: %p\n", &meta_data, meta_data);

	
	if (type == object) {
//		*(element_type*)&data[data_i] = object;
		set_element_type(data, data_i, object);
	} else if(type == array) {
//		*(element_type*)&data[data_i] = array;
		set_element_type(data, data_i, array);
	} else {
		throw jsonP_exception{"Error creating json, type must be 'object' or 'array'"};
	}
	
	data_i += obj_member_sz;
//	*(unsigned int*)&data[data_i] = num_elements;
	set_uint_a_indx(data, data_i, num_elements);
	data_i += obj_root_sz;
	doc_root = 0;
	
	for (size_t i=0; i<num_elements; i++, data_i += obj_member_sz) {
//		*(element_type*)&data[data_i] = empty;
		set_element_type(data, data_i, empty);
	}
	
//	*(element_type*)&data[data_i] = extended;
	set_element_type(data, data_i, extended);
//	*(unsigned int*)&data[data_i + obj_member_key_offx] = 0;
	set_key_offx_value(data, data_i, 0);
	data_i += obj_member_sz;
	
	get_next_array_buf = (void*) malloc(sizeof(double) * 2);
	dont_sort_keys = (options & DONT_SORT_KEYS) ? true : false;
	weak_ref = (options & WEAK_REF) ? true : false;
	convert_numerics = (options & CONVERT_NUMERICS) ? true : false;
	
}


jsonP_json::jsonP_json(const jsonP_json &o) :
data_length{o.data_length},
data_i{o.data_i},
data_eq_meta{o.data_eq_meta},
meta_length{o.meta_length},
meta_i{o.meta_i},
doc_root{o.doc_root},
get_next_array_indx{o.get_next_array_indx},
get_next_array_mem_cnt{o.get_next_array_mem_cnt},
get_next_array_id{o.get_next_array_id},
get_next_array_ext_next{o.get_next_array_ext_next},
dont_sort_keys{o.dont_sort_keys},
weak_ref{false},
convert_numerics{o.convert_numerics}
{
//	std::cout << "this:" << data_eq_meta << ", other:" << o.data_eq_meta << std::endl;
//	std::cout << "this:" << data_length << ", other:" << o.data_length << std::endl;
	data = (byte*)malloc(o.data_length);
	memcpy(data, o.data, data_length);
	
	if (!data_eq_meta) {
		meta_data = (byte*)malloc(o.meta_length);
		memcpy(meta_data, o.meta_data, meta_length);
		meta_length_ptr = &meta_length;
		meta_i_ptr = &meta_i;
	} else {
		meta_data = data;
		meta_length_ptr = &data_length;
		meta_i_ptr = &data_i;
	}
	
	
	get_next_array_buf = (void*) malloc(sizeof(double) * 2);
}


jsonP_json::~jsonP_json()
{
//	std::cout << ">>>>>>>>>>  jsonP_json destruct <<<<<<<<<<<, convert: " << convert_numerics << std::endl;
//	printf("&data: %p, data: %p\n", &data, data);
//	printf("&meta_data: %p, meta_data: %p\n", &meta_data, meta_data);
	free(get_next_array_buf);
	
	if (weak_ref) {
//		std::cout << "WEAK REF, no free\n";
		return;
	}
	
	if (!data_eq_meta) {
//		std::cout << "<<<<<<<<<, data != meta_data >>>>>>>>>>>>" << std::endl;
		free(meta_data);
	} 
	
	free(data);
}



object_id jsonP_json::get_object_id(search_path_element * path, unsigned int path_count)
{
	return get_object_id(path, path_count, false);
}


object_id jsonP_json::get_object_id(search_path_element * path, unsigned int path_count, bool ret_ptr)
{
	if (path_count < 1)
		return 0;
		
	unsigned int result = 0;
	object_id start = doc_root + obj_member_sz;
	unsigned int num_keys = get_key_count(meta_data, start);
	start += obj_root_sz;
	size_t i=0;
	element_type type;
	bool is_array_indx = false;

	while (true) {
		result = 0;
//		std::cout << "get_id for: "<< path[i].key << ", i=" << i << ", path_count=" << path_count << 
//				", num keys: " << num_keys << std::endl;

		if (path[i].type == array_indx) {
			unsigned int indx = atoi(path[i].key);
			
			if (indx < num_keys) {
				if (get_element_type(meta_data, start + (indx * obj_member_sz)) != empty)
					result = start + (indx * obj_member_sz);
				else
					result = 0;
			} else {
				// check if it is in an ext slot
				object_id ext_start = get_ext_start(meta_data, start + obj_member_sz * num_keys);
				unsigned int k = num_keys;
//				std::cout << "ext_start: " << ext_start << "\n\n";				

				while (ext_start > 0) {
					if (k == indx) {
						result = ext_start;
						break;
					} else {
						ext_start = get_ext_next(meta_data, ext_start);
						k++;
						result = 0;
					}
				}
//				std::cout << "<<<<<<<<<<<<<< retult: " << result << "<<<<<<<<<<<<<\n\n";
			}
		} else {
			if (i+1 == path_count) {
				if (ret_ptr)
					result = search_keys(path[i].key, start, (start + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, true, dont_sort_keys);
				else
					result = search_keys(path[i].key, start, (start + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, false, dont_sort_keys);
			} else {
				result = search_keys(path[i].key, start, (start + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, false, dont_sort_keys);
			}
		}
		
		type = get_element_type(meta_data, result);
		
		if (result > 0) {
			i++;
//			std::cout << "bsearh type: " << type << ", i=" << i << ", path_count=" << path_count << std::endl;
			
			if (type ==  object) { // && i < path_count) {
				is_array_indx = false;
				
				if (i == path_count) {
					return result;
				} else {
					start = result + obj_member_sz;
					num_keys = get_key_count(meta_data, start);
					start += obj_root_sz;
				}
			} else if (type ==  array) { //_ptr && i < path_count) {
				if (i == path_count) {
					return result;
				} else {
					is_array_indx = true;
					start = result + obj_member_sz;
					num_keys = get_key_count(meta_data, start);
					start += obj_root_sz;
				}
			} else if (type == object_ptr || type == array_ptr) {
				is_array_indx = false;
				
				if (!ret_ptr) {
					result = get_key_location(meta_data, result);
					result -= element_type_sz;
				}

//type = get_element_type(meta_data, result);
//std::cout << "PTR type 2: " << type << std::endl;

				if (i == path_count) {
					return result;
				} else {
					start = result + obj_member_sz;
					num_keys = get_key_count(meta_data, start);
					start += obj_root_sz;
				}
			} else if (i < path_count) {
				//error trying to get a node below a leaf node
				return 0;
			} else if (i == path_count) {
				return result;
			}
		} else {
			return result;
		}
	}
	
	return result;
}


object_id jsonP_json::get_object_id(const char *path, const char *delim)
{
	return get_object_id((char*)path, delim, false, NULL);
}


object_id jsonP_json::get_object_id(char *path, const char *delim, bool ret_ptr, object_id *parent_id)
{
	char tok_path[strlen(path) + 1];
	char *tok = strtok(strcpy(tok_path, path), delim);
	char *next_tok = NULL;
	
	if (tok == NULL)
		return 0;
	
	unsigned int result = doc_root;
	object_id start = doc_root + obj_member_sz;
	unsigned int num_keys = get_key_count(meta_data, start);
	start += obj_root_sz;
//	size_t i=0;
	element_type type;
	element_type parent = get_element_type(meta_data, doc_root);
	bool is_array_indx = false;

	while (tok != NULL) {
		next_tok = strtok(NULL, delim);
//std::cout << "\ntok: "<< tok << ", next_tok: " << (next_tok == NULL ? "NULL" : next_tok) << std::endl;
		if (parent_id != NULL)
			*parent_id = result;
		
		result = 0;

//		if (path[i].type == array_indx) {
		if (parent == array) {
//			unsigned int indx = atoi(path[i].key);
			unsigned int indx = atoi(tok);

			if (indx == 0 && tok[0] != '0')			//invalid value return
				return 0;
				
			if (indx < num_keys) {
				if (get_element_type(meta_data, start + (indx * obj_member_sz)) != empty)
					result = start + (indx * obj_member_sz);
				else
					result = 0;
			} else {
				// check if it is in an ext slot
				object_id ext_start = get_ext_start(meta_data, start + obj_member_sz * num_keys);
				unsigned int k = num_keys;
//std::cout << "ext_start: " << ext_start << "\n\n";				

				while (ext_start > 0) {
					if (k == indx) {
						result = ext_start;
						break;
					} else {
						ext_start = get_ext_next(meta_data, ext_start);
						k++;
						result = 0;
					}
				}
			}
		} else {
			if (next_tok == NULL) {
				if (ret_ptr)
					result = search_keys(tok, start, (start + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, true, dont_sort_keys);
				else
					result = search_keys(tok, start, (start + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, false, dont_sort_keys);
			} else {
				result = search_keys(tok, start, (start + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, false, dont_sort_keys);
			}
		}
		
		type = get_element_type(meta_data, result);
//std::cout << "<<< retult: " << result << "<<<<type: " << type << ", ret_ptr: " << ret_ptr << std::endl;
		
		if (result > 0) {
			
			if (type ==  object) { // && i < path_count) {
				is_array_indx = false;
				
				if (next_tok == NULL) {
					return result;
				} else {
					start = result + obj_member_sz;
					parent = get_element_type(meta_data, result);
					num_keys = get_key_count(meta_data, start);
					start += obj_root_sz;
				}
			} else if (type ==  array) { //_ptr && i < path_count) {
				if (next_tok == NULL) {
					return result;
				} else {
					is_array_indx = true;
					parent = get_element_type(meta_data, result);
					start = result + obj_member_sz;
					num_keys = get_key_count(meta_data, start);
					start += obj_root_sz;
				}
			} else if (type == object_ptr || type == array_ptr) {
				is_array_indx = false;
				
				if (!ret_ptr) {
					result = get_key_location(meta_data, result);
					result -= element_type_sz;
				}

				if (next_tok == NULL) {
					return result;
				} else {
					parent = get_element_type(meta_data, result);
					start = result + obj_member_sz;
					num_keys = get_key_count(meta_data, start);
					start += obj_root_sz;
				}
			} else if (next_tok != NULL) {
				//error trying to get a node below a leaf node
				return 0;
			} else if (next_tok == NULL) {
				return result;
			}
		} else {
			return result;
		}
		
		tok = next_tok;
	}
	
	return result;
}



object_id jsonP_json::add_container(const char* key, unsigned int num_keys, object_id id, element_type container_type)
{
	element_type parent_type = get_element_type(meta_data, id);
	
	if (parent_type != object && parent_type != array)
		throw jsonP_exception{"object_id is not an object or an array"};
	if (container_type != object && container_type != array)
		throw jsonP_exception{"container type is not an object or array"};
		
	if (num_keys < 1)
		num_keys = 1;
		
	//get a key slot from the parent object
	bool is_ext = false;
	object_id meta_slot = get_meta_slot(id + obj_member_sz, is_ext, container_type);
		
	//copy the new objects keys to the data buffer if parent container is an object
	unsigned int key_loc;
	
	if (parent_type == object) {
		size_t k_len = strlen(key);
		data_length = increase_dat_buffer(k_len+2, data_length, data_i);
		memcpy(data+data_i, key, k_len+1);
		key_loc = data_i;
		data_i += k_len+1;
	}
	
	//start to create the new object block and set key to th data location from previous step
	*meta_length_ptr = increase_meta_buffer((5 + num_keys) * obj_member_sz, *meta_length_ptr, *meta_i_ptr);
	set_element_type(meta_data, *meta_i_ptr, container_type);

	if (parent_type == object)
		set_key_offx_value(meta_data, *meta_i_ptr, key_loc);

	object_id id_to_return = *meta_i_ptr;
	
	//assign the parent object key a type of pointer and point to the start of the new block
	if (container_type == object)
		set_element_type(meta_data, meta_slot, object_ptr);
	else
		set_element_type(meta_data, meta_slot, array_ptr);

	set_key_offx_value(meta_data, meta_slot, *meta_i_ptr+1);
	*meta_i_ptr += obj_member_sz;
	
	//set number of keys for new object or num elements for an array
	set_uint_a_indx(meta_data, *meta_i_ptr, num_keys);
	*meta_i_ptr += obj_root_sz;
	
	for (size_t i=0; i<num_keys; i++, *meta_i_ptr += obj_member_sz) {
		set_element_type(meta_data, *meta_i_ptr, empty);
	}
	
	set_element_type(meta_data, *meta_i_ptr, extended);
	set_key_offx_value(meta_data, *meta_i_ptr, 0);
	*meta_i_ptr += obj_member_sz;

	if (!is_ext && parent_type == object && !dont_sort_keys) {
//		std::cout << "add_container sorting keys, key count: " << get_key_count(meta_data, id + obj_member_sz) << std::endl;
		sort_keys(&meta_data[id + obj_root_sz + obj_member_sz], 
				&meta_data[id + obj_root_sz + obj_member_sz + (get_key_count(meta_data, id + obj_member_sz) * obj_member_sz)], meta_data, data);
	}

	return id_to_return;
}



int jsonP_json::add_value_type(element_type e_type, object_id id, const char* key, void* value)
{
	element_type container_type = get_element_type(meta_data, id);

	if (container_type != object && container_type != array) {
		std::cerr << "object_id is not an object or and array" << std::endl;
		return -1;
	}
		
	size_t v_len = sizeof(double);
	char temp[64];
	
	if (e_type == string) {
		v_len = strlen((char*)value);
	} else if (e_type == numeric_long && !convert_numerics) {
		sprintf(temp, "%ld", *((long*)value));
		value = temp;
		v_len = strlen((char*)value);
	} else if (e_type == numeric_double && !convert_numerics) {
		sprintf(temp, "%lf", *((double*)value));
		value = temp;
		v_len = strlen((char*)value);
	} 

	bool is_ext = true;
	object_id meta_slot = get_meta_slot(id + obj_member_sz, is_ext, container_type);

	if (container_type == object) {
		size_t k_len = strlen(key);
		data_length = increase_dat_buffer(k_len+v_len+4, data_length, data_i); //, data);
		memcpy(data+data_i, key, k_len+1);
		set_element_type(meta_data, meta_slot, e_type);
		set_key_offx_value(meta_data, meta_slot, data_i);
		data_i += k_len+1;
	} else {
		data_length = increase_dat_buffer(v_len+4, data_length, data_i); //, data);
		set_element_type(meta_data, meta_slot, e_type);
		set_key_offx_value(meta_data, meta_slot, data_i);
	}

	if (convert_numerics && e_type == numeric_double) { //(e_type == numeric_double || e_type == numeric_double_cvt)) {
		*(double*)&data[data_i] = *(double*) value;
		data_i += sizeof(double);
	} else if (convert_numerics && e_type == numeric_long) { //(e_type == numeric_long || e_type == numeric_long_cvt)) {
		*(long*)&data[data_i] = *(long*) value;
		data_i += sizeof(long);
	} else if (e_type == string || e_type == numeric_double || e_type == numeric_long) {
		memcpy(data+data_i, value, v_len+1);
		data_i += v_len+1;
	} 

	if (!is_ext && container_type == object && !dont_sort_keys) {
		unsigned int num_keys = get_key_count(meta_data, id + obj_member_sz);
		id += (obj_root_sz + obj_member_sz);
		sort_keys(&meta_data[id], &meta_data[id + (num_keys * obj_member_sz)], meta_data, data);
	}
	
	return 1;
}



int jsonP_json::update_value(object_id id, index_type container_type, element_type type, void *value)
{
	element_type orig_type = get_elements_type(id);
	
	if (orig_type == object || orig_type == array)
		return -1;
	
	if (type == bool_false) {
		set_element_type(meta_data, id, bool_false);
		return 1;
	} else if (type == bool_true) {
		set_element_type(meta_data, id, bool_true);
		return 1;
	} else if (type == null) {
		set_element_type(meta_data, id, null);
		return 1;
	} else if (type == boolean) {
		bool bool_value = *(bool*)value;
		
		if (bool_value)
			set_element_type(meta_data, id, bool_true);
		else
			set_element_type(meta_data, id, bool_false);
			
		return 1;
	} 
	
	char *v;
	char temp[64];
	size_t len;
	size_t orig_vsize = 0;
	
	if (type == string) {
		v = (char*)value;
		set_element_type(meta_data, id, string);
		len = strlen(v) + 1;
	} else if (type == numeric_double) { 
		if (!convert_numerics) {
			sprintf(temp, "%lf", *((double*)value));
			v = temp;
			set_element_type(meta_data, id, numeric_double);
			len = strlen(v) + 1;
		} else {
			v = (char*)value;
			len = sizeof(double);
			set_element_type(meta_data, id , numeric_double);
			orig_vsize = sizeof(double);
		}
	} else if (type == numeric_long) { 
		if (!convert_numerics) {
			sprintf(temp, "%ld", *(long*)value);
			v = (char*)temp;
			set_element_type(meta_data, id, numeric_long);
			len = strlen(v) + 1;
		} else {
			v = (char*)value;
			len = sizeof(long);
			set_element_type(meta_data, id, numeric_long);
			orig_vsize = sizeof(long);
		}
	} else {
		return -2;
	}
	
	if (container_type == object_key) {
		unsigned int k_loc = get_key_location(meta_data, id);
		size_t k_len = strlen(&data[k_loc]) +1;
		
		if (orig_type == string || ((orig_type == numeric_double || orig_type == numeric_long) && !convert_numerics))
			orig_vsize = strlen((char*)&data[k_loc + k_len]);

		if (len <= orig_vsize) {
			//reuse space in data buffer since it fits
			memcpy(&data[k_loc + k_len], v, len);
		} else {
			//create new space at end of data buffer, and disconnect old space ****dont reclaim for later***
			increase_dat_buffer(len + k_len, data_length, data_i);
			set_key_offx_value(meta_data, id, data_i);
			memcpy(&data[data_i], &data[k_loc], k_len);
			data_i += k_len;
			memcpy(&data[data_i], v, len);
			data_i += len;
		}
	} else {
		unsigned int v_loc = get_val_location(meta_data, id);
		size_t v_len = strlen(&data[v_loc]) +1;
		
		if (len <= v_len) {
			memcpy(&data[v_loc], v, len);
		} else {
			increase_dat_buffer(len + 1, data_length, data_i);
			set_key_offx_value(meta_data, id, data_i);
			memcpy(&data[data_i], v, len);
			data_i += len;
		}
	}
	
	return 1;
}
	
	
int jsonP_json::update_value(search_path_element *path, unsigned int path_count, element_type type, void *value)
{
	index_type parent_type = path[path_count-1].type;
	return update_value(get_object_id(path, path_count), parent_type, type, value);
}
	
int jsonP_json::update_value(const char *path, const char *delim, element_type type, void *value)
{
	object_id parent_id;
	
	return update_value(get_object_id((char*)path, (char*)delim, false, &parent_id), 
		((get_element_type(meta_data, parent_id) == object) ? object_key : array_indx), type, value);
}
	
	
int jsonP_json::delete_value(const char *path, const char *delim, /*char *key,*/  error *err)
{
	object_id parent_container;
	object_id id = get_object_id((char*)path, delim, true, &parent_container);
	
	return delete_value(id, parent_container, /*key,*/ err);
}


int jsonP_json::delete_value(search_path_element *path, unsigned int path_count, /*char *key,*/ error *err)
{
	object_id id = get_object_id(path, path_count, true);
	object_id parent_container;

	if (path_count == 1) {
		parent_container = doc_root;
	} else {
		parent_container = get_object_id(path, path_count-1);
	}
	
	return delete_value(id, parent_container, /*ey,*/ err);
}
	
	
int jsonP_json::delete_value(object_id id, object_id parent_container, /*char *key,*/ error *err)
{
	//if later a map is kept with free memory areas will find length from data but leave for now
	
	if (id == 0) {
		//cant delete root
		*err = not_found;
		return -1;
	}
	
	unsigned int num_keys = get_key_count(meta_data, parent_container + obj_member_sz);
	unsigned int first_ext = parent_container + obj_member_sz + obj_root_sz + (obj_member_sz * num_keys);

//	std::cout << "id: " << id << ", parent_container: " << parent_container << ", num_keys: " << num_keys <<
//			", first_ext: " << first_ext << std::endl;

	if (id > parent_container && id < first_ext) {
		// element to delete is part of sorted set
		//check if there are any ext elements, if so one will need to be moved to sorted set
		bool has_extended = false;
		
		if (get_ext_start(meta_data, first_ext) != 0)
			has_extended = true;
			
		if (has_extended) {
			memcpy(&meta_data[id], 
				&meta_data[*(object_id*)&meta_data[first_ext + obj_member_key_offx]], obj_member_sz);

//			std::cout << "*(object_id*)&meta_data[first_ext + obj_member_key_offx]: " << *(object_id*)&meta_data[first_ext + obj_member_key_offx] << std::endl;
//			std::cout << "*(object_id*)&meta_data[first_ext + obj_member_ext_next_offx]: " << *(object_id*)&meta_data[first_ext + obj_member_ext_next_offx] << std::endl;
//			std::cout << "data_i: " << data_i << std::endl;

			unsigned int temp = *(object_id*)&meta_data[*(object_id*)&meta_data[first_ext + obj_member_key_offx] + obj_member_ext_next_offx];

			memcpy(&meta_data[first_ext + obj_member_key_offx],
				&temp, obj_root_sz);
			
		} else {
			if (get_element_type(meta_data, parent_container) == array || dont_sort_keys) {
				// cant just create a gap in an array find last element and move here
				// set to empty first in case this was the last element and another is not found after
				set_element_type(meta_data, id, empty);
				
				for (object_id x = first_ext-obj_member_sz; x > id; x -= obj_member_sz) {
					if (get_element_type(meta_data, x) != empty) {
						memcpy(&meta_data[id], &meta_data[x], obj_member_sz);
						set_element_type(meta_data, x, empty);
						break;
					}
				}
			} else {
				set_element_type(meta_data, id, empty);
			}
		}
		
		if (num_keys > 0 && get_element_type(meta_data, parent_container) == object && !dont_sort_keys) {
			parent_container += (obj_member_sz + obj_root_sz);
			sort_keys(&meta_data[parent_container], &meta_data[parent_container + (num_keys * obj_member_sz)], 
				meta_data, data);
		}
		
	} else {
		// element to delete is part of ext linked list
		object_id ids_next = get_ext_next(meta_data, id);

		if (ids_next > 0) {
			//not the end of the list, no need to transverse
			memcpy(&meta_data[id], &meta_data[ids_next], obj_member_ext_sz);
		} else {
			object_id end = get_uint_a_indx(meta_data, first_ext + obj_member_key_offx);
			object_id prior = end;

			while (end != id && get_ext_next(meta_data, end) > 0) {
				prior = end;
				end = get_uint_a_indx(meta_data, end + obj_member_ext_next_offx);
			}
			
			memcpy(&meta_data[prior + obj_member_ext_next_offx], &meta_data[id + obj_member_ext_next_offx], obj_root_sz);
		}
		
	}
	
	return 1;
}


element_type jsonP_json::get_value(object_id id, index_type parent_type, const char *& value)
{
	if (parent_type != object_key && parent_type != array_indx) {
		return empty;
	} else if (id == 0) {
		return invalid;
	}
	
	element_type type = get_element_type(meta_data, id);
	unsigned int start = get_key_location(meta_data, id);
	
	if (parent_type == object_key) {
		//eat the key
		char *key = (char*)&data[start];
		start += strlen(key)+1;
	}
	
	if (type == string || type == numeric_double || type == numeric_long || type == boolean) 
		value = (const char*)&data[start];
	
	return type;
}



double jsonP_json::get_double_value(search_path_element *path, unsigned int path_count, error *err)
{
	index_type parent_type = path[path_count-1].type;
	return get_double_value(get_object_id(path, path_count), parent_type, err);
}

double jsonP_json::get_double_value(const char *path, const char *delim, error *err)
{
	object_id parent_container;
	object_id id = get_object_id((char*)path, delim, false, &parent_container);
	
	return get_double_value(id, ((get_element_type(meta_data, parent_container) == object) ? object_key : array_indx), err);
}

double jsonP_json::get_double_value(object_id id, index_type parent_type, error *err) 
{
	const char *value;
	element_type type = get_value(id, parent_type, value);
	
	if (type == numeric_double && !convert_numerics) {
		return parse_float(value);
	} else if (type == numeric_double) {
		return *(double*)value;
	} else if (type == invalid) {
		*err = not_found;
	} else if (type == null) {
		*err = is_null;
	} else {
		*err = not_double;
	}
	
	return 0;
}

double jsonP_json::get_double_value(const char *key, object_id parent_id, error *err)
{
	if (get_element_type(meta_data, parent_id) != object) {
		*err = invalid_container;
		return 0;
	}
	
	parent_id += obj_member_sz;
	unsigned int num_keys = get_key_count(meta_data, parent_id);
	parent_id += obj_root_sz;
	object_id id = search_keys(key, parent_id, (parent_id + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, false, dont_sort_keys);
	
	return get_double_value(id, object_key, err);
}
	
long jsonP_json::get_long_value(search_path_element *path, unsigned int path_count, error *err)
{
	index_type parent_type = path[path_count-1].type;
	return get_long_value(get_object_id(path, path_count), parent_type, err);
}

long jsonP_json::get_long_value(const char *path, const char *delim, error *err)
{
	object_id parent_container;
	object_id id = get_object_id((char*)path, delim, false, &parent_container);
	
	return get_long_value(id, ((get_element_type(meta_data, parent_container) == object) ? object_key : array_indx), err);
}
	
long jsonP_json::get_long_value(object_id id, index_type parent_type, error *err)
{
	const char *value;
	element_type type = get_value(id, parent_type, value);
	
	if (type == numeric_long && !convert_numerics) {
		return parse_long(value);
	} else if (type == numeric_long) {
		return *(long*)value;
	} else if (type == invalid) {
		*err = not_found;
	} else if (type == null) {
		*err = is_null;
	} else {
		*err = not_long;
	}
	
	return 0;
}

long jsonP_json::get_long_value(const char *key, object_id parent_id, error *err)
{
	if (get_element_type(meta_data, parent_id) != object) {
		*err = invalid_container;
		return 0;
	}
	
	parent_id += obj_member_sz;
	unsigned int num_keys = get_key_count(meta_data, parent_id);
	parent_id += obj_root_sz;
	object_id id = search_keys(key, parent_id, (parent_id + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, false, dont_sort_keys);
	
	return get_long_value(id, object_key, err);
}

	
bool jsonP_json::get_bool_value(search_path_element *path, unsigned int path_count, error *err)
{
	index_type parent_type = path[path_count-1].type;
	return get_bool_value(get_object_id(path, path_count), parent_type, err);
}


bool jsonP_json::get_bool_value(const char *path, const char *delim, error *err)
{
	object_id parent_container;
	object_id id = get_object_id((char*)path, delim, false, &parent_container);
	
	return get_bool_value(id, ((get_element_type(meta_data, parent_container) == object) ? object_key : array_indx), err);
}

	
bool jsonP_json::get_bool_value(object_id id, index_type parent_type, error *err)
{
	const char *value;
	element_type type = get_value(id, parent_type, value);
	
	if (type == bool_true) {
		return true;
	} else if (type == bool_false) {
		return false;
	} else if (type == boolean) {
		if (value[0] == '1')
			return true;
		else
			return false;
	} else if (type == invalid) {
		*err = not_found;
	} else if (type == null) {
		*err = is_null;
	} else {
		*err = not_bool;
	}
	
	return false;
}

bool jsonP_json::get_bool_value(const char *key, object_id parent_id, error *err)
{
	if (get_element_type(meta_data, parent_id) != object) {
		*err = invalid_container;
		return false;
	}
	
	parent_id += obj_member_sz;
	unsigned int num_keys = get_key_count(meta_data, parent_id);
	parent_id += obj_root_sz;
	object_id id = search_keys(key, parent_id, (parent_id + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, false, dont_sort_keys);
	
	return get_bool_value(id, object_key, err);
}
	
	
const char* jsonP_json::get_string_value(search_path_element *path, unsigned int path_count, error *err)
{
	index_type parent_type = path[path_count-1].type;
	return get_string_value(get_object_id(path, path_count), parent_type, err);
}


const char* jsonP_json::get_string_value(const char *path, const char *delim, error *err)
{
	object_id parent_container;
	object_id id = get_object_id((char*)path, delim, false, &parent_container);
	
	return get_string_value(id, ((get_element_type(meta_data, parent_container) == object) ? object_key : array_indx), err);
}

	
const char* jsonP_json::get_string_value(object_id id, index_type parent_type, error *err)
{
	const char *value;
	element_type type = get_value(id, parent_type, value);
	
	if (type == string) {
		return value;
	} else if (type == invalid) {
		*err = not_found;
	} else {
		*err = not_string;
	}
	
	return NULL;
}
	
	
const char* jsonP_json::get_string_value(const char *key, object_id parent_id, error *err)
{
	if (get_element_type(meta_data, parent_id) != object) {
		*err = invalid_container;
		return NULL;
	}
	
	parent_id += obj_member_sz;
	unsigned int num_keys = get_key_count(meta_data, parent_id);
	parent_id += obj_root_sz;
	object_id id = search_keys(key, parent_id, (parent_id + (num_keys * obj_member_sz) - obj_member_sz), meta_data, data, false, dont_sort_keys);
	
	return get_string_value(id, object_key, err);
}


unsigned int jsonP_json::get_members_count(object_id id)
{
	if (get_element_type(meta_data, id) == object || get_element_type(meta_data, id) == array) {
		unsigned int cnt = get_key_count(meta_data, id + obj_member_sz);
		object_id ext = get_ext_start(meta_data, id + obj_member_sz + obj_root_sz + (cnt * obj_member_sz));

		if (ext > 0) {
			while (ext > 0) {
				cnt++;
				ext = get_ext_next(meta_data, ext);
			}
		} else {
			//if there is no extension check if there are any empty keys
			ext = id + obj_root_sz + (cnt * obj_member_sz);
			 while (get_element_type(meta_data, ext) == empty) {
				 cnt--;
				 ext -= obj_member_sz;
			 }
		}

		return cnt;
	} else { 
		return 0;
	}
}


unsigned int jsonP_json::get_members_count(search_path_element* path, unsigned int path_count)
{
	return get_members_count(get_object_id(path, path_count));
}


unsigned int jsonP_json::get_members_count(const char *path, const char *delim)
{
	return get_members_count(get_object_id((char*)path, delim));
}


unsigned int jsonP_json::get_keys(search_path_element * path, unsigned int path_count, struct object_key *&keys)
{
	return get_keys(get_object_id(path, path_count), keys);
}


unsigned int jsonP_json::get_keys(const char *path, const char *delim, struct object_key *&keys)
{
	return get_keys(get_object_id((char*)path, delim), keys);
}


unsigned int jsonP_json::get_keys(object_id id, struct object_key *&keys)
{
	if (get_elements_type(id) != object)
		return 0;
		
	unsigned int mem_cnt = get_members_count(id);

	if (mem_cnt == 0)
		return 0;

	keys = (struct object_key *) malloc(sizeof(struct object_key) * mem_cnt);

	unsigned int i = 0;
	id += obj_member_sz;
	unsigned int key_cnt = get_key_count(meta_data, id);
	id += obj_root_sz;
	
	while (i < key_cnt && i < mem_cnt) {
		keys[i].type = get_element_type(meta_data, id);
		
		if (keys[i].type == object_ptr || keys[i].type == array_ptr) {
			keys[i++].key = data + get_uint_a_indx(meta_data, get_key_location(meta_data, id));
		} else {
			keys[i++].key = data + get_key_location(meta_data, id);
		}
		id += obj_member_sz;
	}
	
	if (i < mem_cnt) {
		// id should be at the ext ptr
		id = get_ext_start(meta_data, id);
		
		if (id > 0) {
			while (id > 0) {
				keys[i].type = get_element_type(meta_data, id);
				
				if (keys[i].type == object_ptr || keys[i].type == array_ptr) {
					keys[i++].key = data + get_uint_a_indx(meta_data, get_key_location(meta_data, id));
				} else {
					keys[i++].key = data + get_key_location(meta_data, id);
				}
				id = get_ext_next(meta_data, id);
			}
		} else {
			std::cerr << "jsonP_json::get_keys(object_id, object_key *&) -- SHOULD NOT SEE THIS *****" << std::endl;
		}
	}

	return i;
}


element_type jsonP_json::get_next_array_element(object_id id, const void *&value)
{
	if (id != 0) {
		get_next_array_id = id;
		get_next_array_indx = 0;
		get_next_array_mem_cnt = get_key_count(meta_data, id + arry_member_sz);
		get_next_array_ext_next = get_ext_first(meta_data, id, get_next_array_mem_cnt);
	}
	
	object_id member = 0;
	
	if (get_next_array_mem_cnt > get_next_array_indx) {
		//its part of the set
		member = get_next_array_id + arry_member_sz + arry_root_sz + (arry_member_sz * get_next_array_indx);
	} else {
		//its either an ext or does not exist
		if (get_next_array_ext_next > 0) {
			member = get_next_array_ext_next;
			get_next_array_ext_next = get_ext_next(meta_data, get_next_array_ext_next);
		} else {
			return empty;
		}
	}
	
	const char *val;
	element_type type = get_value(member, array_indx, val);

//	std::cout << "member at index: " << member << ", array_indx: " << get_next_array_indx << ", type: " << type << 
//	", temp_val: " << val << std::endl;	

	get_next_array_indx++;
	
	if (type == string ) {
		value = val;
	} else if (type == numeric_long) {
		if (!convert_numerics)
			*(long*)get_next_array_buf = (const long) parse_long(val);
		else
			*(long*)get_next_array_buf = *(const long*) val; 

		value = get_next_array_buf;
	} else if (type == numeric_double) {
		if (!convert_numerics)
			*(double*)get_next_array_buf = (const double) parse_float(val);
		else
			*(double*)get_next_array_buf = *(const double*) val;
			
		value = get_next_array_buf;
	} else if (type == boolean) {
		if (val[0] == '1')
			type = bool_true;
		else 
			type = bool_false;
	} else if (type == bool_false || type == bool_true || type == null) { 
		//do nothing
	} else if (type == object_ptr) {
		*(object_id*)get_next_array_buf = get_container_loc(meta_data, member) - element_type_sz;
		value = get_next_array_buf;
		type = object;
	} else if (type == array_ptr) {
		*(object_id*)get_next_array_buf = get_container_loc(meta_data, member) - element_type_sz;
		value = get_next_array_buf;
		type = array;
	} else {
		std::cerr << "jsonP_json::get_next_array_element should never see this, type: " << type << std::endl;
		type = empty;
	}

	return type;
}


element_type jsonP_json::get_next_array_element(search_path_element *path, unsigned int path_count, const void *&value)
{
	if (path_count > 0) {
		return get_next_array_element(get_object_id(path, path_count), value);
	} else {
		return get_next_array_element(0, value);
	}
}


element_type jsonP_json::get_next_array_element(const char *path, const char *delim, const void *&value)
{
	if (path == NULL) {
		return get_next_array_element(0, value);
	} else {
		return get_next_array_element(get_object_id((char*)path, delim), value);
	}
}


unsigned int jsonP_json::get_meta_slot(unsigned int start, bool &is_ext, element_type container_type)
{
	unsigned int num_keys = get_key_count(meta_data, start);
	start += obj_root_sz;
	is_ext = false;
	
	if (container_type == object) {
		//if there is an empty spot return it
		if (get_element_type(meta_data, start + ((num_keys-1) * obj_member_sz)) == empty) {
			if (!dont_sort_keys) {
				//keys sorted so return and the caller will call sort 
				return start + ((num_keys-1) * obj_member_sz);
			} else {
				//keys not sorts find first empty
				for (size_t i=0; i<num_keys; i++) {
					if (get_element_type(meta_data, start + (i*1) * obj_member_sz) == empty) {
						return start + (i*1) * obj_member_sz;
					}
				}
			}
		}
	} else {
		//for array start from front and see if there is an empty slot
		for (size_t i=0; i<num_keys; i++) {
			if (get_element_type(meta_data, start + (i*1) * obj_member_sz) == empty) {
				return start + (i*1) * obj_member_sz;
			}
		}
	}
		
	//if all key spots are used fall back to slow linked list
	is_ext = true;
	// make sure there is enough room
	*meta_length_ptr = increase_meta_buffer(obj_member_ext_sz, *meta_length_ptr, *meta_i_ptr);

	
	if (get_element_type(meta_data, start + (num_keys * obj_member_sz)) == extended) {
//		std::cout << "extended 1" << std::endl;

		if (get_key_location(meta_data, start + (num_keys * obj_member_sz)) == 0) {
//			std::cout << "extended 2" << std::endl;
			// adding first extended key/val
			set_key_offx_value(meta_data, start + (num_keys * obj_member_sz), *meta_i_ptr);
			 set_uint_a_indx(meta_data, *meta_i_ptr + obj_member_sz, 0);
			 *meta_i_ptr += obj_member_ext_sz;
			 return *meta_i_ptr - obj_member_ext_sz;
		} else {
			// follow list to end
			object_id end = get_key_location(meta_data, start + (num_keys * obj_member_sz));
			
//			std::cout << "END: " << end << ", type: " << *(element_type*)&meta_data[end] << ", kloc: " <<
//				*(object_id*)&meta_data[end + obj_member_key_offx] << ", next: " << *(object_id*)&meta_data[end + obj_member_sz] << std::endl;

			while (get_uint_a_indx(meta_data, end + obj_member_sz) > 0) {
				end = get_uint_a_indx(meta_data, end + obj_member_sz);
			}

//			std::cout << "final end: " << end << std::endl;

			set_uint_a_indx(meta_data, end + obj_member_sz, *meta_i_ptr);
			set_uint_a_indx(meta_data, *meta_i_ptr + obj_member_sz, 0);
			 *meta_i_ptr += obj_member_ext_sz;
//			std::cout << "extended returning: " << *meta_i_ptr - obj_member_ext_sz << std::endl;
			 
			 return *meta_i_ptr - obj_member_ext_sz;
		}
	} else {
		std::cout << "SHOULD NEVER see this get_meta_slot" << std::endl;
	}
	return 0;
}


long jsonP_json::parse_long(const char *str)
{
	int index = 0;
	char c = str[index];
	bool sign {true};
	long d = 0;
	long mult = 1;

	while (c != '\0') {
		if (c >= '0' && c <= '9') {
			d = d * 10 + ((int)c-48);
		} else if ((c == '-' || c == '+') && sign) {
			if (c == '-') 
				mult = -1;
		} else {
			std::cout << "Error getting long value: " << str << std::endl;
			return 0;
		}
		
		sign = false;
		c = str[++index];
	}
		
	return d * mult;
}


double jsonP_json::parse_float(const char *str)
{
	int index = 0;
	char c = str[index];
	bool sign {true};
	bool exp{false};
	bool exp_sign{false};
	bool dec{true};
	double d = 0;
	double multiplier = 10;
	long mult = 1;
	long exp_mult = 1;
	int e = 0;

	while (c != '\0') {
		if (c >= '0' && c <= '9') {
			exp_sign = false;
			
			if (multiplier == 10) {
				d = d * multiplier + ((int)c-48);
			} else if (multiplier > 0){
				d += multiplier * ((int)c-48);
				multiplier *= .1;
			} else {
				e = e * 10 + ((int)c-48);
			}
		} else if ((c == 'e' || c == 'E') && !exp) {
			exp = true;
			exp_sign = true;
			dec = false;
			multiplier = -1;
		} else if (c == '.' && dec) {
			exp = false;
			multiplier = .1;
		} else if ((c == '-' || c == '+') && (sign || exp_sign)) {
			if (c == '-') {
				if (sign) 
					mult = -1;
				else 
					exp_mult = -1;
			} 

			exp_sign = false;
		} else {
			std::cout << "Error getting float value: " << str << std::endl;
			return 0.0;
		}
		
		sign = false;
		c = str[++index];
	}
		
	if (e > 0) {
		for (size_t i=0; i<e; i++, d *= ((exp_mult == 1) ? 10 : .1));
	}
	
	return d * mult;
}



/*
 * Methods uded for stringify 
 */
char* jsonP_json::stringify(int precision)
{
	element_type type = get_element_type(meta_data, doc_root);
//	std::cout << "\n\nDOC Type: " << type << "\n\n";

	unsigned int len = data_length/4;
	unsigned int i = 0;
	unsigned int meta_i = doc_root + obj_member_sz;

	char *txt = (char*)malloc(len);
	
	char float_format[32];
	float_format[0] = '%';
	float_format[1] = '.';
	sprintf(float_format+2, "%dlf", precision);
	
	if (type == object)
		parse_object(len, meta_i, i, txt, (const char*)float_format);
	else if (type == array)
		parse_array(len, meta_i, i, txt, (const char*)float_format);

	txt[i] = '\0';
	txt = (char*) realloc(txt, i+1);

	return txt;
}


void jsonP_json::parse_object(unsigned int &len, unsigned int &meta_i, unsigned int &i, char *& txt, const char *float_format)
{
	unsigned int k_cnt = get_key_count(meta_data, meta_i);
	meta_i += obj_root_sz;
	txt[i++] = '{';
//	std::cout << "\n\nparse_object key count: " << k_cnt << ", meta_i: " << meta_i << std::endl;
	unsigned int loc;
	unsigned int k_loc;
	size_t k_len;
	size_t v_len;
	element_type type;
	unsigned int j=0;
	bool keep_going = true;
	object_id ext_loc = get_key_location(meta_data, meta_i + (k_cnt * obj_member_sz));
//	std::cout << "ext_loc b4 loop: " << ext_loc << std::endl;

	while (j < k_cnt || keep_going) {
	
		if (j >= k_cnt) {
			if (ext_loc > 0) {
				meta_i = ext_loc;
				ext_loc = get_ext_next(meta_data, meta_i);
//				std::cout << "next ext_loc in loop: " << ext_loc << std::endl;
			} else {
				keep_going = false;
				continue;
			}
		}
	
		type = get_element_type(meta_data, meta_i);
		
		if (type == empty) {
			//all empty slots should be in end
			j = k_cnt;
			continue;
		}
			
		if (j != 0)
			txt[i++] = ',';
		
//		std::cout << "type of ele: " << type << ", meta_i: " << meta_i << std::endl;
		loc = get_key_location(meta_data, meta_i);
		k_loc = loc;
		meta_i += obj_member_sz;
//		std::cout << "k_loc: " << k_loc << std::endl;		

		if (type == object_ptr || type == array_ptr) {
			k_loc = get_uint_a_indx(meta_data, loc);
		}		

		k_len = strlen(data+k_loc);
		txt[i++] = '"';
		
		len = increase_txt_buffer(k_len, len, i, txt);
		memcpy(&txt[i], &data[k_loc], k_len);
		i += k_len;
		
		txt[i++] = '"';
		txt[i++] = ':';
		
		if (type == string) {
			txt[i++] = '"';
			v_len = strlen(data + k_loc + k_len + 1);
			len = increase_txt_buffer(v_len, len, i, txt);
			memcpy(&txt[i], &data[k_loc + k_len + 1], v_len);
			i+=v_len;
			txt[i++] = '"';
		} else if (type == numeric_long || type == numeric_double) {
			if (!convert_numerics) {
				v_len = strlen(data + k_loc + k_len + 1);
				len = increase_txt_buffer(v_len, len, i, txt);
				memcpy(&txt[i], &data[k_loc + k_len + 1], v_len);
				i+=v_len;
			} else if (type == numeric_long) {
				len = increase_txt_buffer(sizeof(long), len, i, txt);
				i += sprintf(&txt[i], "%ld", *(long*)&data[k_loc + k_len + 1]);
			} else {
				len = increase_txt_buffer(sizeof(double), len, i, txt);
				i += sprintf(&txt[i], float_format, *(double*)&data[k_loc + k_len + 1]);
			}
		} else if (type == boolean) {
			if (data[k_loc + k_len + 1] == '1') {
				txt[i++] = 't';
				txt[i++] = 'r';
				txt[i++] = 'u';
				txt[i++] = 'e';
			} else {
				txt[i++] = 'f';
				txt[i++] = 'a';
				txt[i++] = 'l';
				txt[i++] = 's';
				txt[i++] = 'e';
			}
		} else if (type == bool_true) {
			txt[i++] = 't';
			txt[i++] = 'r';
			txt[i++] = 'u';
			txt[i++] = 'e';
		} else if (type == bool_false) {
			txt[i++] = 'f';
			txt[i++] = 'a';
			txt[i++] = 'l';
			txt[i++] = 's';
			txt[i++] = 'e';
		} else if (type == object_ptr) {
			loc += sizeof(object_id);
			parse_object(len, loc, i, txt, float_format);
		} else if (type == object) {
			//do nothing should never happen
		} else if (type == array_ptr) {
			loc += sizeof(object_id);
			parse_array(len, loc, i, txt, float_format);
		} else if (type == null) {
			txt[i++] = 'n';
			txt[i++] = 'u';
			txt[i++] = 'l';
			txt[i++] = 'l';
		}
		
		j++;
	}
	
	txt[i++] = '}';
}


void jsonP_json::parse_array(unsigned int &len, unsigned int &meta_i, unsigned int &i, char *& txt, const char *float_format)
{
	unsigned int num_elements = get_key_count(meta_data, meta_i); 
	meta_i += arry_root_sz;
	size_t v_len;
	unsigned int v_loc;
	element_type type; 
	int k=0;
	bool keep_going = true;
	object_id ext_loc = get_val_location(meta_data, meta_i + (num_elements * arry_member_sz));

	txt[i++] = '[';
	
	while (k < num_elements || keep_going) {
	
		if (k >= num_elements) {
			if (ext_loc > 0) {
				meta_i = ext_loc;
				//assign next ext value
				ext_loc = get_ext_next(meta_data, meta_i);
//				std::cout << "next ext_loc in loop: " << ext_loc << std::endl;
			} else {
				keep_going = false;
				continue;
			}
		}
		
		type = get_element_type(meta_data, meta_i);
		
		if (type == empty) {
			k = num_elements;
			continue;
		}
		
		if (k != 0)
			txt[i++] = ',';
			
		v_loc = get_key_location(meta_data, meta_i);
		meta_i += arry_member_sz;
		
		if (type == string) {
			txt[i++] = '"';
			v_len = strlen(data + v_loc);
			len = increase_txt_buffer(v_len, len, i, txt);
			memcpy(&txt[i], &data[v_loc], v_len);
			i += v_len;
			txt[i++] = '"';
		} else if (type == numeric_long || type == numeric_double) {
			if (!convert_numerics) {
				v_len = strlen(data + v_loc);
				len = increase_txt_buffer(v_len, len, i, txt);
				memcpy(&txt[i], &data[v_loc], v_len);
				i+=v_len;
			} else if (type == numeric_long) {
				len = increase_txt_buffer(sizeof(long), len, i, txt);
				i += sprintf(&txt[i], "%ld", *(long*)&data[v_loc]);
			} else {
				len = increase_txt_buffer(sizeof(double), len, i, txt);
				i += sprintf(&txt[i], float_format, *(double*)&data[v_loc]);
			}
		} else if (type == boolean) {
			if (data[v_loc] == '1') {
				txt[i++] = 't';
				txt[i++] = 'r';
				txt[i++] = 'u';
				txt[i++] = 'e';
			} else {
				txt[i++] = 'f';
				txt[i++] = 'a';
				txt[i++] = 'l';
				txt[i++] = 's';
				txt[i++] = 'e';
			}
		} else if (type == bool_true) {
			txt[i++] = 't';
			txt[i++] = 'r';
			txt[i++] = 'u';
			txt[i++] = 'e';
		} else if (type == bool_false) {
			txt[i++] = 'f';
			txt[i++] = 'a';
			txt[i++] = 'l';
			txt[i++] = 's';
			txt[i++] = 'e';
		} else if (type == object_ptr) {
			v_loc += sizeof(object_id);
			parse_object(len, v_loc, i, txt, float_format);
		} else if (type == object) {
			//nothing - should never happen
		} else if (type == array_ptr) {
			v_loc += sizeof(object_id);
			parse_array(len, v_loc, i, txt, float_format);
		} else if (type == null) {
			txt[i++] = 'n';
			txt[i++] = 'u';
			txt[i++] = 'l';
			txt[i++] = 'l';
		} 
		
		k++;
	}
	
	txt[i++] = ']';
}


char* jsonP_json::stringify_pretty(int precision)
{
	char *raw = stringify(precision); 
	size_t raw_len = strlen(raw);
	unsigned int sz = (unsigned int)strlen(raw) * 1.2;
	char *pretty = (char*)malloc(sz);
	unsigned int pretty_i = 0;
	
	bool indent{false};
	bool parsing_value{false};
	size_t indent_l{0};
	
	for (size_t i{0}; i < raw_len; i++) {
		sz = increase_txt_buffer(100 + indent_l, sz, pretty_i, pretty);
		
		if (raw[i] == '"') {
			
			if (parsing_value) {
				int j{1}, k{2};
				
				while (raw[i-j] == '\\') {
					j++;
					k++;
				}
					
				if (k%2 == 0)
					parsing_value = false;
				else
					parsing_value = true;
			} else {
				parsing_value = true;
			}
			
			pretty[pretty_i++] = raw[i];
		} else if (!parsing_value && (raw[i] == '{' || raw[i] == '[')) {
			indent = true;
			indent_l += 2;
			pretty[pretty_i++] = raw[i];
			pretty[pretty_i++] = '\n';
			
			for (size_t t{0}; t<indent_l; t++)
				pretty[pretty_i++] = ' ';
			
		} else if (!parsing_value && (raw[i] == ']' || raw[i] == '}')) {
			indent_l -= 2;
			indent = (indent_l > 0) ? true : false;
			pretty[pretty_i++] = '\n';
			
			if (indent)
				for (size_t t{0}; t<indent_l; t++)
					pretty[pretty_i++] = ' ';
					
			pretty[pretty_i++] = raw[i];
		} else if (!parsing_value && (raw[i] == ',')) {
			pretty[pretty_i++] = raw[i];
			pretty[pretty_i++] = '\n';
			
			if (indent)
				for (size_t t{0}; t<indent_l; t++)
					pretty[pretty_i++] = ' ';
					
		} else if (!parsing_value && (raw[i] == ':')) {
			pretty[pretty_i++] = raw[i];
			pretty[pretty_i++] = ' ';
		} else {
			pretty[pretty_i++] = raw[i];
		}
	}
	
	pretty[pretty_i] = '\0';
	free(raw);
//	std::cout << sz << ", " << pretty_i << std::endl;
	pretty = (char*) realloc(pretty, pretty_i+1);
	
	return pretty;
}


int jsonP_json::build_search_path(char *path, const char *delim, search_path_element *paths)
{
	char *tok = strtok((char*)path, delim);
	unsigned int i = 0;

	while (tok != NULL) {
		if (tok[0] == 'O') {
			if ((tok = strtok(NULL, delim)) != NULL) {
				paths[i].type = object_key;
				paths[i++].key = tok;
			} else {
				return -1;
			}
		} else if (tok[0] == 'A') {
			if ((tok = strtok(NULL, delim)) != NULL) {
				paths[i].type = array_indx;
				paths[i++].key = tok;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
			
		tok = strtok(NULL, delim);
	}
		
	return 0;
}

