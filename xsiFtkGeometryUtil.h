
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XSIFTKGEOMETRYUTIL_H__
#define __XSIFTKGEOMETRYUTIL_H__


namespace ZBPlugin
{

class CObjFile;
class ::CSLModel;
class ::CSLScene;

// CXsiFtkGeometryUtil: Material utility class
class CXsiFtkGeometryUtil
{
public:
	static Vector3D TransformAndConvertVector(CSLTransform* pTransform, CSIBCVector3D* pVector3D);

	static bool SetModelDefaultAttributes(CSLModel* pModel, SI_Char* szName);

	static CSLCustomPSet* AddCustomPSet(CSLTemplate* io_pTemplate);
	static bool AddSubdivitionPSet(CSLModel* pModel);


private:

};

} // End ZBPlugin namespace

#endif // __XSIFTKGEOMETRYUTIL_H__