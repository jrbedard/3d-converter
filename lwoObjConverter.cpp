
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "objFile.h"
#include "lwoFile.h"

#include "lwoObjConverter.h"


CLwoObjConverter::CLwoObjConverter():
m_pLwoFile(NULL),
m_pOptions(NULL)
{
}

CLwoObjConverter::~CLwoObjConverter()
{
}

bool CLwoObjConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CLwoObjOptions();
	CToObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}

bool CLwoObjConverter::Convert(CFile* pOtherFile, CFile* pObjFile)
{
	// Initialize LWO -> OBJ
	m_pLwoFile = InitConversion<CLwoFile>(pOtherFile, pObjFile);
	OBJ_ASSERT(m_pLwoFile && m_pOptions);
	if(!m_pLwoFile || !m_pOptions)
		return false;

	MSG_INFO("Converting LWO file format into OBJ file format..."); 

	// Get the Obj group vector
	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();

	// Get the LWO layer vector
	CLwoFile::LayerVector& layers = m_pLwoFile->GetLayerVector();
	CLwoFile::LayerVector::iterator layerIt;
	CLwoFile::LayerVector::const_iterator layerEnd = layers.end();

	// more than one layer is hazardous with this code
	//OBJ_ASSERT(layers.size() > 1);

	for(layerIt = layers.begin(); layerIt != layerEnd; ++layerIt) // for each LWO layer
	{
		ushort layerID = layerIt->GetLayerID();
		std::string layerName(layerIt->GetLayerName());

		if(!CheckStr(layerName))
		{
			std::stringstream ssLayerName;
			ssLayerName << "layer" << layerID;
			layerName = ssLayerName.str();
		}

		CObjFile::CGroup group(layerName);

		// "Append" Vertex positions
		{
			Vector3DVector& objVertexVector = m_pObjFile->GetVertexPositionVector();
			Vector3DVector& lwoVertexVector = layerIt->GetVertexPositionVector();
			objVertexVector.reserve(lwoVertexVector.size());
			objVertexVector.insert(objVertexVector.end(), lwoVertexVector.begin(), lwoVertexVector.end()); // Append new Vertices
		}

		// TexCoords
		{
			Vector3DVector& objUvVector = m_pObjFile->GetTextureCoordVector();
			CLwoFile::CLayer::TexCoordMap& texCoords = layerIt->GetTexCoordMap();
			CLwoFile::CLayer::TexCoordMap::iterator texCoordIt, texCoordEnd;
			texCoordEnd = texCoords.end();
			Vector3D uv(3);

			for(texCoordIt = texCoords.begin(); texCoordIt != texCoordEnd; ++texCoordIt) // For each LWO texCoord
			{
				std::vector< CLwoFile::CLayer::CTexCoord >& uvVector = texCoordIt->second;
				std::vector< CLwoFile::CLayer::CTexCoord >::iterator uvVectorIt, uvVectorEnd;
				uvVectorEnd = uvVector.end();

				for(uvVectorIt = uvVector.begin(); uvVectorIt != uvVectorEnd; ++uvVectorIt) // For each LWO texCoord
				{
					uv = uvVectorIt->m_texCoord;
					objUvVector.push_back(uv);
				}
			}
		}


		// Faces
		CLwoFile::CLayer::FaceVector& faces = layerIt->GetFaceVector();
		CLwoFile::CLayer::FaceVector::iterator faceIt;
		CLwoFile::CLayer::FaceVector::const_iterator faceEnd = faces.end();

		group.GetFaceVector().reserve(faces.size()); // reserve for face vector
		uint faceIndex = 0;

		for(faceIt = faces.begin(); faceIt != faceEnd; ++faceIt) // for each LWO face
		{
			faceIndex = (faceIt - faces.begin());
			CObjFile::CFace objFace;

			// Set smoothing group
			CLwoFile::SmoothingGroupPair sgPair = faceIt->m_smoothingGroup;
			objFace.SetSmoothingGroup(sgPair);

			// Set material ID
			objFace.SetMaterialID(faceIt->m_surface);

			// TODO : use "faceIt->m_part" to create new group when detected

			uint vertexIndex = 0;
			std::pair< bool, uint > uvPair;
			std::pair< bool, uint > normalPair;


			// TODO: THIS LOOP IS HORRIBLY SLOW!!!!! (in debug, on large model)
			for( uint v=0; v < faceIt->VertexCount(); ++v ) // for each vertex in the face
			{
				uvPair = std::make_pair(false, 0);
				normalPair = std::make_pair(false, 0);

				OBJ_ASSERT(v < faceIt->GetVertexIndexVector().size());
				vertexIndex = faceIt->GetVertexIndexVector()[v];

				// UV
				if(layerIt->GetTexCoordMap().size() > 0)
				{
					CLwoFile::CLayer::TexCoordMap& texCoords = layerIt->GetTexCoordMap();
					CLwoFile::CLayer::TexCoordMap::iterator texCoordIt, texCoordEnd;
					texCoordEnd = texCoords.end();

					uint uvCounter = 0;
					for(texCoordIt = texCoords.begin(); texCoordIt != texCoordEnd; ++texCoordIt) // For each LWO texCoord
					{
						std::vector< CLwoFile::CLayer::CTexCoord >& uvVector = texCoordIt->second;
						std::vector< CLwoFile::CLayer::CTexCoord >::iterator uvVectorIt, uvVectorEnd;
						uvVectorEnd = uvVector.end();

						for(uvVectorIt = uvVector.begin(); uvVectorIt != uvVectorEnd; ++uvVectorIt, ++uvCounter) // For each LWO texCoord
						{
							if((vertexIndex == texCoordIt->first) &&
							   (uvVectorIt->m_faceIndex == -1 || faceIndex == uvVectorIt->m_faceIndex))
							{
								uvPair = std::make_pair(true, uvCounter + 1); // LWO is 0-based index -> OBJ is 1-based index
							}
						}
					}
				}


				// No normal in LWO
				normalPair = std::make_pair(false, 0);

				// Add vertex to face
				objFace.AddVertex( CObjFile::CVertex( vertexIndex + 1, uvPair, normalPair ) ); // LWO is 0-based index -> OBJ is 1-based index 

			} // End for each LWO vertex 

			group.AddFace(objFace); // Add face to group

		} // End for each LWO face

		m_pObjFile->AddGroup(group); // Add group to OBJ file

		// Create MTL
		CreateObjMaterialLibrary(*layerIt);
	}

	return true;
}



