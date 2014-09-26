
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __STLREADER_H__
#define __STLREADER_H__


#include "reader.h"
#include "stlFile.h"

namespace ZBPlugin
{

class CFile;

// CStlReader: STL file reader
class CStlReader : public CReader
{
public:
	CStlReader(CFile* pFile, const fs::path& fileName);

private:
	CStlReader(const CStlReader&); // Copy constructor
	void operator=(const CStlReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug();

private:
	CStlFile* m_pStlFile;
};

} // End ZBPlugin namespace

#endif // __STLREADER_H__