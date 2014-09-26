
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdk.h" // FBX SDK

#include "fbxSdkUtil.h"
#include "fbxSdkMaterialUtil.h"

#include "objFile.h"
#include "fbxFile.h"

#include "fbxObjConverter.h"


CFbxObjConverter::CFbxObjConverter():
m_pFbxFile(NULL),
m_pOptions(NULL),
m_materialIndexOffset(0)
{
}

CFbxObjConverter::~CFbxObjConverter()
{
}

bool CFbxObjConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CFbxObjOptions();
	CToObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}

// Convert a FBX file into an OBJ file
bool CFbxObjConverter::Convert(CFile* pOtherFile, CFile* pObjFile)
{
	// Initialize FBX -> OBJ
	m_pFbxFile = InitConversion<CFbxFile>(pOtherFile, pObjFile);
	OBJ_ASSERT(m_pFbxFile && m_pOptions);
	if(!m_pFbxFile || !m_pOptions)
		return false;

	MSG_INFO("Converting FBX file format into OBJ file format...");

	// Get FBX SDK objects
	KFbxSdkManager* pFbxSdkManager = m_pFbxFile->GetFbxSdkManager();
	KFbxScene* pScene = m_pFbxFile->GetFbxScene();

	OBJ_ASSERT(pFbxSdkManager && pScene);
	if(!pFbxSdkManager || !pScene)
		return false;

	// Get the scene meta data
	CFbxSdkUtil::GetSceneInfo(pScene);

	KFbxNode* pRootNode = pScene->GetRootNode();
	OBJ_ASSERT(pRootNode);

	if(pRootNode)
	{
		for(int i = 0; i < pRootNode->GetChildCount(); ++i)
		{
			NodeRecurse(pRootNode->GetChild(i));
		}
	}

	return true;
}


bool CFbxObjConverter::NodeRecurse(KFbxNode* pNode)
{
	OBJ_ASSERT(pNode);
	if(!pNode)
		return false;

	KFbxNodeAttribute::EAttributeType attributeType;

	if(pNode->GetNodeAttribute() == NULL)
	{
		MSG_WARNING("NULL Node Attribute");
	}
	else
	{
		attributeType = pNode->GetNodeAttribute()->GetAttributeType();
		switch(attributeType)
		{
		case KFbxNodeAttribute::eMARKER:
			MSG_DEBUG("Marker detected and skipped");
			break;

		case KFbxNodeAttribute::eSKELETON:
			MSG_DEBUG("Skeleton detected and skipped");
			break;

		case KFbxNodeAttribute::eMESH:
			ConvertMesh(pNode);
			break;

		case KFbxNodeAttribute::eNURB:
			MSG_DEBUG("NURB detected and skipped");
			break;

		case KFbxNodeAttribute::ePATCH:
			MSG_DEBUG("Patch detected and skipped");
			break;

		case KFbxNodeAttribute::eCAMERA:
			MSG_DEBUG("Camera detected and skipped");
			break;

		case KFbxNodeAttribute::eLIGHT:
			MSG_DEBUG("Light detected and skipped");
			break;
		}
	}

	for(int i = 0; i < pNode->GetChildCount(); ++i)
	{
		NodeRecurse(pNode->GetChild(i));
	}

	return true;
}


bool CFbxObjConverter::ConvertMesh(KFbxNode* pNode)
{
	OBJ_ASSERT(pNode);
	if(!pNode)
		return false;

	KFbxMesh* pMesh = (KFbxMesh*)pNode->GetNodeAttribute();
	OBJ_ASSERT(pMesh);
	if(!pMesh)
		return false;

	bool status = false;

	CObjFile::CGroup group(pNode->GetName());


	KFbxMatrix transMat = CFbxSdkUtil::GetGeometricTransform(pNode);
	// Vertex position, and normals
	ConvertControlPoints(pMesh, transMat);

	// Polygons
	ConvertPolygons(pMesh, group);

	// Material mapping
	status = CFbxSdkMaterialUtil::ConvertMaterialMapping(pMesh, m_pObjFile, group);
	OBJ_ASSERT(status);

	// Texture Mapping
	status = CFbxSdkMaterialUtil::ConvertTextureMapping(pMesh, m_pObjFile, group);
	OBJ_ASSERT(status);

	// Add group to OBJ
	m_pObjFile->AddGroup(group);

	return true;
}




