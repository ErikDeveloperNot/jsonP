#ifndef ICHUNK_READER_H
#define ICHUNK_READER_H

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

#endif  //ICHUNK_READER_H