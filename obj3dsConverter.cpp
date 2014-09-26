
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "3dsFile.h"

#include "obj3dsConverter.h"


CObj3dsConverter::CObj3dsConverter():
m_p3dsFile(NULL),
m_pOptions(NULL)
{

}

CObj3dsConverter::~CObj3dsConverter()
{
}


bool CObj3dsConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CObj3dsOptions();
	CFromObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}


// Convert an OBJ file into a 3DS file
bool CObj3dsConverter::Convert(CFile* pObjFile, CFile* pOtherFile)
{
	// Initialize OBJ -> 3DS
	m_p3dsFile = InitConversion<C3dsFile>(pObjFile, pOtherFile);
	OBJ_ASSERT(m_p3dsFile && m_pOptions);
	if(!m_p3dsFile || !m_pOptions)
		return false;

	MSG_INFO("Converting OBJ file format into 3DS file format..."); 

	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();
	CObjFile::GroupVector::iterator groupIt;
	CObjFile::GroupVector::const_iterator groupEnd = groups.end();

	for(groupIt = groups.begin(); groupIt != groupEnd; ++groupIt) // for each OBJ groups
	{
		std::string objectName(groupIt->GetGroupName());

		// verify that it is a valid group
		if(groupIt->GetFaceCount() <= 0 )
			continue;

		// a group corresponds to a 3DS object
		C3dsFile::CObject object(objectName);
		C3dsFile::CObject::CMesh mesh;

		// Extract only the refered block of vertex, texture coordinates and normals
		CObjFile::CGroup::SChannelsRange channelsRange = groupIt->GetChannelsRange();

		// Vertex positions
		{
			OBJ_ASSERT(mesh.VertexCount() < SIZE_USHORT); // max number of vertice
			if(mesh.VertexCount() >= SIZE_USHORT)
			{
				MSG_WARNING("Vertex number overflow, The 3DS file format only support less than " << SIZE_USHORT << " vertices per mesh, aborting mesh " << objectName);
				continue;
			}

			Vector3DVector& vertices = m_pObjFile->GetVertexPositionVector();
			Vector3DVector::iterator objVertexBegin, objVertexEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(CreateVertexCopyIterators(channelsRange, vertices, objVertexBegin, objVertexEnd))
			{
				Vector3DVector& maxVertexVec = mesh.GetVertexPositionVector();
				maxVertexVec.reserve(objVertexEnd - objVertexBegin);
				maxVertexVec.insert(maxVertexVec.end(), objVertexBegin, objVertexEnd);
			}
		}


		// UV coordinates
		{
			Vector3DVector& texCoords = m_pObjFile->GetTextureCoordVector();
			Vector3DVector::iterator objTexCoordBegin, objTexCoordEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(m_pOptions->m_bTranslateUVs && CreateTexCoordCopyIterators(channelsRange, texCoords, objTexCoordBegin, objTexCoordEnd))
			{
				OBJ_ASSERT(mesh.MapCoordCount() < SIZE_USHORT); // max number of UVs
				if(mesh.MapCoordCount() >= SIZE_USHORT)
				{
					MSG_WARNING("UV number overflow, The 3DS file format only support less than " << SIZE_USHORT << " UVs per mesh, aborting mesh " << objectName);
					continue;
				}

				Vector3DVector& maxLayerTexCoordVec = mesh.GetMapCoordVector();
				maxLayerTexCoordVec.reserve(objTexCoordEnd - objTexCoordBegin);
				maxLayerTexCoordVec.insert(maxLayerTexCoordVec.end(), objTexCoordBegin, objTexCoordEnd);
			}
		}



		// Faces
		{
			CObjFile::FaceVector& objFaces = groupIt->GetFaceVector();
			CObjFile::FaceVector::iterator faceIt;
			CObjFile::FaceVector::const_iterator faceEnd = objFaces.end();

			OBJ_ASSERT(mesh.FaceCount() < SIZE_USHORT); // max number of face
			if(mesh.FaceCount() >= SIZE_USHORT)
			{
				MSG_WARNING("Polygon number overflow, The 3DS file format only support less than " << SIZE_USHORT << " polygon per mesh, aborting mesh " << objectName);
				continue;
			}

			mesh.GetFaceVector().reserve(objFaces.size());

			for(faceIt = objFaces.begin(); faceIt != faceEnd; ++faceIt) // for each OBJ face
			{
				uint vertexCount = faceIt->VertexCount();
				OBJ_ASSERT(vertexCount <= 3); // triangle only in 3DS files

				/*
				if(vertexCount == 4) // HACK : we must triangulate, 3DS file format only support triangles
				{
					C3dsFile::CObject::CMesh::CFace face3ds2;
					ushort vertexPosition;

					CObjFile::CVertex vertex1 = faceIt->Vertex(2);
					vertexPosition = (ushort)vertex1.Position()-1; // OBJ is 1-based index -> 3DS is 0-based index
					face3ds2.m_vertexIndex.push_back(vertexPosition);

					CObjFile::CVertex vertex2 = faceIt->Vertex(3);
					vertexPosition = (ushort)vertex2.Position()-1; // OBJ is 1-based index -> 3DS is 0-based index
					face3ds2.m_vertexIndex.push_back(vertexPosition);

					CObjFile::CVertex vertex3 = faceIt->Vertex(0);
					vertexPosition = (ushort)vertex3.Position()-1; // OBJ is 1-based index -> 3DS is 0-based index
					face3ds2.m_vertexIndex.push_back(vertexPosition);

					face3ds2.m_flag = 5; // TODO : set this to a good value
					mesh.AddFace(face3ds2);

					vertexCount = 3;
				}
				*/
				C3dsFile::CObject::CMesh::CFace face3ds;

				for(uint v=0; v < vertexCount; ++v )
				{
					CObjFile::CVertex vertex = faceIt->Vertex(v);
					ushort vertexPosition = (ushort)vertex.Position()-1; // OBJ is 1-based index -> 3DS is 0-based index
					face3ds.m_vertexIndices.push_back(vertexPosition);
				}

				face3ds.m_flag = 5; // TODO : set this to a good value, visible vertices in face (triangle)
				mesh.AddFace(face3ds); // add face to 3DS mesh

			} // End for each OBJ face
		}

		object.AddMesh(mesh); // add mesh in 3DS object
		m_p3dsFile->AddObject(object); // add object in 3DS file
	}

	// Materials
	if(m_pOptions->m_bTranslateMaterials)
	{
		CMtlFile* pMtlFile = m_pObjFile->GetMaterialFile();

		if(!pMtlFile)
			return true;

		// Convert to a 3ds material node here!
	}

	return true;
}