bool CFbxObjConverter::ConvertControlPoints(KFbxMesh* pMesh, KFbxMatrix& transMat)
{
	OBJ_ASSERT(pMesh);
	if(!pMesh)
		return false;

	int controlPointsCount = pMesh->GetControlPointsCount();
	KFbxVector4* pControlPoints = pMesh->GetControlPoints();

	Vector3DVector& objVertices = m_pObjFile->GetVertexPositionVector();
	Vector3DVector& objNormals = m_pObjFile->GetNormalVector();
	Vector3DVector& objTexCoords = m_pObjFile->GetTextureCoordVector();

	Vector3D vertexPos(3);
	Vector3D texCoord(3);
	Vector3D normal(3);

	for(int cp = 0; cp < controlPointsCount; ++cp) // For each FBX control point (mesh vertex)
	{
		vertexPos = CFbxSdkUtil::TransformAndConvertKFbxVector(pControlPoints[cp], transMat);
		objVertices.push_back(vertexPos);

		for(int layer = 0; layer < pMesh->GetLayerCount(); ++layer) // For each FBX layer
		{
			// UVs
			KFbxLayerElementUV* pLayerElementUV = pMesh->GetLayer(layer)->GetUVs();
			if(pLayerElementUV)
			{
				if(pLayerElementUV->GetMappingMode() == KFbxLayerElement::eBY_CONTROL_POINT) // PER CP
				{
					if(pLayerElementUV->GetReferenceMode() == KFbxLayerElement::eDIRECT)
					{
						KFbxVector2& fbxUV = pLayerElementUV->GetDirectArray().GetAt(cp);
						texCoord = CFbxSdkUtil::ConvertKFbxVector(fbxUV);
						objTexCoords.push_back(texCoord);
					}
					else if(pLayerElementUV->GetReferenceMode() == KFbxLayerElement::eINDEX_TO_DIRECT)
					{
						int id = pLayerElementUV->GetIndexArray().GetAt(cp);
						KFbxVector2& fbxUV = pLayerElementUV->GetDirectArray().GetAt(id);
						texCoord = CFbxSdkUtil::ConvertKFbxVector(fbxUV);
						objTexCoords.push_back(texCoord);
					}
					else
					{
						OBJ_ASSERT(false); // test
					}
				}
			}

			// Normals
			KFbxLayerElementNormal* pLayerElementNormal = pMesh->GetLayer(layer)->GetNormals();
			if(pLayerElementNormal)
			{
				if(pLayerElementNormal->GetMappingMode() == KFbxLayerElement::eBY_CONTROL_POINT) // PER CP
				{
					if(pLayerElementNormal->GetReferenceMode() == KFbxLayerElement::eDIRECT)
					{
						KFbxVector4& fbxNormal = pLayerElementNormal->GetDirectArray().GetAt(cp);
						normal = CFbxSdkUtil::ConvertKFbxVector(fbxNormal);
						objNormals.push_back(normal);
					}
					else if(pLayerElementNormal->GetReferenceMode() == KFbxLayerElement::eINDEX_TO_DIRECT)
					{
						int id = pLayerElementNormal->GetIndexArray().GetAt(cp);
						KFbxVector4& fbxNormal = pLayerElementNormal->GetDirectArray().GetAt(id);
						normal = CFbxSdkUtil::ConvertKFbxVector(fbxNormal);
						objNormals.push_back(normal);
					}
					else
					{
						OBJ_ASSERT(false);
					}
				}
			}

		} // End for each layer

	} // End for each control point

	return true;
}



