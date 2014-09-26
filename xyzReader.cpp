
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "xyzReader.h"


using namespace boost::spirit;


CXyzReader::CXyzReader(CFile* pFile, const fs::path& fileName, ushort sampling)
{
	CXyzFile* pXyzFile = static_cast<CXyzFile*>(pFile);

	m_pXyzFile = pXyzFile;

	m_fileName = fileName;

	m_sampling = 1 << (sampling - 9);
	if(m_sampling < 1)
		m_sampling = 1; // avoid dividing by 0
}



bool CXyzReader::Read()
{
	OBJ_ASSERT(m_pXyzFile);
	if(!m_pXyzFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	const int maxline = 1000;
	char line[maxline];

	MSG_INFO("Reading .XYZ file '" << fileName << "'.");

	m_ifs.open(fileName.c_str());
	if( !m_ifs.is_open() )
	{
		MSG_ERROR("Couldn't open .XYZ file '" << fileName << "'");
		return false;
	}

	int lineNum = 0;

	while( m_ifs )
	{
		++lineNum;

		m_ifs.getline( line, maxline ); // get the curent line pointer

		Vector3D vertex(3);
		Vector3D texCoord(3);

		parse_info<char const*> info = parse( line,
			real_p[assign(vertex[0])] >> real_p[assign(vertex[1])] >> real_p[assign(vertex[2])] >>
			!( real_p[assign(texCoord[0])] >> real_p[assign(texCoord[1])] >> real_p[assign(texCoord[2])] ), // UV not mandatory
			space_p );

		//vertex /= 500.0f;

		if(!(lineNum % m_sampling))
		{
			m_pXyzFile->GetVertexPositionVector().push_back(vertex); // Add vertex position to XYZ file
			m_pXyzFile->GetTextureCoordVector().push_back(texCoord); // Add texCoord position to XYZ file
		}

		if(!info.full)
		{
			MSG_WARNING("Invalid line, file: '" << fileName << "', line: " << lineNum);
			continue;
		}
	}

	return true;
}