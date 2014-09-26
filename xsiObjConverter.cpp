
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "SemanticLayer.h"

#include "objFile.h"
#include "xsiFile.h"

#include "xsiFtkUtil.h"
#include "xsiFtkGeometryUtil.h"
#include "xsiFtkMaterialUtil.h"

#include "xsiObjConverter.h"


CXsiObjConverter::CXsiObjConverter():
m_pXsiFile(NULL),
m_pOptions(NULL)
{
}

CXsiObjConverter::~CXsiObjConverter()
{
}


bool CXsiObjConverter::ParseOptions(const std::string& optionString)
{
	m_pOptions = new CXsiObjOptions();
	CToObjConverter::ParseOptions(optionString, m_pOptions);
	return true;
}

// Convert a XSI file into an OBJ file
bool CXsiObjConverter::Convert(CFile* pOtherFile, CFile* pObjFile)
{
	// Initialize XSI -> OBJ
	m_pXsiFile = InitConversion<CXsiFile>(pOtherFile, pObjFile);
	OBJ_ASSERT(m_pXsiFile && m_pOptions);
	if(!m_pXsiFile || !m_pOptions)
		return false;

	MSG_INFO("Converting XSI file format into OBJ file format...");

	::CSLScene* pScene = m_pXsiFile->GetScene();

	OBJ_ASSERT(pScene);
	if(!pScene)
		return false;

	// Look at the scene file informations
	DotXSILoadFileInfo(pScene);

	// Load material library
	CXsiFtkMaterialUtil::DotXSILoadMaterialLibrary(m_pObjFile, pScene);

	// Start the recursion from the scene's root model
	DotXSILoadMeshes(pScene->Root());

	return true;
}






bool CXsiObjConverter::DotXSILoadFileInfo(CSLScene* pScene)
{
	// Does this file have SI_FileInfo template instance?
	CSLFileInfo* pFileInfo = pScene->FileInfo();

	OBJ_ASSERT(pFileInfo);
	if(!pFileInfo)
		return false;

	MSG_DEBUG("Project Name: " << pFileInfo->GetProjectName());
	MSG_DEBUG("User Name: " << pFileInfo->GetUsername());
	MSG_DEBUG("Last Time Modified: " << pFileInfo->GetSaveDateTime());
	MSG_DEBUG("Built in: " << pFileInfo->GetOriginator());

	return true;
}


bool CXsiObjConverter::DotXSILoadMeshes(CSLModel* pModel)
{
	// Load the meshes
	if (!pModel)
		return false;

	// Get the children
	CSLModel** ppModels = pModel->GetChildrenList();
	int iChildrenCount = pModel->GetChildrenCount();
	for(int i=0;i<iChildrenCount;i++)
	{
		CSLModel* pThisModel = ppModels[i];

		// Only interested in meshes
		if (pThisModel->GetPrimitiveType() == CSLModel::SI_MESH)
		{
			std::string modelName;

			// Load the model
			if(DotXSILoadModel(pThisModel, modelName) == true)
			{
				CObjFile::CGroup group(modelName);

				DotXSILoadMesh(pThisModel, group);	// Load the mesh

				m_pObjFile->AddGroup(group); // Add group to OBJ file
			}			
		}

		// Recurse for children
		DotXSILoadMeshes(pThisModel);
	}

	return true;
}


bool CXsiObjConverter::DotXSILoadModel(CSLModel* pModel, std::string& modelName)
{
	OBJ_ASSERT(pModel);
	if(!pModel)
		return false;

	// Get the model's name
	modelName = pModel->GetName();

	MSG_DEBUG("Model Name: " << modelName);

	return true;
}


