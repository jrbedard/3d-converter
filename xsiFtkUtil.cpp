
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "SemanticLayer.h"

#include "TriangleList.h"
#include "TriangleStrip.h"
#include "TriangleStripList.h"
#include "Shape_35.h"
#include "Mesh.h"
#include "UVCoordArray.h"
#include "primitive.h"
#include "model.h"
#include "transform.h"
#include "XSIShader.h"
#include "XSIImage.h"
#include "MaterialLibrary.h"
#include "XSIMaterial.h"
#include "ShaderInstanceData.h"
#include "CustomPSet.h"

#include "xsiFtkUtil.h"


CSLModel* CXsiFtkUtil::Reparent(CSLModel* in_pChildModel, CSLModel* in_pNewRootModel)
{
	CSLScene *l_pScene = in_pChildModel->Scene();
	if(in_pChildModel->Template())
	{
		CdotXSITemplate* l_pParentTemplate = in_pChildModel->Template()->Parent();
		int loop, count = l_pParentTemplate->Children().GetCount();
		for(loop = 0; loop < count; loop++)
		{
			if(l_pParentTemplate->Children().Item(loop) == in_pChildModel->Template())
			{
				l_pParentTemplate->Children().Remove(loop);
				if(in_pNewRootModel->Template())
				{
					in_pNewRootModel->Template()->Children().Add(in_pChildModel->Template());
					in_pChildModel->Template()->SetParent(in_pNewRootModel->Template());
				}
				else
				{
					l_pScene->Parser()->dotXSITemplate()->Add(in_pChildModel->Template());
				}

				return in_pChildModel;
				break;
			}
		}
	}
	else
	{
		CdotXSITemplate* l_pParentTemplate = in_pChildModel->Primitive()->Template()->Parent();
		int loop, count = l_pParentTemplate->Children().GetCount();
		for(loop = 0; loop < count; loop++)
		{
			if(l_pParentTemplate->Children().Item(loop) == in_pChildModel->Primitive()->Template())
			{
				l_pParentTemplate->Children().Remove(loop);
				if(in_pNewRootModel->Template())
				{
					in_pNewRootModel->Template()->Children().Add(in_pChildModel->Primitive()->Template());
					in_pChildModel->Primitive()->Template()->SetParent(in_pNewRootModel->Template());
				}
				else
				{
					l_pScene->Parser()->dotXSITemplate()->Add(in_pChildModel->Primitive()->Template());
				}

				return in_pChildModel;
				break;
			}
		}
	}
	return NULL;
}

void CXsiFtkUtil::UpdateTransform(CSLModel* in_pModel)
{
	CSIBCVector3D	scale, rotate, translate;

	in_pModel->Transform()->GetMatrix().GetTransforms(scale, rotate, translate);

	rotate.m_fX = FTK_RADTODEG(rotate.m_fX);
	rotate.m_fY = FTK_RADTODEG(rotate.m_fY);
	rotate.m_fZ = FTK_RADTODEG(rotate.m_fZ);

	in_pModel->Transform()->SetScale(scale);
	in_pModel->Transform()->SetEulerRotation(rotate);
	in_pModel->Transform()->SetTranslation(translate);
	in_pModel->Transform()->ComputeLocalMatrix();
}

CSLVariantParameter* CXsiFtkUtil::SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, float in_fValue)
{
	CSLVariantParameter* l_pParameter = in_pShader->AddParameter();
	SI_TinyVariant	l_Value;

	l_pParameter->SetName(in_pName);
	l_Value.variantType = SI_VT_FLOAT;
	l_Value.fVal = in_fValue;
	l_pParameter->SetValue(&l_Value);

	return l_pParameter;
}

CSLVariantParameter* CXsiFtkUtil::SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, int in_lValue)
{
	CSLVariantParameter* l_pParameter = in_pShader->AddParameter();
	SI_TinyVariant	l_Value;

	l_pParameter->SetName(in_pName);
	l_Value.variantType = SI_VT_INT;
	l_Value.nVal = in_lValue;
	l_pParameter->SetValue(&l_Value);

	return l_pParameter;
}

CSLVariantParameter* CXsiFtkUtil::SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, LONG in_lValue)
{
	CSLVariantParameter* l_pParameter = in_pShader->AddParameter();
	SI_TinyVariant	l_Value;

	l_pParameter->SetName(in_pName);
	l_Value.variantType = SI_VT_LONG;
	l_Value.lVal = in_lValue;
	l_pParameter->SetValue(&l_Value);

	return l_pParameter;
}

CSLVariantParameter* CXsiFtkUtil::SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, bool in_bValue)
{
	CSLVariantParameter* l_pParameter = in_pShader->AddParameter();
	SI_TinyVariant	l_Value;

	l_pParameter->SetName(in_pName);
	l_Value.variantType = SI_VT_BOOL;
	l_Value.bVal = in_bValue;
	l_pParameter->SetValue(&l_Value);

	return l_pParameter;
}

