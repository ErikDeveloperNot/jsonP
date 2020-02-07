# jsonP
jsonP is a semi compliant c++ json parser/constructor that performs fast in my comparison tests to other c/c++ parsers with a smaller memory footprint.  

jsonP has 3 different types of parsers that can be configured with a number of options.
- Standard parser which requires a char* to an in memory json document
- Buffered parser allows to parse a document being buffered in from disk or network
- Push parser which uses the buffer parser and relies on a call backs to only return elements of interest  
  
---
### Performance Comparison Charts  
##### System Tested On:
Intel Core i7, 2.2 GHz  
1 processor, 4 cores  
Memory 16GB  
Apple LLVM version 9.1.0 (clang-902.0.39.2)
c++17 compiled with -O3 as a dynamic library  
  
All tests were run with the same driver program under the same conditions. The non jsonP parsers are configured based of documentation from their sites, however their maybe ways to tweak their performance that I am not aware of. For test 1 ([Chart 1](#chart-1)) a 1.5GB json text file (many objects/arrays with many float numerics and strings) is loaded into memory before parsing, except with the jsonP buffer/push parsers which parse while reading in the json. For test 2 ([Chart 2](#chart-2)) a 3.4kb webapp.json file is parsed/destroyed 1000 times.
  
#### Chart 1
| Parser                                                                                                       | Memory b4 parse | Memory aft parse                                                                                             | Parse time | Stringify time | Pretty stringify time | free/delete time |
|--------------------------------------------------------------------------------------------------------------|-----------------|--------------------------------------------------------------------------------------------------------------|------------|----------------|-----------------------|------------------|
| [**RapidJSON**](https://github.com/miloyip/rapidjson) -not sure if keys sorted -destroys original json       |     1471 MB     |                                                    2165 MB                                                   |  2s 818ms  |     2s 6ms     |           --          |        --        |
| [**SimpleJSON**](https://github.com/MJPA/SimpleJSON)                                                         |     1471 MB     |                                                    3877 MB                                                   | 28s 593ms  |       --       |       53s 461ms       |     7s 395ms     |
| [**cJson**](https://github.com/DaveGamble/cJSON)                                                             |     1471 MB     |                                                    3175 MB                                                   |  8s 127ms  |       --       |        11s 85ms       |       65ms       |
| [**sajson**](https://github.com/chadaustin/sajson) -not sure if keys sorted -destroys original json          |     1471 MB     |                                                    2001 MB                                                   |  2s 675ms  |       --       |           --          |        --        |
|                                                                                                              |                 |                                                                                                              |            |                |                       |                  |
| **jsonP standard** -don't sort keys -destroy original json                                                   |     1471 MB     |                                                    1736 MB                                                   |  2s 328ms  |      769ms     |        2s 736ms       |       87ms       |
| **jsonP standard** -sort keys -destroy original json                                                         |     1471 MB     |                                                    1736 MB                                                   |  4s 293ms  |    1s 759ms    |        3s 617ms       |       77ms       |
| **jsonP standard** -preserve orig json, don't sort keys                                                      |     1471 MB     |                                                    2139 MB                                                   |  2s 796ms  |      749ms     |        2s 704ms       |       40ms       |
| **jsonP standard** -convert numerics (implicitly preserve orig json), don't sort keys                        |     1471 MB     |                                                    2003 MB                                                   |  2s 711ms  |    2s 776ms    |        4s 644ms       |       28 ms      |
|                                                                                                              |                 |                                                                                                              |            |                |                       |                  |
| **jsonP buffer parser** (json parsed while read from a file) -don't sort keys.  8k buffer used to read file. |      360 KB     |                                                    483 MB                                                    |  3s 301ms  |      804ms     |        2s 656ms       |                  |
| **jsonP buffer parser** (json parsed while read from a file) -sort keys. 8k buffer used to read file.        |      360 KB     |                                                    446 MB                                                    |  5s 195ms  |     1s 88ms    |        2s 939ms       |                  |
| **jsonP push parser** (return false for all keys, so pull no elements, just parse), use an 8k buffer.        |      340 KB     |                          572 KB (this is the largest memory footprint during parse)                          |  4s 899ms  |                |                       |                  |
| **jsonP push parser** (return true for 9838 keys, pulls 59 MB of data in total), use an 8k buffer.           |      340 KB     | 784 KB  (this is the largest memory foot print during parse. After each object is pulled out it is deleted)  |  5s 935ms  |                |                       |                  |
  
#### Chart 2
| Parser                                                | Time to parse 1000 times |
|-------------------------------------------------------|:------------------------:|
| [**RapidJSON**](https://github.com/miloyip/rapidjson) |           34ms           |
| [**SimpleJSON**](https://github.com/MJPA/SimpleJSON)  |           190ms          |
| [**cJson**](https://github.com/DaveGamble/cJSON)      |           39ms           |
| [**sajson**](https://github.com/chadaustin/sajson)    |           21ms           |
| **jsonP standard** -don't sort keys                   |           21ms           |
| **jsonP standard** -sort keys                         |           24ms           |

  
---
  
## Usage
[Standard Parser](#standard-parser)  
[Buffer Parser](#buffer-parser)  
[Push Parser](#push-parser)  
[jsonP_json](#jsonP_json)   

*A sample driver program demostrating much of the functionality can be found at [**jsonP_dyn_drvr**](https://github.com/ErikDeveloperNot/jsonP_dyn_drvr)*
  
### Standard Parser
Create a parser object with one of two constructors  
```c++
jsonP_parser(std::string &json, unsigned short options = 0);
jsonP_parser(char *json, unsigned int json_len, unsigned short options = 0);
```
Options include:
- **PRESERVE_JSON** *(allocates space for all meta and element data leaving the original json unmodified)*
- **SHRINK_BUFS** *(on parsing completion shrink meta and data buffers to free unused space, requires extra realloc call)*
- **DONT_SORT_KEYS** *(add performance when not sorting object keys, trade off is searches are done as linked list)*
- **CONVERT_NUMERICS** *(the default is to not convert numerics until accessed. converting numerics during parse implicitly means PRESERVE_JSON)*
  
Options can be or'd together
```c++
jsonP_parser parser(my_json, strlen(my_json), PRESERVE_JSON | DONT_SORT_KEYS);
```
parse can then be called which will return a jsonP_json object.
```c++
#include "jsonP_parser.h"

jsonP_parser *parser = new jsonP_parser(my_json, strlen(my_json), PRESERVE_JSON | DONT_SORT_KEYS);
jsonP_json *json_doc = parser->parse();

delete parser;
delete json_doc;
```
Both the parser **jsonP_parser** and the document object `jsonP_json` need to be freed separately.
### Buffer Parser
The **jsonP_buffer_parser** allows parsing of a json while reading it from disk or another source. There are two constructors available, one that takes a `std::string` for a file name/path and another that takes a class that implements the [IChunk_reader](#ichunk_reader) interface. Both constructors take an int paramter for the buffer size (min 1024), the samller the buffer the more reads that need to be done. Both constructors also take an unsigned short for extra options.
```c++
jsonP_buffer_parser(std::string file_name, int buf_sz, unsigned short options_ = 0);
jsonP_buffer_parser(IChunk_reader *reader, int buf_sz, unsigned short options_ = 0);
```
Options include:
- **DONT_SORT_KEYS** *(add performance when not sorting object keys, trade off is searches are done as linked list)*
- **CONVERT_NUMERICS** *(the default is to not convert numerics until accessed. converting numerics during parse implicitly means PRESERVE_JSON)*
  
parse can then be called which will return a `jsonP_json` object.
```c++
#include "jsonP_buffer_parser.h"

jsonP_buffer_parser *buf_parser = new jsonP_buffer_parser{"/dir1/large.json", 8192};
jsonP_json *doc_buf = buf_parser->parse();

delete buf_parser;
delete doc_buf;
```
*note: if a class implementing IChunk_reader is used, when the jsonP_buffer_parser is deleted it will also delete the reader interface*
### Push Parser
The **jsonP_push_parser** is a parser that uses the buffer parser to parse a document while read. Unlike the buffer parser which produces a full `jsonP_json` object model of the raw json, the push parser relies on a supplied [IPush_handler](#ipush_handler) to handle call backs as each element is parsed. See the `IPush_handler` section for more details.  
There are two constructors available. Both take an instance of a class that implements the `IPush_handler`, as well as a buffer size (min 1024) to be used with the buffer parser. Like the buffer parser either a `std::string` for a file name/path can be supplied or a class that implements the [IChunk_reader](#ichunk_reader) interface.
```c++
jsonP_push_parser(std::string file_name, IPush_handler *handler, int buf_sz);
jsonP_push_parser(IChunk_reader* reader, IPush_handler *handler, int buf_sz);
```
parse can then be called, no `jsonP_json` object will be returned since the supplied `IPush_handler` will handle all the call backs. Both parser and the `IPush_handler` need to be deleted, if an `IChunk_reader` was supplied it will be deleted when the parser is deleted.
```c++
#include "jsonP_push_parser.h"

IPush_handler *handler = new test_push_handler{};
jsonP_push_parser *push_parser = new jsonP_push_parser{"../samples/sample1.json", handler};
push_parser->parse();
	
delete push_parser;
delete handler;
```
---
# jsonP_json
The `jsonP_json` class represents a json document object model. A pointer to a `jsonP_json` object is returned after parsing or can be used to create a json document. The two constructors available are
```c++
jsonP_json(element_type type, unsigned int element_cnt, unsigned int buf_sz = 102400, unsigned short options = 0);
jsonP_json(const jsonP_json &other);
```
The first takes a type for the json, either `object` or `array`, the number of elements, the size of a buffer to use (for large jsons starting off with a large buffer will limit the number of `reallocs` if performance is important), and parser options.
  
Options include:
- **DONT_SORT_KEYS** *(add performance when not sorting object keys, trade off is searches are done as linked list)*
- **CONVERT_NUMERICS** *(the default is to not convert numerics until accessed. converting numerics during parse implicitly means PRESERVE_JSON)*
Options can be Or'd together `DONT_SORT_KEYS | CONVERT_NUMERICS`
  
The second takes another `jsonP_json` and makes a copy.
  
##### Note on objects and arrays
*When creating an object/array, the number of elements it will contain is specified. If keys are sorted, the default, this means that if at a later time more elements need to be added to the object/array those elements will be added to a linked list and will not be sorted in the case of an object. This can slow down access times the longer this list gets.*
  
*When a json text file is parsed all the objects and arrays will be created with the exact number of elements contained in the json. If more elements are added after parse those elements will be added to the linked list of that object/array.*
  
## jsonP_json methods
[add_container](#add_container)  
[add_value_type](#add_value_type)  
[update_value](#update_value)  
[delete_value](#delete_value)  
[get_object_id](#get_object_id)  
[get_members_count](#get_members_count)  
[get_keys](#get_keys)  
[get_next_array_element](#get_next_array_element)  
[get_elements_type](#get_elements_type)  
[get_value](#get_value)  
[stringify](#stringify)  
[element_type enum](#element_type-enum)  
[error enum](#error-enum)  


---
### add_container
Adds an object/array element to an existing object/array element and returns the new elements object_id.
```c++
object_id add_container(const char* key, unsigned int num_keys, object_id id, element_type container_type);
```
**key** - if the parent element is an object a key name needs to be provided, if the parent is an array pass `NULL`. 
**num_keys** - number of elements the new container will hold.  
**id** - the `object_id` of the parent object/array.  
**container_type** - the type of element of the new container, `object` or `array`.  

---
### add_value_type
Add a non container type of element. Returns 1 on success -1 on failure.
```c++
int add_value_type(element_type e_type, object_id id, const char* key, void* value);
```
**e_type** - the element type.  
**id** - the object__id of the parent containter.  
**key** - if the parent element is an object the name of the new elements key, NULL if the parent is an array.  
**value** - pointer to the value of the new element. The pointer value will be copied.  

---
### update_value
Updates an existing elements value. Returns 1 on success.  
```c++
int update_value(object_id element_id, index_type indx_type, element_type type, void *value);
int update_value(search_path_element *path, unsigned int cnt, element_type type, void *value);
int update_value(const char *path, const char *delim, element_type type, void *value);
```
**element_id** - the object_id of the element being updated.  
**index_type** - if the elements parent is an object use `object_key`, if it an array use `array_indx`.  
**type** - the type of element being updated.  
**value** - pointer to the new value. The pointers contents will be copied.  

For the search_path_element version see [using search_path_element](#using-search_path_element).  
For the path/delim version see [using json path](#using-json-path).  

---
### delete_value
Removes an existing element, returns 1 on success.  
```c++
int delete_value(search_path_element *path, unsigned int cnt, error *err);
int delete_value(const char *path, const char *delim, error *err);
int delete_value(object_id id, object_id parent, error *err);
```
**id** - object_id of the element to delete.  
**parent** - object_id of the parent container.  
**err** - pointer to an error struct. On a failure will contain the error message.  
  
For the search_path_element version see [using search_path_element](#using-search_path_element).  
For the path/delim version see [using json path](#using-json-path).  
  
---
### get_object_id
`get_doc_root` gets the root element object_id of the json. The `get_object_id` methods get an elements object_id. Returns `0` if the element is not found.  
```c++
object_id get_doc_root();
object_id get_object_id(search_path_element *path, unsigned int cnt);
object_id get_object_id(const char *path, const char *delim);
```
For the search_path_element version see [using search_path_element](#using-search_path_element).  
For the path/delim version see [using json path](#using-json-path).  
  
---
### get_members_count
Get the number of elements in an object or array.  
```c++
unsigned int get_members_count(object_id id);
unsigned int get_members_count(search_path_element *path, unsigned int cnt);
unsigned int get_members_count(const char *path, const char *delim);
```
**id** - object_id of the object/array element to get the number of elements for.  

For the search_path_element version see [using search_path_element](#using-search_path_element).  
For the path/delim version see [using json path](#using-json-path).  

---
### get_keys
Get the keys of an element object. The keys are populated in the supplied object_key struct pointer. Returns the number of keys.  
```c++
struct object_key
{
	char *key;
	element_type type;
};

unsigned int get_keys(object_id id, struct object_key *& keys);
unsigned int get_keys(search_path_element *path, unsigned int cnt, struct object_key *& keys);
unsigned int get_keys(const char *path, const char *delim, struct object_key *& keys);
```
**id** - object_id of the object element to get the keys for.  
**keys** - an object_key struct pointer that will be populated with the keys.  
  
For the search_path_element version see [using search_path_element](#using-search_path_element).  
For the path/delim version see [using json path](#using-json-path).  
  
---
### get_next_array_element
The `get_next_array_element` method is a way to iterate through an array of elements. On the first call use any of the three versions supplying the information for the array. On the second and subsequent calls use the object_id version of the method and supply `0` for the id. All methods require a void pointer that will be set to point to the value of the next element. The method returns the type of element the pointer points to. When the all elements have been iterated through the returned element type will be `empty`. Note an internal memory buffer is used to hold the value that the pointer points to. This memory buffer might be overwritten on subsequent calls, so values that are needed should be copied.  
```c++
element_type get_next_array_element(object_id id, const void *& value);
element_type get_next_array_element(search_path_element *path, unsigned int cnt, const void *& value);
element_type get_next_array_element(const char *path, const char *delim, const void *& value);
```
**id** - object_id of the array element to iterate through. *only for the first call, see above.*  
**value** - void pointer that will be set with next element.  
  
For the search_path_element version see [using search_path_element](#using-search_path_element).  
For the path/delim version see [using json path](#using-json-path).  
  
---
### get_elements_type
Return the type of an element. If the object_id or search_path_element is not valid returns `invalid` for the element_type.  
```c++
element_type get_elements_type(object_id id);
element_type get_elements_type(search_path_element *path, unsigned int path_count);
```
**id** - the object_id of the elements whose type will be returned.  
  
For the search_path_element version see [using search_path_element](#using-search_path_element).  
  
---
### get_value
There are four different types of get value methods, `get_string_value`, `get_long_value`, `get_double_value`, `get_bool_value`. Each of these come in four different versions. Below only the get_string shows the 4 versions, but all 4 access methods have the same four versions.  
```c++
enum index_type : u_int8_t { object_key=0, array_indx=1 };

const char* get_string_value(search_path_element *path, unsigned int cnt, error *err);
const char* get_string_value(const char *path, const char *delim, error *err);
const char* get_string_value(object_id id, index_type type, error *err);
const char* get_string_value(const char *key, object_id parent, error *err);

long get_long_value(object_id id, index_type type, error *err);
double get_double_value(object_id id, index_type type, error *err);
bool get_bool_value(object_id id, index_type type, error *err);
```
For the `get_string_value(object_id id, index_type type, error *err)` version  
**id** - the object_id of the element whose value is to be returned.  
**type** - index_type is the parent container type of the element. Valid value is `object_key` only.  
**err** - pointer to an error struct. On a failure will contain the error message.  
  
For the `get_string_value(const char *key, object_id parent, error *err)` version  
**key** - the elements key name.  
**parent** - the object_id of the parent container.  
**err** - pointer to an error struct. On a failure will contain the error message.  
*Note this version of the method only works if the parent container is an object, not with arrays.*  
  
For the search_path_element version see [using search_path_element](#using-search_path_element).  
For the path/delim version see [using json path](#using-json-path).  
  
---
### stringify
stringify and stringify_pretty methods create txt representation of the json model. The stringify method creates a compact version with no white space or returns, stringify_pretty creates a formatted version. In both versions the returned pointer needs to be freed by the caller when no longer needed.  
```c++
char * stringify(int precision = 6);
char * stringify_pretty(int precision = 6);
```
If numerics are set to be converted, not the default, then there is the option to provide the precision of floats when the stringify methods are called. The stringify_pretty methods has a performance penalty when compared to stringify.  
  
---
### using search_path_element  
Many of the access/manipulate methods have a version that take a pointer to an array of `search_path_element`. One method is the `const char* get_string_value(search_path_element *path, unsigned int cnt, error *err)` which will be used as an example.  
```c++
enum index_type : u_int8_t { object_key=0, array_indx=1 }; 

struct search_path_element 
{
	index_type type;
	const char * key;
};
```
A `search_path_element` consists of an `index_type` and a char pointer which points to either an object key or array index. For the json:  
```json
{
  "key_1" : "string_1",
  "key_2" : {
    "key_3" : 88237755,
    "key_4" : [
      "value_1", "value_2"
    ]
  }
}
```
In order to get the key_4 array element number 1 a search_path_element would be built like the following  
```c++
error err;
search_path_element p[3];
p[0].type = object_key;
p[0].key = "key_2";
p[1].type = object_key;
p[1].key = "key_4";
p[2].type = array_indx;
p[2].key = "1";

json.get_string_value(p, 3, &err);
```
This would return **value_2**  
  
---
### using json path  
Many of the access/manipulate methods have a version that take a char pointer to a key and a delim char. One method is the `const char* get_string_value(const char *path, const char *delim, error *err)` which will be used as an example.  
For the json:  
```json
{
  "key_1" : "string_1",
  "key_2" : {
    "key_3" : 88237755,
    "key_4" : [
      "value_1", "value_2"
    ]
  }
}
```
In order to get the key_4 array element number 1 a search_path_element would be built like the following  
```c++
error err;
json.get_string_value("/key_2/key_4/1", "/", &err)l
```
This would return **value_2**  
  
---  
## Interfaces
### IChunk_reader
An instance of the **IChunk_reader** interface is used by the buffer parser to handle feeding a json to the parser. It consists of a single method to implement and destructor.
```c++
class IChunk_reader
{
public:
	virtual ~IChunk_reader(){}
	// user implemented callback, 'buf' is a char[] passed to the callback; 'cnt' is the max number of chars to insert.
	// adding '\0' is not needed; 
	// either returns the number of chars inserted, 0 when the stream is done and parsing can complete, negative value
	//		on error.
	virtual int get_next(char * buf, int cnt) = 0;
};
```
The `get_next()` method is called each time the parser needs more data. The `cnt` variable indicates the max number for chars to copy to the supplied `buf`. A null character does not need to be added to the end and the number of characters copied to the `buf` is returned. When there is no more data to provide to the parser a 0 should be returned. If an error occurs a -1 should be returned to the parser so it can exit. The pointer to `buf` should not be copied or used outside of the callback method `get_next()`. It is created and destroyed by the parser. An example of a class that implements this interface is `file_chunk_impl`, [file_chunk_impl.h](https://github.com/ErikDeveloperNot/jsonP_dyn/blob/master/file_chunk_impl.h)/[file_chunk_impl.cpp](https://github.com/ErikDeveloperNot/jsonP_dyn/blob/master/file_chunk_impl.cpp)
  
### IPush_handler
An instance of the **IPush_handler** is used by the push parser to handle events while parsing a json. It consists of two methods to implement and a destructor.
```c++
class IPush_handler 
{
public:
	virtual ~IPush_handler() {}
	//callbacks
	virtual bool get_element(const char *path) = 0;

	// memory that holds data for *val is only guaranteed to exist as long as this callback, 
	//if needded longer needs to be copied
	virtual void element_parsed(const char * path, element_type, const void *val) = 0;
};
```
The `get_element` method is called as each element is parsed to check if that element should be returned. If the element is an object or array and `true` is returned, this method will not be called for all sub elements of that object/array. Once the parent element completes a document representing that object/array is returned by calling the `element_parsed` method callback. After that method finishes the `get_element` will start to be called again for the remaining elements until parsing is complete. Returning `true` indicates to return the element, returning `false` means the parser will not return that element. The char pointer `path` lists the json path for the element, more on this below.  
  
The `element_parsed` method is called when an element in which *true* was returned for the `get_element` call has finished being parsed. `path` is the json path of the element, element_type represents the type of element, and `val` is a void pointer to the element returned. Note the memory that is pointed to is owned by the parser and is only guaranteed to exists during the life of this callback method. If this content needs to live longer then a copy should be made.
  
The source for **IPush_handler** has a sample implementation class [test_push_handler](https://github.com/ErikDeveloperNot/jsonP_dyn/blob/master/IPush_handler.h) that shows both of these methods. It shows how to test for the element_type returned and cast it appropriately. *note that null/bool_true/bool_false do not contain any data and whatever is pointed to by val will be garbage.* Thhis test_push_handler is demonstrated in the driver test program found in [**jsonP_dyn_drvr**](https://github.com/ErikDeveloperNot/jsonP_dyn_drvr).
  
The element paths returned by both methods start with a leading **'/'** followed by an element name and more slashes if the element is an object. Array elements will be numbered starting with zero. Samples:  
```
/dont_use/bool_false   
/widget/embed_array/4                
```
The first is an element with a key named bool_false in an object named dont_use 
  
The second is the numer 5 element in array embed_array which is a member of the widget object  
  
 ---
 ### element_type enum
 ```c++
 enum element_type : u_int8_t {object=1, string=2, numeric_int=3, numeric_long=4, numeric_double=5, array=7, boolean=8, null=9, empty=11, bool_true=12, bool_false=13};
```
  
### error enum
```c++
enum error : u_int8_t { none=0, is_null=1, not_string=2, not_long=3, not_double=4, not_bool=5, not_found=6, invalid_container=7, invalid_id=8 };
```
