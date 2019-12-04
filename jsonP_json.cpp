#include "jsonP_json.h"



jsonP_json::jsonP_json(byte *data, byte *meta_data, unsigned long data_length, unsigned int doc_root) : 
data{data},
meta_data{meta_data}, 
data_length{data_length},
doc_root{doc_root}
{
	
}

jsonP_json::~jsonP_json()
{
	if (data != meta_data) {
		free(data);
	} 
	
	free(meta_data);
}


char* jsonP_json::stringify()
{
	//guess with about 1/2 size of data length
	unsigned int len = data_length/3;
	unsigned int i = 0;
	unsigned int meta_i = doc_root;
	char *txt = (char*)malloc(len);
	
	parse_object(len, meta_i, i, txt);
	txt[i] = '\0';
	txt = (char*) realloc(txt, i+1);
	
	return txt;
}


void jsonP_json::parse_object(unsigned int &len, unsigned int &meta_i, unsigned int &i, char *& txt)
{
	unsigned int k_cnt = get_key_count(meta_data, meta_i);
	meta_i += obj_root_sz;
	txt[i++] = '{';
	
	unsigned int loc;
	unsigned int k_loc;
	size_t k_len;
	size_t v_len;
	element_type type;
	
	for (int j=0; j<k_cnt; j++) {
		if (j != 0)
			txt[i++] = ',';
			
		type = get_element_type(meta_data, meta_i);
		loc = get_key_location(meta_data, meta_i + obj_member_key_offx);
		k_loc = loc;
		meta_i += obj_member_sz;
		
		if (type == object_ptr || type == array_ptr)
			k_loc = get_key_location(meta_data, loc);
			
		k_len = strlen(data+k_loc);
		txt[i++] = '"';
		
		increase_buffer(k_len, len, i, txt);
		memcpy(&txt[i], &data[k_loc], k_len);
		i += k_len;
		
		txt[i++] = '"';
		txt[i++] = ':';
		
		if (type == string) {
			txt[i++] = '"';
			v_len = strlen(data + k_loc + k_len + 1);
			increase_buffer(v_len, len, i, txt);
			memcpy(&txt[i], &data[k_loc + k_len + 1], v_len);
			i+=v_len;
			txt[i++] = '"';
		} else if (type == numeric_long || type == numeric_double) {
			v_len = strlen(data + k_loc + k_len + 1);
			increase_buffer(v_len, len, i, txt);
			memcpy(&txt[i], &data[k_loc + k_len + 1], v_len);
			i+=v_len;
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
		} else if (type == object_ptr) {
			loc += 4;
			parse_object(len, loc, i, txt);
		} else if (type == object) {
			//do nothing should never happen
		} else if (type == array_ptr) {
			loc += 4;
			parse_array(len, loc, i, txt);
		} else if (type == null) {
			txt[i++] = 'n';
			txt[i++] = 'u';
			txt[i++] = 'l';
			txt[i++] = 'l';
		}
	}
	
	txt[i++] = '}';
}


void jsonP_json::parse_array(unsigned int &len, unsigned int &meta_i, unsigned int &i, char *& txt)
{
	unsigned int num_elements = get_key_count(meta_data, meta_i); 
	meta_i += arry_root_sz;
	size_t v_len;
	unsigned int v_loc;
	element_type type; 
	
	txt[i++] = '[';
	
	for (int k=0; k<num_elements; k++) {
		if (k != 0)
			txt[i++] = ',';
			
		type = get_element_type(meta_data, meta_i);
		v_loc = get_key_location(meta_data, meta_i + arry_member_val_offx);
		meta_i += arry_member_sz;
		
		if (type == string) {
			txt[i++] = '"';
			v_len = strlen(data + v_loc);
			increase_buffer(v_len, len, i, txt);
			memcpy(&txt[i], &data[v_loc], v_len);
			i += v_len;
			txt[i++] = '"';
		} else if (type == numeric_long || type == numeric_double) {
			v_len = strlen(data + v_loc);
			increase_buffer(v_len, len, i, txt);
			memcpy(&txt[i], &data[v_loc], v_len);
			i+=v_len;
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
		} else if (type == object_ptr) {
			v_loc += 4;
			parse_object(len, v_loc, i, txt);
		} else if (type == object) {
			//nothing - should never happen
		} else if (type == array_ptr) {
			v_loc += 4;
			parse_array(len, v_loc, i, txt);
		} else if (type == null) {
			txt[i++] = 'n';
			txt[i++] = 'u';
			txt[i++] = 'l';
			txt[i++] = 'l';
		} 
	}
	
	txt[i++] = ']';
}


char* jsonP_json::stringify_pretty()
{
	char *raw = stringify();
	size_t raw_len = strlen(raw);
	unsigned int sz = (unsigned int)strlen(raw) * 1.2;
	char *pretty = (char*)malloc(sz);
	unsigned int pretty_i = 0;
	
	bool indent{false};
	bool parsing_value{false};
	size_t indent_l{0};
	
	for (size_t i{0}; i < raw_len; i++) {
		
//		if (i%15 == 0)
		increase_buffer(100, sz, pretty_i, pretty);
		
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
	std::cout << sz << ", " << pretty_i << std::endl;
	pretty = (char*) realloc(pretty, pretty_i+1);
	
	return pretty;
}