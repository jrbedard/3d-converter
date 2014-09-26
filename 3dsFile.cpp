
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "3dsFile.h"


// Uhhh, TODO : un-nest these classes
C3dsFile::CObject::CMesh::CFace::CFace():
m_flag(0),
m_smoothingGroup(false,0),
m_materialID(0)
{

}

C3dsFile::CObject::CMesh::CFace::~CFace()
{

}


uint C3dsFile::AddMaterial(const std::string& materialName)
{
	static uint materialID = 0;
	m_materials.insert( std::make_pair(materialID, C3dsFile::CMaterial(materialName)));
	return (++materialID - 1);
}


uint C3dsFile::FindMaterial(const std::string& materialName)
{
	C3dsFile::MaterialMap::iterator materialIt;
	C3dsFile::MaterialMap::const_iterator materialEnd = m_materials.end();

	for( materialIt = m_materials.begin(); materialIt != materialEnd; ++materialIt )
	{
		if( materialIt->second.GetMaterialName() == materialName )
			return materialIt->first;
	}

	MSG_WARNING("Invalid Material Name: '" << materialName << "'. Returning the first material.");

	return 0;
}



