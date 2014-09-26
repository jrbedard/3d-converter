
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxFile.h"
#include "objReaderFbx.h"

using namespace FBXSDK_NAMESPACE;


CObjReaderFbx::CObjReaderFbx(CFile* pFile, const fs::path& fileName)
{
	CFbxFile* pFbxFile = static_cast<CFbxFile*>(pFile);

	m_pFbxFile = pFbxFile;

	m_fileName = fileName;
}



bool CObjReaderFbx::Read()
{
	OBJ_ASSERT(m_pFbxFile);
	if(!m_pFbxFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string
	
	int lMajor, lMinor, lRevision;
	KString lCurrentTakeName;
	bool bStatus;

	MSG_INFO("Parsing .OBJ file '" << fileName << "'.");

	// Create an importer.
	KFbxImporter* lImporter = m_pFbxFile->GetFbxSdkManager()->CreateKFbxImporter();
	lImporter->SetFileFormat(KFbxImporter::eALIAS_OBJ);

	// Initialize the importer by providing a filename.
	if(lImporter->Initialize(fileName.c_str()) == false)
	{
		MSG_ERROR("Call to KFbxImporter::Initialize() failed. Error returned: " << lImporter->GetLastErrorString());

		if (lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_YET ||
			lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_ANYMORE)
		{
			KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
			lImporter->GetFileVersion(lMajor, lMinor, lRevision);
			MSG_ERROR("FBX version number for this version of the FBX SDK is " << lMajor << "." << lMinor << "." << lRevision << " \n" <<
					  "FBX version number for file '" << m_fileName.string() << "' is " << lMajor << "." << "." << lMinor << "." << lRevision);
		}

		return false;
	}

	// Import the scene.
	bStatus = lImporter->Import(*m_pFbxFile->GetFbxScene());
	
	if(!bStatus)
	{
		MSG_ERROR("Error importing the 3DS scene: " << lImporter->GetLastErrorString());
		return false;
	}


	return true;
}

bool CObjReaderFbx::ReadDebug()
{
	return true;
}
