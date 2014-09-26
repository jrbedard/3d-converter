
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __FBXFILE_H__
#define __FBXFILE_H__


#include <stdio.h>

#include "file.h"
#include "fileException.h"


namespace FBXSDK_NAMESPACE
{ 
	class KFbxSdkManager;
	class KFbxScene;
}

namespace ZBPlugin
{

// CFbxFile: FBX SDK file
class CFbxFile : public CFile
{
public:
	CFbxFile();
	~CFbxFile();

	inline FBXSDK_NAMESPACE::KFbxSdkManager* GetFbxSdkManager() const {return m_pFbxSdkManager;}
	inline FBXSDK_NAMESPACE::KFbxScene* GetFbxScene() const {return m_pScene;}

private:
	bool InitializeFbxSdkObjects();
	bool DestroyFbxSdkObjects();

private:
	FBXSDK_NAMESPACE::KFbxSdkManager* m_pFbxSdkManager;
	FBXSDK_NAMESPACE::KFbxScene* m_pScene;
};

} // End ZBPlugin namespace

#endif // __FBXFILE_H__