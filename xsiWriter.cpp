
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "Scene.h"

#include "XSIParser.h"
#include "xsiWriter.h"


CXsiWriter::CXsiWriter(CFile* pFile, const fs::path& fileName)
{
	CXsiFile* pXsiFile = static_cast<CXsiFile*>(pFile);

	m_pXsiFile = pXsiFile;

	m_fileName = fileName;
}


bool CXsiWriter::Write()
{
	OBJ_ASSERT(m_pXsiFile);
	if(!m_pXsiFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	SI_Error result = SI_SUCCESS;


	// SL way of writing dotXSI, but doesnt not write to Binary. So we use the Parser instead
	{
		// Save the file.
		//m_pXsiFile->m_scene.Write(const_cast<char*>(m_fileName.c_str()));

		// Close the file.
		//m_pXsiFile->m_scene.Close();
	}

	CSLScene* pScene = m_pXsiFile->GetScene();
	OBJ_ASSERT(pScene);
	if(!pScene)
		return false;

	pScene->Synchronize();

	CXSIParser*	pParser = pParser = pScene->Parser();

	OBJ_ASSERT(pParser);
	if(!pParser)
		return false;


	// let's open the file using CXSIParser::Write
	pParser->SetOpenMode(OPEN_WRITE);

	if(m_bWriteBinary)
	{
		pParser->SetdotXSIFormat(FORMAT_BINARY);
	}
	else
	{
		pParser->SetdotXSIFormat(FORMAT_TEXT);
	}


	CSIBCString l_lFilename;
	l_lFilename = _SI_TEXT(fileName.c_str());

	_SI_CALL(pParser->Open(l_lFilename), "CXSIParser::Open"); // Open to write
	if(result == SI_SUCCESS)
	{
		MSG_INFO("Writing the dotXSI file \'" << fileName << "\'");  

		_SI_CALL(pParser->Write(), "CXSIParser::Write");
		if(result == SI_SUCCESS)
		{
			_SI_CALL(pParser->Close(), "CXSIParser::Close");
		}
		else
		{
			MSG_ERROR("Error writing to the file \'" << fileName << "\'");
			// let's attempt to close the file
			pParser->Close();
		}
	}
	else
	{
		MSG_ERROR("\'" << fileName << "\' could not be open for writing");
	}



	if(result == SI_SUCCESS)
		return true;
	else
		return false;
}
