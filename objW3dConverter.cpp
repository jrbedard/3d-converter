
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "w3dFile.h"

#include "objW3dConverter.h"


CObjW3dConverter::CObjW3dConverter():
m_pW3dFile(NULL),
m_pOptions(NULL)
{
}

CObjW3dConverter::~CObjW3dConverter()
{
}


bool CObjW3dConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CObjW3dOptions();
	CFromObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}

// Convert an OBJ file into a W3D file
bool CObjW3dConverter::Convert(CFile* pObjFile, CFile* pOtherFile)
{
	// Initialize OBJ -> W3D
	m_pW3dFile = InitConversion<CW3dFile>(pObjFile, pOtherFile);
	OBJ_ASSERT(m_pW3dFile && m_pOptions);
	if(!m_pW3dFile || !m_pOptions)
		return false;

	MSG_INFO("Converting OBJ file format into W3D file format...");

	return true;
}

