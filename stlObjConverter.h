
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __STLOBJCONVERTER_H__
#define __STLOBJCONVERTER_H__

#include "toObjConverter.h"

namespace ZBPlugin
{

class CStlObjOptions;


// CStlObjConverter: STL to OBJ converter
class CStlObjConverter : public CToObjConverter
{
public:
	CStlObjConverter();
	~CStlObjConverter();

private:
	CStlObjConverter(const CStlObjConverter&); // Copy constructor
	void operator=(const CStlObjConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pOtherFile, CFile* pObjFile);

private:
	CStlFile*		m_pStlFile;
	CStlObjOptions*	m_pOptions;
};


class CStlObjOptions : public CToObjOptions
{

};

} // End ZBPlugin namespace

#endif // __STLOBJCONVERTER_H__
