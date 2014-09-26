
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxFile.h"
#include "fbxReader.h"

using namespace FBXSDK_NAMESPACE;


CFbxReader::CFbxReader(CFile* pFile, const fs::path& fileName)
{
	CFbxFile* pFbxFile = static_cast<CFbxFile*>(pFile);
	OBJ_ASSERT(pFbxFile);

	m_pFbxFile = pFbxFile;

	m_fileName = fileName;
}


bool CFbxReader::Read()
{
	OBJ_ASSERT(m_pFbxFile);
	if(!m_pFbxFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	return LoadScene(m_pFbxFile->GetFbxSdkManager(), m_pFbxFile->GetFbxScene(), fileName.c_str());
}


bool CFbxReader::ReadDebug()
{

	return true;
}



bool CFbxReader::LoadScene(KFbxSdkManager* pSdkManager, KFbxScene* pScene, const char* pFilename)
{
	int lMajor, lMinor, lRevision;
	int i, lTakeCount;
	KString lCurrentTakeName;
	bool bStatus;
	char lPassword[1024];

	// Create an importer.
	KFbxImporter* lImporter = pSdkManager->CreateKFbxImporter();
	SetFileFormat(pFilename, lImporter); 

	// Initialize the importer by providing a filename.
	if(lImporter->Initialize(pFilename) == false)
	{
		MSG_ERROR("Call to KFbxImporter::Initialize() failed. Error returned: " << lImporter->GetLastErrorString());

		if (lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_YET ||
			lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_ANYMORE)
		{
			KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
			lImporter->GetFileVersion(lMajor, lMinor, lRevision);
			MSG_ERROR("FBX version number for this version of the FBX SDK is" << lMajor << "." << lMinor << "." << lRevision << " \n" <<
					  "FBX version number for file " << pFilename << " is " << lMajor << "." << lMinor << "." << lRevision);
		}

		return false;
	}

	KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
	MSG_DEBUG("FBX version number for this version of the FBX SDK is " << lMajor << "." << lMinor << "." << lRevision);

	bool isFbx = lImporter->GetFileFormat() == KFbxImporter::eFBX_ENCRYPTED ||
				 lImporter->GetFileFormat() == KFbxImporter::eFBX_ASCII ||
				 lImporter->GetFileFormat() == KFbxImporter::eFBX_BINARY;

	if(isFbx)
	{
		lImporter->GetFileVersion(lMajor, lMinor, lRevision);
		MSG_DEBUG("FBX version number for file " << pFilename << " is " << lMajor << "." << lMinor << "." << lRevision);

		// From this point, it is possible to access take information without
		// the expense of loading the entire file.

		MSG_DEBUG("Take Information");

		lTakeCount = lImporter->GetTakeCount();

		MSG_DEBUG("\tNumber of takes: " << lTakeCount);
		MSG_DEBUG("\tCurrent take: \"" << lImporter->GetCurrentTakeName() << "\"");


		for(i = 0; i < lTakeCount; i++)
		{
			KFbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

			MSG_DEBUG("\t\tTake " << i);
			MSG_DEBUG("\t\t\tName: \"" << lTakeInfo->mName.Buffer() << "\"");
			MSG_DEBUG("\t\t\tDescription: \"" << lTakeInfo->mDescription.Buffer() << "\"");

			// Change the value of the import name if the take should be imported 
			// under a different name.
			MSG_DEBUG("\t\t\tImport Name: \"" << lTakeInfo->mImportName.Buffer() << "\"");

			// Set the value of the import state to false if the take should be not
			// be imported. 
			MSG_DEBUG("\t\t\tImport State: " << (lTakeInfo->mSelect ? "true" : "false"));
		}

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		lImporter->SetState(KFbxImporter::eIMPORT_MATERIAL, true);
		lImporter->SetState(KFbxImporter::eIMPORT_TEXTURE, true);
		lImporter->SetState(KFbxImporter::eIMPORT_LINK, true);
		lImporter->SetState(KFbxImporter::eIMPORT_SHAPE, true);
		lImporter->SetState(KFbxImporter::eIMPORT_GOBO, true);
		lImporter->SetState(KFbxImporter::eIMPORT_ANIMATION, true);
		lImporter->SetState(KFbxImporter::eIMPORT_GLOBAL_SETTINGS, true);
	}

	// Import the scene.
	bStatus = lImporter->Import(*pScene);

	if(!bStatus && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
	{
		MSG_INFO("Please enter password: ");

		lPassword[0] = '\0';

		std::cin >> lPassword;

		lImporter->SetPassword(lPassword);

		bStatus = lImporter->Import(*pScene);

		if(!bStatus && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
		{
			MSG_ERROR("Password is wrong, import aborted");
		}
	}

	// Destroy the importer.
	pSdkManager->DestroyKFbxImporter(lImporter);

	return bStatus;
}




void CFbxReader::SetFileFormat(const char* pFilename, FBXSDK_NAMESPACE::KFbxImporter* pImporter)
{
	OBJ_ASSERT(pFilename && pImporter);
	if(!pFilename || !pImporter)
		return;

	pImporter->SetFileFormat(KFbxImporter::eFBX_BINARY);
	KString lFileName(pFilename);
	int lIndex;

	// Find the '.' separating the file extension
	if ((lIndex = lFileName.ReverseFind('.')) > -1)
	{
		// extension found
		KString ext = lFileName.Right(lFileName.GetLen() - lIndex - 1);
		if (ext.CompareNoCase("obj") == 0)
		{
			pImporter->SetFileFormat(KFbxImporter::eALIAS_OBJ);
		}
		else if (ext.CompareNoCase("3ds") == 0)
		{
			pImporter->SetFileFormat(KFbxImporter::e3D_STUDIO_3DS);
		}
		else if (ext.CompareNoCase("dxf") == 0)
		{
			pImporter->SetFileFormat(KFbxImporter::eAUTOCAD_DXF);
		}
#ifdef ARUBA
		else if (ext.CompareNoCase("apf") == 0)
		{
			pImporter->SetFileFormat(KFbxImporter::eALIAS_APF);
		}
#endif
	}
}


