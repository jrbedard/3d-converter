
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "SemanticLayer.h"

#include "xsiFtkUtil.h"
#include "xsiFtkMaterialUtil.h"

#include "objFile.h"


// Create the XSI material library
CSLMaterialLibrary* CXsiFtkMaterialUtil::CreateMaterialLibrary(CObjFile* pObjFile, CSLScene* pScene, bool bTranslateMaps)
{
	CMtlFile* pMtlFile = pObjFile->GetMaterialFile();
	OBJ_ASSERT(pMtlFile);
	if(!pMtlFile)
		return NULL;

	CSLMaterialLibrary* pMaterialLibrary = pScene->GetMaterialLibrary();
	OBJ_ASSERT(pMaterialLibrary);
	if(!pMaterialLibrary)
		return NULL;

	pMaterialLibrary->SetName("MATLIB-Scene");

	// Create global material : DefaultLib.Scene_Material, always there even if no mesh has more than 1 material assigned
	{
		CMtlFile::CMaterial sceneMaterial("Scene_Material");
		bool stat = AddXSIMaterial(&sceneMaterial, pScene, bTranslateMaps);
		OBJ_ASSERT(stat);
	}

	// Material map
	CMtlFile::MaterialMap& materials = pMtlFile->GetMaterialMap();
	CMtlFile::MaterialMap::iterator materialIt;
	CMtlFile::MaterialMap::const_iterator materialEnd = materials.end();

	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each MTL material
	{
		CMtlFile::CMaterial* pMaterial = &materialIt->second;

		if(!pMaterial)
			continue;

		bool stat = AddXSIMaterial(pMaterial, pScene, bTranslateMaps);
		OBJ_ASSERT(stat);

	} // End for each MTL material

	return pMaterialLibrary;
}



// LEGACY : we use AddXSIMaterial now
bool CXsiFtkMaterialUtil::AddSLMaterial(CMtlFile::CMaterial* pMaterial, CSLMaterialLibrary* pMaterialLibrary, bool bTranslateMaps)
{
	OBJ_ASSERT(pMaterial && pMaterialLibrary);
	if(!pMaterial || !pMaterialLibrary)
		return false;

	std::string materialName(pMaterial->GetMaterialName());

	// Adding a new material to the XSI material library.
	CSLMaterial* pSLMaterial = pMaterialLibrary->AddMaterial();
	pSLMaterial->SetName((char*)materialName.c_str());

	// default values:
	{
		pSLMaterial->SetDiffuseColor(CSIBCColorf(0.7f, 0.7f, 0.7f));
		pSLMaterial->SetEmissiveColor(CSIBCColorf(0.0f, 0.0f, 0.0f));
		pSLMaterial->SetShadingModel(CSLMaterial::PHONG);
		pSLMaterial->SetSpecularColor(CSIBCColorf(0.5f, 0.5f, 0.5f));
		pSLMaterial->SetSpecularDecay(50.0f);
		pSLMaterial->SetAmbientColor(CSIBCColorf(0.0f, 0.0f, 0.0f));
	}

	if(bTranslateMaps && pMaterial->HasTexMap()) // Add a texture to this material.
	{
		pSLMaterial->CreateTexture2D()->SetImageFileName((char*)pMaterial->GetTexMap().c_str());
	}

	return true;
}



