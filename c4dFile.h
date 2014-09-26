
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __C4DFILE_H__
#define __C4DFILE_H__

#include "file.h"
#include "fileException.h"


// Cinema4D R8 File format is not Open:
//http://www.plugincafe.com/forum/display_topic_threads.asp?ForumID=2&TopicID=777&SearchPagePosition=1&search=c4d+file+format&searchMode=allwords&searchIn=Topic&forum=0&searchSort=dateDESC&ReturnPage=Search

namespace ZBPlugin
{

#define FALSE 0
#define TRUE 1
#define MAXREAL 10000000.0
#define BOOL unsigned char // BOOL is a 1 byte unsigned character
#define CHAR char // CHAR is a 1 byte integer
#define WORD short // WORD is a 2 byte integer
#define UWORD unsigned short// UWORD is a 2 byte unsigned integer
#define LONG long // LONG is a 4 byte integer
//#define LONG unsigned long // ULONG is a 4 byte unsigned integer
#define Real float // Real is a 4 byte float value
typedef struct Vector
{
	float x,y,z;
} sVector;


// CC4dFile: C4D file format
class CC4dFile : public CFile
{
public:

	// Header
	static const int FORM = 0x464F524D;
	static const int MC4D = 0x4D433444;

	//#pragma pack(2)
	struct SChunkC4d
	{
		long	Flag;
		long	Size;
	};
	//#pragma pack()


public:
	CC4dFile(){}
	~CC4dFile(){}

private:

};

} // End ZBPlugin namespace

#endif // __C4DFILE_H__