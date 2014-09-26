
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "fbxSdkUtil.h"
#include "fbxFile.h"


bool CFbxSdkUtil::GetSceneInfo(KFbxScene* pScene)
{
	OBJ_ASSERT(pScene);
	if(!pScene)
		return false;

	KFbxSceneInfo* pSceneInfo = pScene->GetSceneInfo();
	if(pSceneInfo)
	{
		MSG_DEBUG("Meta-Data:");
		MSG_DEBUG("Title: " << pSceneInfo->mTitle.GetBuffer());
		MSG_DEBUG("Subject: " << pSceneInfo->mSubject.GetBuffer());
		MSG_DEBUG("Author: " << pSceneInfo->mAuthor.GetBuffer());
		MSG_DEBUG("Keywords: " << pSceneInfo->mKeywords.GetBuffer());
		MSG_DEBUG("Revision: " << pSceneInfo->mRevision.GetBuffer());
		MSG_DEBUG("Comment: " << pSceneInfo->mComment.GetBuffer());
	}

	return true;
}

KFbxMatrix CFbxSdkUtil::GetGeometricTransform(KFbxNode* pNode)
{
	OBJ_ASSERT(pNode);
	if(!pNode)
		return KFbxMatrix();

	KFbxMatrix transMat = pNode->GetGlobalMatrix(KFbxNode::eDESTINATION_SET);
	return transMat;
}

Vector3D CFbxSdkUtil::TransformAndConvertKFbxVector(KFbxVector4& vec, KFbxMatrix& transMat)
{
	vec[3] = 1.0f; // allow transformations
	Vector3D vector(3);

	// Note: Cant use KFbxVector4::DotProduct because its doesnt considerate W. So we do our own dot product

	KFbxVector4 transCol0 = transMat.GetColumn(0);
	KFbxVector4 transCol1 = transMat.GetColumn(1);
	KFbxVector4 transCol2 = transMat.GetColumn(2);

	vector[0] = (transCol0[0] * vec[0]) + (transCol0[1] * vec[1]) + (transCol0[2] * vec[2]) + (transCol0[3] * vec[3]); 
	vector[1] = (transCol1[0] * vec[0]) + (transCol1[1] * vec[1]) + (transCol1[2] * vec[2]) + (transCol1[3] * vec[3]); 
	vector[2] = (transCol2[0] * vec[0]) + (transCol2[1] * vec[1]) + (transCol2[2] * vec[2]) + (transCol2[3] * vec[3]); 
	return vector;
}




