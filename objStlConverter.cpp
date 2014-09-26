
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "stlFile.h"

#include "objStlConverter.h"


CObjStlConverter::CObjStlConverter():
m_pStlFile(NULL),
m_pOptions(NULL)
{
}

CObjStlConverter::~CObjStlConverter()
{
}


bool CObjStlConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CObjStlOptions();
	CFromObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}

// Convert an OBJ file into a STL file
bool CObjStlConverter::Convert(CFile* pObjFile, CFile* pOtherFile)
{
	// Initialize OBJ -> STL
	m_pStlFile = InitConversion<CStlFile>(pObjFile, pOtherFile);
	OBJ_ASSERT(m_pStlFile && m_pOptions);
	if(!m_pStlFile || !m_pOptions)
		return false;

	MSG_INFO("Converting OBJ file format into STL file format...");

	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();
	CObjFile::GroupVector::iterator groupIt;
	CObjFile::GroupVector::const_iterator groupEnd = groups.end();

	for(groupIt = groups.begin(); groupIt != groupEnd; ++groupIt) // for each OBJ groups
	{
		std::string objectName(groupIt->GetGroupName());

		// verify that it is a valid group
		if(groupIt->GetFaceCount() <= 0 )
			continue;

		CStlFile::CSolid solid(objectName);

		// Faces
		{
			CObjFile::FaceVector& objFaces = groupIt->GetFaceVector();
			CObjFile::FaceVector::iterator faceIt;
			CObjFile::FaceVector::const_iterator faceEnd = objFaces.end();

			for(faceIt = objFaces.begin(); faceIt != faceEnd; ++faceIt) // for each OBJ face
			{

				CStlFile::CSolid::CFacet newFacet;

				uint vertexCount = faceIt->VertexCount();
				OBJ_ASSERT(vertexCount <= 3); // triangle only in STL files
				if(vertexCount > 3)
					vertexCount = 3;

				for(uint v=0; v < vertexCount; ++v) // for each OBJ vertex in face
				{
					CObjFile::CVertex vertex = faceIt->Vertex(v);
					uint vertexPosition = vertex.Position()-1; // OBJ is 1-based index

					OBJ_ASSERT(vertexPosition < m_pObjFile->GetVertexPositionVector().size());
					if(vertexPosition >= m_pObjFile->GetVertexPositionVector().size())
						continue;

					Vector3D vertPos = m_pObjFile->GetVertexPositionVector()[vertexPosition];
					newFacet.m_vertices.push_back(vertPos);

					// TODO: process the facet normal...
/*
					Vector3D normal(3);
					normal[0] = 0.0f;
					normal[1] = 0.0f;
					normal[2] = 1.0f;
					newFacet.m_normal = normal;
*/
				}

				solid.GetFacetVector().push_back(newFacet);

			}
		}

		m_pStlFile->GetSolidVector().push_back(solid);
	}

	return true;
}

