
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __3DSOBJCONVERTER_H__
#define __3DSOBJCONVERTER_H__


#include "toObjConverter.h"


namespace ZBPlugin
{

class C3dsFile;
class C3dsObjOptions;


// C3dsObjConverter: Converts 3DS into OBJ
class C3dsObjConverter : public CToObjConverter
{
public:
	C3dsObjConverter();
	~C3dsObjConverter();

private:
	C3dsObjConverter(const C3dsObjConverter&); // Copy constructor
	void operator=(const C3dsObjConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pOtherFile, CFile* pObjFile);

private:
	C3dsFile*		m_p3dsFile;
	C3dsObjOptions*	m_pOptions;
};


class C3dsObjOptions : public CToObjOptions
{
public:
	bool m_bool1;
	bool m_bool2;
};

} // End ZBPlugin namespace




#endif // __3DSOBJCONVERTER_H__