
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __FBXSDKUTIL_H__
#define __FBXSDKUTIL_H__

#include "fbxSdk.h" // FBX SDK

using namespace FBXSDK_NAMESPACE;

namespace ZBPlugin
{

// CFbxSdkUtil: FBX SDK general utilities
class CFbxSdkUtil
{
public:
	// scene
	static bool GetSceneInfo(KFbxScene* pScene);

	// transform
	static KFbxMatrix GetGeometricTransform(KFbxNode* pNode);

	// vector
	static inline Vector3D ConvertKFbxVector(KFbxVector2& vec);
	static inline Vector3D ConvertKFbxVector(KFbxVector4& vec);
	static Vector3D TransformAndConvertKFbxVector(KFbxVector4& vec, KFbxMatrix& transMat);
	static inline KFbxVector4 ConvertVector3D(Vector3D& vec);

	// color
	static inline Vector3D  ConvertKFbxColor(KFbxColor& color);
	static inline KFbxColor ConvertColor3(Vector3D& color);
};



inline Vector3D CFbxSdkUtil::ConvertKFbxVector(KFbxVector2& vec)
{
	Vector3D newVector(3);
	newVector[0] = vec[0];
	newVector[1] = vec[1];
	newVector[2] = 0.0f;
	return newVector;
}



Vector3D CFbxSdkUtil::ConvertKFbxVector(KFbxVector4& vec)
{
	Vector3D newVector(3);
	newVector[0] = vec[0];
	newVector[1] = vec[1];
	newVector[2] = vec[2];
	return newVector;
}


inline Vector3D CFbxSdkUtil::ConvertKFbxColor(KFbxColor& color)
{
	Vector3D newColor(3);
	newColor[0] = color.mRed;
	newColor[1] = color.mGreen;
	newColor[2] = color.mBlue;
	return newColor;
}

inline KFbxColor CFbxSdkUtil::ConvertColor3(Vector3D& color)
{
	KFbxColor newColor(color[0], color[1], color[2]);
	return newColor;
}

inline KFbxVector4 CFbxSdkUtil::ConvertVector3D(Vector3D& vec)
{
	KFbxVector4 newVector(vec[0], vec[1], vec[2]);
	return newVector;
}

} // End ZBPlugin namespace


#endif // __FBXSDKUTIL_H__