CSLVariantParameter* CXsiFtkUtil::SetShaderParameterValue(CSLXSIShader* in_pShader, char* in_pName, char* in_pValue)
{
	CSLVariantParameter* l_pParameter = in_pShader->AddParameter();
	SI_TinyVariant	l_Value;

	l_pParameter->SetName(in_pName);
	l_Value.variantType = SI_VT_PCHAR;
	l_Value.p_cVal = in_pValue;
	l_pParameter->SetValue(&l_Value);

	return l_pParameter;
}

float CXsiFtkUtil::GetShaderParameterFloatValue(CSLXSIShader* in_pShader, char* in_pName)
{
	int loop;
	for(loop = 0; loop < in_pShader->GetParameterCount(); loop ++)
	{
		if(strcmp(in_pShader->GetParameterList()[loop]->GetName(), in_pName) == 0)
		{
			if(in_pShader->GetParameterList()[loop]->GetValue()->variantType == SI_VT_FLOAT)
			{
				return in_pShader->GetParameterList()[loop]->GetValue()->fVal;
			}
			else
			{
				return (float) in_pShader->GetParameterList()[loop]->GetValue()->dVal;				
			}
			break;
		}
	}
	return 0;
}

int	CXsiFtkUtil::GetShaderParameterIntValue(CSLXSIShader* in_pShader, char* in_pName)
{
	int loop;
	for(loop = 0; loop < in_pShader->GetParameterCount(); loop ++)
	{
		if(strcmp(in_pShader->GetParameterList()[loop]->GetName(), in_pName) == 0)
		{
			return in_pShader->GetParameterList()[loop]->GetValue()->nVal;
			break;
		}
	}
	return 0;
}

long CXsiFtkUtil::GetShaderParameterLongValue(CSLXSIShader* in_pShader, char* in_pName)
{
	int loop;
	for(loop = 0; loop < in_pShader->GetParameterCount(); loop ++)
	{
		if(strcmp(in_pShader->GetParameterList()[loop]->GetName(), in_pName) == 0)
		{
			return in_pShader->GetParameterList()[loop]->GetValue()->lVal;
			break;
		}
	}
	return 0;
}
bool CXsiFtkUtil::GetShaderParameterBoolValue(CSLXSIShader* in_pShader, char* in_pName)
{
	int loop;
	for(loop = 0; loop < in_pShader->GetParameterCount(); loop ++)
	{
		if(strcmp(in_pShader->GetParameterList()[loop]->GetName(), in_pName) == 0)
		{
			return in_pShader->GetParameterList()[loop]->GetValue()->boolVal ? true : false;
			break;
		}
	}
	return 0;
}
char* CXsiFtkUtil::GetShaderParameterStringValue(CSLXSIShader* in_pShader, char* in_pName)
{
	int loop;
	for(loop = 0; loop < in_pShader->GetParameterCount(); loop ++)
	{
		if(strcmp(in_pShader->GetParameterList()[loop]->GetName(), in_pName) == 0)
		{
			return in_pShader->GetParameterList()[loop]->GetValue()->p_cVal;
			break;
		}
	}
	return 0;
}

CSLConnectionPoint* CXsiFtkUtil::ConnectShader(CSLXSIShader* in_pSource, CSLXSIMaterial* in_pDestination, char* in_pConnection)
{
	CSLConnectionPoint* l_pConnection = NULL;

	OBJ_ASSERT(in_pSource && in_pDestination && strlen(in_pConnection) > 0);
	if(!in_pSource || !in_pDestination ||  strlen(in_pConnection) == 0)
		return l_pConnection;

	l_pConnection = in_pDestination->AddConnectionPoint();
	l_pConnection->SetName(in_pConnection);
	l_pConnection->SetShader(in_pSource);

	return l_pConnection;
}


CSLShaderConnectionPoint* CXsiFtkUtil::ConnectShader(CSLXSIShader* in_pSource, CSLXSIShader* in_pDestination, char* in_pConnection)
{
	CSLShaderConnectionPoint* l_pConnection = NULL;

	OBJ_ASSERT(in_pSource && in_pDestination && strlen(in_pConnection) > 0);
	if(!in_pSource || !in_pDestination ||  strlen(in_pConnection) == 0)
		return l_pConnection;

	l_pConnection = in_pDestination->AddConnectionPoint();
	l_pConnection->SetName(in_pConnection);
	l_pConnection->SetShader(in_pSource);

	SI_TinyVariant l_Value;
	l_Value.variantType = SI_VT_PCHAR;
	l_Value.p_cVal = "SHADER";
	in_pDestination->Template()->Params().Item(in_pDestination->Template()->Params().GetCount() - 1)->SetValue(l_Value);

	return l_pConnection;
}

