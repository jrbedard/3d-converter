
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "3dsFile.h"

#include "3dsObjConverter.h"


C3dsObjConverter::C3dsObjConverter():
m_p3dsFile(NULL),
m_pOptions(NULL)
{
}

C3dsObjConverter::~C3dsObjConverter()
{
}

bool C3dsObjConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new C3dsObjOptions();
	CToObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}


// Convert a 3DS file into an OBJ file
bool C3dsObjConverter::Convert(CFile* pOtherFile, CFile* pObjFile)
{
	// Initialize 3DS -> OBJ
	m_p3dsFile = InitConversion<C3dsFile>(pOtherFile, pObjFile);
	OBJ_ASSERT(m_p3dsFile && m_pOptions);
	if(!m_p3dsFile || !m_pOptions)
		return false;

	MSG_INFO("Converting 3DS file format into OBJ file format...");

	// Get the OBJ group vector
	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();

	// Get the 3DS object vector
	C3dsFile::ObjectVector& objects = m_p3dsFile->GetObjectVector();
	C3dsFile::ObjectVector::iterator objectIt;
	C3dsFile::ObjectVector::const_iterator objectEnd = objects.end();


	for(objectIt = objects.begin(); objectIt != objectEnd; ++objectIt) // for each 3DS object
	{
		std::string objectName(objectIt->GetObjectName());

		CObjFile::CGroup group(objectName);

		// Get the 3DS mesh vector
		C3dsFile::CObject::MeshVector& meshes = objectIt->GetMeshVector();
		C3dsFile::CObject::MeshVector::iterator meshIt;
		C3dsFile::CObject::MeshVector::const_iterator meshEnd = meshes.end();

		for(meshIt = meshes.begin(); meshIt != meshEnd; ++meshIt) // for each 3DS mesh
		{
			// "Append" Vertex positions
			{
				Vector3DVector& maxVertices = meshIt->GetVertexPositionVector();
				Vector3DVector& objVertices = m_pObjFile->GetVertexPositionVector();
				objVertices.reserve(maxVertices.size());
				objVertices.insert(objVertices.end(), maxVertices.begin(), maxVertices.end()); // Insert vertices
			}

			// "Append" Texture coordinates
			{
				Vector3DVector& maxTexCoords = meshIt->GetMapCoordVector();
				Vector3DVector& objTexCoords = m_pObjFile->GetTextureCoordVector();
				objTexCoords.reserve(maxTexCoords.size());
				objTexCoords.insert(objTexCoords.end(), maxTexCoords.begin(), maxTexCoords.end()); // Insert UVs
			}

			bool bHasUVs = false;
			if(meshIt->MapCoordCount() > 0)
			{
				bHasUVs = true;
			}

			// Generate Normals here ????

			// Faces
			{
				C3dsFile::CObject::CMesh::FaceVector& faces = meshIt->GetFaceVector();
				C3dsFile::CObject::CMesh::FaceVector::iterator faceIt;
				C3dsFile::CObject::CMesh::FaceVector::const_iterator faceEnd = faces.end();

				group.GetFaceVector().reserve(faces.size());

				for(faceIt = faces.begin(); faceIt != faceEnd; ++faceIt) // for each 3DS face (triangle)
				{
					CObjFile::CFace objFace;

					C3dsFile::SmoothingGroupPair sgPair = faceIt->m_smoothingGroup;
					objFace.SetSmoothingGroup(std::make_pair(sgPair.first, sgPair.second));
					objFace.SetMaterialID(faceIt->m_materialID);

					for( uint v=0; v < faceIt->VertexCount(); ++v ) // for each vertex in the face
					{
						uint vertexIndex = faceIt->m_vertexIndices[v];
						uint uvIndex = 0;

						if(bHasUVs);
						{
							uvIndex = vertexIndex;
						}

						objFace.AddVertex( CObjFile::CVertex( vertexIndex + 1, // 3DS is 0-based index -> OBJ is 1-based index  
															  std::make_pair(bHasUVs, uvIndex + 1), 
															  std::make_pair(false, 0) )); // no normals in 3DS
					} // End for each 3DS vertex

					group.AddFace(objFace); // add face in OBJ group

				} // End for each 3DS face
			}

		}

		m_pObjFile->AddGroup(group);
	}

	
	// Materials
	{
		CMtlFile* pMtlFile = m_pObjFile->GetMaterialFile();

		OBJ_ASSERT(pMtlFile);
		if(!pMtlFile)
			return false;

		C3dsFile::MaterialMap& materials = m_p3dsFile->GetMaterialMap();
		C3dsFile::MaterialMap::iterator materialIt;
		C3dsFile::MaterialMap::const_iterator materialEnd = materials.end();

		for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each 3DS material
		{
			uint materialID(materialIt->first);
			C3dsFile::CMaterial* pMaterial = &materialIt->second;

			if(!pMaterial)
				continue;

			std::string materialName(pMaterial->GetMaterialName());

			CMtlFile::CMaterial newMaterial(materialName);

			// TODO : parse other parameter : diffuse, ambient, spec, etc

			if(pMaterial->HasTexMap()) // if there is a texture
			{
				std::string texMapFileName = pMaterial->GetTexMap();
				newMaterial.SetTexMap(texMapFileName);
			}

			materialID = m_pObjFile->AddMaterial(newMaterial); // should be done together
		}
	}

	return true;
}




