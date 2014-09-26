
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxFile.h"


using namespace FBXSDK_NAMESPACE;


CFbxFile::CFbxFile()
{
	m_pFbxSdkManager = NULL;
	m_pScene = NULL;

	InitializeFbxSdkObjects();
}


CFbxFile::~CFbxFile()
{
	DestroyFbxSdkObjects();

	m_pFbxSdkManager = NULL;
	m_pScene = NULL;
}



bool CFbxFile::InitializeFbxSdkObjects()
{
	// The first thing to do is to create the FBX SDK manager which is the 
	// object allocator for almost all the classes in the SDK.
	// Only one FBX SDK manager can be created. Any subsequent call to 
	// KFbxSdkManager::CreateKFbxSdkManager() will return NULL.
	m_pFbxSdkManager = KFbxSdkManager::CreateKFbxSdkManager();

	if (!m_pFbxSdkManager)
	{
		MSG_ERROR("Unable to create the FBX SDK manager");
		return false;
	}

	// Create the entity that will hold the scene.
	m_pScene = m_pFbxSdkManager->CreateKFbxScene();
	return true;
}

bool CFbxFile::DestroyFbxSdkObjects()
{
	// Delete the FBX SDK manager. All the objects that have been allocated 
	// using the FBX SDK manager and that haven't been explicitely destroyed 
	// are automatically destroyed at the same time.
	OBJ_SAFE_DELETE(m_pFbxSdkManager);
	return true;
}