bool CFbxObjConverter::ConvertPolygons(KFbxMesh* pMesh, CObjFile::CGroup& objGroup)
{
	OBJ_ASSERT(pMesh);
	if(!pMesh)
		return false;

	Vector3DVector& objNormals = m_pObjFile->GetNormalVector();
	Vector3DVector& objTexCoords = m_pObjFile->GetTextureCoordVector();

	Vector3D texCoord(3);

	// UVs LIST
	for(int layer = 0; layer < pMesh->GetLayerCount(); ++layer) // For each FBX Layer
	{
		// UVs
		KFbxLayerElementUV* pLayerElementUV = pMesh->GetLayer(layer)->GetUVs();
		if(pLayerElementUV)
		{
			if(pLayerElementUV->GetMappingMode() == KFbxLayerElement::eBY_POLYGON_VERTEX) // PER poly-vertex
			{
				if(pLayerElementUV->GetReferenceMode() == KFbxLayerElement::eDIRECT ||
				   pLayerElementUV->GetReferenceMode() == KFbxLayerElement::eINDEX_TO_DIRECT)
				{
					KArrayKFbxVector2& fbxUVs = pLayerElementUV->GetDirectArray();
					objTexCoords.reserve(fbxUVs.GetCount());

					for(int uvIndex=0; uvIndex < fbxUVs.GetCount(); ++uvIndex)
					{
						texCoord = CFbxSdkUtil::ConvertKFbxVector(fbxUVs.GetAt(uvIndex));
						objTexCoords.push_back(texCoord);
					}
				}
			}
		}
	}




	KFbxVector4* pControlPoints = pMesh->GetControlPoints();
	int polygonCount = pMesh->GetPolygonCount();
	int vertexCounter = 0;
	objGroup.GetFaceVector().reserve(polygonCount); // reserve face vector

	for(int poly = 0; poly < polygonCount; ++poly) // For each FBX polygon
	{
		CObjFile::CFace objFace;

		// Smoothing groups
		for(int layer = 0; layer < pMesh->GetLayerCount(); ++layer) // For each FBX Layer
		{
			KFbxLayerElementPolygonGroup* pLayerElementPGroup = pMesh->GetLayer(layer)->GetPolygonGroups(); // per Polygon
			if(pLayerElementPGroup)
			{
				switch(pLayerElementPGroup->GetMappingMode())
				{
				case KFbxLayerElement::eBY_POLYGON:
					if(pLayerElementPGroup->GetReferenceMode() == KFbxLayerElement::eINDEX)
					{
						// Smoothing group = polygroup?
						int polyGroupID = pLayerElementPGroup->GetIndexArray().GetAt(poly);
						if(polyGroupID >= 0)
						{
							objFace.SetSmoothingGroup(std::make_pair(true, polyGroupID));
						}
						break;
					}
				default:
					OBJ_ASSERT(false); // any other mapping modes don't make sense
					break;
				}
			}
		}

		std::pair< bool, uint > uvPair;
		std::pair< bool, uint > normalPair;
		int polygonSize = pMesh->GetPolygonSize(poly);

		for(int vert = 0; vert < polygonSize; ++vert) // For each vertex in polygon
		{
			uvPair = std::make_pair(false, 0);
			normalPair = std::make_pair(false, 0);

			int controlPointIndex = pMesh->GetPolygonVertex(poly, vert);

			int polyGroupID = pMesh->GetPolygonGroup(poly);
			if(polyGroupID >= 0)
			{
				objFace.SetSmoothingGroup(std::make_pair(true, polyGroupID));
			}


			for(int layer = 0; layer < pMesh->GetLayerCount(); ++layer) // For each FBX layer
			{
				// Vertex Color Ignored
				KFbxLayerElementVertexColor* pLayerElementVC = pMesh->GetLayer(layer)->GetVertexColors();

				// UVs
				KFbxLayerElementUV* pLayerElementUV = pMesh->GetLayer(layer)->GetUVs();
				if(pLayerElementUV)
				{
					if(pLayerElementUV->GetMappingMode() == KFbxLayerElement::eBY_POLYGON_VERTEX)
					{
						int textureUVIndex = pMesh->GetTextureUVIndex(poly, vert);
						uvPair = std::make_pair(true, textureUVIndex + 1); // OBJ is 1-based index
					}
					else if(pLayerElementUV->GetMappingMode() == KFbxLayerElement::eBY_CONTROL_POINT)
					{
						uvPair = std::make_pair(true, vertexCounter + 1); // OBJ is 1-based index
					}
				}


				// Normals
				KFbxLayerElementNormal* pLayerElementNormal = pMesh->GetLayer(layer)->GetNormals();
				if(pLayerElementNormal)
				{
					Vector3D normal(3);
					if(pLayerElementNormal->GetMappingMode() == KFbxLayerElement::eBY_POLYGON_VERTEX)
					{
						if(pLayerElementNormal->GetReferenceMode() == KFbxLayerElement::eDIRECT)
						{
							KFbxVector4& fbxNormal = pLayerElementNormal->GetDirectArray().GetAt(vertexCounter);
							normal = CFbxSdkUtil::ConvertKFbxVector(fbxNormal);
							objNormals.push_back(normal);
							normalPair = std::make_pair(true, vertexCounter + 1); // OBJ is 1-based index
						}
						else if(pLayerElementNormal->GetReferenceMode() == KFbxLayerElement::eINDEX_TO_DIRECT && pLayerElementNormal->GetIndexArray())
						{
							OBJ_ASSERT(false); // never seen this, debug this
							int normalID = pLayerElementNormal->GetIndexArray().GetAt(vertexCounter);
							KFbxVector4& fbxNormal = pLayerElementNormal->GetDirectArray().GetAt(normalID);
							normal = CFbxSdkUtil::ConvertKFbxVector(fbxNormal);
							objNormals.push_back(normal);
							normalPair = std::make_pair(true, vertexCounter + 1); // OBJ is 1-based index
						}
						else
						{
							OBJ_ASSERT(false);
						}
					}
					else if(pLayerElementNormal->GetMappingMode() == KFbxLayerElement::eBY_CONTROL_POINT)
					{
						normalPair = std::make_pair(true, controlPointIndex + 1);  // OBJ is 1-based index
					}
				}
			}

			objFace.AddVertex(CObjFile::CVertex(controlPointIndex + 1, uvPair, normalPair)); // Add vertex to the OBJ face
			vertexCounter++; // vertex ID

		} // End for each vertex in FBX polygon

		objGroup.AddFace(objFace); // Add face to the OBJ group

	} // End for each FBX polygon

	return true;
}






