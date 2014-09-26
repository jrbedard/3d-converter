
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XYZOBJCONVERTER_H__
#define __XYZOBJCONVERTER_H__


#include "toObjConverter.h"

namespace ZBPlugin
{

class CXyzFile;
class CXyzObjOptions;

// CXyzObjConverter: XYZ to OBJ converter
class CXyzObjConverter : public CToObjConverter
{
public: // Conversion options
	
public:
	CXyzObjConverter();
	~CXyzObjConverter();

private:
	CXyzObjConverter(const CXyzObjConverter&); // Copy constructor
	void operator=(const CXyzObjConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pOtherFiles, CFile* pObjFile);

private:
	CXyzFile*		m_pXyzFile;
	CXyzObjOptions*	m_pOptions;
};


class CXyzObjOptions : public CToObjOptions
{
	uint m_sampling;
};


} // End ZBPlugin namespace

#endif // __XYZOBJCONVERTER_H__