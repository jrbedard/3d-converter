
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XSIREADER_H__
#define __XSIREADER_H__


#include "reader.h"
#include "xsiFile.h"

namespace ZBPlugin
{

class CFile;

// CXsiReader: XSI file reader
class CXsiReader : public CReader
{
public:
	CXsiReader(CFile* pFile, const fs::path& fileName);

private:
	CXsiReader(const CXsiReader&); // Copy constructor
	void operator=(const CXsiReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug();

private:
	bool ReadXsiHeader();

private:
	CXsiFile* m_pXsiFile;
};

} // End ZBPlugin namespace

#endif // __XSIREADER_H__