
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __W3DWRITER_H__
#define __W3DWRITER_H__


#include "writer.h"
#include "w3dFile.h"

namespace ZBPlugin
{

class CFile;

// CW3dWriter: W3D file writer
class CW3dWriter : public CWriter
{
public:
	CW3dWriter(CFile* pFile, const fs::path& fileName, bool bExportBinary);

private:
	CW3dWriter(const CW3dWriter&); // Copy constructor
	void operator=(const CW3dWriter&); // Assignment operator

public:
	bool Write();

private:
	CW3dFile* m_pW3dFile;
};

} // End ZBPlugin namespace

#endif // __W3DWRITER_H__