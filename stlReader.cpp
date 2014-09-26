
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "stlReader.h"



CStlReader::CStlReader(CFile* pFile, const fs::path& fileName)
{
	CStlFile* pStlFile = static_cast<CStlFile*>(pFile);

	m_pStlFile = pStlFile;

	m_fileName = fileName;
}


bool CStlReader::Read()
{
	OBJ_ASSERT(m_pStlFile);
	if(!m_pStlFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	//const int maxline = 1000;
	//char line[maxline];

	MSG_INFO("Reading .STL file '" << fileName << "'.");
	m_ifs.open(fileName.c_str());
	if( !m_ifs.is_open() )
	{
		MSG_ERROR("Couldn't open .STL file '" << fileName << "'");
		return true; // There is not stl file
	}

	return true;
}


bool CStlReader::ReadDebug()
{
	return true;
}