bool CXsiObjConverter::DotXSILoadMesh(CSLModel* pModel, CObjFile::CGroup& objGroup)
{
	OBJ_ASSERT(pModel);
	if(!pModel)
		return false;

	CSLTransform* pTransform = pModel->Transform(); // Get the transform of thie model
	OBJ_ASSERT(pTransform);
	if(!pTransform)
		return false;

	CSLMesh* pMesh = (CSLMesh*)pModel->Primitive(); // Get the mesh of this model
	OBJ_ASSERT(pMesh);
	if(!pMesh)
		return false;

	CSLBaseShape* pShape = pMesh->Shape(); // get the Shape of this model
	OBJ_ASSERT(pShape);
	if(!pShape)
		return false;

	// Vertex Positions
	{
		CSLBaseShape::CSLVector3DArray* pVertices = pShape->GetVertexList();
		Vector3DVector& objVertices = m_pObjFile->GetVertexPositionVector();
		CSIBCVector3D* pVector3D = NULL;
		Vector3D vertPos(3);
		objVertices.reserve(pVertices->GetSize());

		for(long vertexIt = 0; vertexIt < pVertices->GetSize(); ++vertexIt) // for each OBJ vertex position
		{
			pVector3D = &(*pVertices)[vertexIt];
			vertPos = CXsiFtkGeometryUtil::TransformAndConvertVector(pTransform, pVector3D); // Transform vertex position
			objVertices.push_back(vertPos); // add a vertex position
		}
	}

	// UV coordinates
	{
		long uvCoordArrayCount = ((CSLShape_35*)pShape)->GetUVCoordArrayCount();
		if( uvCoordArrayCount > 0 )
		{
			Vector3DVector& objTexCoords = m_pObjFile->GetTextureCoordVector();
			CSLUVCoordArray** pUvCoordsArrays = ((CSLShape_35*)pShape)->UVCoordArrays();

			for(long uvArrayIndex = 0; uvArrayIndex < uvCoordArrayCount; ++uvArrayIndex) // for each uv array
			{
				CSLUVCoordArray* pUvCoordArray = pUvCoordsArrays[uvArrayIndex];
				long uvCoordCount = pUvCoordArray->GetUVCoordCount();

				if(uvCoordCount > 0)
				{
					CSLBaseShape::CSLVector2DArray* pVector2DArray = pUvCoordArray->GetUVCoordList();
					CSIBCVector2D* pVector2D = NULL;
					Vector3D uvCoord(3);
					objTexCoords.reserve(pVector2DArray->GetSize());

					for(long uvCoordIndex = 0; uvCoordIndex < uvCoordCount; ++uvCoordIndex) // for each OBJ texCoord
					{
						pVector2D = &(*pVector2DArray)[uvCoordIndex];
						uvCoord[0] = pVector2D->GetX();
						uvCoord[1] = pVector2D->GetY();
						uvCoord[2] = 0.0f; // no W in dotXSI
						objTexCoords.push_back(uvCoord); // add a UV coordinate
					}
				}
			}
		}
	}


	// Normals
	{
		long normalCount = pShape->GetNormalCount();
		if(normalCount > 0)
		{
			CSLBaseShape::CSLVector3DArray* pNormals = pShape->GetNormalList();
			Vector3DVector& objNormals = m_pObjFile->GetNormalVector();
			CSIBCVector3D* pVector3D = NULL;
			Vector3D normal(3);
			objNormals.reserve(pNormals->GetSize());

			for(long normalIt = 0; normalIt < pNormals->GetSize(); ++normalIt) // for each OBJ normals
			{
				pVector3D = &(*pNormals)[normalIt];
				normal[0] = pVector3D->GetX();
				normal[1] = pVector3D->GetY();
				normal[2] = pVector3D->GetZ();
				objNormals.push_back(normal); // add a normal
			}
		}
	}


	bool bPolygonList = false;
	bool bTriangleList = false;
	bool bTriangleStripList = false;

	if(pMesh->GetPolygonListCount() > 0)
	{
		bPolygonList = true;
		DotXSILoadPolygonList(pMesh, objGroup);
	}
	else if(pMesh->GetTriangleListCount() > 0)
	{
		bTriangleList = true;
		DotXSILoadTriangleList(pMesh, objGroup);
	}
	else if(pMesh->GetTriangleStripListCount() > 0)
	{
		bTriangleStripList = true;
		DotXSILoadTriStripList(pMesh, objGroup);
	}

	return true;
}


// TODO : remove redundancy between DotXSILoadPolygonList, DotXSILoadTriangleList and DotXSILoadTriStripList


