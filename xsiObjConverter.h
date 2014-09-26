
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XSIOBJCONVERTER_H__
#define __XSIOBJCONVERTER_H__


#include "toObjConverter.h"


class CSLScene;
class CSLModel;
class CSLMesh;


namespace ZBPlugin
{

class CXsiObjOptions;

// CXsiObjConverter: XSI to OBJ converter
class CXsiObjConverter : public CToObjConverter
{
public:
	CXsiObjConverter();
	~CXsiObjConverter();

private:
	CXsiObjConverter(const CXsiObjConverter&); // Copy constructor
	void operator=(const CXsiObjConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pOtherFile, CFile* pObjFile);

private:
	// Load XSI
	bool DotXSILoadFileInfo(CSLScene* pScene);
	bool DotXSILoadMeshes(CSLModel* pModel);
	bool DotXSILoadModel(CSLModel* pModel, std::string& modelName);
	bool DotXSILoadMesh(CSLModel* pModel, CObjFile::CGroup& objGroup);

	bool DotXSILoadPolygonList(CSLMesh* pMesh, CObjFile::CGroup& objGroup);
	bool DotXSILoadTriangleList(CSLMesh* pMesh, CObjFile::CGroup& objGroup);
	bool DotXSILoadTriStripList(CSLMesh* pMesh, CObjFile::CGroup& objGroup);

private:
	CXsiFile*		m_pXsiFile;
	CXsiObjOptions*	m_pOptions;
};


class CXsiObjOptions : public CToObjOptions
{
	
};


} // End ZBPlugin namespace

#endif // __XSIOBJCONVERTER_H__
