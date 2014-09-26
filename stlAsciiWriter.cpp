
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "stlAsciiWriter.h"


CStlAsciiWriter::CStlAsciiWriter(CFile* pFile, const fs::path& fileName, bool bExportBinary)
{
	CStlFile* pStlFile = static_cast<CStlFile*>(pFile);

	m_pStlFile = pStlFile;

	m_fileName = fileName;
}


bool CStlAsciiWriter::Write()
{
	OBJ_ASSERT(m_pStlFile);
	if(!m_pStlFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	// Verify here that we have something to actually write
	OBJ_ASSERT(m_pStlFile->GetSolidVector().size() > 0);

	if(m_pStlFile->GetSolidVector().size() <= 0)
	{
		MSG_ERROR("No STL information to write!");
		return false;
	}

	OBJ_ASSERT(CheckStr(fileName));
	if(!CheckStr(fileName))
		return false;

	m_ofs.open(fileName.c_str());

	MSG_INFO("Writing STL file (.STL) : '" << fileName << "'.");
	if( !m_ofs.is_open() )
	{
		MSG_ERROR("Couldn't write to STL file '" << fileName << "'");
		return false;
	}

	// Get the STL solid vector
	CStlFile::SolidVector& solids = m_pStlFile->GetSolidVector();
	CStlFile::SolidVector::iterator solidIt;
	CStlFile::SolidVector::const_iterator solidEnd = solids.end();

	for(solidIt = solids.begin(); solidIt != solidEnd; ++solidIt) // for each STL solid (object)
	{
		std::string solidName(solidIt->GetSolidName());

		m_ofs << "solid " << solidName << std::endl;

		CStlFile::CSolid::FacetVector& facets = solidIt->GetFacetVector();
		CStlFile::CSolid::FacetVector::iterator facetIt;
		CStlFile::CSolid::FacetVector::const_iterator facetEnd = facets.end();

		// verify that it is a valid mesh
		if(facets.size() <= 0)
			continue;

		for(facetIt = facets.begin(); facetIt != facetEnd; ++facetIt) // for each STL facet
		{
			m_ofs << "  facet normal " << facetIt->m_normal[0] << " " << facetIt->m_normal[1] << " " << facetIt->m_normal[2] << std::endl;

			Vector3DVector& vertices = facetIt->m_vertices;
			Vector3DVector::iterator vertexIt;
			Vector3DVector::const_iterator vertexEnd = vertices.end();

			m_ofs << "    outer loop" << std::endl;

			for(vertexIt = vertices.begin(); vertexIt != vertexEnd; ++vertexIt) // for each STL vertex position
			{
				uint index = (vertexIt - vertices.begin());
				m_ofs << "      vertex " << (*vertexIt)[0] << " " << (*vertexIt)[1] << " " << (*vertexIt)[2] << std::endl;
			}

			m_ofs << "    endloop" << std::endl;
			m_ofs << "  endfacet" << std::endl;
		}

		m_ofs << "endsolid " << solidName << std::endl;
	}

	m_ofs.close();

	return true;
}
