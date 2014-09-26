
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "lwoFile.h"


CLwoFile::CLayer::CLayer(ushort layerID):
m_layerID(layerID)
{
	m_layerName = "";
	m_flag = 0;

	Vector3D pivot(3);
	pivot[0] = 0.0f;
	pivot[1] = 0.0f;
	pivot[2] = 0.0f;
	m_pivot = pivot;
	m_parent = 0;
}

CLwoFile::CLayer::~CLayer()
{
	

}

CLwoFile::CLayer::CSurface::CSurface():
m_surfaceName(""),
m_imageIndex(-1),
m_stateSet(0),
m_vertexColor(3),
m_size(0)
{
	m_vertexColor[0] = 0.0f;
	m_vertexColor[1] = 0.0f;
	m_vertexColor[2] = 0.0f;
}

CLwoFile::CLayer::CSurface::~CSurface()
{
	
}

std::string CLwoFile::CLayer::GetImagePath(int imageIndex)
{
	std::string imagePath("");
	if(imageIndex == -1 || m_images.size() == 0)
	{
		return imagePath;
	}
	return m_images[imageIndex];
}



bool CLwoFile::CLayer::AddImage(int index, const std::string& imagePath)
{
	OBJ_ASSERT(index >= 0);
	if(index+1 > (int)m_images.size())
	{
		m_images.resize(index+1);
	}
	m_images[index] = imagePath;
	return true;
}


CLwoFile::CLayer::CFace::CFace():
m_smoothingGroup(false,0),
m_surface(0),
m_part(0)
{
	//m_vertexIndices.reserve( 4 );
}

CLwoFile::CLayer::CFace::~CFace()
{
	
}

