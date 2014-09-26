
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __MAYAOBJCONVERTER_H__
#define __MAYAOBJCONVERTER_H__

#include "toObjConverter.h"

namespace ZBPlugin
{

class CFile;
class CObjFile;
class CMayaFile;
class CMayaObjOptions;

// CMayaObjConverter: Maya to OBJ converter
class CMayaObjConverter : public CToObjConverter
{
public:
	CMayaObjConverter();
	~CMayaObjConverter();

private:
	CMayaObjConverter(const CMayaObjConverter&); // Copy constructor
	void operator=(const CMayaObjConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pOtherFile, CFile* pObjFile);

private:
	CMayaFile*		 m_pMayaFile;
	CMayaObjOptions* m_pOptions;
};


class CMayaObjOptions : public CToObjOptions
{
	
};

} // End ZBPlugin namespace

#endif // __MAYAOBJCONVERTER_H__