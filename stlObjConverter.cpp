
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "stlFile.h"

#include "stlObjConverter.h"


CStlObjConverter::CStlObjConverter():
m_pStlFile(NULL),
m_pOptions(NULL)
{
}

CStlObjConverter::~CStlObjConverter()
{
}


bool CStlObjConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CStlObjOptions();
	CToObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}

// Convert a STL file into an OBJ file
bool CStlObjConverter::Convert(CFile* pOtherFile, CFile* pObjFile)
{
	// Initialize STL -> OBJ
	m_pStlFile = InitConversion<CStlFile>(pOtherFile, pObjFile);
	OBJ_ASSERT(m_pStlFile && m_pOptions);
	if(!m_pStlFile || !m_pOptions)
		return false;

	MSG_INFO("Converting STL file format into OBJ file format...");

	// Get the Obj group vector
	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();

	// Get the STL solid vector
	CStlFile::SolidVector& solids = m_pStlFile->GetSolidVector();
	CStlFile::SolidVector::iterator solidIt;
	CStlFile::SolidVector::const_iterator solidEnd = solids.end();

	for(solidIt = solids.begin(); solidIt != solidEnd; ++solidIt) // for each STL solid (object)
	{
		std::string solidName(solidIt->GetSolidName());

		CObjFile::CGroup group(solidName);

		CStlFile::CSolid::FacetVector& facets = solidIt->GetFacetVector();
		CStlFile::CSolid::FacetVector::iterator facetIt;
		CStlFile::CSolid::FacetVector::const_iterator facetEnd = facets.end();

		// verify that it is a valid mesh
		if(facets.size() <= 0 )
			continue;

		MSG_DEBUG(solidName << "...");

		for(facetIt = facets.begin(); facetIt != facetEnd; ++facetIt) // for each STL facet
		{
			CObjFile::CFace objFace;

			Vector3DVector vertices = facetIt->m_vertices;
			Vector3DVector::iterator vertexIt;
			Vector3DVector::const_iterator vertexEnd = vertices.end();

			for(vertexIt = vertices.begin(); vertexIt != vertexEnd; ++vertexIt) // for each STL vertex position
			{
				m_pObjFile->GetVertexPositionVector().push_back(*vertexIt);

				size_t pos = m_pObjFile->GetVertexPositionVector().size();

				// Add vertex to the OBJ face
				objFace.AddVertex( CObjFile::CVertex( pos, // OBJ is 1-index based
								   std::make_pair(false, 0), // no UVs in STL 
								   std::make_pair(false, 0) )); // no normals in STL
			}

			group.AddFace(objFace); // Add face to the group
		}

		m_pObjFile->AddGroup(group); // Add group to the OBJ file
	}

	return true;
}

