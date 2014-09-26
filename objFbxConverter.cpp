
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxSdkUtil.h"
#include "fbxSdkMaterialUtil.h"

#include "objFile.h"
#include "fbxFile.h"

#include "objFbxConverter.h"


CObjFbxConverter::CObjFbxConverter():
m_pFbxFile(NULL),
m_pOptions(NULL)
{
}

CObjFbxConverter::~CObjFbxConverter()
{
}


bool CObjFbxConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CObjFbxOptions();
	CFromObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}


// Convert an OBJ file into a FBX file
bool CObjFbxConverter::Convert(CFile* pObjFile, CFile* pOtherFile)
{
	// Initialize OBJ -> FBX
	m_pFbxFile = InitConversion<CFbxFile>(pObjFile, pOtherFile);
	OBJ_ASSERT(m_pFbxFile && m_pOptions);
	if(!m_pFbxFile || !m_pOptions)
		return false;

	MSG_INFO("Converting OBJ file format into FBX file format...");

	// Get FBX SDK objects
	KFbxSdkManager* pFbxSdkManager = m_pFbxFile->GetFbxSdkManager();
	KFbxScene* pScene = m_pFbxFile->GetFbxScene();

	OBJ_ASSERT(pFbxSdkManager && pScene);
	if(!pFbxSdkManager || !pScene)
		return false;

	// Create Scene info
	{
		KFbxSceneInfo* pSceneInfo = pFbxSdkManager->CreateKFbxSceneInfo();
		pSceneInfo->mTitle = "Title";
		pSceneInfo->mSubject = "Subject";
		pSceneInfo->mAuthor = "Author";
		pSceneInfo->mKeywords = "Keywords";
		pSceneInfo->mRevision = "Revision";
		pSceneInfo->mComment = "Comment";
		pScene->SetSceneInfo(pSceneInfo);
	}

	KFbxNode* pRootNode = pScene->GetRootNode();

	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();
	CObjFile::GroupVector::iterator groupIt;
	CObjFile::GroupVector::const_iterator groupEnd = groups.end();

	for(groupIt = groups.begin(); groupIt != groupEnd; ++groupIt) // for each OBJ groups
	{
		std::string objectName(groupIt->GetGroupName());

		// verify that it is a valid group
		if(groupIt->GetFaceCount() <= 0 )
			continue;

		KFbxNode* pModelNode = pFbxSdkManager->CreateKFbxNode((char*)objectName.c_str());
		KFbxMesh* pMesh = pFbxSdkManager->CreateKFbxMesh();

		// Extract only the refered block of vertex, texture coordinates and normals
		CObjFile::CGroup::SChannelsRange channelsRange = groupIt->GetChannelsRange();

		// Vertex Positions
		{
			Vector3DVector& vertices = m_pObjFile->GetVertexPositionVector();
			Vector3DVector::iterator vertexIt, vertexBegin, vertexEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(CreateVertexCopyIterators(channelsRange, vertices, vertexBegin, vertexEnd))
			{
				pMesh->InitControlPoints(vertexEnd - vertexBegin);
				KFbxVector4* pControlPoints = pMesh->GetControlPoints(); // pMesh->InitControlPoints() must have been called

				for(vertexIt = vertexBegin; vertexIt != vertexEnd; ++vertexIt) // for each OBJ vertex position
				{
					KFbxVector4 vertexPosition((*vertexIt)[0], (*vertexIt)[1], (*vertexIt)[2]);

					uint vPosIndex = vertexIt - vertexBegin;
					pControlPoints[vPosIndex] = vertexPosition;
				}
			}
		}


		// Master Layer
		KFbxLayer* pMasterLayer = pMesh->GetLayer(0); // Place everything on Layer 0

		if(pMasterLayer == NULL)
		{
			pMesh->CreateLayer();
			pMasterLayer = pMesh->GetLayer(0);
		}

		// Texture mapping
		{
			// Set texture mapping.
			KFbxLayerElementTexture* pTextureLayer = new KFbxLayerElementTexture();
			pTextureLayer->SetMappingMode(KFbxLayerElement::eBY_POLYGON);
			pTextureLayer->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
			pMasterLayer->SetTextures(pTextureLayer);
		}


		// PolyGroup
		KFbxLayerElementPolygonGroup* pLayerElementPG = NULL;
		KArraykInt pgArray;
		{
			if(pMasterLayer == NULL) // if Master layer not created yet
			{
				pMesh->CreateLayer();
				pMasterLayer = pMesh->GetLayer(0);
			}

			// Create UV layer element.
			pLayerElementPG = new KFbxLayerElementPolygonGroup();
			pLayerElementPG->SetMappingMode(KFbxLayerElement::eBY_POLYGON);
			pLayerElementPG->SetReferenceMode(KFbxLayerElement::eINDEX);
			// the pg indices are set per-poly in "pMesh->BeginPolygon"
		}



		// Texture coordinates
		KFbxLayerElementUV* pLayerElementUV = NULL;
		{
			KArrayKFbxVector2 uvArray;
			Vector3DVector& texCoords = m_pObjFile->GetTextureCoordVector();
			Vector3DVector::iterator texCoordIt, texCoordBegin, texCoordEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(m_pOptions->m_bTranslateUVs && CreateTexCoordCopyIterators(channelsRange, texCoords, texCoordBegin, texCoordEnd))
			{
				KFbxVector2 uv; // texCoord

				for(texCoordIt = texCoordBegin; texCoordIt != texCoordEnd; ++texCoordIt) // for each OBJ UV
				{
					uv.Set((*texCoordIt)[0], (*texCoordIt)[1]);
					uvArray.Add(uv);
				}

				if(pMasterLayer == NULL) // if Master layer not created yet
				{
					pMesh->CreateLayer();
					pMasterLayer = pMesh->GetLayer(0);
				}

				// Create UV layer element.
				pLayerElementUV = new KFbxLayerElementUV();
				pLayerElementUV->SetMappingMode(KFbxLayerElement::eBY_POLYGON_VERTEX);
				pLayerElementUV->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
				pLayerElementUV->GetDirectArray() = uvArray;
				pMasterLayer->SetUVs(pLayerElementUV);
				// the uv indices are set per-poly in "pMesh->AddPolygon"
			}
		}



		// Normals
		KFbxLayerElementNormal* pLayerElementNormal = NULL;
		KArrayKFbxVector4 normalArray;
		{
			Vector3DVector& normals = m_pObjFile->GetNormalVector();
			Vector3DVector::iterator normalIt, normalBegin, normalEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(m_pOptions->m_bTranslateNormals && CreateNormalCopyIterators(channelsRange, normals, normalBegin, normalEnd))
			{
				if(pMasterLayer == NULL) // if Master layer not created yet
				{
					pMesh->CreateLayer();
					pMasterLayer = pMesh->GetLayer(0);
				}

				// Create normal layer element
				pLayerElementNormal = new KFbxLayerElementNormal();
				pLayerElementNormal->SetMappingMode(KFbxLayerElement::eBY_POLYGON_VERTEX);
				pLayerElementNormal->SetReferenceMode(KFbxLayerElement::eDIRECT); // no way to index them ?
				// the normal vectors are set per-vertex/poly in the loop below
			}
		}



		// Faces
		{
			CObjFile::FaceVector& faces = groupIt->GetFaceVector();
			CObjFile::FaceVector::iterator faceIt;
			CObjFile::FaceVector::const_iterator faceEnd = faces.end();

			pMesh->InitMaterialIndices(KFbxLayerElement::eBY_POLYGON);
			pMesh->InitTextureIndices(KFbxLayerElement::eBY_POLYGON);

			m_materials.clear(); // clear material map for each mesh
			SFbxMaterial fbxMaterial;
			fbxMaterial.fbxMaterialID = -1;
			fbxMaterial.fbxTextureID = -1;

			Vector3D objNormal(3);
			KFbxVector4 fbxNormal;

			for(faceIt = faces.begin(); faceIt != faceEnd; ++faceIt) // For each OBJ face
			{
				uint vertexCount = faceIt->VertexCount();

				// Materials
				if(m_pOptions->m_bTranslateMaterials)
				{
					int materialID = faceIt->GetMaterialID();
					MaterialMap::iterator it;
					it = m_materials.find(materialID);
					if(it == m_materials.end()) // material ID was not found in the map
					{
						fbxMaterial = CreateMaterial(materialID, pMasterLayer); // Create FBX material
						m_materials[materialID] = fbxMaterial; // Add material to the material map
					}
					else // material ID was found in the map
					{
						fbxMaterial = m_materials[materialID]; // Get FBX material struct
					}
				}

				// Smoothing Group
				CObjFile::SmoothingGroupPair smoothingGroup = faceIt->GetSmoothingGroup();
				int sg = (smoothingGroup.first ? smoothingGroup.second : -1);

				// Begin FBX polygon
				pMesh->BeginPolygon(fbxMaterial.fbxMaterialID, fbxMaterial.fbxTextureID, sg);
				pgArray.Add(sg);

				for(uint v=0; v < vertexCount; ++v) // For each vertex in the OBJ face
				{
					CObjFile::CVertex vertex = faceIt->Vertex(v); // Get OBJ vertex
					uint vertexPositionIndex = vertex.Position() - channelsRange.firstVertex.Position(); // v pos index, OBJ is 1-based

					// Vertex position with UV
					if(m_pOptions->m_bTranslateUVs && vertex.HasTextureCoordinate())
					{
						uint uvIndex = vertex.TextureCoordinate() - channelsRange.firstVertex.TextureCoordinate(); // UV index, OBJ is 1-based
						pMesh->AddPolygon(vertexPositionIndex, uvIndex); // Assign vertex position index and UV index
					}
					else // Vertex position without UV
					{
						pMesh->AddPolygon(vertexPositionIndex);
					}

					// Vertex normal
					if(m_pOptions->m_bTranslateNormals && vertex.HasNormal() && pLayerElementNormal)
					{
						objNormal = m_pObjFile->Normal(vertex.Normal()); // Retreive normal for this polygon vertex
						fbxNormal = CFbxSdkUtil::ConvertVector3D(objNormal);
						normalArray.Add(fbxNormal); // Add normal to array
					}
				}

				// End FBX polygon
				pMesh->EndPolygon();
			}

			if(m_pOptions->m_bSmoothNormal && !pLayerElementNormal)
			{
				//pMesh->ComputeVertexNormals(); // Is seems to be smoothed anyways
			}
		}

		// If we added a master layer
		if(pMasterLayer)
		{
			if(pLayerElementPG) // If PolyGroups
			{
				pLayerElementPG->GetIndexArray() = pgArray;
				pMasterLayer->SetPolygonGroups(pLayerElementPG); // Set poly group
			}

			if(pLayerElementNormal) // If normals
			{
				pLayerElementNormal->GetDirectArray() = normalArray;
				pMasterLayer->SetNormals(pLayerElementNormal); // Set normal layer
			}
		}

		// Finish the mesh
		pModelNode->SetNodeAttribute(pMesh);
		pModelNode->SetShadingMode(KFbxNode::eTEXTURE_SHADING);
		pRootNode->AddChild(pModelNode);
	}

	// Identify current take when file is loaded.
	pScene->SetCurrentTake("Show all faces");

	return true;
}