// Add XSI Material to the material library
bool CXsiFtkMaterialUtil::AddXSIMaterial(CMtlFile::CMaterial* pMaterial, CSLScene* pScene, bool bTranslateMaps)
{
	OBJ_ASSERT(pMaterial && pScene);
	if(!pMaterial || !pScene)
		return false;

	std::string materialName(pMaterial->GetMaterialName());

	std::string defaultLibMateralName("DefaultLib.");
	defaultLibMateralName += materialName;

	// TODO : Check if the materialName already exsits before creating

	// Adding a new material to the XSI material library.
	CSLXSIMaterial* pXSIMaterial = pScene->GetMaterialLibrary()->AddXSIMaterial();
	pXSIMaterial->SetName((char*)defaultLibMateralName.c_str());


	// By default, its a Phong material
	CSLXSIShader* pXSIShader = pXSIMaterial->AddShader();
	pXSIShader->SetName("Phong");

	// Connect Shader to Material
	CSLConnectionPoint* pConnectionPoint;
	pConnectionPoint = CXsiFtkUtil::ConnectShader(pXSIShader, pXSIMaterial, "surface");
	pConnectionPoint = CXsiFtkUtil::ConnectShader(pXSIShader, pXSIMaterial, "shadow");
	pConnectionPoint = CXsiFtkUtil::ConnectShader(pXSIShader, pXSIMaterial, "Photon");


	pXSIShader->SetProgID("Softimage.material-phong.1");

	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "Name", "Phong");

	// default values // TODO : look if the MTL contains real values

	// Ambient color
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "ambient.red",	  0.3f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "ambient.green", 0.3f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "ambient.blue",  0.3f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "ambient.alpha", 0.0f);

	// Diffuse color
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "diffuse.red",	  0.7f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "diffuse.green", 0.7f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "diffuse.blue",  0.7f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "diffuse.alpha", 0.0f);

	// Specular color
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "specular.red",   1.0f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "specular.green", 1.0f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "specular.blue",  1.0f);
	CXsiFtkUtil::SetShaderParameterValue(pXSIShader, "specular.alpha", 0.0f);

	// Shinny
	// TODO : add other values... a lot of other values...


	// if this Material has a texture.
	if(bTranslateMaps && pMaterial->HasTexMap())
	{
		std::string textureSourceFile(pMaterial->GetTexMap());
		CSLXSIShader* pTexShader = AddImage(pXSIMaterial, textureSourceFile, pScene);

		OBJ_ASSERT(pTexShader);
		if(pTexShader)
		{
			CSLShaderConnectionPoint* pShaderConnection;
			pShaderConnection = CXsiFtkUtil::ConnectShader(pTexShader, pXSIShader, "diffuse");
		}
	}


	// If the material has displacement map
	if(bTranslateMaps && pMaterial->HasDispMap())
	{
		std::string dispSourceFile(pMaterial->GetDispMap());
		double alphaDepthFactor = 0.5f;
		if(pMaterial->HasDispADF())
		{
			alphaDepthFactor = pMaterial->GetDispADF();
		}

		CSLXSIShader* pDispShader = AddDisplacementMap(pXSIMaterial, dispSourceFile, alphaDepthFactor, pScene);

		OBJ_ASSERT(pDispShader);
		if(pDispShader)
		{
			CSLConnectionPoint* pConnectionPoint;
			pConnectionPoint = CXsiFtkUtil::ConnectShader(pDispShader, pXSIMaterial, "displacement");
		}
	}



	// Material Info
	{
		CSLXSIMaterialInfo* pXSIMaterialInfo = pXSIMaterial->CreateMaterialInfo();
		pXSIMaterialInfo->SetUWrap(CSLXSIMaterialInfo::SI_REPEAT);
		pXSIMaterialInfo->SetVWrap(CSLXSIMaterialInfo::SI_REPEAT);
	}

	return true;
}




// Add displacement map shader
CSLXSIShader* CXsiFtkMaterialUtil::AddDisplacementMap(CSLXSIMaterial* pXSIMaterial, const std::string& dispFileName, double alphaDepthFactor, CSLScene* pScene)
{
	OBJ_ASSERT(pXSIMaterial && CheckStr(dispFileName) && pScene);
	if(!pXSIMaterial || !CheckStr(dispFileName) || !pScene)
		return NULL;

	// By default, its a Phong material
	CSLXSIShader* pDispShader = pXSIMaterial->AddShader();
	pDispShader->SetName("Softimage_sib_interp_linear");


	pDispShader->SetProgID("Softimage.sib_interp_linear.1");
	CXsiFtkUtil::SetShaderParameterValue(pDispShader, "Name", "Softimage_sib_interp_linear");

	// TODO : de-harcode parameters
	CXsiFtkUtil::SetShaderParameterValue(pDispShader, "input", (float)alphaDepthFactor);
	CXsiFtkUtil::SetShaderParameterValue(pDispShader, "oldrange_min", 0.0f);
	CXsiFtkUtil::SetShaderParameterValue(pDispShader, "oldrange_max", 1.0f);
	CXsiFtkUtil::SetShaderParameterValue(pDispShader, "newrange_min", -0.417f);
	CXsiFtkUtil::SetShaderParameterValue(pDispShader, "newrange_max", 0.417f);

	// Create Color2Scalar Shader
	CSLXSIShader* pColor2ScalarShader = pXSIMaterial->AddShader();
	{
		pColor2ScalarShader->SetName("Color2scalar");

		CSLShaderConnectionPoint* pShaderConnection;
		pShaderConnection = CXsiFtkUtil::ConnectShader(pColor2ScalarShader, pDispShader, "input");

		pColor2ScalarShader->SetProgID("Softimage.sib_color_to_scalar.1");
		CXsiFtkUtil::SetShaderParameterValue(pColor2ScalarShader, "Name", "Color2scalar");

		// TODO : de-harcode parameters
		CXsiFtkUtil::SetShaderParameterValue(pColor2ScalarShader, "input.red", 0.0f);
		CXsiFtkUtil::SetShaderParameterValue(pColor2ScalarShader, "input.green", 1.0f);
		CXsiFtkUtil::SetShaderParameterValue(pColor2ScalarShader, "input.blue", 0.0f);
		CXsiFtkUtil::SetShaderParameterValue(pColor2ScalarShader, "input.alpha", 0.0f);
		//CXsiFtkUtil::SetShaderParameterValue(pColor2ScalarShader, "alpha", true);


		CSLXSIShader* pTexShader = AddImage(pXSIMaterial, dispFileName, pScene);
		OBJ_ASSERT(pTexShader);
		if(pTexShader)
		{
			CSLShaderConnectionPoint* pShaderConnection;
			pShaderConnection = CXsiFtkUtil::ConnectShader(pTexShader, pColor2ScalarShader, "input");
		}

	}

	return pDispShader;
}



