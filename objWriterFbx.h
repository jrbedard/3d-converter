
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJWRITERFBX_H__
#define __OBJWRITERFBX_H__


#include "writer.h"

namespace ZBPlugin
{

class CFile;
class CFbxFile;

// CObjWriterFbx: Write OBJ File format using the FBX SDK
class CObjWriterFbx : public CWriter
{
public:
	CObjWriterFbx(CFile* pFile, const fs::path& fileName);

private:
	CObjWriterFbx(const CObjWriterFbx&); // Copy constructor
	void operator=(const CObjWriterFbx&); // Assignment operator

public:
	bool Write();

private:
	bool ReplaceFbxSdkHeader(const std::string& fileName);

private:
	CFbxFile* m_pFbxFile;
};

} // End ZBPlugin namespace

#endif // __OBJWRITERFBX_H__