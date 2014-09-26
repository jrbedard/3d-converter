
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJ3DSCONVERTER_H__
#define __OBJ3DSCONVERTER_H__


#include "fromObjConverter.h"

namespace ZBPlugin
{

class CObjFile;
class C3dsFile;
class CObj3dsOptions;

// CObj3dsConverter: Converts OBJ into 3DS
class CObj3dsConverter : public CFromObjConverter
{
public:
	CObj3dsConverter();
	~CObj3dsConverter();

private:
	CObj3dsConverter(const CObj3dsConverter&); // Copy constructor
	void operator=(const CObj3dsConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pObjFile, CFile* pOtherFile);

private:
	C3dsFile*		m_p3dsFile;
	CObj3dsOptions*	m_pOptions;
};


class CObj3dsOptions : public CFromObjOptions
{
	bool m_bool1;
	bool m_bool2;
};

} // End ZBPlugin namespace

#endif // __OBJ3DSCONVERTER_H__


