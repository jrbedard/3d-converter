
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJW3DCONVERTER_H__
#define __OBJW3DCONVERTER_H__

#include "fromObjConverter.h"

namespace ZBPlugin
{

class CFile;
class CObjFile;
class CW3dFile;
class CObjW3dOptions;


// CObjW3dConverter: OBJ to W3D converter
class CObjW3dConverter : public CFromObjConverter
{
public:
	CObjW3dConverter();
	~CObjW3dConverter();

private:
	CObjW3dConverter(const CObjW3dConverter&); // Copy constructor
	void operator=(const CObjW3dConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pObjFile, CFile* pOtherFile);

private:
	CW3dFile*		m_pW3dFile;
	CObjW3dOptions*	m_pOptions;
};


class CObjW3dOptions : public CFromObjOptions
{
	
};

} // End ZBPlugin namespace

#endif // __OBJW3DCONVERTER_H__

