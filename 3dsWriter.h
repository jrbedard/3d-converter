
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __3DSWRITER_H__
#define __3DSWRITER_H__


#include "binaryWriter.h"
#include "3dsFile.h"

namespace ZBPlugin
{

class CFile;

// C3dsWriter: 3DS file format reader
class C3dsWriter : public CBinaryWriter
{
public:
	C3dsWriter(CFile* pFile, const fs::path& fileName);

private:
	C3dsWriter(const C3dsWriter&); // Copy constructor
	void operator=(const C3dsWriter&); // Assignment operator

public:
	bool Write();
	inline bool IsBigEndian(){m_bigEndian = false; return false;} // 3DS files are little-endian

private:
	void BuildChunkLengths();
	void WriteChunk(C3dsFile::CHUNK chunkName);

private:
	C3dsFile* m_p3dsFile;

	// chunk lengths
	std::map< C3dsFile::CHUNK, ulong > m_chunkLengthMap;
};

} // End ZBPlugin namespace

#endif // __3DSWRITER_H__