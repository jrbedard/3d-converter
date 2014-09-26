
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __W3DOBJCONVERTER_H__
#define __W3DOBJCONVERTER_H__

#include "toObjConverter.h"

namespace ZBPlugin
{

class CW3dObjOptions;


// CW3dObjConverter: W3D to OBJ converter
class CW3dObjConverter : public CToObjConverter
{
public:
	CW3dObjConverter();
	~CW3dObjConverter();

private:
	CW3dObjConverter(const CW3dObjConverter&); // Copy constructor
	void operator=(const CW3dObjConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pOtherFile, CFile* pObjFile);

private:
	CW3dFile*		m_pW3dFile;
	CW3dObjOptions*	m_pOptions;
};


class CW3dObjOptions : public CToObjOptions
{

};

} // End ZBPlugin namespace

#endif // __W3DOBJCONVERTER_H__