// TODO : duplicated material across meshes ?
CObjFbxConverter::SFbxMaterial CObjFbxConverter::CreateMaterial(int materialID, KFbxLayer* pMasterLayer)
{
	SFbxMaterial fbxMaterial;
	fbxMaterial.fbxMaterialID = -1;
	fbxMaterial.fbxTextureID = -1;

	OBJ_ASSERT(pMasterLayer);
	if(!pMasterLayer)
		return fbxMaterial;

	// Get FBX SDK objects
	KFbxSdkManager* pFbxSdkManager = m_pFbxFile->GetFbxSdkManager();
	KFbxScene* pScene = m_pFbxFile->GetFbxScene();	
	CMtlFile* pMtlFile = m_pObjFile->GetMaterialFile();

	OBJ_ASSERT(pFbxSdkManager && pScene && pMtlFile);
	if(!pFbxSdkManager || !pScene || !pMtlFile)
		return fbxMaterial;

	std::string materialName = m_pObjFile->GetMaterialNameFromID(materialID);
	CMtlFile::CMaterial* pMtlMaterial = &pMtlFile->GetMaterial(materialName);

	OBJ_ASSERT(pMtlMaterial);
	if(!pMtlMaterial)
		return fbxMaterial;

	KFbxMaterial* pFbxMaterial = pFbxSdkManager->CreateKFbxMaterial((char*)materialName.c_str());
	pFbxMaterial->SetShadingModel("phong");


	// Ambient
	if(pMtlMaterial->HasKa())
		pFbxMaterial->SetAmbient(CFbxSdkUtil::ConvertColor3(pMtlMaterial->GetKa()));

	// Diffuse
	if(pMtlMaterial->HasKd())
		pFbxMaterial->SetDiffuse(CFbxSdkUtil::ConvertColor3(pMtlMaterial->GetKd()));

	// Specular
	if(pMtlMaterial->HasKs())
		pFbxMaterial->SetSpecular(CFbxSdkUtil::ConvertColor3(pMtlMaterial->GetKs()));

	// Shininess
	if(pMtlMaterial->HasNs())
		pFbxMaterial->SetShininess(pMtlMaterial->GetNs());

	// Texture map
	if(m_pOptions->m_bTranslateMaps && pMtlMaterial->HasTexMap())
	{
		std::string textureName(pMtlMaterial->GetTexMap());

		// Create FbxTexture
		// TODO : may be a problem for special characters in texturefile path
		KFbxTexture* pFbxTexture = pFbxSdkManager->CreateKFbxTexture((char*)textureName.c_str());
		pFbxTexture->SetFileName((char*)textureName.c_str());
		pFbxTexture->SetTextureUse(KFbxTexture::eSTANDARD);
		pFbxTexture->SetMappingType(KFbxTexture::eUV);
		pFbxTexture->SetMaterialUse(KFbxTexture::eMODEL_MATERIAL);
		pFbxTexture->SetSwapUV(false);
		pFbxTexture->SetTranslation(0.0, 0.0);
		pFbxTexture->SetScale(1.0, 1.0);
		pFbxTexture->SetRotation(0.0, 0.0);

		// Add texture to mesh
		fbxMaterial.fbxTextureID = pMasterLayer->GetTextures()->GetDirectArray().Add(pFbxTexture);
	}

	// Add material to mesh
	fbxMaterial.fbxMaterialID = pMasterLayer->GetMaterials()->GetDirectArray().Add(pFbxMaterial);

	return fbxMaterial;
}
