
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJMAYACONVERTER_H__
#define __OBJMAYACONVERTER_H__

#include "fromObjConverter.h"

namespace ZBPlugin
{

class CFile;
class CObjFile;
class CMayaFile;
class CObjMayaOptions;


// CObjMayaConverter: OBJ to Maya converter
class CObjMayaConverter : public CFromObjConverter
{
public:
	CObjMayaConverter();
	~CObjMayaConverter();

private:
	CObjMayaConverter(const CObjMayaConverter&); // Copy constructor
	void operator=(const CObjMayaConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pObjFile, CFile* pOtherFile);

private: // Utils
	void MayaStringCheck(std::string& name);
	std::vector<uint> ConvertObjToEdgeVector(const CObjFile::CFace& face, Vector3DVector& meshEdges, uint vertexIndexOffset);

private:
	CMayaFile*		  m_pMayaFile;
	CObjMayaOptions*  m_pOptions;
};


class CObjMayaOptions : public CFromObjOptions
{

};

} // End ZBPlugin namespace

#endif // __OBJMAYACONVERTER_H__