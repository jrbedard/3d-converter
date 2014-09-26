
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XSIFTKMATERIALUTIL_H__
#define __XSIFTKMATERIALUTIL_H__

#include "mtlFile.h"

namespace ZBPlugin
{

class ::CObjFile;
class ::CSLModel;
class ::CSLScene;

// CXsiFtkMaterialUtil: Material utility class
class CXsiFtkMaterialUtil
{
public:
	// Create XSI
	static CSLMaterialLibrary* CreateMaterialLibrary(CObjFile* pObjFile, CSLScene* pScene, bool bTranslateMaps);

	static bool AssignMaterialToModel(CSLMaterialLibrary* pMaterialLibrary, CSLModel* pModel, const std::string& materialName, const CSLGlobalMaterial::EPropagationType& propagType);
	static bool AssignMaterialToPoly(CSLMaterialLibrary* pMaterialLibrary, CSLModel* pModel, const std::string& materialName, const CSLGlobalMaterial::EPropagationType& propagType);

	// Load XSI
	static bool DotXSILoadMaterialLibrary(CObjFile* pObjFile, CSLScene* pScene);


private:
	// TODO : make a low level lib with below, like int colladaExportImport.vcproj
	static CSLShaderConnectionPoint* GetConnectionPoint(CSLXSIShader* pShader, char* pName);

	static bool AddSLMaterial(CMtlFile::CMaterial* pMaterial, CSLMaterialLibrary* pMaterialLibrary, bool bTranslateMaps);
	static bool AddXSIMaterial(CMtlFile::CMaterial* pMaterial, CSLScene* pScene, bool bTranslateMaps);

	static CSLXSIShader* AddImage(CSLXSIMaterial* pXSIMaterial, const std::string& textureFileName, CSLScene* pScene);
	static CSLXSIShader* AddDisplacementMap(CSLXSIMaterial* pXSIMaterial, const std::string& dispFileName, double alphaDepthFactor, CSLScene* pScene);

	static bool CreateImageLibrary(CMtlFile::CMaterial* pMaterial, CSLScene* pScene);


	static Vector3D ConvertCSIBCColorf(const CSIBCColorf& color);
};

} // End ZBPlugin namespace

#endif // __XSIFTKMATERIALUTIL_H__

