// LWO File format writer
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter), 2005

#ifndef __LWOWRITER_H__
#define __LWOWRITER_H__


#include "binaryWriter.h"
#include "lwoFile.h"

class CFile;

// CLwoWriter: LWO file writer
class CLwoWriter : public CBinaryWriter
{
public:
	CLwoWriter(CFile* pFile, const fs::path& fileName);

private:
	CLwoWriter(const CLwoWriter&); // Copy constructor
	void operator=(const CLwoWriter&); // Assignment operator

public:
	bool Write();
	bool IsBigEndian(){m_bigEndian = true; return true;} // LWO files are big-endian

private:
	void BuildChunkLengths();
	void WriteChunk(ulong chunkName);
	void WriteTag(ulong tagName);

	// LWO writes
	bool WriteHeader();
	bool WriteTagStrings();
	bool WriteLayer();
	bool WritePoints();
	bool WriteVertexMapping();
	bool WriteDiscVertexMapping();
	bool WritePolygons();
	bool WritePolygonTagMapping();
	bool WriteImageDefinitions();
	bool WriteSurfaces();

private:
	CLwoFile* m_pLwoFile;

	CLwoFile::CLayer m_curLayer;

	// chunk lengths : TODO: put this in CLayer (per-layer)
	std::map< ulong, ulong > m_chunkLengthMap;
};

#endif // __LWOWRITER_H__