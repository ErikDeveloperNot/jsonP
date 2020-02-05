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
### jsonP_json
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
  
##### Note on `objects` and `arrays`
***When creating an object/array, the number of elements it will contain is specified. If keys are sorted, the default, this means that if at a later time more elements need to be added to the object/array those elements will be added to a linked list and will not be sorted in the case of an object. This can slow down access times the longer this list gets.
  
***When a json text file is parsed all the objects and arrays will be created with the exact number of elements contained in the json. If more elements are added after parse those elements will be added to the linked list of that object/array.
  
##### Using the jsonP_json
[add_container](#add_container)  
[add_value_type](#add_value_type)  
[Stringify](#stringify)
  
### add_container
### Stringify
  
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

