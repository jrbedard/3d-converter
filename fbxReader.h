
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __FBXREADER_H__
#define __FBXREADER_H__


#include "reader.h"


namespace FBXSDK_NAMESPACE
{ 
	class KFbxSdkManager;
	class KFbxScene;
	class KFbxImporter;
}


namespace ZBPlugin
{

class CFile;
class CFbxFile;

// CFbxReader: FBX file reader
class CFbxReader : public CReader
{
public:
	CFbxReader(CFile* pFile, const fs::path& fileName);

private:
	CFbxReader(const CFbxReader&); // Copy constructor
	void operator=(const CFbxReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug();

private:
	bool LoadScene(FBXSDK_NAMESPACE::KFbxSdkManager* pSdkManager, FBXSDK_NAMESPACE::KFbxScene* pScene, const char* pFilename);
	void SetFileFormat(const char* pFilename, FBXSDK_NAMESPACE::KFbxImporter* pImporter);

private:
	CFbxFile* m_pFbxFile;
};

} // End ZBPlugin namespace

#endif // __FBXREADER_H__