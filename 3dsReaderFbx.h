
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __3DSREADERFBX_H__
#define __3DSREADERFBX_H__


#include "reader.h"

namespace ZBPlugin
{

class CFile;
class CFbxFile;

// C3dsReaderFbx: Read 3DS file format using the FBX SDK
class C3dsReaderFbx : public CReader
{
public:
	C3dsReaderFbx(CFile* pFile, const fs::path& fileName);

private:
	C3dsReaderFbx(const C3dsReaderFbx&); // Copy constructor
	void operator=(const C3dsReaderFbx&); // Assignment operator

public:
	bool Read();
	bool ReadDebug();

private:
	CFbxFile* m_pFbxFile;
};

} // End ZBPlugin namespace

#endif // __3DSREADERFBX_H__