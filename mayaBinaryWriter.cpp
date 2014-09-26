
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "mayaBinaryWriter.h"



CMayaBinaryWriter::CMayaBinaryWriter(CFile* pFile, const fs::path& fileName)
{
	CMayaFile* pMayaFile = static_cast<CMayaFile*>(pFile);
	OBJ_ASSERT(pMayaFile);

	m_pMayaFile = pMayaFile;

	m_fileName = fileName;
}




bool CMayaBinaryWriter::Write()
{
	OBJ_ASSERT(m_pMayaFile);
	if(!m_pMayaFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	// Verify here that we have something to actually write
	OBJ_ASSERT(m_pMayaFile->GetMeshVector().size() > 0);

	if(m_pMayaFile->GetMeshVector().size() > 0)
	{
		MSG_ERROR("Invalid Maya file!");
		return false;
	}

	// Write Maya binary file version 6.0

	OBJ_ASSERT(CheckStr(fileName));
	if(!CheckStr(fileName))
		return false;

	m_ofs.open(fileName.c_str());

	MSG_INFO("Writing Maya Binary file (.MB) : '" << fileName << "'.");
	if( !m_ofs.is_open() )
	{
		MSG_ERROR("Couldn't write to Maya file '" << fileName << "'");
		return false;
	}

	// Too difficult to write....  :(

	m_ofs.close();

	return true;
}