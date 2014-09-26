
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __OBJFBXCONVERTER_H__
#define __OBJFBXCONVERTER_H__


#include "fromObjConverter.h"


namespace FBXSDK_NAMESPACE
{ 
	class KFbxLayer;
}


namespace ZBPlugin
{

class CFbxFile;
class CObjFbxOptions;


// CObjFbxConverter: Converts OBJ into FBX
class CObjFbxConverter : public CFromObjConverter
{
public:
	CObjFbxConverter();
	~CObjFbxConverter();

private:
	CObjFbxConverter(const CObjFbxConverter&); // Copy constructor
	void operator=(const CObjFbxConverter&); // Assignment operator

public:
	bool ParseOptions(const std::string& optionString);
	bool Convert(CFile* pObjFile, CFile* pOtherFile);

private:
	struct SFbxMaterial
	{
		int fbxMaterialID;
		int fbxTextureID;
	};
	typedef std::map<int, SFbxMaterial> MaterialMap;  // < OBJ mat index, < FBX mat ID, FBX tex ID > >

	SFbxMaterial CreateMaterial(int materialID, FBXSDK_NAMESPACE::KFbxLayer* pMasterLayer);
	MaterialMap m_materials;

private:
	CFbxFile*		m_pFbxFile;
	CObjFbxOptions*	m_pOptions;
};


class CObjFbxOptions : public CFromObjOptions
{
	//bool m_b3dsConversion;
	bool m_bEmbedTexture;
};

} // End ZBPlugin namespace

#endif // __OBJFBXCONVERTER_H__