bool CXsiObjConverter::DotXSILoadPolygonList(CSLMesh* pMesh, CObjFile::CGroup& objGroup)
{
	uint polyListCount = pMesh->GetPolygonListCount();

	CSLPolygonList** polygonLists = pMesh->PolygonLists();
	uint materialID = 0;

	for(uint polyListIndex = 0; polyListIndex < polyListCount; ++polyListIndex) // For each polygon list
	{
		CSLPolygonList* pPolyList = polygonLists[polyListIndex]; // Retreive this polygon list

		CSLPolygonList::CSLIntArray* pVertCountList = pPolyList->GetPolygonVertexCountList();
		CSLPolygonList::CSLIntArray* pVertIndices = pPolyList->GetVertexIndices();
		CSLPolygonList::CSLIntArray* pUvIndices = pPolyList->GetUVIndices(0); // support only 1 UV set for now
		CSLPolygonList::CSLIntArray* pNormalIndices = pPolyList->GetNormalIndices();
		OBJ_ASSERT(pPolyList->GetUVArrayCount() <= 1); // support only 1 UV set for now

		// material for this PolygonList
		CSLBaseMaterial* pBaseMaterial = pPolyList->GetMaterial();
		if(pBaseMaterial)
		{
			std::string materialName(pBaseMaterial->GetName());
			materialID = m_pObjFile->GetMaterialIDFromName(materialName);
		}

		uint polyCount = pPolyList->GetPolygonCount();
		objGroup.GetFaceVector().reserve(polyCount); // reserve face vector
		long vertCounter = 0;

		for(uint polyIndex = 0; polyIndex < polyCount; ++polyIndex) // for each polygon
		{
			uint vertCount = (*pVertCountList)[polyIndex]; // number of vertices for this polygon

			CObjFile::CFace objFace;

			// Indices
			uint vertexIndex = 0;
			uint uvIndex = 0;
			uint normalIndex = 0;
			// Pairs
			std::pair< bool, uint > uvPair;
			std::pair< bool, uint > normalPair;

			for(uint vertIndex = 0; vertIndex < vertCount; ++vertIndex) // for each vertex on the face
			{
				vertexIndex = (*pVertIndices)[vertCounter] + 1; // OBJ is 1-based index
				uvIndex = 0;
				normalIndex = 0;

				uvPair = std::make_pair(false, uvIndex);
				normalPair = std::make_pair(false, normalIndex);

				// UVs
				if(pUvIndices && vertCounter < pUvIndices->GetSize())
				{
					uvIndex = (*pUvIndices)[vertCounter];
					uvPair = std::make_pair(true, uvIndex + 1); // OBJ is 1-based index
				}

				// Normals
				if(pNormalIndices && vertCounter < pNormalIndices->GetSize())
				{
					normalIndex = (*pNormalIndices)[vertCounter];
					normalPair = std::make_pair(true, normalIndex + 1); // OBJ is 1-based index
				}

				objFace.AddVertex( CObjFile::CVertex(vertexIndex, uvPair, normalPair) );
				objFace.SetMaterialID(materialID);

				++vertCounter;

			} // End for each vertex

			objGroup.AddFace(objFace); // Add polygon face to OBJ Group

		} // End for each polygon

	} // End for each polygon list

	return true;
}




bool CXsiObjConverter::DotXSILoadTriangleList(CSLMesh* pMesh, CObjFile::CGroup& objGroup)
{
	uint triangleListCount = pMesh->GetTriangleListCount();

	CSLTriangleList** triangleLists = pMesh->TriangleLists();
	uint materialID = 0;

	for(uint triangleListIndex = 0; triangleListIndex < triangleListCount; ++triangleListIndex) // For each triangle list
	{
		CSLTriangleList* pTriangleList = triangleLists[triangleListIndex]; // Retreive this triangle list

		CSLTriangleList::CSLTriangleArray* pVertIndices = pTriangleList->GetVertexIndices();
		CSLTriangleList::CSLTriangleArray* pUvIndices = pTriangleList->GetUVIndices(0); // support only 1 UV set for now
		CSLTriangleList::CSLTriangleArray* pNormalIndices = pTriangleList->GetNormalIndices();
		OBJ_ASSERT(pTriangleList->GetUVArrayCount() <= 1); // support only 1 UV set for now

		// material for this TriangleList
		CSLBaseMaterial* pBaseMaterial = pTriangleList->GetMaterial();
		if(pBaseMaterial)
		{
			std::string materialName(pBaseMaterial->GetName());
			materialID = m_pObjFile->GetMaterialIDFromName(materialName);
		}

		uint triCount = pTriangleList->GetTriangleCount();
		objGroup.GetFaceVector().reserve(triCount); // reserve face vector

		for(uint triIndex = 0; triIndex < triCount; ++triIndex) // for each triangle
		{
			CObjFile::CFace objFace;

			SI_Int* vertIndices = &(*pVertIndices)[triIndex].m_iVtx1; // Retreive array of 3 vertex indices for this triangle
			SI_Int* uvIndices = NULL;
			SI_Int* normalIndices = NULL;

			if(pUvIndices)
				uvIndices = &(*pUvIndices)[triIndex].m_iVtx1; // Retreive array of 3 texCoord indices for this triangle

			if(pNormalIndices)
				normalIndices = &(*pNormalIndices)[triIndex].m_iVtx1; // Retreive array of 3 normal indices for this triangle

			// Indices
			uint vertexIndex = 0;
			uint uvIndex = 0;
			uint normalIndex = 0;
			// Pairs
			std::pair< bool, uint > uvPair;
			std::pair< bool, uint > normalPair;

			for(uint vertIndex = 0; vertIndex < 3; ++vertIndex) // for each vertex in the triangle
			{
				vertexIndex = vertIndices[vertIndex] + 1;  // OBJ is 1-based index
				uvIndex = 0;
				normalIndex = 0;

				uvPair = std::make_pair(false, uvIndex);
				normalPair = std::make_pair(false, normalIndex);

				// UVs
				if(pUvIndices && uvIndices)
				{
					uvIndex = uvIndices[vertIndex];
					uvPair = std::make_pair(true, uvIndex + 1); // OBJ is 1-based index
				}

				// Normals
				if(pNormalIndices && normalIndices)
				{
					normalIndex = normalIndices[vertIndex];
					normalPair = std::make_pair(true, normalIndex + 1); // OBJ is 1-based index
				}

				objFace.AddVertex( CObjFile::CVertex(vertexIndex, uvPair, normalPair) );
				objFace.SetMaterialID(materialID); // Set material ID

			} // End for each vertex

			objGroup.AddFace(objFace); // Add triangle face to OBJ Group

		} // End for each triangle

	} //  End for each triangle list

	return true;
}


