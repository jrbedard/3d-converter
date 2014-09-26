
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdkUtil.h"
#include "fbxSdkMaterialUtil.h"

#include "objFile.h"


// Convert the material assignment to polygons
bool CFbxSdkMaterialUtil::ConvertMaterialMapping(KFbxMesh* pMesh, CObjFile* pObjFile, CObjFile::CGroup& objGroup)
{
	OBJ_ASSERT(pMesh && pObjFile);
	if(!pMesh || !pObjFile)
		return false;

	for(int layer = 0; layer < pMesh->GetLayerCount(); ++layer)
	{
		KFbxLayerElementMaterial* pLayerElementMat = pMesh->GetLayer(layer)->GetMaterials();
		if(pLayerElementMat)
		{
			if(!(pLayerElementMat->GetMappingMode() == KFbxLayerElement::eBY_POLYGON) &&
			   !(pLayerElementMat->GetMappingMode() == KFbxLayerElement::eALL_SAME))
			{
				OBJ_ASSERT(false); // Unknown mapping mode for materials
				continue;
			}

			if(!(pLayerElementMat->GetReferenceMode() == KFbxLayerElement::eDIRECT) &&
			   !(pLayerElementMat->GetReferenceMode() == KFbxLayerElement::eINDEX_TO_DIRECT))
			{
				OBJ_ASSERT(false); // Unknown reference mode
				continue;
			}

			ulong polyCount = 0;
			int* pMaterialIndices = NULL;


			if(pLayerElementMat->GetMappingMode() == KFbxLayerElement::eBY_POLYGON)
				polyCount = pLayerElementMat->GetIndexArray().GetCount(); // polyCount = correspond to number of material Indices
			else if(pLayerElementMat->GetMappingMode() == KFbxLayerElement::eALL_SAME)
				polyCount = pMesh->GetPolygonCount();


			if(pLayerElementMat->GetReferenceMode() == KFbxLayerElement::eDIRECT)
			{
				OBJ_ASSERT(false); // is this right?
				pMaterialIndices = pLayerElementMat->GetIndexArray().GetArray();
			}
			else if(pLayerElementMat->GetReferenceMode() == KFbxLayerElement::eINDEX_TO_DIRECT)
				pMaterialIndices = pLayerElementMat->GetIndexArray().GetArray();

			int materialID = -1;
			int lastMaterialID = -1;
			int objMaterialID = -1;

			for(ulong p = 0; p < polyCount; ++p) // For each polygon
			{
				if(pLayerElementMat->GetMappingMode() == KFbxLayerElement::eBY_POLYGON)
					materialID = pMaterialIndices[p];
				else if(pLayerElementMat->GetMappingMode() == KFbxLayerElement::eALL_SAME)
					materialID = pMaterialIndices[0];

				if(materialID != lastMaterialID) // speed this loop
				{
					objMaterialID = CreateMaterial(pObjFile, pLayerElementMat, materialID);
					lastMaterialID = materialID;
				}

				OBJ_ASSERT(p < objGroup.GetFaceVector().size());
				if(p < objGroup.GetFaceVector().size())
				{
					objGroup.GetFaceVector()[p].SetMaterialID(objMaterialID); // Set OBJ material ID
				}
			}
		
		} // End if pLayerElementMat

	} // End for each FBX layer

	return true;
}



