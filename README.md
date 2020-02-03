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
[Document Creation](#document-creation)   
[Stringify](#stringify)

*A sample driver program demostrating much of the functionality can be found at [**jsonP_dyn_drvr**](https://github.com/ErikDeveloperNot/jsonP_dyn_drvr)*
  
### Standard Parser
Create a parser object with one of two constructors  
```c++
#include "jsonP_parser.h"

jsonP_parser(std::string & json, unsigned short options = 0);
jsonP_parser(char * json, unsigned int json_len, unsigned short options = 0);
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
jsonP_json json_doc = parser.parse();
```
Both the parser **jsonP_parser** and the document object **jsonP_json** need to be freed separately.
### Buffer Parser
  
  
### Push Parser
  
  
### Document Creation
  
  
### Stringify


