
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __DAEWRITER_H__
#define __DAEWRITER_H__


#include "writer.h"
#include "daeFile.h"

namespace ZBPlugin
{

class CFile;

// CDaeWriter: collada file format writer
class CDaeWriter : public CWriter
{
public:
	CDaeWriter(CFile* pFile, const fs::path& fileName);

private:
	CDaeWriter(const CDaeWriter&); // Copy constructor
	void operator=(const CDaeWriter&); // Assignment operator

public:
	bool Write();

private:
	CDaeFile* m_pDaeFile;
};

} // End ZBPlugin namespace

#endif // __DAEWRITER_H__