
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "mayaFile.h"

#include "objMayaConverter.h"


CObjMayaConverter::CObjMayaConverter():
m_pMayaFile(NULL),
m_pOptions(NULL)
{
}

CObjMayaConverter::~CObjMayaConverter()
{
}


bool CObjMayaConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CObjMayaOptions();
	CFromObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}


// Convert an OBJ file into a Maya file
bool CObjMayaConverter::Convert(CFile* pObjFile, CFile* pOtherFile)
{
	// Initialize OBJ -> MAYA
	m_pMayaFile = InitConversion<CMayaFile>(pObjFile, pOtherFile);
	OBJ_ASSERT(m_pMayaFile && m_pOptions);
	if(!m_pMayaFile || !m_pOptions)
		return false;

	MSG_INFO("Converting OBJ file format into Maya file format..."); 

	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();
	CObjFile::GroupVector::iterator groupIt;
	CObjFile::GroupVector::const_iterator groupEnd = groups.end();


	// for every groups
	for(groupIt = groups.begin(); groupIt != groupEnd; ++groupIt) // for each OBJ group
	{
		std::string objectName(groupIt->GetGroupName());
		MayaStringCheck(objectName);

		// verify that it is a valid group
		if(groupIt->GetFaceCount() <= 0 )
			continue;


		MSG_DEBUG(objectName << "...");

		// a group corresponds to a Maya mesh node
		CMayaFile::CMesh mesh(objectName);


		// Extract only the refered block of vertex, texture coordinates and normals
		CObjFile::CGroup::SChannelsRange channelsRange = groupIt->GetChannelsRange();


		// "Copy" Vertex positions block for this mesh
		{
			Vector3DVector& objVertexVector = m_pObjFile->GetVertexPositionVector();
			Vector3DVector::iterator vertexBegin, vertexEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(CreateVertexCopyIterators(channelsRange, objVertexVector, vertexBegin, vertexEnd))
			{
				Vector3DVector& mayaVertexVector = mesh.GetVertexPositionVector();
				mayaVertexVector.reserve(vertexEnd - vertexBegin);
				mayaVertexVector.insert(mayaVertexVector.end(), vertexBegin, vertexEnd); // Copy Vertice
			}
		}


		// "Copy" Texture coordinates block for this mesh
		{
			Vector3DVector& texCoordinates = m_pObjFile->GetTextureCoordVector();
			Vector3DVector::iterator texCoordBegin, texCoordEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(m_pOptions->m_bTranslateUVs && CreateTexCoordCopyIterators(channelsRange, texCoordinates, texCoordBegin, texCoordEnd))
			{
				Vector3DVector& mayaUvVector = mesh.GetTextureCoordVector();
				mayaUvVector.reserve(texCoordEnd - texCoordBegin);
				mayaUvVector.insert(mayaUvVector.end(), texCoordBegin, texCoordEnd); // Copy UVs
			}
		}


		Vector3DVector& mayaNormalVector = mesh.GetNormalVector();
/*
		// "Copy" Normals block for this mesh
		{
			Vector3DVector& objNormalVector = m_pObjFile->GetNormalVector();
			Vector3DVector::iterator normalBegin, normalEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(m_options.m_bTranslateNormals && CreateNormalCopyIterators(channelsRange, objNormalVector, normalBegin, normalEnd))
			{
				
				mayaNormalVector.insert(mayaNormalVector.end(), normalBegin, normalEnd); // Copy Normals
			}
		}
*/


		// Edges
		Vector3DVector meshEdges;

		// Faces
		{
			CObjFile::FaceVector& faces = groupIt->GetFaceVector();
			CObjFile::FaceVector::iterator faceIt;
			CObjFile::FaceVector::const_iterator faceEnd = faces.end();

			uint vertexIndexOffset = (channelsRange.firstVertex.Position()-1); // Vertex index Offset
			uint uvOffset = (channelsRange.firstVertex.TextureCoordinate()-1); // UV Offset

			CMayaFile::CMesh::PolyFaceVector polyFaces;
			polyFaces.reserve(faces.size());

			for(faceIt = faces.begin(); faceIt != faceEnd; ++faceIt) // for each OBJ face
			{
				uint vertexCount = faceIt->VertexCount();

				CMayaFile::CMesh::CPolyFace polyFace;
				CMayaFile::CMesh::CPolyFace::CFace face;
				CMayaFile::CMesh::CPolyFace::CMu mu;

				// Generate the Edge Vector
				std::vector<uint>& faceEdgeIndices = ConvertObjToEdgeVector(*faceIt, meshEdges, vertexIndexOffset);
				std::vector<uint>::iterator faceEdgesIt;
				std::vector<uint>::const_iterator faceEdgesEnd = faceEdgeIndices.end();

				for(faceEdgesIt = faceEdgeIndices.begin(); faceEdgesIt != faceEdgesEnd; ++faceEdgesIt) // for each edge composing the face
				{
					face.AddEdge(*faceEdgesIt); // add the edge index for this face
				}

				polyFace.SetFace(face);

				for(uint v=0; v < vertexCount; ++v ) // For each OBJ vertex
				{
					CObjFile::CVertex vertex = faceIt->Vertex(v);

					// UV
					if(m_pOptions->m_bTranslateUVs && vertex.HasTextureCoordinate())
					{
						mu.AddUV( (vertex.TextureCoordinate()-1) - uvOffset); // Maya is 0-based index, offset
					}

					// Normal
					if(m_pOptions->m_bTranslateNormals && vertex.HasNormal())
					{
						Vector3D normal = m_pObjFile->Normal(vertex.Normal());
						mayaNormalVector.push_back(normal); // add normal for this per-poly-vertex
						face.AddNormal(v);
					}

				}
				polyFace.SetMu(mu);
				polyFaces.push_back(polyFace);
			}

			mesh.GetPolyFaceVector() = polyFaces;
		}

		mesh.GetEdgeVector() = meshEdges; // edge pool for the mesh
		m_pMayaFile->AddMesh(mesh); // Add Mesh to Maya file
	}


	// Materials
	if(m_pOptions->m_bTranslateMaterials)
	{
		CMtlFile* pMtlFile = m_pObjFile->GetMaterialFile();

		m_pMayaFile->SetMaterialFile(pMtlFile); // Set OBJ material to Maya file
	}

	return true;
}



