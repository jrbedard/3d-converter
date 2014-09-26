
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "SemanticLayer.h"

#include "objFile.h"
#include "xsiFile.h"

#include "xsiFtkUtil.h"
#include "xsiFtkGeometryUtil.h"
#include "xsiFtkMaterialUtil.h"

#include "objXsiConverter.h"



CObjXsiConverter::CObjXsiConverter():
m_pXsiFile(NULL),
m_pOptions(NULL)
{
}

CObjXsiConverter::~CObjXsiConverter()
{
}


bool CObjXsiConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CObjXsiOptions();
	CFromObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}


// Convert an OBJ file into a XSI file
bool CObjXsiConverter::Convert(CFile* pObjFile, CFile* pOtherFile)
{
	// Initialize OBJ -> XSI
	m_pXsiFile = InitConversion<CXsiFile>(pObjFile, pOtherFile);
	OBJ_ASSERT(m_pXsiFile && m_pOptions);
	if(!m_pXsiFile || !m_pOptions)
		return false;

	MSG_INFO("Converting OBJ file format into XSI file format...");

	::CSLScene* pScene = m_pXsiFile->GetScene();

	OBJ_ASSERT(pScene);
	if(!pScene)
		return false;

	std::string fileName(m_pXsiFile->m_fileName.string()); // Extract native file path string

	pScene->Create(const_cast<char*>(fileName.c_str()), 3, 60); // version 3.6

	// Set the header file info
	SetXsiFileInfo(pScene);

	// Set Scene default parameters
	SetDefaultSceneParamaters(pScene);

	CSLMaterialLibrary* pMaterialLibrary = NULL;
	if(m_pOptions->m_bTranslateMaterials)
	{
		// Create the XSI material library
		pMaterialLibrary = CXsiFtkMaterialUtil::CreateMaterialLibrary(m_pObjFile, pScene, m_pOptions->m_bTranslateMaps);
		OBJ_ASSERT(pMaterialLibrary);
	}

	CObjFile::GroupVector& groups = m_pObjFile->GetGroupVector();
	CObjFile::GroupVector::iterator groupIt;
	CObjFile::GroupVector::const_iterator groupEnd = groups.end();

	for(groupIt = groups.begin(); groupIt != groupEnd; ++groupIt) // for each OBJ groups
	{
		std::string objectName(groupIt->GetGroupName());

		// verify that it is a valid group
		if(groupIt->GetFaceCount() <= 0 )
			continue;

		// Create a simple mesh.
		CSLModel* pMeshModel = pScene->Root()->AddMesh();

		// Fill the model attributes and assign material
		CXsiFtkGeometryUtil::SetModelDefaultAttributes(pMeshModel, (char*)objectName.c_str());

		// Retreive mesh and shape from model
		CSLMesh* pMesh = (CSLMesh*)pMeshModel->Primitive();
		CSLBaseShape* pShape = pMesh->Shape();

		// Extract only the refered block of vertex, texture coordinates and normals
		CObjFile::CGroup::SChannelsRange channelsRange = groupIt->GetChannelsRange();


		// Vertex Positions
		{
			Vector3DVector& vertices = m_pObjFile->GetVertexPositionVector();
			Vector3DVector::iterator vertexIt, vertexBegin, vertexEnd;
			CSLBaseShape::CSLVector3DArray* pVertices = pShape->GetVertexList();

			// Generate the copy begin and end iterators to copy this group's channels
			if(CreateVertexCopyIterators(channelsRange, vertices, vertexBegin, vertexEnd))
			{
				CSIBCVector3D newVertex;
				// TODO : pVertices->Reserve()
				for(vertexIt = vertexBegin; vertexIt != vertexEnd; ++vertexIt) // for each OBJ vertex position
				{
					newVertex.Set((float)(*vertexIt)[0], (float)(*vertexIt)[1], (float)(*vertexIt)[2]);
					pVertices->Add(newVertex); // add vertex position to array

				} // End for each OBJ vertex position
			}
		}


		// Normals
		{
			Vector3DVector& normals = m_pObjFile->GetNormalVector();
			Vector3DVector::iterator normalIt, normalBegin, normalEnd;
			CSLBaseShape::CSLVector3DArray* pNormals = pShape->CreateNormals();

			// Generate the copy begin and end iterators to copy this group's channels
			if(m_pOptions->m_bTranslateNormals && CreateNormalCopyIterators(channelsRange, normals, normalBegin, normalEnd))
			{
				CSIBCVector3D newNormal;
				// TODO : pNormals->Reserve()
				for(normalIt = normalBegin; normalIt != normalEnd; ++normalIt) // for each OBJ normal
				{
					newNormal.Set((float)(*normalIt)[0], (float)(*normalIt)[1], (float)(*normalIt)[2]);
					pNormals->Add(newNormal); // add normal to array

				} // End for each OBJ normal

			}
		}


		// UV coordinates
		{
			Vector3DVector& texCoordinates = m_pObjFile->GetTextureCoordVector();
			Vector3DVector::iterator texCoordIt, texCoordBegin, texCoordEnd;

			// Generate the copy begin and end iterators to copy this group's channels
			if(m_pOptions->m_bTranslateUVs && CreateTexCoordCopyIterators(channelsRange, texCoordinates, texCoordBegin, texCoordEnd))
			{
				CSLBaseShape::CSLVector2DArray* pTexCoords = NULL;

				if(pShape->Type() == CSLTemplate::SI_SHAPE)
				{
					pTexCoords = ((CSLShape*)pShape)->CreateUVCoords();
				}
				else
				{
					CSLUVCoordArray* pUVCoords = ((CSLShape_35*)pShape)->AddUVCoordArray();
					pUVCoords->SetTextureProjection("Texture_Projection");
					pTexCoords = pUVCoords->GetUVCoordList();
				}

				CSIBCVector2D newTexCoord;
				// TODO : pTexCoords->Reserve()
				for(texCoordIt = texCoordBegin; texCoordIt != texCoordEnd; ++texCoordIt) // For each OBJ texture coordinate
				{
					newTexCoord.Set((float)(*texCoordIt)[0], (float)(*texCoordIt)[1]); // ignore W
					if(pTexCoords)
					{
						pTexCoords->Add(newTexCoord); // Add UV coord to array
					}

				} // End for each OBJ TexCoord
			}
		}

		// TODO : offer to create PolygonList, TriangleList or TriangleStripList

		// Create PolygonList map and Cluster map
		std::map<int, CSLPolygonList*> polyListMap;
		std::map<int, CSLCluster*>	   clusterMap;
		{
			CSLPolygonList* pPolyList = NULL;
			CSLCluster* pCluster = NULL;

			std::string polyListName(objectName);
			CObjFile::MaterialMap materialMap = m_pObjFile->GetMaterialMap(objectName);

			if(materialMap.size() > 1) // More than 1 material assigned to this mesh
			{
				CSLCluster* pCluster = NULL;
				CObjFile::MaterialMap::iterator materialIt;
				CObjFile::MaterialMap::const_iterator materialEnd = materialMap.end();

				for(materialIt = materialMap.begin(); materialIt != materialEnd; ++materialIt) // For each material assigned to this mesh
				{
					// Create PolyList
					pPolyList = pMesh->AddPolygonList(); // Create polyList
					pPolyList->SetName((char*)(polyListName + materialIt->second).c_str()); // mesh name + material name
					polyListMap[materialIt->first] = pPolyList; // add new polygon list to the polyList map

					// Create Cluster
					pCluster = pMeshModel->AddCluster(); // Create Cluster
					pCluster->SetName((char*)(materialIt->second).c_str()); // material name
					pCluster->SetWeightingType(CSLCluster::EClusterWeightingType::SI_AVERAGE);

					// TODO : Find a way to add "XSI_ClusterInfo { "POLY" }" like the XSI's importer does

					clusterMap[materialIt->first] = pCluster;  // add new cluster to the cluster map
				}
			}
			else // 0 or 1 material assigned to this mesh
			{
				pPolyList = pMesh->AddPolygonList(); // Create polyList
				pPolyList->SetName((char*)polyListName.c_str()); // mesh name

				if(materialMap.size() == 0)
					polyListMap[-1] = pPolyList; // add polyList to the map, no material assigned
				else
					polyListMap[materialMap.begin()->first] = pPolyList; // add polyList to the map, 1 material assigned
			}
		}



		// Faces : Fill each PolygonList
		{
			std::map<int, CSLPolygonList*>::iterator polyListIt;
			std::map<int, CSLPolygonList*>::const_iterator polyListEnd = polyListMap.end();

			for(polyListIt = polyListMap.begin(); polyListIt != polyListEnd; ++polyListIt) // For each polygon list
			{
				int materialID = polyListIt->first;
				CSLPolygonList* pPolyList = polyListIt->second;
				OBJ_ASSERT(pPolyList);
				if(!pPolyList)
					continue;

				CSLCluster* pCluster = NULL;
				if(polyListMap.size() > 1) // if more than one polyList : we have a cluster
				{
					pCluster = clusterMap[materialID];
				}

				CSLPolygonList::CSLIntArray* pVertPerPolys = pPolyList->GetPolygonVertexCountList();
				CSLPolygonList::CSLIntArray* pVertices =	 pPolyList->GetVertexIndices();
				CSLPolygonList::CSLIntArray* pTexCoords =	 NULL;
				CSLPolygonList::CSLIntArray* pNormals =		 pPolyList->CreateNormalIndices(); // always create it (else it crash)
				CSLPolygonList::CSLIntArray* pColorArray =	 NULL;

				// Assign Material to this polyList
				{
					bool status = false;
					if(m_pOptions->m_bTranslateMaterials && materialID != -1) // If there is a material is assigned to this mesh
					{
						std::string materialName = m_pObjFile->GetMaterialNameFromID(materialID);
						std::string defaultLibMateralName("DefaultLib.");
						defaultLibMateralName += materialName;

						OBJ_ASSERT(pMaterialLibrary);
						CSLBaseMaterial* pBaseMaterial = pMaterialLibrary->FindMaterial((char*)defaultLibMateralName.c_str());
						OBJ_ASSERT(pBaseMaterial);
						pPolyList->SetMaterial(pBaseMaterial); // Assign material to polyList

						// Assign material to the model
						if(polyListMap.size() > 1) // More than 1 material assigned to this mesh
						{
							status = CXsiFtkMaterialUtil::AssignMaterialToModel(pMaterialLibrary, pMeshModel, "Scene_Material", CSLGlobalMaterial::SI_BRANCH);
						}
						else // 1 material assigned to this mesh
						{
							int assignCount = m_pObjFile->GetAssignementCount(materialID);
							if(assignCount > 1) // assigned on other meshes
								status = CXsiFtkMaterialUtil::AssignMaterialToModel(pMaterialLibrary, pMeshModel, materialName, CSLGlobalMaterial::SI_INHERITED);
							else
								status = CXsiFtkMaterialUtil::AssignMaterialToModel(pMaterialLibrary, pMeshModel, materialName, CSLGlobalMaterial::SI_NODE);
						}
					}
					else // No material assigned to this mesh, assign scene material
					{
						status = CXsiFtkMaterialUtil::AssignMaterialToModel(pMaterialLibrary, pMeshModel, "Scene_Material", CSLGlobalMaterial::SI_BRANCH);
					}
					OBJ_ASSERT(status);
				}


				CObjFile::FaceVector& faces = groupIt->GetFaceVector();
				CObjFile::FaceVector::iterator faceIt;
				CObjFile::FaceVector::const_iterator faceEnd = faces.end();

				uint faceIndex = 0;
				uint vertexCount = 0;
				for(faceIt = faces.begin(); faceIt != faceEnd; ++faceIt) // for each OBJ face
				{
					int faceMaterialId = faceIt->GetMaterialID();
					if(faceMaterialId != materialID)
						continue; // this face doesnt belong to this polyList because of its assigned material

					faceIndex = (faceIt - faces.begin());
					if(pCluster)
					{
						pCluster->GetVertexIndicesList()->Add(faceIndex); // Add face index
					}

					vertexCount = faceIt->VertexCount(); // Number of vertices on the face
					pVertPerPolys->Add(vertexCount); // Set the number of vertices for this polygon

					// Indices
					uint vertexIndex = 0;
					uint uvIndex = 0;
					uint normalIndex = 0;

					for(uint v=0; v < vertexCount; ++v ) // For each vertex in face
					{
						CObjFile::CVertex vertex = faceIt->Vertex(v);
						vertexIndex = vertex.Position() - channelsRange.firstVertex.Position(); // OBJ is 1-based index -> XSI is 0-based index
						pVertices->Add(vertexIndex); // Add vertex index

						// Texture coordinate Index
						if(m_pOptions->m_bTranslateUVs && vertex.HasTextureCoordinate())
						{
							if(!pTexCoords)
								pTexCoords = pPolyList->AddUVArray(); // Create UVs

							uvIndex = vertex.TextureCoordinate() - channelsRange.firstVertex.TextureCoordinate(); // OBJ is 1-based index -> XSI is 0-based index
							pTexCoords->Add(uvIndex); // Add tex coord index
						}

						// Normal Index
						if(m_pOptions->m_bTranslateNormals && vertex.HasNormal())
						{
							normalIndex = vertex.Normal() - channelsRange.firstVertex.Normal(); // OBJ is 1-based index -> XSI is 0-based index
							pNormals->Add(normalIndex); // Add normal index
						}
						else
						{
							pNormals->Add(0); // else it crash, TODO : Cause bugs?
						}
					}

				} // End for each OBJ face

			} // End for each PolyList

		} // End faces

		//CXsiFtkUtil::CompressMeshData(l_pMesh); // TODO : looks interesting

	} // End for each OBJ group

	return true;

}




