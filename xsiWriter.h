
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XSIWRITER_H__
#define __XSIWRITER_H__


#include "writer.h"
#include "xsiFile.h"

namespace ZBPlugin
{

class CFile;

// CXsiWriter: XSI file writer
class CXsiWriter : public CWriter
{
public:
	CXsiWriter(CFile* pFile, const fs::path& fileName);

private:
	CXsiWriter(const CXsiWriter&); // Copy constructor
	void operator=(const CXsiWriter&); // Assignment operator

public:
	bool Write();

private:
	CXsiFile* m_pXsiFile;
};

} // End ZBPlugin namespace

#endif // __XSIWRITER_H__