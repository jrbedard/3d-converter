
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxFile.h"
#include "fbxWriter.h"

using namespace FBXSDK_NAMESPACE;


CFbxWriter::CFbxWriter(CFile* pFile, const fs::path& fileName)
{
	CFbxFile* pFbxFile = static_cast<CFbxFile*>(pFile);
	OBJ_ASSERT(pFbxFile);

	m_pFbxFile = pFbxFile;

	m_fileName = fileName;
}



bool CFbxWriter::Write()
{
	OBJ_ASSERT(m_pFbxFile);
	if(!m_pFbxFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	KFbxSdkManager* pSdkManager = m_pFbxFile->GetFbxSdkManager();
	KFbxScene* pScene = m_pFbxFile->GetFbxScene();
	const char* pFilename = fileName.c_str();

	bool pSaveAsBinary = m_bWriteBinary;
	bool pEmbedMedia = false;
	bool pSaveAsVersion5 = false; // max's FBX importer doesnt support version 6 for now

	int lMajor, lMinor, lRevision;

	// Create an exporter.
	KFbxExporter* lExporter = pSdkManager->CreateKFbxExporter();

	// Initialize the exporter by providing a filename.
	if(lExporter->Initialize(pFilename) == false)
	{
		MSG_ERROR("Call to KFbxExporter::Initialize() failed. Error returned: " << lExporter->GetLastErrorString());
		return false;
	}

	KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
	MSG_DEBUG("FBX version number for this version of the FBX SDK is " << lMajor << "." << lMinor << "." << lRevision);

	// Set the export states. By default, the export states are always set to 
	// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
	// shows how to change these states.
	lExporter->SetState(KFbxExporter::eEXPORT_MATERIAL, true);
	lExporter->SetState(KFbxExporter::eEXPORT_TEXTURE, true);
	lExporter->SetState(KFbxExporter::eEXPORT_TEXTURE_AS_EMBEDDED, pEmbedMedia);
	lExporter->SetState(KFbxExporter::eEXPORT_LINK, true);
	lExporter->SetState(KFbxExporter::eEXPORT_SHAPE, true);
	lExporter->SetState(KFbxExporter::eEXPORT_GOBO, true);
	lExporter->SetState(KFbxExporter::eEXPORT_ANIMATION, true);
	lExporter->SetState(KFbxExporter::eEXPORT_GLOBAL_SETTINGS, true);


	// Set the file mode to binary
	if(pSaveAsVersion5)
	{
		MSG_INFO("writing FBX file version 5");
		lExporter->SetFileFormat((pSaveAsBinary) ? KFbxExporter::eFBX_50_BINARY : KFbxExporter::eFBX_50_ASCII);
	}
	else
	{
		MSG_INFO("writing FBX file version 6");
		lExporter->SetFileModeBinary(pSaveAsBinary);
	}

	// Export the scene.
	bool bStatus = lExporter->Export(*pScene);

	if(!bStatus)
	{
		MSG_ERROR("Error Returned: " << lExporter->GetLastErrorString());
	}

	// Destroy the exporter.
	pSdkManager->DestroyKFbxExporter(lExporter);

	MSG_DEBUG("Done.");

	return bStatus;
}