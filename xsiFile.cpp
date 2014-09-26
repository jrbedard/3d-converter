
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "Scene.h"

#include "XSIParser.h" // XSI FTK
#include "xsiFile.h"


//#include "../libraries/XSIFTK_3.6.3/collada/XSItoCollada.h"



CXsiFile::CXsiFile(const fs::path& fileName):
m_pScene(NULL)
{
	InitializeXsiFtkObjects();

	m_fileName = fileName;
}


CXsiFile::~CXsiFile()
{
	DestroyXsiFtkObjects();
}



void CXsiFile::InitializeXsiFtkObjects()
{
	m_pScene = new CSLScene();
}

void CXsiFile::DestroyXsiFtkObjects()
{
	m_pScene->Close();
	//delete m_pScene;
	m_pScene = NULL;
}



bool CXsiFile::ConvertXsiToCollada(const fs::path& xsiFileName, const fs::path& daeFileName)
{
	MSG_INFO("Collada conversion is disabled in this version..."); 
/*
	XMLPlatformUtils::Initialize();

	CXSItoCollada* pDaeConverter = new CXSItoCollada();

	MSG_INFO("Converting XSI file into a DAE (collada) file...");

	pDaeConverter->ReadXSIFile(const_cast<char*>(xsiFileName.string().c_str()));
	pDaeConverter->WriteColladaFile(const_cast<char*>(daeFileName.string().c_str()));
	pDaeConverter->ReleaseColladaFile();
	pDaeConverter->ReleaseXSIFile();

	delete pDaeConverter;
*/
	return false;
}