bool CXsiObjConverter::DotXSILoadTriStripList(CSLMesh* pMesh, CObjFile::CGroup& objGroup)
{
	uint triStripListCount = pMesh->GetTriangleStripListCount();

	CSLTriangleStripList** pTriStripLists = pMesh->TriangleStripLists();
	uint materialID = 0;


	for(uint triStripListIndex = 0; triStripListIndex < triStripListCount; ++triStripListIndex) // For each triangle strip list
	{
		CSLTriangleStripList* pTriangleList = pTriStripLists[triStripListIndex]; // Retreive this triangle list

		// material for this TriangleStripList
		CSLBaseMaterial* pBaseMaterial = pTriangleList->GetMaterial();
		if(pBaseMaterial)
		{
			std::string materialName(pBaseMaterial->GetName());
			materialID = m_pObjFile->GetMaterialIDFromName(materialName);
		}

		// Tri Strips
		uint triStripCount = pTriangleList->GetTriangleStripCount(); // number of tri strips
		CSLTriangleStrip** pTriStrips = pTriangleList->TriangleStrips();

		for(uint triStripIndex = 0; triStripIndex < triStripCount; ++triStripIndex) // For each triangle strip
		{
			CSLTriangleStrip* pTriStrip = pTriStrips[triStripIndex]; //  Retreive this triangle strip

			CSLTriangleStrip::CSLIntArray* pVertIndices = pTriStrip->GetVertexIndices();
			CSLTriangleStrip::CSLIntArray* pUvIndices = pTriStrip->GetUVIndices(0); // support only 1 UV set for now
			CSLTriangleStrip::CSLIntArray* pNormalIndices = pTriStrip->GetNormalIndices();
			OBJ_ASSERT(pTriStrip->GetUVArrayCount() <= 1); // support only 1 UV set for now

			uint vertexCount = pTriStrip->GetVertexCount();

			// un-stripping
			int second = 0;
			int last = 0;

			bool bHasUvs = (pUvIndices != NULL);
			bool bHasNormals = (pNormalIndices != NULL);

			// TODO : objGroup.GetFaceVector().reserve()

			for(uint vertIndex = 0; vertIndex < vertexCount-2; ++vertIndex) // for each vertex in the triangle strip
			{
				CObjFile::CFace objFace;
				objFace.SetMaterialID(materialID); // Set material ID

				// Un-Stripping
				second = vertIndex+1;
				last = vertIndex+2;
				if(vertIndex & 1)
				{
					second = vertIndex+2;
					last = vertIndex+1;
				}

				// Add vertices to face
				objFace.AddVertex( CObjFile::CVertex((*pVertIndices)[vertIndex] + 1,  std::make_pair(bHasUvs, (*pUvIndices)[vertIndex] + 1),  std::make_pair(bHasNormals, (*pNormalIndices)[vertIndex] + 1)) );
				objFace.AddVertex( CObjFile::CVertex((*pVertIndices)[second] + 1,  std::make_pair(bHasUvs, (*pUvIndices)[second] + 1),  std::make_pair(bHasNormals, (*pNormalIndices)[second] + 1)) );
				objFace.AddVertex( CObjFile::CVertex((*pVertIndices)[last] + 1,  std::make_pair(bHasUvs, (*pUvIndices)[last] + 1),  std::make_pair(bHasNormals, (*pNormalIndices)[last] + 1)) );
				objGroup.AddFace(objFace); // Add face to OBJ Group

			} // End for each vertex in triangle strip

		} // End for each triangle strip
	
	} // End for each triangle strip list

	return true;
}


