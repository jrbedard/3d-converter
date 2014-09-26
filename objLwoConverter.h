
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJLWOCONVERTER_H__
#define __OBJLWOCONVERTER_H__

#include "fromObjConverter.h"

namespace ZBPlugin
{

class CObjFile;
class CLwoFile;
class CObjLwoOptions;


// CObjLwoConverter: OBJ to LWO converter
class CObjLwoConverter : public CFromObjConverter
{
public:
	CObjLwoConverter();
	~CObjLwoConverter();

private:
	CObjLwoConverter(const CObjLwoConverter&); // Copy constructor
	void operator=(const CObjLwoConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pObjFile, CFile* pOtherFile);

private: // save LWO
	bool CreateLwoMaterialLibrary(CLwoFile::CLayer& layer);

private:
	CLwoFile*		m_pLwoFile;
	CObjLwoOptions*	m_pOptions;
};


class CObjLwoOptions : public CFromObjOptions
{
	
};

} // End ZBPlugin namespace

#endif // __OBJLWOCONVERTER_H__
