
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJSTLCONVERTER_H__
#define __OBJSTLCONVERTER_H__

#include "fromObjConverter.h"

namespace ZBPlugin
{

class CFile;
class CObjFile;
class CStlFile;
class CObjStlOptions;


// CObjStlConverter: OBJ to STL converter
class CObjStlConverter : public CFromObjConverter
{
public:
	CObjStlConverter();
	~CObjStlConverter();

private:
	CObjStlConverter(const CObjStlConverter&); // Copy constructor
	void operator=(const CObjStlConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pObjFile, CFile* pOtherFile);

private:
	CStlFile*		m_pStlFile;
	CObjStlOptions*	m_pOptions;
};


class CObjStlOptions : public CFromObjOptions
{
	
};

} // End ZBPlugin namespace

#endif // __OBJSTLCONVERTER_H__

