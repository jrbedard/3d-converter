
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __LWOOBJCONVERTER_H__
#define __LWOOBJCONVERTER_H__


#include "toObjConverter.h"

namespace ZBPlugin
{

class CObjFile;
class CLwoFile;
class CLwoObjOptions;


// LWO to OBJ converter
class CLwoObjConverter : public CToObjConverter
{
public:
	CLwoObjConverter();
	~CLwoObjConverter();

private:
	CLwoObjConverter(const CLwoObjConverter&); // Copy constructor
	void operator=(const CLwoObjConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pOtherFile, CFile* pObjFile);

private: // load LWO
	bool CreateObjMaterialLibrary(CLwoFile::CLayer& layer);

private:
	CLwoFile*		m_pLwoFile;
	CLwoObjOptions*	m_pOptions;
};


class CLwoObjOptions : public CToObjOptions
{
	
};

} // End ZBPlugin namespace

#endif // __LWOOBJCONVERTER_H__
