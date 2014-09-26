
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XYZREADER_H__
#define __XYZREADER_H__


#include "reader.h"
#include "xyzFile.h"

namespace ZBPlugin
{

class CFile;

// CXyzReader: XYZ file reader
class CXyzReader : public CReader
{
public:
	CXyzReader(CFile* pFile, const fs::path& fileName, ushort sampling); // v1hack

private:
	CXyzReader(const CXyzReader&); // Copy constructor
	void operator=(const CXyzReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug(){return true;}

private:
	CXyzFile* m_pXyzFile;
	ushort m_sampling;
};

} // End ZBPlugin namespace

#endif // __XYZREADER_H__
