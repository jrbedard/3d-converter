
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "w3dReader.h"



CW3dReader::CW3dReader(CFile* pFile, const fs::path& fileName)
{
	CW3dFile* pW3dFile = static_cast<CW3dFile*>(pFile);

	m_pW3dFile = pW3dFile;

	m_fileName = fileName;
}


bool CW3dReader::Read()
{
	OBJ_ASSERT(m_pW3dFile);
	if(!m_pW3dFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	return true;
}


bool CW3dReader::ReadDebug()
{
	return true;
}