CSLXSIShader* CXsiFtkMaterialUtil::AddImage(CSLXSIMaterial* pXSIMaterial, const std::string& textureFileName, CSLScene* pScene)
{
	OBJ_ASSERT(pXSIMaterial && CheckStr(textureFileName) && pScene);
	if(!pXSIMaterial || !CheckStr(textureFileName) || !pScene)
		return NULL;

	std::string textureName = CXsiFtkUtil::ReplaceDotWithUnderscore(textureFileName);

	CSLXSIShader* pTexShader = pXSIMaterial->AddShader(); // Create Shader

	std::string imageName("Softimage_txt2d-image-explicit");

	static int imageNb = 0;
	std::stringstream ssImageName;
	ssImageName << imageName;
	ssImageName << imageNb;
	imageNb++;

	pTexShader->SetName((char*)ssImageName.str().c_str());
	pTexShader->SetProgID("Softimage.txt2d-image-explicit.1");

	CXsiFtkUtil::SetShaderParameterValue(pTexShader, "Name", (char*)ssImageName.str().c_str());

	// Add texture image to the image library
	{
		if(!pScene->GetImageLibrary())
		{
			pScene->CreateImageLibrary();
		}

		CSLImageLibrary* pImageLibrary = pScene->GetImageLibrary();
		OBJ_ASSERT(pImageLibrary);

		CSLImage* pXSIImage = pImageLibrary->FindImage((char*)textureName.c_str());
		if(!pXSIImage) // If the texture doesnt already exist
		{
			pXSIImage = pImageLibrary->AddImage();

			pXSIImage->SetName((char*)textureName.c_str());
			pXSIImage->SetSourceFile((char*)textureFileName.c_str()); // Set the texture path

			// Cause bug
			//CSLImageFX* pImageFX = pXSIImage->CreateImageFX();
		}

		// Connect Image to the current material's tex shader
		CXsiFtkUtil::ConnectImage(pXSIImage, pTexShader, "tex");
	}

	return pTexShader;
}




bool CXsiFtkMaterialUtil::CreateImageLibrary(CMtlFile::CMaterial* pMaterial, CSLScene* pScene)
{
	OBJ_ASSERT(pMaterial && pScene);
	if(!pMaterial || !pScene)
		return false;

	return true;
}



// LEGACY, we assign material per polygon now, see AssignMaterialToPoly
bool CXsiFtkMaterialUtil::AssignMaterialToModel(CSLMaterialLibrary* pMaterialLibrary, CSLModel* pModel, const std::string& materialName, const CSLGlobalMaterial::EPropagationType& propagType )
{
	OBJ_ASSERT(pMaterialLibrary && pModel);
	if(!pMaterialLibrary || !pModel)
		return false;

	std::string defaultLibMateralName("DefaultLib.");
	defaultLibMateralName += materialName;

	// Find material
	CSLBaseMaterial* pBaseMaterial = pMaterialLibrary->FindMaterial((char*)defaultLibMateralName.c_str());
	OBJ_ASSERT(pBaseMaterial);
	if(!pBaseMaterial)
		return false;

	// Make sure we have a GlobalMaterial.
	if(!pModel->GlobalMaterial())
	{
		pModel->AddGlobalMaterial();
	}

	// TODO : not sure about those
	//propagType = CSLGlobalMaterial::SI_NODE,SI_BRANCH,SI_INHERITED

	pModel->GlobalMaterial()->SetPropagationType(propagType); 
	pModel->GlobalMaterial()->SetMaterial(pBaseMaterial);

	return true;
}