bool CLwoObjConverter::CreateObjMaterialLibrary(CLwoFile::CLayer& layer)
{
	CLwoFile::CLayer::SurfaceMap& surfaces = layer.GetSurfaceMap();
	CLwoFile::CLayer::SurfaceMap::iterator surfaceIt;
	CLwoFile::CLayer::SurfaceMap::const_iterator surfaceEnd = surfaces.end();

	for(surfaceIt = surfaces.begin(); surfaceIt != surfaceEnd; ++surfaceIt) // for each material
	{
		std::string surfaceName(surfaceIt->first);
		CLwoFile::CLayer::CSurface* pSurface = &surfaceIt->second;

		OBJ_ASSERT(pSurface && surfaceName == pSurface->GetSurfaceName());
		if(!pSurface)
			continue;

		CMtlFile::CMaterial newMaterial(surfaceName);

		// TODO : parse other parameter : diffuse, ambient, spec, etc

		if(pSurface->m_imageIndex != -1) // if there is a texture
		{
			std::string imagePath(layer.GetImagePath(pSurface->m_imageIndex));
			imagePath = imagePath.substr(0,imagePath.size()-1); // remove trailing
			newMaterial.SetTexMap(imagePath);
		}

		uint materialID = m_pObjFile->AddMaterial(newMaterial);
	}

	return true;
}

