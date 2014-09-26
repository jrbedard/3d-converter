
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __STLASCIIREADER_H__
#define __STLASCIIREADER_H__


#include "reader.h"
#include "stlFile.h"

namespace ZBPlugin
{

class CFile;

// CStlAsciiReader: STL file reader
class CStlAsciiReader : public CReader
{
public:
	CStlAsciiReader(CFile* pFile, const fs::path& fileName);

private:
	CStlAsciiReader(const CStlAsciiReader&); // Copy constructor
	void operator=(const CStlAsciiReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug();

private:
	CStlFile* m_pStlFile;
};

} // End ZBPlugin namespace

#endif // __STLASCIIREADER_H__