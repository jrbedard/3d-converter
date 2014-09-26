
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "stlAsciiReader.h"

using namespace boost::spirit;


CStlAsciiReader::CStlAsciiReader(CFile* pFile, const fs::path& fileName)
{
	CStlFile* pStlFile = static_cast<CStlFile*>(pFile);

	m_pStlFile = pStlFile;

	m_fileName = fileName;
}


bool CStlAsciiReader::Read()
{
	OBJ_ASSERT(m_pStlFile);
	if(!m_pStlFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	const int maxline = 1000;
	char line[maxline];

	MSG_INFO("Reading Ascii .STL file '" << fileName << "'.");
	m_ifs.open(fileName.c_str());
	if( !m_ifs.is_open() )
	{
		MSG_ERROR("Couldn't open Ascii .STL file '" << fileName << "'");
		return true; // There is not stl file
	}

	int lineNum = 0;


	while( m_ifs )
	{
		++lineNum;

		m_ifs.getline( line, maxline ); // get the curent line pointer

		std::string command;
		parse_info<char const*> info = parse( line, *space_p >> +alpha_p[ append(command) ] );

		if( !info.hit ) // if parsing was'nt successful
			continue;

		const char* curPos = info.stop;

		//MSG_DEBUG("'" << command << "'");

		// Object Solid
		if( command == "solid" ) // new object declaration
		{
			std::string name;
			parse_info<char const*> info = parse( curPos, 
				*graph_p[ append(name) ], space_p );

			if( !info.full )
			{
				MSG_WARNING("Empty object name, " << fileName << ", line: " << lineNum);
				continue;
			}

			// Create the new material
			CStlFile::CSolid solid(name);

			m_pStlFile->GetSolidVector().push_back(solid);
		}


		// Face and Normal
		if( command == "facet" ) // Normal declaration
		{

			Vector3D normal(3);
			parse_info<char const*> info = parse( curPos, "normal " >>
				real_p[assign(normal[0])] >> real_p[assign(normal[1])] >> real_p[assign(normal[2])], 
				space_p );

			if( !info.full )
			{
				MSG_WARNING("Invalid normal, file: '" << fileName << "', line: " << lineNum);
				continue;
			}

			CStlFile::CSolid::CFacet newFacet;
			m_pStlFile->GetCurrentSolid().GetFacetVector().push_back(newFacet);

			m_pStlFile->GetCurrentSolid().GetCurrentFacet().m_normal = normal;
		}
 

		// Vertex
		if( command == "vertex" ) // Vertex declaration
		{
			Vector3D vertex(3);
			parse_info<char const*> info = parse( curPos, 
				real_p[assign(vertex[0])] >> real_p[assign(vertex[1])] >> real_p[assign(vertex[2])],
				space_p );

			if( !info.full )
			{
				MSG_WARNING("Invalid vertex, file: '" << fileName << "', line: " << lineNum);
				continue;
			}

			m_pStlFile->GetCurrentSolid().GetCurrentFacet().m_vertices.push_back( vertex );
		}

	}

	return true;
}


bool CStlAsciiReader::ReadDebug()
{
	return true;
}

