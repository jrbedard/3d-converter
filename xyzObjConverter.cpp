
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

// undef min and max from xsi ftk
#undef min
#undef max


#include "objFile.h"
#include "xyzFile.h"

#include "xyzObjConverter.h"



//Powercrust entry point
//extern void adapted_main(CXyzFile* pXyzFile); 



CXyzObjConverter::CXyzObjConverter():
m_pXyzFile(NULL),
m_pOptions(NULL)
{
}

CXyzObjConverter::~CXyzObjConverter()
{
}


bool CXyzObjConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CXyzObjOptions();
	CToObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}

bool CXyzObjConverter::Convert(CFile* pOtherFile, CFile* pObjFile)
{
	// Initialize XSI -> OBJ
	m_pXyzFile = InitConversion<CXyzFile>(pOtherFile, pObjFile);
	OBJ_ASSERT(m_pXyzFile && m_pOptions);
	if(!m_pXyzFile || !m_pOptions)
		return false;

	MSG_INFO("Converting XYZ file format into OBJ file format..."); 


	MSG_INFO("Creating mesh...");
	//adapted_main(m_pXyzFile); 


	//MSG_INFO

	// "Copy" Vertex positions
	{
		Vector3DVector& xyzVertexVector = m_pXyzFile->GetFinalSurface().GetPoints();
		Vector3DVector& objVertexVector = m_pObjFile->GetVertexPositionVector();
		objVertexVector.reserve(xyzVertexVector.size());
		objVertexVector.insert(objVertexVector.end(), xyzVertexVector.begin(), xyzVertexVector.end()); // Append new Vertices
	}

	/*
	// "Copy" Texture coordinates
	Vector3DVector& xyzUvVector = m_pXyzFile->GetTextureCoordVector();
	Vector3DVector& objUvVector = m_pObjFile->GetTextureCoordVector();
	objUvVector.reserve(xyzUvVector.size());
	objUvVector.insert(objUvVector.end(), xyzUvVector.begin(), xyzUvVector.end()); // Append new UVs
	*/

	CObjFile::CGroup objGroup;

	CXyzFile::CPolyData::PolyVector& xyzPolys = m_pXyzFile->GetFinalSurface().GetPolys();
	CXyzFile::CPolyData::PolyVector::iterator xyzPolyIt;
	CXyzFile::CPolyData::PolyVector::iterator xyzPolyEnd = xyzPolys.end();

	objGroup.GetFaceVector().reserve(xyzPolys.size()); // reserve face vector

	for(xyzPolyIt = xyzPolys.begin(); xyzPolyIt != xyzPolyEnd; ++xyzPolyIt) // for each XYZ face
	{
		CObjFile::CFace objFace;

		// Pairs
		std::pair< bool, uint > uvPair;
		std::pair< bool, uint > normalPair;

		CXyzFile::CPolyData::CPoly& xyzPoly = *xyzPolyIt;
		uint vertexCount = xyzPoly.m_vertIndices.size();

		for(uint vertexID = 0; vertexID < vertexCount; ++vertexID)
		{
			uvPair = std::make_pair(false, 0);
			normalPair = std::make_pair(false, 0);

			objFace.AddVertex( CObjFile::CVertex(xyzPoly.m_vertIndices[vertexID]+1, uvPair, normalPair) );
		}

		objGroup.AddFace(objFace);
	}

	m_pObjFile->AddGroup(objGroup); // Add group to OBJ file

	

	return true;
}




