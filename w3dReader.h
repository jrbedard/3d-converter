
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __W3DREADER_H__
#define __W3DREADER_H__


#include "reader.h"
#include "w3dFile.h"

namespace ZBPlugin
{

class CFile;

// CW3dReader: W3D file reader
class CW3dReader : public CReader
{
public:
	CW3dReader(CFile* pFile, const fs::path& fileName);

private:
	CW3dReader(const CW3dReader&); // Copy constructor
	void operator=(const CW3dReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug();

private:
	CW3dFile* m_pW3dFile;
};

} // End ZBPlugin namespace

#endif // __W3DREADER_H__