
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XSIFTKUTIL_H__
#define __XSIFTKUTIL_H__


#define FTK_DEGTORAD(x) ((x)*0.017453292519f)
#define FTK_RADTODEG(x) ((x)*57.2957795130f)
#define FTK_EPS 0.000001f
#define FTK_PI 3.14159265f

namespace ZBPlugin
{

// CSIBCVector3DIndexed:
class CSIBCVector3DIndexed : public CSIBCVector3D
{
public:
	CSIBCVector3DIndexed() {};
	~CSIBCVector3DIndexed() {};
	int index;
};

// CSIBCVector2DIndexed:
class CSIBCVector2DIndexed : public CSIBCVector2D
{
public:
	CSIBCVector2DIndexed() {};
	~CSIBCVector2DIndexed() {};
	int index;
};

// CSIBCColorfIndexed:
class CSIBCColorfIndexed : public CSIBCColorf
{
public:
	CSIBCColorfIndexed() {};
	~CSIBCColorfIndexed() {};
	int index;
};


// CXsiFtkUtil: XSI FTK utilities
class CXsiFtkUtil
{
public:
	//CXsiFtkUtil(){}
	//~CXsiFtkUtil(){}

	// hierarchy/transform 
	static CSLModel* Reparent(CSLModel* in_pChildModel, CSLModel* in_pNewRootModel);
	static void	UpdateTransform(CSLModel* in_pModel);

	// Shader helpers
	static CSLVariantParameter*	SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, float in_fValue);
	static CSLVariantParameter*	SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, int in_lValue);
	static CSLVariantParameter*	SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, LONG in_lValue);
	static CSLVariantParameter*	SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, bool in_bValue);
	static CSLVariantParameter*	SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, char* in_pValue);

	static float GetShaderParameterFloatValue(CSLXSIShader* in_pShader, char* in_pName);
	static int	 GetShaderParameterIntValue(CSLXSIShader* in_pShader, char* in_pName);
	static long	 GetShaderParameterLongValue(CSLXSIShader* in_pShader, char* in_pName);
	static bool	 GetShaderParameterBoolValue(CSLXSIShader* in_pShader, char* in_pName);
	static char* GetShaderParameterStringValue(CSLXSIShader* in_pShader, char* in_pName);

	static CSLConnectionPoint*		 ConnectShader(CSLXSIShader* in_pSource, CSLXSIMaterial* in_pDestination, char* in_pConnection);
	static CSLShaderConnectionPoint* ConnectShader(CSLXSIShader* in_pSource, CSLXSIShader* in_pDestination, char* in_pConnection);
	static CSLShaderConnectionPoint* ConnectImage(CSLImage* in_pSource, CSLXSIShader* in_pDestination, char* in_pConnection);

	static void	GetUsedTexCoord(CSLModel* in_pModel, CSLXSIMaterial* in_pMaterial, int* out_pNbTexCoord, int** out_pTexCoordIndices, bool **out_pTexCoordRepeated, bool in_bAll);
	static void	SetTextureSpaceID(CSLModel* in_pModel, CSLXSIMaterial* in_pMaterial, char* in_pTextureProjectionName, int in_nShaderIndex);
	
	static CSLShaderConnectionPoint* GetConnection(CSLXSIShader* in_pShader, char* in_pName);	
	static CSLXSIShader* GetPreviousRTS(CSLXSIShader* in_pShader);



	// Mesh compression
	static void	CompressMeshData(CSLMesh* in_pMesh);
	static void	CompressMeshNormals(CSLMesh* in_pMesh);
	static void	CompressMeshTexCoords(CSLMesh* in_pMesh);
	static void	CompressMeshColors(CSLMesh* in_pMesh);


	// Math
	static float RollFromUpVector(CSIBCVector3D& in_Position, CSIBCVector3D& in_Interest, CSIBCVector3D& in_UpVector, int in_UpAxis);
	static void	 UpVectorFromRoll(CSIBCVector3D& in_Position, CSIBCVector3D& in_Interest, float in_Roll, CSIBCVector3D& out_UpVector);

	// CustomPSet helper
	static CSLCustomPSet* FindCustomPSet(CSLTemplate* in_pOwner, char* in_pCustomPSetName);


	static std::string ReplaceDotWithUnderscore(const std::string& inString);


private:
	static int CompareCSIBCColorf(const void *elem1, const void *elem2 );
	static int CompareCSIBCVector2D(const void* elem1, const void* elem2 );
	static int CompareCSIBCVector3D(const void* elem1, const void* elem2 );

};

} // End ZBPlugin namespace

#endif // __XSIFTKUTIL_H__