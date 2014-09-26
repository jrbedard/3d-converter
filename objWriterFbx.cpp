
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxFile.h"
#include "objWriterFbx.h"

using namespace FBXSDK_NAMESPACE;


CObjWriterFbx::CObjWriterFbx(CFile* pFile, const fs::path& fileName)
{
	CFbxFile* pFbxFile = static_cast<CFbxFile*>(pFile);

	m_pFbxFile = pFbxFile;

	m_fileName = fileName;
}


bool CObjWriterFbx::Write()
{
	OBJ_ASSERT(m_pFbxFile);
	if(!m_pFbxFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	// Create an exporter.
	KFbxExporter* lExporter = m_pFbxFile->GetFbxSdkManager()->CreateKFbxExporter();

	// Initialize the exporter by providing a filename.
	if(lExporter->Initialize(fileName.c_str()) == false)
	{
		MSG_ERROR("Call to KFbxExporter::Initialize() failed. Error Returned : " << lExporter->GetLastErrorString());
		return false;
	}

	MSG_INFO("writing .OBJ file '" << fileName << "'.");

	// Set the export states. By default, the export states are always set to 
	// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED.
	lExporter->SetState(KFbxExporter::eEXPORT_MODEL, true);
	lExporter->SetState(KFbxExporter::eEXPORT_MATERIAL, true);
	lExporter->SetState(KFbxExporter::eEXPORT_TEXTURE, true);
	//lExporter->SetState(KFbxExporter::eEXPORT_TEXTURE_AS_EMBEDDED, true);

	// use the SetFileFormat method to specifiy ascii or binary.
	// This is the typical way to proceed.
	lExporter->SetFileFormat(KFbxExporter::eALIAS_OBJ);

	//lExporter->SetExportOptionsInfo(eSTREAMOPTIONS_PARSE_FILE);
	KStreamOptions* pExportOptions= lExporter->GetExportOptions();	

	// Export the scene.
	bool bStatus = lExporter->Export(*m_pFbxFile->GetFbxScene(), pExportOptions);

	if(!bStatus)
	{
		MSG_ERROR("Error returned: " << lExporter->GetLastErrorString() );
	}

	// Destroy the exporter.
	m_pFbxFile->GetFbxSdkManager()->DestroyKFbxExporter(lExporter);

	// Remove FBX SDK header in the OBJ file, kind of a cheat...
	//ReplaceFbxSdkHeader(fileName);

	fs::path mtlFilePath = fs::change_extension(m_fileName, ".mtl"); // replace the ".obj" by ".mtl" extension
	std::string mtlFileName(mtlFilePath.string());

	// Remove FBX SDK header in the MTL file, kind of a cheat...
	//ReplaceFbxSdkHeader(mtlFileName); // TODO : the mtl doesnt seems to be created each time...

	return bStatus;
}


bool CObjWriterFbx::ReplaceFbxSdkHeader(const std::string& fileName)
{
	std::ifstream ifs;
	ifs.open(fileName.c_str(), std::ios_base::in);
	if( !ifs.is_open() ) // open the OBJ for reading
	{
		MSG_ERROR("Couldn't open file '" << fileName << "'");
		return false;
	}

	std::stringstream tempStream;
	tempStream << ifs.rdbuf();
	ifs.close();

	std::ofstream ofs;
	ofs.open(fileName.c_str(), std::ios_base::out);
	if( !ofs.is_open() ) // open the OBJ for writing
	{
		MSG_ERROR("Couldn't open file '" << fileName << "'");
		return false;
	}

	// Write the .OBJ header
	ofs << "# File Created by the OBJ file format converter" << std::endl;
	ofs << "# jrbedard.com" << std::endl;
	ofs << "# " << std::endl;

	// Remove 3 lines from the original header
	tempStream.seekg(54); // 54 is the magical number, to be changed if the FBX SDK change

	ofs << tempStream.rdbuf();

	return true;
}



