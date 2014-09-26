
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __STLWRITER_H__
#define __STLWRITER_H__


#include "writer.h"
#include "stlFile.h"

namespace ZBPlugin
{

class CFile;

// CStlWriter: STL file writer
class CStlAsciiWriter : public CWriter
{
public:
	CStlAsciiWriter(CFile* pFile, const fs::path& fileName, bool bExportBinary);

private:
	CStlAsciiWriter(const CStlAsciiWriter&); // Copy constructor
	void operator=(const CStlAsciiWriter&); // Assignment operator

public:
	bool Write();

private:
	CStlFile* m_pStlFile;
};

} // End ZBPlugin namespace

#endif // __STLWRITER_H__