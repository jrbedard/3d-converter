
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJREADERFBX_H__
#define __OBJREADERFBX_H__


#include "reader.h"

namespace ZBPlugin
{

class CFbxFile;
class CFile;

// CObjReaderFbx: Read OBJ File format using the FBX SDK
class CObjReaderFbx : public CReader
{
public:
	CObjReaderFbx(CFile* pFile, const fs::path& fileName);

private:
	CObjReaderFbx(const CObjReaderFbx&); // Copy constructor
	void operator=(const CObjReaderFbx&); // Assignment operator

public:
	bool Read();
	bool ReadDebug();

private:
	CFbxFile* m_pFbxFile;
};

} // End ZBPlugin namespace

#endif // __OBJREADERFBX_H__