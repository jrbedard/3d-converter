
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxFile.h"
#include "3dsWriterFbx.h"


using namespace FBXSDK_NAMESPACE;


// Write 3DS file format using the FBX SDK

C3dsWriterFbx::C3dsWriterFbx(CFile* pFile, const fs::path& fileName)
{
	CFbxFile* pFbxFile = static_cast<CFbxFile*>(pFile);

	m_pFbxFile = pFbxFile;

	m_fileName = fileName;

	m_bWriteBinary = true;
}


bool C3dsWriterFbx::Write()
{
	OBJ_ASSERT(m_pFbxFile);
	if(!m_pFbxFile)
		return false;

	int lMajor, lMinor, lRevision;
	std::string fileName(m_fileName.string()); // Extract native file path string

	MSG_INFO("writing .3DS file '" << fileName << "'.");

	// Create an exporter.
	KFbxExporter* lExporter = m_pFbxFile->GetFbxSdkManager()->CreateKFbxExporter();

	// Initialize the exporter by providing a filename.
	if(lExporter->Initialize(fileName.c_str()) == false)
	{
		MSG_ERROR("Call to KFbxExporter::Initialize() failed. Error returned: " << lExporter->GetLastErrorString());
		return false;
	}

	KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
	//MSG_DEBUG("FBX version number for this version of the FBX SDK is " << lMajor << "." << lMinor << "." << lRevision);

	// Set the export states. By default, the export states are always set to 
	// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
	// shows how to change these states.
	lExporter->SetState(KFbxExporter::eEXPORT_MODEL, true);
	lExporter->SetState(KFbxExporter::eEXPORT_MATERIAL, true);
	lExporter->SetState(KFbxExporter::eEXPORT_TEXTURE, true);

	lExporter->SetState(KFbxExporter::eEXPORT_LINK, true);
	lExporter->SetState(KFbxExporter::eEXPORT_SHAPE, true);


	// use the SetFileFormat method to specifiy ascii or binary.
	lExporter->SetFileFormat(KFbxExporter::e3D_STUDIO_3DS);

	//lExporter->SetExportOptionsInfo(eSTREAMOPTIONS_PARSE_FILE);

	KStreamOptions* testes = lExporter->GetExportOptions();	

	// Export the scene.
	bool bStatus = lExporter->Export(*m_pFbxFile->GetFbxScene(), testes);

	if(!bStatus)
	{
		MSG_ERROR("Error returned: " << lExporter->GetLastErrorString());
	}

	// Destroy the exporter.
	m_pFbxFile->GetFbxSdkManager()->DestroyKFbxExporter(lExporter);

	return bStatus;
}
