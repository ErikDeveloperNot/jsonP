#ifndef ICHUNK_READER_H
#define ICHUNK_READER_H

class IChunk_reader
{
public:
//	virtual ~IChunk_reader();
	// user implemented callback, next is a char[] passed to the callback; cnt is the max number of chars to insert
	// including '\0' into next; returns the number of chars inserted
	virtual int get_next(char * buf, int cnt) = 0;
};

#endif  //ICHUNK_READER_H