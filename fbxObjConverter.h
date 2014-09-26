
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __FBXOBJCONVERTER_H__
#define __FBXOBJCONVERTER_H__

#include "toObjConverter.h"


namespace FBXSDK_NAMESPACE
{ 
	class KFbxNode;
	class KFbxMesh;
	class KFbxMatrix;
}

namespace ZBPlugin
{

class CObjFile;
class CFbxFile;
class CFbxObjOptions;

// CFbxObjConverter: Converts FBX into OBJ
class CFbxObjConverter : public CToObjConverter
{
public:
	CFbxObjConverter();
	~CFbxObjConverter();

private:
	CFbxObjConverter(const CFbxObjConverter&); // Copy constructor
	void operator=(const CFbxObjConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pObjFile, CFile* pOtherFile);

private:
	// Load
	bool NodeRecurse(FBXSDK_NAMESPACE::KFbxNode* pNode);

	// TODO : split thoses in utility classes
	bool ConvertMesh(FBXSDK_NAMESPACE::KFbxNode* pNode);
	bool ConvertControlPoints(FBXSDK_NAMESPACE::KFbxMesh* pMesh, FBXSDK_NAMESPACE::KFbxMatrix& transMat);
	bool ConvertPolygons(FBXSDK_NAMESPACE::KFbxMesh* pMesh, CObjFile::CGroup& objGroup);

private:
	int m_materialIndexOffset;

private:
	CFbxFile*		m_pFbxFile;
	CFbxObjOptions*	m_pOptions;
};


class CFbxObjOptions : public CToObjOptions
{
public:
};

} // End ZBPlugin namespace

#endif // __FBXOBJCONVERTER_H__
