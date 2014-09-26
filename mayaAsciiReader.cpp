
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "mayaAsciiReader.h"

using namespace boost::spirit;


CMayaAsciiReader::CMayaAsciiReader(CFile* pFile, const fs::path& fileName)
{
	CMayaFile* pMayaFile = static_cast<CMayaFile*>(pFile);
	OBJ_ASSERT(pMayaFile);

	m_pMayaFile = pMayaFile;

	m_fileName = fileName;
}


enum ParseZone
{
	eFile,
	eMesh,
	eTextureCoordinate,
	eVertexPosition,
	eEdge,
	eNormal,
	ePolyFaces
};



bool CMayaAsciiReader::Read()
{
	OBJ_ASSERT(m_pMayaFile);
	if(!m_pMayaFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	int faceID = -1;
	int materialID = -1;
	int normalIndex = 0;

	const int maxline = 1000;
	char line[maxline];

	int lineNum = 0;
	ParseZone parseZone = eFile;
	CMayaFile::CMesh curMesh("default");

	m_ifs.open(fileName.c_str());

	MSG_INFO("Reading Maya .MA file '" << fileName << "'.");
	if( !m_ifs.is_open() )
	{
		MSG_ERROR("Couldn't open .MA file '" << fileName << "'");
		return false;
	}


	while( m_ifs )
	{
		++lineNum;

		m_ifs.getline( line, maxline ); // get the curent line pointer

		const char* curPos = line;

		std::string command = "";




		parse_info<char const*> info;
		
		if( parseZone < eTextureCoordinate || parseZone == ePolyFaces ) // not during numerical parsing
		{
			info = parse( line, lexeme_d[ *space_p >> +alpha_p[ append(command) ] ] ); // skip spaces

			//if( !info.hit ) // if parsing was'nt successful
			//	continue;

			curPos = info.stop;
		}






		if( command == "createNode" ) // new Material declaration
		{
			std::string meshName, parentName;

			parse_info<char const*> info = parse( curPos, 
				" mesh -n \"" >> +alnum_p[ append(meshName) ] >> "\" -p \"" >> +alnum_p[ append(parentName) ] >> "\";" );

			CMayaFile::CMesh mesh(meshName);
			curMesh = mesh;

			parseZone = eMesh;
		}
		else if( command == "setAttr" )
		{
			int count;
			std::string attribute;

			// Parse the setAttr command, the -s XXXX is optional

			parse_info<char const*> info = parse( curPos,
				!(" -s " >> int_p[assign(count)]) >> " \"." >> *alpha_p[ append(attribute) ] >> "[");

			curPos = info.stop;



			if(attribute == "uvst") // texture coordinates
			{
				uint beginIndex, endIndex;
				uint vectorSize;

				parse_info<char const*> infoUV = parse( curPos,
					"0].uvsp[" >> int_p[assign(beginIndex)] >> ":" >> int_p[assign(endIndex)] >> "]\" " >>
					!("-type \"float" >> int_p[assign(vectorSize)] >> "\"") );

				curPos = infoUV.stop;
				parseZone = eTextureCoordinate;
			
			}
			else if(attribute == "vt") // vertex positions
			{
				uint beginIndex, endIndex;

				parse_info<char const*> infoVT = parse( curPos,
					int_p[assign(beginIndex)] >> ":" >> int_p[assign(endIndex)] >> "]\" ");

				curPos = infoVT.stop;
				parseZone = eVertexPosition;

			}
			else if(attribute == "ed") // edges
			{
				uint beginIndex, endIndex;

				parse_info<char const*> infoED = parse( curPos,
					int_p[assign(beginIndex)] >> ":" >> int_p[assign(endIndex)] >> "]\" ");

				curPos = infoED.stop;
				parseZone = eEdge;

			}
			else if(attribute == "n") // normals
			{
				uint beginIndex, endIndex;
				uint vectorSize;

				parse_info<char const*> infoN = parse( curPos,
					int_p[assign(beginIndex)] >> ":" >> int_p[assign(endIndex)] >> "]\" " >>
					!("-type \"float" >> int_p[assign(vectorSize)] >> "\"") );

				curPos = infoN.stop;
				parseZone = eNormal;

			}
			else if(attribute == "fc") // polyFaces
			{
				uint beginIndex, endIndex;
				std::string meshType;

				parse_info<char const*> infoFC = parse( curPos,
					int_p[assign(beginIndex)] >> ":" >> int_p[assign(endIndex)] >> "]\" " >>
					!("-type \"" >> +alpha_p[ append(meshType) ] >> "\"") );

				if(meshType != "polyFaces")
				{
					OBJ_ASSERT(false);
					MSG_WARNING("Not polyface");
				}

				normalIndex = 0; // reset the normalIndex for this new mesh

				curPos = infoFC.stop;
				parseZone = ePolyFaces;

			}
		}





		std::string numricalStr;

		// Numerical parsing
		if(parseZone == eTextureCoordinate ||
		   parseZone == eVertexPosition ||
		   parseZone == eEdge ||
		   parseZone == eNormal)
		{
			numricalStr += curPos;
			while(!strchr(line, ';')) // concatenate all the numerical coords until the line contains ';'
			{
				m_ifs.getline( line, maxline );
				++lineNum;
				numricalStr += line; 
			}
		}


		if(parseZone == eTextureCoordinate) // Texture coordinates
		{
			while(numricalStr.c_str())
			{
				Vector3D tex(3);
				tex[2] = 0.0f; // w defaults to 0

				parse_info<char const*> info = parse( numricalStr.c_str(),
					real_p[assign(tex[0])] >> real_p[assign(tex[1])],
					space_p );

				if(!info.hit) // end of texture coordinates
					break;

				curMesh.GetTextureCoordVector().push_back(tex);
				numricalStr = info.stop;
			}
			parseZone = eMesh;

		}
		else if(parseZone == eVertexPosition) // Vertex positions
		{
			while(numricalStr.c_str())
			{
				Vector3D pos(3);

				parse_info<char const*> info = parse( numricalStr.c_str(), 
					real_p[assign(pos[0])] >> real_p[assign(pos[1])] >> real_p[assign(pos[2])],
					space_p );

				if(!info.hit) // end of vertex positions
					break;

				curMesh.GetVertexPositionVector().push_back(pos);
				numricalStr = info.stop;
			}
			parseZone = eMesh;

		}
		else if(parseZone == eEdge) // Edges
		{
			while(numricalStr.c_str())
			{
				Vector3D edge(3);

				parse_info<char const*> info = parse( numricalStr.c_str(), 
					real_p[assign(edge[0])] >> real_p[assign(edge[1])] >> real_p[assign(edge[2])], // the 3rd is always 0 (2 vertex per edge)
					space_p );

				if(!info.hit) // end of edges
					break;

				curMesh.GetEdgeVector().push_back(edge); // TODO : should be integers instead of double...
				numricalStr = info.stop;
			}
			parseZone = eMesh;

		}
		else if(parseZone == eNormal) // Normals
		{
			while(numricalStr.c_str())
			{
				Vector3D normal(3);

				parse_info<char const*> info = parse( numricalStr.c_str(), 
					real_p[assign(normal[0])] >> real_p[assign(normal[1])] >> real_p[assign(normal[2])],
					space_p );

				if(!info.hit) // end of normals
					break;

				curMesh.GetNormalVector().push_back(normal);
				numricalStr = info.stop;
			}
			parseZone = eMesh;

		}




		// faces
		if(command == "f" && parseZone == ePolyFaces)
		{
			const char* endLine = line + strlen(line);
			uint edgeCount;

			CMayaFile::CMesh::CPolyFace polyface;
			CMayaFile::CMesh::CPolyFace::CFace face;

			parse_info<char const*> info = parse( curPos, int_p[assign(edgeCount)], space_p);
			curPos = info.stop;

			while( curPos < endLine ) // For each edge index
			{
				int edgeId;

				parse_info<char const*> infoEd = parse( curPos, int_p[assign(edgeId)], space_p);

				if( !info.hit )
				{
					MSG_WARNING("Invalid vertex, " << fileName << ", line:" << lineNum);
				}

				face.AddEdge(edgeId); // add edge index to the face
				
				if(curMesh.GetNormalVector().size() > 0) // curent mesh contain normals
				{
					face.AddNormal(normalIndex); // add normal index to the face (1 per polygon vertex)
					++normalIndex; // increment index;
				}

				curPos = infoEd.stop;

				if(!strcmp(curPos,";")) // end of PolyFaces
					break;
			}

			faceID++;
			polyface.SetFace(face);
			curMesh.GetPolyFaceVector().push_back(polyface);
		}

		// Mus
		if(command == "mu" && parseZone == ePolyFaces)
		{
			const char* endLine = line + strlen(line);
			uint uvSet, uvCount;

			CMayaFile::CMesh::CPolyFace::CMu mu;

			parse_info<char const*> info = parse( curPos, int_p[assign(uvSet)] >> int_p[assign(uvCount)], space_p);
			curPos = info.stop;

			while( curPos < endLine ) // for each UV coord index
			{
				int uvId;

				parse_info<char const*> infoMu = parse( curPos, int_p[assign(uvId)], space_p);

				if( !info.hit )
				{
					MSG_WARNING("Invalid vertex, " << fileName << ", line:" << lineNum);
				}

				mu.AddUV(uvId);
				curPos = infoMu.stop;

				if(!strcmp(curPos,";")) // end of polyfaces
					break;
			}

			curMesh.GetPolyFaceVector().back().SetMu(mu); // add texture coordinates to the current polyface
		}


		if(parseZone == ePolyFaces)
		{
			if(!strcmp(curPos,";")) // if we reached the end of polyface
			{
				parseZone = eMesh;
				m_pMayaFile->AddMesh(curMesh);
			}
		}


		// TODO : shading groups / materials / textures

	}

	m_ifs.close();

	return true;
}