void CObjMayaConverter::MayaStringCheck(std::string& name)
{
	size_t pos;

	pos = name.find("#");
	if(pos != std::string::npos)
		name.replace(pos, 1, "_");

	pos = name.find(":");
	if(pos != std::string::npos)
		name.replace(pos, 1, "_");

	pos = name.find("-");
	if(pos != std::string::npos)
		name.replace(pos, 1, "_");
}




// Build a vector of Edge objects
std::vector<uint> CObjMayaConverter::ConvertObjToEdgeVector(const CObjFile::CFace& face, Vector3DVector& meshEdges, uint vertexIndexOffset)
{
	std::vector<uint> faceEdgeIndices;

	uint vCount, v1, v2;
	vCount = face.VertexCount();

	for(uint v=0; v < vCount; ++v) // for each vertex on the face
	{
		// 1st Vertex index, OBJ is 1-based index, Maya is 0-based index
		v1 = face.Vertex( v ).Position() - 1;
		v1 -= vertexIndexOffset; // substract the global vertex position offset

		// 2nd Vertex index (1 increment), OBJ is 1-based index, Maya is 0-based index
		v2 = face.Vertex( v==(vCount-1) ? 0 : v+1 ).Position() - 1;
		v2 -= vertexIndexOffset; // substract the global vertex position offset

		Vector3DVector::iterator edgeIt;
		Vector3DVector::const_iterator edgeEnd = meshEdges.end();

		// look if the edge is already in the vector
		int edgeId = -1;
		for(edgeIt = meshEdges.begin(); edgeIt != edgeEnd; ++edgeIt) // for each mesh edge
		{
			if((v1 == (*edgeIt)[0] || v1 == (*edgeIt)[1]) &&
				(v2 == (*edgeIt)[0] || v2 == (*edgeIt)[1]))
			{
				edgeId = edgeIt - meshEdges.begin(); // existing edge at
			}
		}

		if(edgeId == -1) // new edge
		{
			Vector3D edge(3); // 3D for consistency
			edge[0] = v1;
			edge[1] = v2;
			edge[2] = 0;
			meshEdges.push_back(edge);
			edgeId = meshEdges.size()-1;
		}

		faceEdgeIndices.push_back(edgeId); // add the edge to the face
	}

	return faceEdgeIndices; // One step closer to the edge
}

