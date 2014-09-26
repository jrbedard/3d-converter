
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __MAYABINARYWRITER_H__
#define __MAYABINARYWRITER_H__


#include "writer.h"
#include "mayaFile.h"

namespace ZBPlugin
{

class CFile;

// CMayaBinaryWriter: Maya binary writer
class CMayaBinaryWriter : public CWriter
{
public:
	CMayaBinaryWriter(CFile* pFile, const fs::path& fileName);
	virtual ~CMayaBinaryWriter(){}

private:
	CMayaBinaryWriter(const CMayaBinaryWriter&); // Copy constructor
	void operator=(const CMayaBinaryWriter&); // Assignment operator

public:
	bool Write();

private:
	CMayaFile* m_pMayaFile;
};

} // End ZBPlugin namespace

#endif // __MAYABINARYWRITER_H__