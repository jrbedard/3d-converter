
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "mayaFile.h"

#include "mayaObjConverter.h"


CMayaObjConverter::CMayaObjConverter():
m_pMayaFile(NULL),
m_pOptions(NULL)
{
}

CMayaObjConverter::~CMayaObjConverter()
{
}


bool CMayaObjConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CMayaObjOptions();
	CToObjConverter::ParseOptions(optionString, m_pOptions);
	return true;	
}

// Convert a Maya file into an OBJ file
bool CMayaObjConverter::Convert(CFile* pOtherFile, CFile* pObjFile)
{
	// Initialize MAYA -> OBJ
	m_pMayaFile = InitConversion<CMayaFile>(pOtherFile, pObjFile);
	OBJ_ASSERT(m_pMayaFile && m_pOptions);
	if(!m_pMayaFile || !m_pOptions)
		return false;

	MSG_INFO("Converting Maya file format into OBJ file format..."); 

	// Get the Obj group vector
	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();

	// Get the Maya mesh vector
	CMayaFile::MeshVector& meshes = m_pMayaFile->GetMeshVector();
	CMayaFile::MeshVector::iterator meshIt;
	CMayaFile::MeshVector::const_iterator meshEnd = meshes.end();

	for(meshIt = meshes.begin(); meshIt != meshEnd; ++meshIt) // for each Maya mesh
	{
		std::string meshName(meshIt->GetMeshName());

		// verify that it is a valid mesh
		if(meshIt->GetPolyFaceVector().size() <= 0 )
			continue;

		MSG_DEBUG(meshName << "...");

		// a group corresponds to a Maya mesh node
		CObjFile::CGroup group(meshName);


		// "Append" Vertex positions
		{
			Vector3DVector& mayaVertexVector = meshIt->GetVertexPositionVector();
			Vector3DVector& objVertexVector = m_pObjFile->GetVertexPositionVector();
			objVertexVector.reserve(mayaVertexVector.size());
			objVertexVector.insert(objVertexVector.end(), mayaVertexVector.begin(), mayaVertexVector.end()); // Append new Vertices
		}

		// "Append" Texture coordinates
		{
			Vector3DVector& mayaUvVector = meshIt->GetTextureCoordVector();
			Vector3DVector& objUvVector = m_pObjFile->GetTextureCoordVector();
			objUvVector.reserve(mayaUvVector.size());
			objUvVector.insert(objUvVector.end(), mayaUvVector.begin(), mayaUvVector.end()); // Append new UVs
		}

		// "Append" Normals
		{
			Vector3DVector& mayaNormalVector = meshIt->GetNormalVector();
			Vector3DVector& objNormalVector = m_pObjFile->GetNormalVector();
			objNormalVector.reserve(mayaNormalVector.size());
			objNormalVector.insert(objNormalVector.end(), mayaNormalVector.begin(), mayaNormalVector.end()); // Append new Normals
		}

		// Faces
		{
			CMayaFile::CMesh::PolyFaceVector& polyFaces = meshIt->GetPolyFaceVector();
			CMayaFile::CMesh::PolyFaceVector::iterator polyFaceIt;
			CMayaFile::CMesh::PolyFaceVector::const_iterator polyFaceEnd = polyFaces.end();

			group.GetFaceVector().reserve(polyFaces.size()); // reserve face vector

			for(polyFaceIt = polyFaces.begin(); polyFaceIt != polyFaceEnd; ++polyFaceIt) // for each Maya face
			{
				CMayaFile::CMesh::CPolyFace::CFace mayaFace = polyFaceIt->GetFace();
				CMayaFile::CMesh::CPolyFace::CMu mayaMu;

				if(polyFaceIt->HasTextureCoordinate())
					mayaMu = polyFaceIt->GetMu();

				CObjFile::CFace objFace;

				uint edgeCount = mayaFace.EdgeCount();
				uint debugFace = polyFaceIt - polyFaces.begin();

				int vIndex = 0;
				std::pair< bool, uint > uvIndex;
				std::pair< bool, uint > normalIndex;

				for(uint v=0; v < edgeCount; ++v ) // for each vertex in maya polyface
				{
					vIndex = 0;
					uvIndex = std::make_pair(false, 0);
					normalIndex = std::make_pair(false, 0);

					OBJ_ASSERT(v < mayaFace.GetEdgeVector().size());
					int edgeIndex = mayaFace.GetEdgeVector()[v];

					if(edgeIndex >= 0) // edge positive direction : take the first vertex of the edge
					{
						OBJ_ASSERT(edgeIndex < (int)meshIt->GetEdgeVector().size());
						vIndex = (int)meshIt->GetEdgeVector()[edgeIndex][0] + 1; // OBJ is 1-based index
					}
					else // edge negative direction : make the edge index positive, decrement 1 and take the second vertex of the edge
					{
						OBJ_ASSERT(abs(edgeIndex)-1 <= (int)meshIt->GetEdgeVector().size()-1);
						vIndex = (int)meshIt->GetEdgeVector()[abs(edgeIndex)-1][1] + 1; // OBJ is 1-based index
					}

					// UVs
					if(polyFaceIt->HasTextureCoordinate())
						uvIndex = std::make_pair(true, mayaMu.GetUvVector()[v] + 1); // absolute UV index, OBJ is 1-based index

					// Normals
					if(polyFaceIt->HasNormal())
					{
						OBJ_ASSERT(v < mayaFace.GetNormalVector().size());
						normalIndex = std::make_pair(true, mayaFace.GetNormalVector()[v] + 1); // OBJ is 1-based index
					}

					// Add vertex to OBJ face
					objFace.AddVertex( CObjFile::CVertex(vIndex, uvIndex, normalIndex) );
				}

				group.AddFace(objFace); // Add face to OBJ group
			}

		}

		m_pObjFile->AddGroup(group); // Add group to OBJ file
	}


	// Materials
	{
		CMtlFile* pMtlFile = m_pObjFile->GetMaterialFile();

		OBJ_ASSERT(pMtlFile);
		if(!pMtlFile)
			return true;

		// Retreive and set material properties here
	}


	return true;
}