bool CXsiFtkMaterialUtil::AssignMaterialToPoly(CSLMaterialLibrary* pMaterialLibrary, CSLModel* pModel, const std::string& materialName, const CSLGlobalMaterial::EPropagationType& propagType)
{
	

	return true;
}









// LOAD XSI


bool CXsiFtkMaterialUtil::DotXSILoadMaterialLibrary(CObjFile* pObjFile, CSLScene* pScene)
{
	CSLMaterialLibrary* pMatLib = pScene->GetMaterialLibrary();

	OBJ_ASSERT(pMatLib);
	if (!pMatLib)
		return false;

	MSG_DEBUG(pMatLib->GetMaterialCount() <<" materials");

	CSLBaseMaterial** ppMaterials = pMatLib->GetMaterialList();
	for (SI_Int i=0;i<pMatLib->GetMaterialCount();i++)
	{
		// TODO : tolerate the global materials ?

		if(ppMaterials[i]->Type() == CSLTemplate::XSI_MATERIAL) // CSLXSIMaterial
		{
			CSLXSIMaterial* pXsiMaterial = static_cast<CSLXSIMaterial*>(ppMaterials[i]);

			OBJ_ASSERT(pXsiMaterial);
			if(!pXsiMaterial)
				continue;

			std::string newMaterialName( pXsiMaterial->GetName() );

			CMtlFile::CMaterial newMaterial(newMaterialName);

			uint shaderCount = pXsiMaterial->GetShaderCount();
			for(uint shader = 0; shader < shaderCount; ++shader)
			{
				CSLXSIShader* pShader = pXsiMaterial->GetShaderList()[shader];

				if(!pShader)
					continue;

				CSLShaderConnectionPoint* pImageConnection = GetConnectionPoint(pShader, "tex"); // Get connection to the image

				if(!pImageConnection)
					continue;

				std::string sImageName(pImageConnection->GetImage()); // Retreive absolute path

				CSLImageLibrary* pImageLibrary = pScene->GetImageLibrary();
				OBJ_ASSERT(pImageLibrary);
				if(pImageLibrary)
				{
					CSLImage* pXSIImage = pImageLibrary->FindImage((char*)sImageName.c_str());
					OBJ_ASSERT(pXSIImage);
					if(pXSIImage) // If the texture doesnt already exist
					{
						std::string sTexturePath(pXSIImage->GetSourceFile()); // Get the texture path
						newMaterial.SetTexMap(sTexturePath); // set the texture path
					}
				}
			}

			// Add new material to the material library
			uint materialID = pObjFile->AddMaterial(newMaterial);

		}
		else // CSLMaterial
		{
			CSLMaterial* pMaterial = static_cast<CSLMaterial*>(ppMaterials[i]);

			OBJ_ASSERT(pMaterial);
			if(!pMaterial)
				continue;

			std::string newMaterialName( pMaterial->GetName() );

			CMtlFile::CMaterial newMaterial(newMaterialName);

			// Ambient
			Vector3D ambient = ConvertCSIBCColorf(pMaterial->GetAmbientColor());
			newMaterial.SetKa(ambient);

			// Diffuse
			Vector3D diffuse = ConvertCSIBCColorf(pMaterial->GetDiffuseColor());
			newMaterial.SetKd(diffuse);

			// Specular
			Vector3D specular = ConvertCSIBCColorf(pMaterial->GetSpecularColor());
			newMaterial.SetKs(specular);

			// Emissive
			//...

			// Texture Map
			CSLTexture2D* pTexture2D = pMaterial->Texture2D();
			if(pTexture2D)
			{
				std::string textureFileName(pTexture2D->GetImageFileName());
				newMaterial.SetTexMap(textureFileName); // Add the texture map fileName
			}

			// Add new material to the material library
			uint materialID = pObjFile->AddMaterial(newMaterial);
		}

	}	// For each material in the library

	return true;
}


CSLShaderConnectionPoint* CXsiFtkMaterialUtil::GetConnectionPoint(CSLXSIShader* pShader, char* pName)
{
	int conCount = pShader->GetConnectionPointCount();
	for(int conn = 0; conn < conCount; ++conn)
	{
		if(strcmp(pName, pShader->GetConnectionPointList()[conn]->GetName()) == 0)
		{
			return pShader->GetConnectionPointList()[conn];
		}
	}

	return NULL;
}





Vector3D CXsiFtkMaterialUtil::ConvertCSIBCColorf(const CSIBCColorf& color)
{
	Vector3D newColor(3);
	SI_Float r,g,b;
	color.Get(&r, &g, &b);
	newColor[0] = r;
	newColor[1] = g;
	newColor[2] = b;
	return newColor;
}