CSLShaderConnectionPoint* CXsiFtkUtil::ConnectImage(CSLImage* in_pSource, CSLXSIShader* in_pDestination, char* in_pConnection)
{
	CSLShaderConnectionPoint* l_pConnection = NULL;

	OBJ_ASSERT(in_pSource && in_pDestination && strlen(in_pConnection) > 0);
	if(!in_pSource || !in_pDestination ||  strlen(in_pConnection) == 0)
		return l_pConnection;

	l_pConnection = in_pDestination->AddConnectionPoint();
	l_pConnection->SetName(in_pConnection);
	l_pConnection->SetImage(in_pSource->Name().GetText());

	SI_TinyVariant l_Value;
	l_Value.variantType = SI_VT_PCHAR;
	l_Value.p_cVal = "IMAGE";
	in_pDestination->Template()->Params().Item(in_pDestination->Template()->Params().GetCount() - 1)->SetValue(l_Value);

	return l_pConnection;

}

void CXsiFtkUtil::GetUsedTexCoord(CSLModel* in_pModel, CSLXSIMaterial* in_pMaterial, int* out_pNbTexCoord, int** out_pTexCoordIndices, bool** out_pTexCoordRepeated, bool in_bAll)
{
	int loop, count;

	int* l_pTexCoordIndices = NULL;
	bool* l_pTexCoordRepeated = NULL;
	int l_NbTexCoord = 0;

	if(in_bAll)
	{
		CSLShape_35* l_pShape = (CSLShape_35*) ((CSLMesh*)in_pModel->Primitive())->Shape();
		count = l_pShape->GetUVCoordArrayCount();

		l_NbTexCoord = count;

		if(l_NbTexCoord)
		{
			l_pTexCoordIndices = (int*) calloc(sizeof(int), l_NbTexCoord);
			l_pTexCoordRepeated = (bool*) calloc(sizeof(bool), l_NbTexCoord);

			for(loop = 0; loop < count; loop++)
			{
				l_pTexCoordIndices[loop] = loop;
				l_pTexCoordRepeated[loop] = false;
			}
		}
	}
	else
	{
		count = in_pMaterial->GetShaderCount();
		for(loop = 0; loop < count ; loop++)
		{
			CSLXSIShader* l_pShader = in_pMaterial->GetShaderList()[loop];
			int loop2, count2 = l_pShader->GetInstanceDataCount();
			bool l_bFound = FALSE;

			for(loop2 = 0; loop2 < count2; loop2++)
			{
				CSLShaderInstanceData* l_pInstanceData = l_pShader->GetInstanceDataList()[loop2];
				if(l_pInstanceData->GetReference() == in_pModel)
				{
					int loop3, count3 = l_pInstanceData->GetParameterCount();
					for(loop3 = 0; loop3 < count3; loop3++)
					{
						CSLVariantParameter* l_pParameter = l_pInstanceData->GetParameterList()[loop3];
						if(strstr(l_pParameter->GetName(), "tspace"))
						{
							char* l_pTextureProjectionName = l_pParameter->GetValue()->p_cVal;
							CSLShape_35* l_pShape = (CSLShape_35*) ((CSLMesh*) in_pModel->Primitive())->Shape();
							int loop4, count4 = l_pShape->GetUVCoordArrayCount();
							for(loop4 = 0; loop4 < count4; loop4++)
							{
								CSLUVCoordArray* l_pTexCoord = l_pShape->UVCoordArrays()[loop4];
								if(strcmp(l_pTexCoord->GetTextureProjection(), l_pTextureProjectionName) == 0)
								{
									l_NbTexCoord ++;
									l_pTexCoordIndices = (int *) realloc(l_pTexCoordIndices, sizeof(int) * l_NbTexCoord);
									l_pTexCoordIndices[l_NbTexCoord-1] = loop4;
									l_bFound = TRUE;
									break;
								}
							}
						}
					}
				}
			}

			if(!l_bFound)
			{
				int loop3, count3 = l_pShader->GetParameterCount();
				for(loop3 = 0; loop3 < count3; loop3++)
				{
					CSLVariantParameter* l_pParameter = l_pShader->GetParameterList()[loop3];
					if(strstr(l_pParameter->GetName(), "tspace"))
					{
						char* l_pTextureProjectionName = l_pParameter->GetValue()->p_cVal;
						CSLShape_35* l_pShape = (CSLShape_35*) ((CSLMesh*) in_pModel->Primitive())->Shape();
						int loop4, count4 = l_pShape->GetUVCoordArrayCount();
						for(loop4 = 0; loop4 < count4; loop4++)
						{
							CSLUVCoordArray* l_pTexCoord = l_pShape->UVCoordArrays()[loop4];
							if(strcmp(l_pTexCoord->GetTextureProjection(), l_pTextureProjectionName) == 0)
							{
								l_NbTexCoord ++;
								l_pTexCoordIndices = (int*) realloc(l_pTexCoordIndices, sizeof(int) * l_NbTexCoord);
								l_pTexCoordIndices[l_NbTexCoord-1] = loop4;
								break;
							}
							else if(strlen(l_pTextureProjectionName) == 0)
							{
								l_NbTexCoord ++;
								l_pTexCoordIndices = (int*) realloc(l_pTexCoordIndices, sizeof(int) * l_NbTexCoord);
								l_pTexCoordIndices[l_NbTexCoord-1] = loop4;
								break;
							}
						}
					}
				}
			}
		}

		// compute the repeated flags
		if(l_NbTexCoord)
		{
			l_pTexCoordRepeated = (bool*) calloc(sizeof(bool), l_NbTexCoord);
			for(loop = 0; loop < l_NbTexCoord; loop++)
			{
				int loop2; 

				if(loop > 0)
				{
					for(loop2 = 0; loop2 < loop; loop2++)
					{
						if(l_pTexCoordIndices[loop2] == l_pTexCoordIndices[loop])
						{
							l_pTexCoordRepeated[loop] = TRUE;
							break;
						}
					}
				}
			}
		}
	}

	*out_pTexCoordIndices = l_pTexCoordIndices;
	*out_pTexCoordRepeated = l_pTexCoordRepeated;
	*out_pNbTexCoord = l_NbTexCoord;

}

