
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __MAYAASCIIWRITER_H__
#define __MAYAASCIIWRITER_H__


#include "writer.h"
#include "mayaFile.h"


namespace ZBPlugin
{

class CFile;

// CMayaAsciiWriter: Maya ascii writer
class CMayaAsciiWriter : public CWriter
{
public:
	CMayaAsciiWriter(CFile* pFile, const fs::path& fileName);

private:
	CMayaAsciiWriter(const CMayaAsciiWriter&); // Copy constructor
	void operator=(const CMayaAsciiWriter&); // Assignment operator

public:
	bool Write();

private:
	CMayaFile* m_pMayaFile;
};

} // End ZBPlugin namespace

#endif // __MAYAASCIIWRITER_H__