
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "stlFile.h"


CStlFile::CSolid::CSolid(const std::string& solidName):
m_solidName(solidName)
{
}

CStlFile::CSolid::~CSolid()
{
}


CStlFile::CSolid::CFacet::CFacet():
m_normal(3)
{
	m_normal.clear();
	m_vertices.reserve(3);
}

CStlFile::CSolid::CFacet::~CFacet()
{
}