
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "SemanticLayer.h"

#include "xsiFtkUtil.h"
#include "xsiFtkGeometryUtil.h"


// Transform the Mesh/Shape vertex position by the Model's transform.
Vector3D CXsiFtkGeometryUtil::TransformAndConvertVector(CSLTransform* pTransform, CSIBCVector3D* pVector3D)
{
	Vector3D vertPos(3);

	OBJ_ASSERT(pTransform);
	OBJ_ASSERT(pVector3D);

	if(!pTransform || !pVector3D)
		return vertPos; // failed

	CSIBCMatrix4x4 transMatrix4 = pTransform->GetMatrix();
	CSIBCVector4D vector4Dtrans = transMatrix4.Multiply(*pVector3D);

	SI_Float Xf, Yf, Zf, Wf;
	vector4Dtrans.Get(&Xf, &Yf, &Zf, &Wf);

	vertPos[0] = Xf;
	vertPos[1] = Yf; 
	vertPos[2] = Zf;

	return vertPos;
}



bool CXsiFtkGeometryUtil::SetModelDefaultAttributes(CSLModel* pModel, SI_Char* szName)
{
	CSIBCVector3D vDefault(1.0, 1.0, 1.0);

	pModel->SetName(szName);
	pModel->Visibility()->SetVisibility(SI_TRUE);
	pModel->Transform()->SetScale( vDefault );

	// Custom pSet : Mesh_Subdivision_Info
	AddSubdivitionPSet(pModel);

	return true;
}


// Add CustomPSet : not really generic
CSLCustomPSet* CXsiFtkGeometryUtil::AddCustomPSet( CSLTemplate* io_pTemplate )
{
	CSLCustomPSet *l_pNewPSet = io_pTemplate->AddCustomPSet();
	l_pNewPSet->SetName("MyCustomPSet");

	// add a parameter
	CSLVariantParameter *l_pNewParameter = l_pNewPSet->AddParameter();
	l_pNewParameter->SetName("MyParameter");

	SI_TinyVariant l_Variant =
	{
		SI_VT_PCHAR,
			SI_TT_NONE,
			1
	};
	l_Variant.p_cVal = "MyVariantValue";

	l_pNewParameter->SetValue(&l_Variant);

	// add a second parameter
	CSLVariantParameter *l_pNewParameter2 = l_pNewPSet->AddParameter();
	l_pNewParameter2->SetName("MyParameter2");

	SI_TinyVariant l_Variant2 =
	{
		SI_VT_DOUBLE,
			SI_TT_NONE,
			1
	};
	l_Variant2.dVal = 0.12345;

	l_pNewParameter2->SetValue(&l_Variant2);

	CSLCustomPSet *l_pNewPSet2 = io_pTemplate->AddCustomPSet();
	l_pNewPSet2->SetName("MyCustomPSet2");

	// add custom parameter info
	CSLXSICustomParamInfo* l_pInfo = l_pNewPSet->CreateCustomParamInfo(1);

	SI_TinyVariant l_Range =
	{
		SI_VT_DOUBLE,
			SI_TT_NONE,
			1
	};

	l_Range.dVal = 10.0;
	l_pInfo->SetMaxValue(l_Range);
	l_Range.dVal = 0.0;
	l_pInfo->SetMinValue(l_Range);
	return l_pNewPSet;
}

bool CXsiFtkGeometryUtil::AddSubdivitionPSet(CSLModel* pModel)
{
	// TODO
	return true;
}






