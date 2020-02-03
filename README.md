# jsonP
jsonP is a semi compliant c++ json parser/constructor that performs fast in my comparison tests to other c/c++ parsers with a smaller memory footprint.  

jsonP has 3 different types of parsers that can be configured with a number of options.
- Standard parser which requires a char* to an in memory json document
- Buffered parser allows to parse a document being buffered in from disk or network
- Push parser which uses the buffer parser and relies on a call backs to only return elements of interest  
  
---
### Performance Comparison Chart
| Parser                                                        | Memory b4 parse | Memory aft parse | Parse time | Stringify time | Pretty stringify time | free/delete time |
|---------------------------------------------------------------|-----------------|------------------|------------|----------------|-----------------------|------------------|
| [RapidJSON](https://google.com) - not sure if keys are sorted |     1471 MB     |      2165 MB     |  2s 818ms  |     2s 6ms     |           --          |        --        |
| [SimpleJSON](https://google.com)                              |     1471 MB     |      3877 MB     | 28s 593ms  |       --       |       53s 461ms       |     7s 395ms     |
| [cJson](https://google.com)                                   |     1471 MB     |      3175 MB     |  8s 127ms  |       --       |        11s 85ms       |       65ms       |
| [sajson](https://google.com)                                  |     1471 MB     |      2001 MB     |  2s 675ms  |       --       |           --          |        --        |
|                                                               |                 |                  |            |                |                       |                  |
| jsonP - don't sort keys                                       |     1471 MB     |      1736 MB     |  2s 328ms  |      769ms     |        2s 736ms       |       87ms       |  
  
  

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
