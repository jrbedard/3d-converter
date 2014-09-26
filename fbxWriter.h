
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __FBXWRITER_H__
#define __FBXWRITER_H__


#include "writer.h"

namespace ZBPlugin
{

class CFile;
class CFbxFile;

// CFbxWriter: FBX file writer
class CFbxWriter : public CWriter
{
public:
	CFbxWriter(CFile* pFile, const fs::path& fileName);

private:
	CFbxWriter(const CFbxWriter&); // Copy constructor
	void operator=(const CFbxWriter&); // Assignment operator

public:
	bool Write();

private:
	CFbxFile* m_pFbxFile;
};

} // End ZBPlugin namespace

#endif // __FBXWRITER_H__