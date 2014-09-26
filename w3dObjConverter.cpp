
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "w3dFile.h"

#include "w3dObjConverter.h"


CW3dObjConverter::CW3dObjConverter():
m_pW3dFile(NULL),
m_pOptions(NULL)
{
}

CW3dObjConverter::~CW3dObjConverter()
{
}


bool CW3dObjConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CW3dObjOptions();
	CToObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}

// Convert a W3d file into an OBJ file
bool CW3dObjConverter::Convert(CFile* pOtherFile, CFile* pObjFile)
{
	// Initialize W3d -> OBJ
	m_pW3dFile = InitConversion<CW3dFile>(pOtherFile, pObjFile);
	OBJ_ASSERT(m_pW3dFile && m_pOptions);
	if(!m_pW3dFile || !m_pOptions)
		return false;

	MSG_INFO("Converting W3D file format into OBJ file format...");

	return true;
}

