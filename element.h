#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include "jsonP_exception.h"

#include <iostream>
#include <algorithm>


#define get_element_type(buf,indx)			*(element_type*)&(buf)[ (indx) ]
#define set_element_type(buf,indx,typ)		*(element_type*)&(buf)[ (indx) ] = (typ)
#define set_key_offx_value(buf,indx,val)	*(unsigned int*)&(buf)[ (indx) + (obj_member_key_offx) ] = (val)
#define set_uint_a_indx(buf,indx,val)		*(unsigned int*)&(buf)[ (indx) ] = (val)
#define get_key_location(buf,indx)			*(unsigned int*)&(buf)[ (indx) + obj_member_key_offx ]
#define get_val_location(buf,indx)			*(unsigned int*)&(buf)[ (indx) + obj_member_key_offx ]
#define get_uint_a_indx(buf,indx)			*(unsigned int*)&(buf)[ (indx) ]
#define get_key_count(buf,indx)			*(unsigned int*)&(buf)[ (indx) ]
#define get_ext_start(buf,indx)			*(unsigned int*)&(buf)[ (indx) + (obj_member_key_offx) ]
#define get_ext_first(buf,indx,k_cnt)		*(unsigned int*)&(buf)[ (indx) + (obj_member_sz) + (obj_root_sz) + ((k_cnt) * (obj_member_sz)) + (obj_member_key_offx) ]
#define get_ext_next(buf,indx)				*(unsigned int*)&(buf)[ (indx) + (obj_member_ext_next_offx) ]
#define get_container_loc(buf, indx)		*(unsigned int*)&(buf)[ (indx) + (obj_member_key_offx) ]

//#define increase_stack_buffer()			((stack_buf_sz) - (stack_i)) < 50
//#define increase_data_buffer(needed)		(((data_sz) - (data_i)) < (needed))
#define increase_stack_buffer()			(stack_buf_sz) <= ((stack_i) + 50)
#define increase_data_buffer(needed)		((data_sz) <= ((data_i) + (needed))) 

// used for parser options
#define PRESERVE_JSON				1
#define SHRINK_BUFS				PRESERVE_JSON << 1
#define DONT_SORT_KEYS				PRESERVE_JSON << 2
#define WEAK_REF					PRESERVE_JSON << 3
#define CONVERT_NUMERICS			PRESERVE_JSON << 4


typedef char byte;

enum element_type : u_int8_t {object_ptr=0, object=1, string=2, numeric_int=3, numeric_long=4, numeric_double=5, 
								array_ptr=6, array=7, boolean=8, null=9, extended=10, empty=11, bool_true=12, 
								bool_false=13, search=14, invalid=15/*, numeric_long_cvt=16, numeric_double_cvt=17*/};

static const size_t element_type_sz = 1;
static const size_t obj_member_sz = sizeof(element_type) + sizeof(unsigned int);
static const size_t obj_member_ext_sz = obj_member_sz + sizeof(unsigned int);
static const size_t obj_root_sz = sizeof(unsigned int);
static const size_t arry_member_sz = obj_member_sz;
static const size_t arry_member_ext_sz = arry_member_sz + sizeof(unsigned int);
static const size_t arry_root_sz = obj_root_sz;
static const size_t obj_member_key_offx = sizeof(element_type);
static const size_t obj_member_ext_next_offx = obj_member_sz;
static const size_t arry_member_val_offx = sizeof(element_type);
static const size_t arry_member_ext_next_offx = arry_member_sz;


// used to partition each element used during sorting
struct obj_member
{
	byte b[obj_member_sz];      // <-- hardcode for now, see if I can change this with define if else ???
//	byte b[5];
};


static void sort_keys(void *start, void *end, byte *meta, byte *data)
{
	unsigned int lft;
	unsigned int rt;
//	byte * text = (use_json) ? json : data;

	std::sort((obj_member*)start, (obj_member*)end, [&](obj_member l, obj_member r) { 
//		std::cout << "l type: " << *(element_type*)&l.b[0] << ", R type: " << *(element_type*)&r.b[0] << std::endl;
		
//		if (*(element_type*)&l.b[0] == empty)
		if (get_element_type(l.b, 0) == empty)
			return false;
				
//		if (*(element_type*)&r.b[0] == empty)
		if (get_element_type(r.b, 0) == empty)
			return true;
			
//		lft = get_key_location(l.b, obj_member_key_offx);
//		rt = get_key_location(r.b, obj_member_key_offx);
		lft = get_key_location(l.b, 0);
		rt = get_key_location(r.b, 0);

		if (get_element_type(l.b, 0) == object_ptr || get_element_type(l.b, 0) == array_ptr) {
			lft = get_uint_a_indx(meta, lft);
		}
						
		if (get_element_type(r.b, 0) == object_ptr || get_element_type(r.b, 0) == array_ptr) {
			rt = get_uint_a_indx(meta, rt);
		}

//		std::cout << "strcmp(" << data+lft << ", " << data+rt << ") = " << std::strcmp(data+lft, data+rt) << std::endl;

		return std::strcmp(data+lft, data+rt) < 0;
	});
}



