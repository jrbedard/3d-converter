
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __MAYAASCIIREADER_H__
#define __MAYAASCIIREADER_H__


#include "reader.h"
#include "mayaFile.h"

namespace ZBPlugin
{

class CFile;

// CMayaAsciiReader: Maya ascii reader
class CMayaAsciiReader : public CReader
{
public:
	CMayaAsciiReader(CFile* pFile, const fs::path& fileName);

private:
	CMayaAsciiReader(const CMayaAsciiReader&); // Copy constructor
	void operator=(const CMayaAsciiReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug(){return true;}

private:

	CMayaFile*	m_pMayaFile;
};

} // End ZBPlugin namespace

#endif // __MAYAASCIIREADER_H__