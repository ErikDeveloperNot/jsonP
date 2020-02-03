# jsonP
jsonP is a semi compliant c++ json parser/constructor that performs fast in my comparison tests to other c/c++ parsers with a smaller memory footprint.  

jsonP has 3 different types of parsers that can be configured with a number of options.
- Standard parser which requires a char* to an in memory json document
- Buffered parser allows to parse a document being buffered in from disk or network
- Push parser which uses the buffer parser and relies on a call backs to only return elements of interest  
  
---
### Performance Comparison Chart
| Parser                                                                                                   | Memory b4 parse | Memory aft parse                                                                                             | Parse time | Stringify time | Pretty stringify time | free/delete time |
|----------------------------------------------------------------------------------------------------------|-----------------|--------------------------------------------------------------------------------------------------------------|------------|----------------|-----------------------|------------------|
| [RapidJSON](https://google.com) - believe does not sort key                                              |     1471 MB     |                                                    2165 MB                                                   |  2s 818ms  |     2s 6ms     |           --          |        --        |
| [SimpleJSON](https://google.com)                                                                         |     1471 MB     |                                                    3877 MB                                                   | 28s 593ms  |       --       |       53s 461ms       |     7s 395ms     |
| [cJson](https://google.com)                                                                              |     1471 MB     |                                                    3175 MB                                                   |  8s 127ms  |       --       |        11s 85ms       |       65ms       |
| [sajson](https://google.com)                                                                             |     1471 MB     |                                                    2001 MB                                                   |  2s 675ms  |       --       |           --          |        --        |
|                                                                                                          |                 |                                                                                                              |            |                |                       |                  |
| jsonP standard -don't sort keys                                                                          |     1471 MB     |                                                    1736 MB                                                   |  2s 328ms  |      769ms     |        2s 736ms       |       87ms       |
| jsonP standard -sort keys                                                                                |     1471 MB     |                                                    1736 MB                                                   |  4s 293ms  |    1s 759ms    |        3s 617ms       |       77ms       |
| jsonP standard -preserve orig json, don't sort keys                                                      |     1471 MB     |                                                    2139 MB                                                   |  2s 796ms  |      749ms     |        2s 704ms       |       40ms       |
| jsonP standard -convert numerics (implicitly preserve orig json), don't sort keys                        |     1471 MB     |                                                    2003 MB                                                   |  2s 711ms  |    2s 776ms    |        4s 644ms       |       28 ms      |
|                                                                                                          |                 |                                                                                                              |            |                |                       |                  |
| jsonP buffer parser (json parsed while read from a file) -don't sort keys.  8k buffer used to read file. |      360 KB     |                                                    483 MB                                                    |  3s 301ms  |      804ms     |        2s 656ms       |                  |
| jsonP buffer parser (json parsed while read from a file) -sort keys. 8k buffer used to read file.        |      360 KB     |                                                    446 MB                                                    |  5s 195ms  |     1s 88ms    |        2s 939ms       |                  |
| jsonP push parser (return false for all keys, so pull no elements, just parse), use an 8k buffer.        |      340 KB     |                          572 KB (this is the largest memory footprint during parse)                          |  4s 899ms  |                |                       |                  |
| jsonP push parser (return true for 9838 keys, pulls 59 MB of data in total), use an 8k buffer.           |      340 KB     | 784 KB  (this is the largest memory foot print during parse. After each object is pulled out it is deleted)  |  5s 935ms  |                |                       |                  |  

[Standard Parser](#standard-parser)  
[Buffer Parser](#buffer-parser)  
[Push Parser](#push-parser)  
[Document Creation](#document-creation)   
[Stringify](#stringify)


---
#### Standard Parser

---
#### Buffer Parser

---
#### Push Parser

---
#### Document Creation

---
#### Stringify