void CXsiFtkUtil::SetTextureSpaceID(CSLModel* in_pModel, CSLXSIMaterial* in_pMaterial, char* in_pTextureProjectionName, int in_nShaderIndex)
{
	int loop, count = in_pMaterial->GetShaderCount(), index = 0;
	for(loop = 0; loop < count; loop++)
	{
		CSLXSIShader* l_pShader = in_pMaterial->GetShaderList()[loop];

		int loop2, count2 = l_pShader->GetParameterCount();
		for(loop2 = 0; loop2 < count2; loop2++)
		{
			CSLVariantParameter *l_pParameter = l_pShader->GetParameterList()[loop2];
			if (strcmp(l_pParameter->GetName(), "tspace_id") == 0)
			{
				if (index == in_nShaderIndex)
				{
					CSLShaderInstanceData *l_pInstanceData = l_pShader->AddInstanceData();
					l_pInstanceData->SetReference(in_pModel);
					CSLVariantParameter *l_pInstanceDataParameter = l_pInstanceData->AddParameter();
					l_pInstanceDataParameter->SetName("tspace_id");
					SI_TinyVariant l_Value;
					l_Value.variantType = SI_VT_PCHAR;
					l_Value.p_cVal = &(in_pTextureProjectionName[1]);
					l_pInstanceDataParameter->SetValue(&l_Value);


					SI_TinyVariant *l_pValue = l_pParameter->GetValue();
					if(strcmp(l_pValue->p_cVal, "") == 0)
					{	
						l_pParameter->SetValue(&l_Value);
					}
				}

				index ++;
			}
		}
	}
}


