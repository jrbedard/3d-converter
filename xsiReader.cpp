
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "Scene.h"

#include "XSIParser.h" // XSI FTK
#include "xsiReader.h"



CXsiReader::CXsiReader(CFile* pFile, const fs::path& fileName)
{
	CXsiFile* pXsiFile = static_cast<CXsiFile*>(pFile);

	m_pXsiFile = pXsiFile;

	m_fileName = fileName;
}


bool CXsiReader::Read()
{
	OBJ_ASSERT(m_pXsiFile);
	if(!m_pXsiFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	// Check if its 3.6 Binary // v1hack
	{
		m_ifs.open(fileName.c_str(), ios::binary);
		OBJ_ASSERT(m_ifs.is_open());
		if(!m_ifs.is_open())
		{
			MSG_ERROR("Couldn't open .XSI file '" << fileName << "'");
			return false;
		}

		char header[12];
		m_ifs.read(header, 11);
		if(strstr(header, "xsi 0360bin")) // Detect v3.6 binary
		{
			MSG_ERROR("Sorry, the XSI importer does not support dotXSI binary file version 3.6." << std::endl <<
					  "Please re-save this file in dotXSI version 3.6 ascii or 3.5 binary.");
			m_ifs.close();
			return false;
		}
		m_ifs.close();
	}


	SI_Error result = SI_SUCCESS;

/*
	CXSIParser*	pParser = m_pXsiFile->GetXsiParser();

	OBJ_ASSERT(pParser);
	if(!pParser)
		return false;

*/

	CSLScene* pScene = m_pXsiFile->GetScene();

	OBJ_ASSERT(pScene);
	if(!pScene)
		return false;

	MSG_INFO("Reading .XSI file '" << m_fileName.string() << "'.");

	result = pScene->Open((char*)fileName.c_str());
	if (result != SI_SUCCESS)
	{
		MSG_ERROR("Failed to open dotXSI file: " << m_fileName.string());
		return false;
	}

	result = pScene->Read();
	if (result != SI_SUCCESS)
	{
		MSG_ERROR("Failed to open dotXSI file: " << m_fileName.string());
		return false;
	}

	//pParser = pScene->Parser();

	ReadXsiHeader();

	return true; // hack?

	//pScene->Synchronize();

/*
	// let's open the file using CXSIParser::Open
	pParser->SetOpenMode(OPEN_READ);

	CSIBCString l_lFilename;
	l_lFilename = _SI_TEXT(m_fileName.c_str());

	_SI_CALL(pParser->Open(l_lFilename), "CXSIParser::Open"); // Open to read
	if(result == SI_SUCCESS)
	{
		MSG_INFO("Reading the dotXSI file \'" << m_fileName << "\'");  
		_SI_CALL(pParser->Read(), "CXSIParser::Read");
		if(result == SI_SUCCESS)
		{
			_SI_CALL(pParser->Close(), "CXSIParser::Close");

			ReadXsiHeader();
		}
		else
		{
			MSG_ERROR("Error reading the file");
			// let's attempt to close the file
			pParser->Close();
		}
	}
	else
	{
		MSG_ERROR("\'" << m_fileName << "\' could not be open for reading");
	}
*/

	if(result == SI_SUCCESS)
		return true;
	else
		return false;
}


bool CXsiReader::ReadDebug()
{
	return true;
}


bool CXsiReader::ReadXsiHeader()
{
	CSLScene* pScene = m_pXsiFile->GetScene();

	CXSIParser*	pParser = pScene->Parser();

	OBJ_ASSERT(pParser);
	if(!pParser)
		return false;

	// first we dump the file information (header)
	MSG_DEBUG("File version : " << pParser->GetdotXSIFileVersionMajor() << "." << pParser->GetdotXSIFileVersionMinor());

	if(pParser->GetdotXSIFormat() == FORMAT_TEXT)
	{
		MSG_DEBUG("Format : TEXT");
	}
	else if(pParser->GetdotXSIFormat() == FORMAT_BINARY)
	{
		MSG_DEBUG("Format : BINARY" );
	}

	return true;
}



