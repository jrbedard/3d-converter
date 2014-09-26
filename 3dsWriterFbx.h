
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __3DSWRITERFBX_H__
#define __3DSWRITERFBX_H__


#include "writer.h"

namespace ZBPlugin
{

class CFile;
class CFbxFile;

// C3dsWriterFbx: Write 3DS file format using the FBX SDK
class C3dsWriterFbx : public CWriter
{
public:
	C3dsWriterFbx(CFile* pFile, const fs::path& fileName);

private:
	C3dsWriterFbx(const C3dsWriterFbx&); // Copy constructor
	void operator=(const C3dsWriterFbx&); // Assignment operator

public:
	bool Write();

private:
	CFbxFile* m_pFbxFile;
};

} // End ZBPlugin namespace

#endif // __3DSWRITERFBX_H__