int CFbxSdkMaterialUtil::CreateMaterial(CObjFile* pObjFile, KFbxLayerElementMaterial* pLayerElementMat, int fbxMatIndex)
{
	int objMaterialID = -1;

	OBJ_ASSERT(pObjFile && pLayerElementMat);
	if(!pObjFile || !pLayerElementMat)
		return objMaterialID;

	if(pLayerElementMat->GetReferenceMode() == KFbxLayerElement::eINDEX) // materials are in an undefined external table				
		OBJ_ASSERT(false);

	int materialCount = pLayerElementMat->GetDirectArray().GetCount();
	OBJ_ASSERT(fbxMatIndex <= materialCount);
	if(fbxMatIndex < materialCount)
	{
		KFbxMaterial* pMaterial = pLayerElementMat->GetDirectArray().GetAt(fbxMatIndex);
		OBJ_ASSERT(pMaterial);
		if(!pMaterial)
			return objMaterialID;

		std::string materialName(pMaterial->GetName());

		// Check If this material is already created in MTL file
		CMtlFile* pMtlFile = pObjFile->GetMaterialFile();
		OBJ_ASSERT(pMtlFile);

		bool bMaterialExist = pMtlFile->MaterialExist(materialName);

		if(!bMaterialExist) // If material doent exist in MTL
		{
			CMtlFile::CMaterial newMaterial(materialName); // Create new MTL material

			// Shading model
			//lMaterial->GetShadingModel()); 

			// Ambient
			Vector3D ambient = CFbxSdkUtil::ConvertKFbxColor(pMaterial->GetAmbient());
			newMaterial.SetKa(ambient);

			// Diffuse
			Vector3D diffuse = CFbxSdkUtil::ConvertKFbxColor(pMaterial->GetDiffuse());
			newMaterial.SetKd(diffuse);

			// Specular
			Vector3D specular = CFbxSdkUtil::ConvertKFbxColor(pMaterial->GetSpecular());
			newMaterial.SetKs(specular);

			// Shininess
			double shininess = pMaterial->GetShininess();
			newMaterial.SetNs(shininess);

			//DisplayColor("Emissive: ", lMaterial->GetEmissive());
			//DisplayDouble("Opacity: ", lMaterial->GetOpacity());
			//DisplayDouble("Reflectivity: ", lMaterial->GetReflectivity());

			// THE TEXTURE MAP is added in the method ConvertTextureMapping below

			objMaterialID = pObjFile->AddMaterial(newMaterial); // Add material to material library
			OBJ_ASSERT(objMaterialID != -1);
		}
		else
		{
			// Find its index
			objMaterialID = pObjFile->GetMaterialIDFromName(materialName);
			OBJ_ASSERT(objMaterialID != -1);
		}
	}

	return objMaterialID;
}




// Texture assignement to polygons
bool CFbxSdkMaterialUtil::ConvertTextureMapping(KFbxMesh* pMesh, CObjFile* pObjFile, CObjFile::CGroup& objGroup)
{
	OBJ_ASSERT(pMesh && pObjFile);
	if(!pMesh || !pObjFile)
		return false;

	for(int layer = 0; layer < pMesh->GetLayerCount(); ++layer) // For each FBX layer
	{
		KFbxLayerElementTexture* pLayerElementTex = pMesh->GetLayer(layer)->GetTextures();
		if(pLayerElementTex)
		{
			//pLayerElementTex->GetMappingMode();
			//pLayerElementTex->GetBlendMode();
			//pLayerElementTex->GetAlpha();

			ulong textureCount = 0;
			ulong polyCount = 0;

			switch(pLayerElementTex->GetMappingMode())
			{
			case KFbxLayerElement::eBY_POLYGON:
				textureCount = pMesh->GetPolygonCount();
				polyCount = pLayerElementTex->GetIndexArray().GetCount(); // polyCount = correspond to number of texture Indices
				break;
			case KFbxLayerElement::eALL_SAME:
				textureCount = 1;
				polyCount = pMesh->GetPolygonCount();
				break;
			default:
				OBJ_ASSERT(false);
				break; // other type should not occur.
			}

			if(pLayerElementTex->GetReferenceMode() == KFbxLayerElement::eINDEX ||
			   pLayerElementTex->GetReferenceMode() == KFbxLayerElement::eINDEX_TO_DIRECT)
			{
				int* pTextureIndices = pLayerElementTex->GetIndexArray().GetArray();

				int textureID = -1;
				int lastTextureID = -1;
				int materialID = -1;

				for(ulong p = 0; p < polyCount; ++p) // For each OBJ polygon
				{
					if(pLayerElementTex->GetMappingMode() == KFbxLayerElement::eBY_POLYGON)
						textureID = pTextureIndices[p];
					else if(pLayerElementTex->GetMappingMode() == KFbxLayerElement::eALL_SAME)
						textureID = pTextureIndices[0];

					if(textureID != lastTextureID) // speed this loop
					{
						if(!pLayerElementTex->GetDirectArray()) // v1hack
							continue;

						KFbxTexture* pTexture = pLayerElementTex->GetDirectArray().GetAt(textureID);
						OBJ_ASSERT(pTexture);
						if(!pTexture)
							continue;

						std::string textureName(pTexture->GetName());
						std::string textureFileName(pTexture->GetFileName()); // Retreive the texture file path

						OBJ_ASSERT(p < objGroup.GetFaceVector().size());
						materialID = objGroup.GetFaceVector()[p].GetMaterialID(); // Retreive material assigned to this polygon

						std::string materialName = pObjFile->GetMaterialNameFromID(materialID);
						CMtlFile* pMtlFile = pObjFile->GetMaterialFile();
						OBJ_ASSERT(pMtlFile);
						CMtlFile::CMaterial& material = pMtlFile->GetMaterial(materialName);
						material.SetTexMap(textureFileName); // Add texture map filename to this material

						lastTextureID = textureID;
					}

				}
			}
		}
	}

	return true;
}



