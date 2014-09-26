
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "lwoFile.h"

#include "objLwoConverter.h"


CObjLwoConverter::CObjLwoConverter():
m_pLwoFile(NULL),
m_pOptions(NULL)
{
}


CObjLwoConverter::~CObjLwoConverter()
{
}


bool CObjLwoConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CObjLwoOptions();
	CFromObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}


// Convert an OBJ file into a LWO file
bool CObjLwoConverter::Convert(CFile* pObjFile, CFile* pOtherFile)
{
	// Initialize OBJ -> LWO
	m_pLwoFile = InitConversion<CLwoFile>(pObjFile, pOtherFile);
	OBJ_ASSERT(m_pLwoFile && m_pOptions);
	if(!m_pLwoFile || !m_pOptions)
		return false;

	MSG_INFO("Converting OBJ file format into LWO file format..."); 

	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();
	CObjFile::GroupVector::iterator groupIt;
	CObjFile::GroupVector::const_iterator groupEnd = groups.end();

	// Create the LWO layer
	ushort layerID = 0;
	CLwoFile::CLayer layer(layerID);
	layer.SetLayerName(groups.begin()->GetGroupName()); // Set the name to the first group.

	ushort partID = 0;


	for(groupIt = groups.begin(); groupIt != groupEnd; ++groupIt) // for each OBJ groups
	{
		// Should be used for part name
		std::string objectName(groupIt->GetGroupName());

		// Verify that it is a valid group
		if(groupIt->GetFaceCount() <= 0 )
			continue;

		// Add the LWO part name
		layer.GetPartVector().push_back(objectName);

		// Extract only the refered block of vertex, texture coordinates and normals
		CObjFile::CGroup::SChannelsRange channelsRange = groupIt->GetChannelsRange();


		// Vertex Positions
		{
			Vector3DVector& vertices = m_pObjFile->GetVertexPositionVector();
			Vector3DVector::iterator objVertexBegin, objVertexEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(CreateVertexCopyIterators(channelsRange, vertices, objVertexBegin, objVertexEnd))
			{
				Vector3DVector& lwoLayerVertexVec = layer.GetVertexPositionVector();
				lwoLayerVertexVec.reserve(objVertexEnd - objVertexBegin);
				lwoLayerVertexVec.insert(lwoLayerVertexVec.end(), objVertexBegin, objVertexEnd);
			}
		}


		// Faces
		CObjFile::FaceVector& faces = groupIt->GetFaceVector();
		CObjFile::FaceVector::iterator faceIt;
		CObjFile::FaceVector::const_iterator faceEnd = faces.end();

		layer.GetFaceVector().reserve(faces.size()); // reserve face vector
		uint faceIndex = 0;
		uint vertexCount = 0;

		for(faceIt = faces.begin(); faceIt != faceEnd; ++faceIt) // For each OBJ face
		{
			faceIndex = (faceIt - faces.begin());

			// Create LWO polygon "face"
			CLwoFile::CLayer::CFace face;

			// Set the surface ID assigned to the polygon
			if(!m_pOptions->m_bTranslateMaterials || faceIt->GetMaterialID() == -1)
				face.m_surface = 0; // hackish
			else
				face.m_surface = faceIt->GetMaterialID();

			// Set the part ID of the polygon
			face.m_part = partID;

			// Set the smoothing group of the polygon
			CObjFile::SmoothingGroupPair objSmoothingGroup = faceIt->GetSmoothingGroup();
			face.m_smoothingGroup = std::make_pair(objSmoothingGroup.first, objSmoothingGroup.second); // Set smoothing group
			layer.GetSGVector().insert(face.m_smoothingGroup.second); // Insert smoothing group in the set

			vertexCount = faceIt->VertexCount(); // Number of vertices on the face

			for(uint v=0; v < vertexCount; ++v ) // For each vertex in the OBJ face
			{
				CObjFile::CVertex vertex = faceIt->Vertex(v);

				// Vertex position index
				uint vertexIndex = vertex.Position() - 1; // OBJ is 1-based index -> LWO is 0-based index
				face.GetVertexIndexVector().push_back(vertexIndex);

				// Tex coord index
				if(m_pOptions->m_bTranslateUVs && vertex.HasTextureCoordinate())
				{
					Vector3D uv = m_pObjFile->TextureCoordinate(vertex.TextureCoordinate()); // Retreive textureCoordinate from index
					CLwoFile::CLayer::CTexCoord lwoTexCoord;
					lwoTexCoord.m_faceIndex = faceIndex;
					lwoTexCoord.m_texCoord = uv;
					layer.GetTexCoordMap()[vertexIndex].push_back(lwoTexCoord); // push CTexCoord at the vertex position index
				}
			}

			layer.AddFace(face);
		}

		++partID; // increment the partID
	}

	// Create Lwo material library
	if(m_pOptions->m_bTranslateMaterials)
	{
		CreateLwoMaterialLibrary(layer);
	}

	m_pLwoFile->AddLayer(layer);

	return true;
}


bool CObjLwoConverter::CreateLwoMaterialLibrary(CLwoFile::CLayer& layer)
{
	CMtlFile* pMtlFile = m_pObjFile->GetMaterialFile();

	OBJ_ASSERT(pMtlFile);
	if(!pMtlFile)
		return false;

	uint imageIndex = 0;

	CMtlFile::MaterialMap& materials = pMtlFile->GetMaterialMap();
	CMtlFile::MaterialMap::iterator materialIt;
	CMtlFile::MaterialMap::const_iterator materialEnd = materials.end();

	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each material
	{
		std::string materialName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;

		OBJ_ASSERT(pMaterial && materialName == pMaterial->GetMaterialName());
		if(!pMaterial)
			continue;

		CLwoFile::CLayer::CSurface newSurface; // put the name there
		newSurface.SetSurfaceName(materialName);

		// Todo parse other material informations

		// Texture Map
		if(m_pOptions->m_bTranslateMaps && pMaterial->HasTexMap())
		{
			// Add image path to the vector
			layer.AddImage(imageIndex, pMaterial->GetTexMap());

			// Add image index to the surface
			newSurface.m_imageIndex = imageIndex;

			// Increment image index
			++imageIndex;
		}

		layer.GetSurfaceMap()[materialName] = newSurface;
	}

	return true;
}