// Creates scene information, file information and angle information
bool CObjXsiConverter::SetXsiFileInfo(::CSLScene* pScene)
{
	OBJ_ASSERT(pScene);
	if(!pScene)
		return false;

	// File Info
	CSLFileInfo* pFileInfo = pScene->FileInfo();
	pFileInfo->SetOriginator( "ZConverter Version 1.0" );
	pFileInfo->SetProjectName( "ProjectName" ); // TODO
	pFileInfo->SetUsername( "UserName" ); // TODO

	return true;
}


bool CObjXsiConverter::SetDefaultSceneParamaters(::CSLScene* pScene)
{
	OBJ_ASSERT(pScene);
	if(!pScene)
		return false;

	// Scene Info
	CSLSceneInfo* pSceneInfo = pScene->SceneInfo();
	pSceneInfo->SetStart(1.0f);
	pSceneInfo->SetEnd(100.0f);
	pSceneInfo->SetFrameRate(30);
	pSceneInfo->SetTimingType(CSLSceneInfo::SI_FRAMES);

	// Coordinate system
	pScene->CoordinateSystem()->SetCoordinateSystemStyle(CSLCoordinateSystem::SI_SOFTIMAGE_3D);

	// Angle
	CSLAngle* pAngle = pScene->Angle();
	pAngle->SetAngleType(CSLAngle::SI_DEGREES);

	// Set the ambiance color
	CSLAmbience* pAmbiance = pScene->Ambience();
	pAmbiance->SetColor(CSIBCColorf(0.2f, 0.2f, 0.2f));

	return true;
}


