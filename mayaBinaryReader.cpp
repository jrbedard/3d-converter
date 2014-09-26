
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "mayaBinaryReader.h"


CMayaBinaryReader::CMayaBinaryReader(CFile* pFile, const fs::path& fileName)
{
	CMayaFile* pMayaFile = static_cast<CMayaFile*>(pFile);
	OBJ_ASSERT(pMayaFile);

	m_pMayaFile = pMayaFile;

	m_fileName = fileName;
}



void CMayaBinaryReader::MovePC(long forward)
{
	m_pc += forward;
	m_ifs.seekg(m_pc);
}

bool CMayaBinaryReader::ReadBlock(char *dest, long size)
{
	m_pc = m_ifs.tellg();
	if(!m_ifs.read(dest, size) && m_ifs.eof())
		return false;
	m_pc += size;
	m_ifs.seekg(m_pc);
	return true;
}

void CMayaBinaryReader::FileSize()
{
	// get length of file:
	m_ifs.seekg (0, ios::end);
	m_fileSize = m_ifs.tellg();
	m_ifs.seekg (0, ios::beg);
}



bool CMayaBinaryReader::Read()
{
	OBJ_ASSERT(m_pMayaFile);
	if(!m_pMayaFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	m_ifs.open(fileName.c_str(), ios::binary);
	OBJ_ASSERT(m_ifs.is_open());
	if(!m_ifs.is_open())
	{
		MSG_ERROR("Couldn't open .MB file '" << fileName << "'");
		return false;
	}

	const std::string FOR4("FOR4"); // CreateNode
	const std::string XFRMCREA("XFRMCREA"); // "Transform" node
	const std::string DMSHCREA("DMSHCREA"); // "Mesh" node

	const std::string FLGS("FLGS");
	const std::string DBLE("DBLE");
	const std::string FLT2("FLT2");
	const std::string STR("STR");
	const std::string MESH("MESH");


	// Too difficult to parse, and probably illegal...


	m_ifs.close();

	return true;
}