CSLShaderConnectionPoint* CXsiFtkUtil::GetConnection(CSLXSIShader* in_pShader, char* in_pName)
{
	int loop, count = in_pShader->GetConnectionPointCount();
	for(loop = 0; loop < count; loop++)
	{
		if(strcmp(in_pName, in_pShader->GetConnectionPointList()[loop]->GetName()) == 0)
		{	
			return in_pShader->GetConnectionPointList()[loop];
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// utility functions:
//
CSLXSIShader* CXsiFtkUtil::GetPreviousRTS(CSLXSIShader* in_pShader)
{
	for (int loop = 0; loop < in_pShader->GetConnectionPointCount(); loop++)
	{
		if (strcmp(in_pShader->GetConnectionPointList()[loop]->GetName(), "previous") == 0)
			break;
	}

	if (loop >= in_pShader->GetConnectionPointCount())
		return NULL;
	else
		return in_pShader->GetConnectionPointList()[loop]->GetShader();
}


void CXsiFtkUtil::CompressMeshData(CSLMesh* in_pMesh)
{
	CompressMeshNormals(in_pMesh);
	CompressMeshColors(in_pMesh);
	CompressMeshTexCoords(in_pMesh);
}

int CXsiFtkUtil::CompareCSIBCVector3D(const void* elem1, const void* elem2 )
{
	CSIBCVector3DIndexed* l_pVec1 = ((CSIBCVector3DIndexed*) elem1);
	CSIBCVector3DIndexed* l_pVec2 = ((CSIBCVector3DIndexed*) elem2);

	if(l_pVec1->m_fX > l_pVec2->m_fX)
	{
		return 1;
	}
	else if(fabs(l_pVec1->m_fX - l_pVec2->m_fX) < FTK_EPS)
	{
		if(l_pVec1->m_fY > l_pVec2->m_fY)
		{
			return 1;
		}
		else if(fabs(l_pVec1->m_fY - l_pVec2->m_fY) < FTK_EPS)
		{
			if(l_pVec1->m_fZ > l_pVec2->m_fZ)
			{
				return 1;
			}
			else if(fabs(l_pVec1->m_fZ - l_pVec2->m_fZ) < FTK_EPS)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

int CXsiFtkUtil::CompareCSIBCVector2D(const void* elem1, const void* elem2 )
{
	CSIBCVector2DIndexed* l_pVec1 = ((CSIBCVector2DIndexed*) elem1);
	CSIBCVector2DIndexed* l_pVec2 = ((CSIBCVector2DIndexed*) elem2);

	if(l_pVec1->m_fX > l_pVec2->m_fX)
	{
		return 1;
	}
	else if(fabs(l_pVec1->m_fX - l_pVec2->m_fX) < FTK_EPS)
	{
		if(l_pVec1->m_fY > l_pVec2->m_fY)
		{
			return 1;
		}
		else if(fabs(l_pVec1->m_fY - l_pVec2->m_fY) < FTK_EPS)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

int CXsiFtkUtil::CompareCSIBCColorf(const void *elem1, const void *elem2 )
{
	CSIBCColorfIndexed* l_pVec1 = ((CSIBCColorfIndexed*) elem1);
	CSIBCColorfIndexed* l_pVec2 = ((CSIBCColorfIndexed*) elem2);

	if(l_pVec1->m_fR > l_pVec2->m_fR)
	{
		return 1;
	}
	else if(fabs(l_pVec1->m_fR - l_pVec2->m_fR) < FTK_EPS)
	{
		if(l_pVec1->m_fG > l_pVec2->m_fG)
		{
			return 1;
		}
		else if(fabs(l_pVec1->m_fG - l_pVec2->m_fG) < FTK_EPS)
		{
			if(l_pVec1->m_fB > l_pVec2->m_fB)
			{
				return 1;
			}
			else if(fabs(l_pVec1->m_fB - l_pVec2->m_fB) < FTK_EPS)
			{
				if(l_pVec1->m_fA > l_pVec2->m_fA)
				{
					return 1;
				}
				else if(fabs(l_pVec1->m_fA - l_pVec2->m_fA) < FTK_EPS)
				{
					return 0;
				}
				else
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

}

void CXsiFtkUtil::CompressMeshNormals(CSLMesh* in_pMesh)
{
	CSIBCArray<CSIBCVector3DIndexed>	l_UncompressedVectorArray;
	CSIBCArray<int>						l_IndexArray;

	// compress the normals
	int count, loop, loop2, loop3;

	count = in_pMesh->Shape()->GetNormalCount();

	l_UncompressedVectorArray.Extend(count);
	for(loop = 0; loop < count; loop++)
	{
		l_UncompressedVectorArray[loop].Set
			(
			in_pMesh->Shape()->GetNormalListPtr()[loop].GetX(),
			in_pMesh->Shape()->GetNormalListPtr()[loop].GetY(),
			in_pMesh->Shape()->GetNormalListPtr()[loop].GetZ()
			);

		l_UncompressedVectorArray[loop].index = loop;
	}

	qsort(l_UncompressedVectorArray.ArrayPtr(), count, sizeof(CSIBCVector3DIndexed), CompareCSIBCVector3D);

	l_IndexArray.Extend(count);

	// now just fill up the normal array again
	CSLBaseShape::CSLVector3DArray* l_pNormals = in_pMesh->Shape()->GetNormalList();
	l_pNormals->Resize(1);

	int currentIndex = 0;
	int currentCompressedIndex = 0;
	(*l_pNormals)[0].m_fX = l_UncompressedVectorArray[0].m_fX;
	(*l_pNormals)[0].m_fY = l_UncompressedVectorArray[0].m_fY;
	(*l_pNormals)[0].m_fZ = l_UncompressedVectorArray[0].m_fZ;
	l_IndexArray[l_UncompressedVectorArray[0].index] = 0;

	for(loop = 1; loop < count; loop++)
	{
		if(CompareCSIBCVector3D(&l_UncompressedVectorArray[loop], &l_UncompressedVectorArray[currentIndex]) != 0)
		{
			l_pNormals->Extend(1);
			currentCompressedIndex = l_pNormals->GetUsed()-1; 
			(*l_pNormals)[currentCompressedIndex].m_fX = l_UncompressedVectorArray[loop].m_fX;
			(*l_pNormals)[currentCompressedIndex].m_fY = l_UncompressedVectorArray[loop].m_fY;
			(*l_pNormals)[currentCompressedIndex].m_fZ = l_UncompressedVectorArray[loop].m_fZ;
			currentIndex = loop;
		}

		l_IndexArray[l_UncompressedVectorArray[loop].index] = currentCompressedIndex;
	}

	// and reindex the polygons
	count = in_pMesh->GetPolygonListCount();
	for(loop = 0; loop < count; loop++)
	{
		CSLPolygonList::CSLIntArray* l_pNormalIndices = in_pMesh->PolygonLists()[loop]->GetNormalIndices();

		for(loop2 = 0; loop2 < l_pNormalIndices->GetUsed(); loop2++)
		{
			(*l_pNormalIndices)[loop2] = l_IndexArray[(*l_pNormalIndices)[loop2]];
		}
	}

	// and reindex the triangles
	count = in_pMesh->GetTriangleListCount();
	for(loop = 0; loop < count; loop++)
	{
		CSLTriangleList::CSLTriangleArray* l_pNormalIndices = in_pMesh->TriangleLists()[loop]->GetNormalIndices();

		for(loop2 = 0; loop2 < l_pNormalIndices->GetUsed(); loop2++)
		{
			(*l_pNormalIndices)[loop2].m_iVtx1 = l_IndexArray[(*l_pNormalIndices)[loop2].m_iVtx1];
			(*l_pNormalIndices)[loop2].m_iVtx2 = l_IndexArray[(*l_pNormalIndices)[loop2].m_iVtx2];
			(*l_pNormalIndices)[loop2].m_iVtx3 = l_IndexArray[(*l_pNormalIndices)[loop2].m_iVtx3];
		}
	}

	// and reindex the triangle strips
	count = in_pMesh->GetTriangleStripListCount();
	for(loop = 0; loop < count; loop++)
	{
		for(loop2 = 0; loop2 < in_pMesh->TriangleStripLists()[loop]->GetTriangleStripCount(); loop2++)
		{
			CSLTriangleStrip* l_pTriangleStrip = in_pMesh->TriangleStripLists()[loop]->TriangleStrips()[loop2];
			CSLTriangleStrip::CSLIntArray* l_pNormalIndices = l_pTriangleStrip->GetNormalIndices();

			for(loop3 = 0; loop3 < l_pNormalIndices->GetUsed(); loop3++)
			{
				(*l_pNormalIndices)[loop3] = l_IndexArray[(*l_pNormalIndices)[loop3]];
			}
		}
	}
}

void CXsiFtkUtil::CompressMeshTexCoords(CSLMesh* in_pMesh)
{
	CSLShape_35* l_pShape = (CSLShape_35*)in_pMesh->Shape();

	if(l_pShape->UVCoordArrays() == NULL)
		return;

	int TexCoordCount = l_pShape->GetUVCoordArrayCount();
	int TexCoordLoop;

	for(TexCoordLoop = 0; TexCoordLoop < TexCoordCount; TexCoordLoop++)
	{

		// compress the texture coordinates
		CSIBCArray<CSIBCVector2DIndexed>	l_UncompressedVectorArray;
		CSIBCArray<int>						l_IndexArray;

		// compress the normals
		int count, loop, loop2, loop3;

		count = l_pShape->UVCoordArrays()[TexCoordLoop]->GetUVCoordCount();

		l_UncompressedVectorArray.Extend(count);
		for(loop = 0; loop < count; loop++)
		{
			l_UncompressedVectorArray[loop].Set
				(
				l_pShape->UVCoordArrays()[TexCoordLoop]->GetUVCoordListPtr()[loop].GetX(),
				l_pShape->UVCoordArrays()[TexCoordLoop]->GetUVCoordListPtr()[loop].GetY()
				);

			l_UncompressedVectorArray[loop].index = loop;
		}

		qsort(l_UncompressedVectorArray.ArrayPtr(), count, sizeof(CSIBCVector2DIndexed), CompareCSIBCVector2D);

		l_IndexArray.Extend(count);

		// now just fill up the texture coordinate array again
		CSLBaseShape::CSLVector2DArray* l_pTexCoord = l_pShape->UVCoordArrays()[TexCoordLoop]->GetUVCoordList();
		l_pTexCoord->Resize(1);

		int currentIndex = 0;
		int currentCompressedIndex = 0;
		(*l_pTexCoord)[0].m_fX = l_UncompressedVectorArray[0].m_fX;
		(*l_pTexCoord)[0].m_fY = l_UncompressedVectorArray[0].m_fY;
		l_IndexArray[l_UncompressedVectorArray[0].index] = 0;

		for(loop = 1; loop < count; loop++)
		{
			if(CompareCSIBCVector2D(&l_UncompressedVectorArray[loop], &l_UncompressedVectorArray[currentIndex]) != 0)
			{
				l_pTexCoord->Extend(1);
				currentCompressedIndex = l_pTexCoord->GetUsed()-1; 
				(*l_pTexCoord)[currentCompressedIndex].m_fX = l_UncompressedVectorArray[loop].m_fX;
				(*l_pTexCoord)[currentCompressedIndex].m_fY = l_UncompressedVectorArray[loop].m_fY;
				currentIndex = loop;
			}

			l_IndexArray[l_UncompressedVectorArray[loop].index] = currentCompressedIndex;
		}

		// and reindex the polygons
		count = in_pMesh->GetPolygonListCount();
		for(loop = 0; loop < count; loop++)
		{
			CSLPolygonList::CSLIntArray* l_pTexCoordIndices = in_pMesh->PolygonLists()[loop]->GetUVIndices(TexCoordLoop);

			for(loop2 = 0; loop2 < l_pTexCoordIndices->GetUsed(); loop2++)
			{
				(*l_pTexCoordIndices)[loop2] = l_IndexArray[(*l_pTexCoordIndices)[loop2]];
			}
		}

		// and reindex the triangles
		count = in_pMesh->GetTriangleListCount();
		for(loop = 0; loop < count; loop++)
		{
			CSLTriangleList::CSLTriangleArray* l_pTexCoordIndices = in_pMesh->TriangleLists()[loop]->GetUVIndices(TexCoordLoop);

			for(loop2 = 0; loop2 < l_pTexCoordIndices->GetUsed(); loop2++)
			{
				(*l_pTexCoordIndices)[loop2].m_iVtx1 = l_IndexArray[(*l_pTexCoordIndices)[loop2].m_iVtx1];
				(*l_pTexCoordIndices)[loop2].m_iVtx2 = l_IndexArray[(*l_pTexCoordIndices)[loop2].m_iVtx2];
				(*l_pTexCoordIndices)[loop2].m_iVtx3 = l_IndexArray[(*l_pTexCoordIndices)[loop2].m_iVtx3];
			}
		}

		// and reindex the triangle strips
		count = in_pMesh->GetTriangleStripListCount();
		for(loop = 0; loop < count; loop++)
		{
			for(loop2 = 0; loop2 < in_pMesh->TriangleStripLists()[loop]->GetTriangleStripCount(); loop2++)
			{
				CSLTriangleStrip* l_pTriangleStrip = in_pMesh->TriangleStripLists()[loop]->TriangleStrips()[loop2];
				CSLTriangleStrip::CSLIntArray* l_pTexCoordIndices = l_pTriangleStrip->GetUVIndices(TexCoordLoop);

				for(loop3 = 0; loop3 < l_pTexCoordIndices->GetUsed(); loop3++)
				{
					(*l_pTexCoordIndices)[loop3] = l_IndexArray[(*l_pTexCoordIndices)[loop3]];
				}
			}
		}
	}
}

void CXsiFtkUtil::CompressMeshColors(CSLMesh* in_pMesh)
{
	CSIBCArray<CSIBCColorfIndexed>		l_UncompressedVectorArray;
	CSIBCArray<int>						l_IndexArray;

	if(in_pMesh->Shape()->GetColorList() == NULL)
		return;

	// compress the colors
	int count, loop, loop2, loop3;

	count = in_pMesh->Shape()->GetColorCount();

	l_UncompressedVectorArray.Extend(count);
	for(loop = 0; loop < count; loop++)
	{
		l_UncompressedVectorArray[loop].Set
			(
			in_pMesh->Shape()->GetColorListPtr()[loop].m_fR,
			in_pMesh->Shape()->GetColorListPtr()[loop].m_fG,
			in_pMesh->Shape()->GetColorListPtr()[loop].m_fB,
			in_pMesh->Shape()->GetColorListPtr()[loop].m_fA
			);

		l_UncompressedVectorArray[loop].index = loop;
	}

	qsort(l_UncompressedVectorArray.ArrayPtr(), count, sizeof(CSIBCColorfIndexed), CompareCSIBCColorf);

	l_IndexArray.Extend(count);

	// now just fill up the normal array again
	CSLBaseShape::CSLColorArray* l_pColors = in_pMesh->Shape()->GetColorList();
	l_pColors->Resize(1);

	int currentIndex = 0;
	int currentCompressedIndex = 0;
	(*l_pColors)[0].m_fR = l_UncompressedVectorArray[0].m_fR;
	(*l_pColors)[0].m_fG = l_UncompressedVectorArray[0].m_fG;
	(*l_pColors)[0].m_fB = l_UncompressedVectorArray[0].m_fB;
	(*l_pColors)[0].m_fA = l_UncompressedVectorArray[0].m_fA;
	l_IndexArray[l_UncompressedVectorArray[0].index] = 0;

	for(loop = 1; loop < count; loop++)
	{
		if(CompareCSIBCColorf(&l_UncompressedVectorArray[loop], &l_UncompressedVectorArray[currentIndex]) != 0)
		{
			l_pColors->Extend(1);
			currentCompressedIndex = l_pColors->GetUsed()-1; 
			(*l_pColors)[currentCompressedIndex].m_fR = l_UncompressedVectorArray[loop].m_fR;
			(*l_pColors)[currentCompressedIndex].m_fG = l_UncompressedVectorArray[loop].m_fG;
			(*l_pColors)[currentCompressedIndex].m_fB = l_UncompressedVectorArray[loop].m_fB;
			(*l_pColors)[currentCompressedIndex].m_fA = l_UncompressedVectorArray[loop].m_fA;
			currentIndex = loop;
		}

		l_IndexArray[l_UncompressedVectorArray[loop].index] = currentCompressedIndex;
	}


	// and reindex the polygons
	count = in_pMesh->GetPolygonListCount();
	for(loop = 0; loop < count; loop++)
	{
		CSLPolygonList::CSLIntArray* l_pColorIndices = in_pMesh->PolygonLists()[loop]->GetColorIndices();

		for(loop2 = 0; loop2 < l_pColorIndices->GetUsed(); loop2++)
		{
			(*l_pColorIndices)[loop2] = l_IndexArray[(*l_pColorIndices)[loop2]];
		}
	}

	// and reindex the triangles
	count = in_pMesh->GetTriangleListCount();
	for(loop = 0; loop < count; loop++)
	{
		CSLTriangleList::CSLTriangleArray* l_pColorIndices = in_pMesh->TriangleLists()[loop]->GetColorIndices();

		for(loop2 = 0; loop2 < l_pColorIndices->GetUsed(); loop2++)
		{
			(*l_pColorIndices)[loop2].m_iVtx1 = l_IndexArray[(*l_pColorIndices)[loop2].m_iVtx1];
			(*l_pColorIndices)[loop2].m_iVtx2 = l_IndexArray[(*l_pColorIndices)[loop2].m_iVtx2];
			(*l_pColorIndices)[loop2].m_iVtx3 = l_IndexArray[(*l_pColorIndices)[loop2].m_iVtx3];
		}
	}

	// and reindex the triangle strips
	count = in_pMesh->GetTriangleStripListCount();
	for(loop = 0; loop < count; loop++)
	{
		for(loop2 = 0; loop2 < in_pMesh->TriangleStripLists()[loop]->GetTriangleStripCount(); loop2++)
		{
			CSLTriangleStrip* l_pTriangleStrip = in_pMesh->TriangleStripLists()[loop]->TriangleStrips()[loop2];
			CSLTriangleStrip::CSLIntArray* l_pColorIndices = l_pTriangleStrip->GetColorIndices();

			for(loop3 = 0; loop3 < l_pColorIndices->GetUsed(); loop3++)
			{
				(*l_pColorIndices)[loop3] = l_IndexArray[(*l_pColorIndices)[loop3]];
			}
		}
	}
}

float CXsiFtkUtil::RollFromUpVector(CSIBCVector3D& in_Position, CSIBCVector3D& in_Interest, CSIBCVector3D& in_UpVector, int in_UpAxis)
{
	CSIBCVector3D l_XAxis(0,0,0);
	CSIBCVector3D l_YAxis;

	if(in_UpAxis == 0)
	{
		l_YAxis.Set(1,0,0);
	}
	else if(in_UpAxis == 1)
	{
		l_YAxis.Set(0,1,0);
	}
	else if(in_UpAxis == 2)
	{
		l_YAxis.Set(0,0,1);
	}

	CSIBCVector3D	l_ZAxis = in_Interest - in_Position;
	l_ZAxis.Normalize();

	l_XAxis = l_YAxis.CrossProduct(l_ZAxis);
	l_XAxis.Normalize();
	l_YAxis = l_ZAxis.CrossProduct(l_XAxis);
	l_YAxis.Normalize();

	float l_YAxisDot = l_YAxis.GetDot(in_UpVector);
	float l_XAxisDot = l_XAxis.GetDot(in_UpVector);

	float l_fAngle = (l_YAxisDot < 1.0f) ? acosf(l_YAxisDot) : 0;
	float l_fAngle2 = (l_XAxisDot < 1.0f) ? acosf(l_XAxisDot) : 0;

	if(l_fAngle2 > 1.5707963267f)
		l_fAngle = -l_fAngle;

	return l_fAngle;
}

void CXsiFtkUtil::UpVectorFromRoll(CSIBCVector3D& in_Position, CSIBCVector3D& in_Interest, float in_Roll, CSIBCVector3D& out_UpVector)
{
	// Compute the up vector
	CSIBCVector3D	l_LookAt = in_Interest - in_Position;
	l_LookAt.Normalize();
	CSIBCMatrix4x4	l_TmpMatrix;

	l_TmpMatrix.AlignRoll(FTK_DEGTORAD(in_Roll), &l_LookAt);
	l_TmpMatrix.Multiply(out_UpVector, CSIBCVector3D(0,1,0));
}


CSLCustomPSet* CXsiFtkUtil::FindCustomPSet(CSLTemplate* in_pOwner, char* in_pCustomPSetName)
{
	int psetloop;
	for(psetloop = 0; psetloop < in_pOwner->GetCustomPSetCount(); psetloop++)
	{
		if(strcmp(in_pOwner->GetCustomPSetList()[psetloop]->GetName(), in_pCustomPSetName) == 0)
		{
			return in_pOwner->GetCustomPSetList()[psetloop]; 
		}
	}

	return NULL;
}


// TODO : make a better version of this and make it general to all application
std::string CXsiFtkUtil::ReplaceDotWithUnderscore(const std::string& inString)
{
	std::string outString(inString);
	size_t dotPos = outString.find(".");

	while(dotPos != std::string::npos)
	{
		outString.replace(dotPos,1,"_");
		dotPos = outString.find("."); // find next dot
	}

	size_t thePos;
	thePos = outString.rfind("/");
	if(thePos != std::string::npos)
	{
		outString = outString.substr(thePos+1, outString.size());
	}
	else
	{
		thePos = outString.rfind("\\");
		if(thePos != std::string::npos)
		{
			outString = outString.substr(thePos+1, outString.size());
		}
		else
		{
			
		}
	}


	return outString;
}




