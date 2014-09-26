
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __FBXSDKMATERIALUTIL_H__
#define __FBXSDKMATERIALUTIL_H__

#include "objFile.h"
#include "fbxFile.h"

namespace ZBPlugin
{

// CFbxSdkMaterialUtil: FBX SDK material utilities
class CFbxSdkMaterialUtil
{
public:
	// Save FBX

	// Load FBX
	static bool ConvertMaterialMapping(KFbxMesh* pMesh, CObjFile* pObjFile, CObjFile::CGroup& objGroup);
	static int CreateMaterial(CObjFile* pObjFile, KFbxLayerElementMaterial* pLayerElementMat, int fbxMatIndex);
	static bool ConvertTextureMapping(KFbxMesh* pMesh, CObjFile* pObjFile, CObjFile::CGroup& objGroup);
};

} // End ZBPlugin namespace

#endif // __FBXSDKMATERIALUTIL_H__