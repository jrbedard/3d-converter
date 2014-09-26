
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __3DSREADER_H__
#define __3DSREADER_H__


#include "binaryReader.h"
#include "3dsFile.h"

namespace ZBPlugin
{

class CFile;

// C3dsReader: 3DS file format reader class
class C3dsReader : public CBinaryReader
{
public:
	C3dsReader(CFile* pFile, const fs::path& fileName);

private:
	C3dsReader(const C3dsReader&); // Copy constructor
	void operator=(const C3dsReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug(){return true;}

private:
	bool	ReadHeader();
	bool	ParseChunks();

	bool 	ReadEditorChunk();
	bool 	ReadObjBlock();
	bool 	ReadTriMesh();
	bool 	ReadVertList();
	bool 	ReadMapList();
	bool 	ReadFaceList();
	bool 	ReadFaceMat();
	bool 	ReadSmoothList(long chunkSize);
	bool 	ReadTrMatrix();
	bool 	ReadMatName();
	bool 	ReadMapFile();
	bool 	ReadOneUnit();


	// should be generalized and put in binaryReader
	bool 	ReadRGBF();
	bool 	ReadRGBB();

	bool	SkipChunk(C3dsFile::SChunk& chunk);

private:
	// Inheritable functions for polymorphism
	// debug only
	virtual void	UserFaceMaterial(std::string Name, int Number);
	virtual void	UserTransformMatrix(const C3dsFile::STransform3dsMatrix &Transform, const C3dsFile::STranslate3dsMatrix &Translate);
	virtual void	UserMatName(const std::string Name);
	virtual void	UserMapFile(const std::string FileName);
	virtual void	UserOneUnit(float Unit);
	virtual void	UserEndOfFile();

private:
	uint		m_curMaterial;
	C3dsFile*	m_p3dsFile;

};

} // End ZBPlugin namespace

#endif // __3DSREADER_H__
