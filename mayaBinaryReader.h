
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __MAYABINARYREADER_H__
#define __MAYABINARYREADER_H__


#include "reader.h"
#include "mayaFile.h"

namespace ZBPlugin
{

class CFile;

// CMayaBinaryReader: Maya binary reader
class CMayaBinaryReader : public CReader
{
public:
	CMayaBinaryReader(CFile* pFile, const fs::path& fileName);

private:
	CMayaBinaryReader(const CMayaBinaryReader&); // Copy constructor
	void operator=(const CMayaBinaryReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug(){return true;}

private:
	void MovePC(long forward);
	bool ReadBlock(char *dest, long size);
	void FileSize();

private:
	long m_fileSize;
	long m_pc;	// File Pointer

	CMayaFile* m_pMayaFile;
};

} // End ZBPlugin namespace

#endif // __MAYABINARYREADER_H__

