
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxFile.h"
#include "3dsReaderFbx.h"


using namespace FBXSDK_NAMESPACE;


// Read 3DS file format using the FBX SDK

C3dsReaderFbx::C3dsReaderFbx(CFile* pFile, const fs::path& fileName)
{
	CFbxFile* pFbxFile = static_cast<CFbxFile*>(pFile);

	m_pFbxFile = pFbxFile;

	m_fileName = fileName;
}



bool C3dsReaderFbx::Read()
{
	OBJ_ASSERT(m_pFbxFile);
	if(!m_pFbxFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	KString lCurrentTakeName;
	bool lStatus;

	MSG_INFO("Parsing .3DS file '" << fileName << "'.");

	// Create an importer.
	KFbxImporter* lImporter = m_pFbxFile->GetFbxSdkManager()->CreateKFbxImporter();
	lImporter->SetFileFormat(KFbxImporter::e3D_STUDIO_3DS);

	// Initialize the importer by providing a filename.
	if(lImporter->Initialize(fileName.c_str()) == false)
	{
		MSG_ERROR("Call to KFbxImporter::Initialize() failed. Error returned: " << lImporter->GetLastErrorString());
		return false;
	}

	// Import the scene.
	lStatus = lImporter->Import(*m_pFbxFile->GetFbxScene());

	if(!lStatus)
	{
		MSG_ERROR("Error importing the 3DS scene: " << lImporter->GetLastErrorString());
		return false;
	}

	return true;
}

bool C3dsReaderFbx::ReadDebug()
{
	return true;
}
