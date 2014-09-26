
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJXSICONVERTER_H__
#define __OBJXSICONVERTER_H__


#include "fromObjConverter.h"

namespace ZBPlugin
{

class CSLScene;

class CXsiFile;
class CObjXsiOptions;

// CObjXsiConverter: OBJ to XSI converter
class CObjXsiConverter : public CFromObjConverter
{
public:
	CObjXsiConverter();
	~CObjXsiConverter();

private:
	CObjXsiConverter(const CObjXsiConverter&); // Copy constructor
	void operator=(const CObjXsiConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pObjFile, CFile* pOtherFile);

private:
	// Save XSI
	bool SetXsiFileInfo(::CSLScene* pScene);
	bool SetDefaultSceneParamaters(::CSLScene* pScene);

private:
	CXsiFile*		m_pXsiFile;
	CObjXsiOptions*	m_pOptions;
};


class CObjXsiOptions : public CFromObjOptions
{
public:
};


} // End ZBPlugin namespace

#endif // __OBJXSICONVERTER_H__

