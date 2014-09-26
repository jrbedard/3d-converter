// LWO File format reader
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter), 2005


#ifndef __LWOREADER_H__
#define __LWOREADER_H__


#include "binaryReader.h"
#include "lwoFile.h"


class CFile;

// CLwoReader: LWO file reader
class CLwoReader : public CBinaryReader
{
public:
	CLwoReader(CFile* pFile, const fs::path& fileName);

private:
	CLwoReader(const CLwoReader&); // Copy constructor
	void operator=(const CLwoReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug(){return true;}

protected:
	bool ReadBlock(char* dest, long size);

private:
	bool ReadHeader();
	bool ReadTagStrings();
	bool ReadLayer();
	bool ReadPoints();
	bool ReadVertexMapping();
	bool ReadDiscVertexMapping();
	bool ReadPolygons();
	bool ReadPolygonTagMapping();
	bool ReadImageDefinition();
	bool ReadSurface();

	// Debug info
	std::string GetTagString(uint tag);
	std::string GetTypeString(uint type);

private:
	// Generalize this and put in binary reader
	bool ParseChunks();
	CLwoFile::SChunk ReadChunk();
	ulong SkipChunk(ulong size);

private:
	CLwoFile::SChunk m_curChunk;
	CLwoFile::CLayer* m_pCurLayer;

	typedef StringVector TagVector;
	TagVector m_tagVector;

private:
	CLwoFile* m_pLwoFile;
};

#endif // __LWOREADER_H__