static unsigned int search_keys(const char *key, unsigned int start, unsigned int end, byte *meta, byte *data, 
									bool ret_ptr, bool dont_sort_keys)
{
//std::cout << "key: " << key << ", start: " << start << ", end: " << end << ", dont_sort: " << dont_sort_keys << std::endl;
	unsigned int mid; // = (((end - start) / sizeof(obj_member)) / 2) * sizeof(obj_member) + start;
	unsigned int ext = get_ext_start(meta, end + obj_member_sz);
	unsigned int key_cmp;
	element_type type;
	int result;
	
	if (!dont_sort_keys) {
		//keys are sort binary search
		while (start <= end) {
			mid = (unsigned int)(((end - start) / sizeof(obj_member)) / 2) * sizeof(obj_member) + start;
//std::cout << "Mid: " << mid << std::endl;
			type = get_element_type(meta, mid);

			if (type == empty) {
				end = mid - sizeof(obj_member);
				continue;
			}
				
//			key_cmp = get_key_location(meta, mid + obj_member_key_offx);
			key_cmp = get_key_location(meta, mid);
//			std::cout << "--key_cmp: " << key_cmp << std::endl;

			if ( type == object_ptr || type == array_ptr)
//				key_cmp = get_key_location(meta, key_cmp);
				key_cmp = get_uint_a_indx(meta, key_cmp);
			
//			std::cout << "key_cmp = " << key_cmp << ", char: " << data+key_cmp << std::endl;
			result = std::strcmp(key, data+key_cmp);
//			std::cout << "start: " << start << ", mid: " << mid << ", end: " << end << ", result: " << result << std::endl;

			if (result == 0) {
				//found
				if ((type == object_ptr || type == array_ptr) && !ret_ptr) {
//					return get_key_location(meta, mid + obj_member_key_offx) - element_type_sz;
					return get_key_location(meta, mid) - element_type_sz;
				} else {
					return mid;
				}
			} else if (result < 0) {
				end = mid - sizeof(obj_member);
			} else {
				start = mid + sizeof(obj_member);
			}
		}
	} else {
		//keys not sorted go through each
		while (start <= end) {
			type = get_element_type(meta, start);
			key_cmp = get_key_location(meta, start);
			
			if ( type == object_ptr || type == array_ptr)
				key_cmp = get_uint_a_indx(meta, key_cmp);
				
			result = std::strcmp(key, data+key_cmp);

			if (result == 0) {
				//found
				if ((type == object_ptr || type == array_ptr) && !ret_ptr)
					return get_key_location(meta, start) - element_type_sz;
				else
					return start;
			} else {
				start += obj_member_sz;
			}
		}
	}
	
	while (ext > 0) {
		type = get_element_type(meta, ext);
//		key_cmp = get_key_location(meta, ext + obj_member_key_offx);
		key_cmp = get_key_location(meta, ext);

		if ( type == object_ptr || type == array_ptr)
			key_cmp = get_uint_a_indx(meta, key_cmp);

		result = std::strcmp(key, data+key_cmp);

		if (result == 0) {
			//found
			if ((type == object_ptr || type == array_ptr) && !ret_ptr)
//				return get_key_location(meta, ext + obj_member_key_offx) - element_type_sz;
				return get_key_location(meta, ext) - element_type_sz;
			else
				return ext;
		} else {
			ext = get_ext_next(meta, ext);
		}
	}
	
	return 0;
}


static unsigned int search_keys(const char *key, unsigned int start, unsigned int end, byte *meta, byte *data, 
									bool ret_ptr)
{
	return search_keys(key, start, end, meta, data, ret_ptr, false);
}

static std::string get_element_type_string(element_type type)
{
	switch (type)
	{
		case object_ptr:
			return "object pointer";
		case object:
			return "object";
		case string:
			return "string";
		case numeric_long:
//		case numeric_long_cvt:
			return "numeric long";
		case numeric_int:
			return "numeric int";
		case numeric_double:
//		case numeric_double_cvt:
			return "numeric double";
		case array_ptr:
			return "array pointer";
		case array:
			return "array";
		case boolean:
			return "boolean";
		case null:
			return "null";
		case bool_true:
			return "boolean true";
		case bool_false:
			return "boolean false";
		case extended:
			return "extended";
		case empty:
			return "empty";
		case invalid:
			return "invalid";
		case search:
			return "search";
		default:
			return "invalid";
	}
}


#endif // _ELEMENT_H_
