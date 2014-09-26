
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XSIFILE_H__
#define __XSIFILE_H__


#include <stdio.h>

#include "file.h"
#include "fileException.h"


class CXSIParser;
class CSLScene;


namespace ZBPlugin
{

// CXsiFile: XSI file format
class CXsiFile : public CFile
{
public:
	CXsiFile(const fs::path& fileName);
	virtual ~CXsiFile();

	inline CSLScene* GetScene(){return m_pScene;}

	bool ConvertXsiToCollada(const fs::path& xsiFileName, const fs::path& daeFileName);

private:
	void InitializeXsiFtkObjects();
	void DestroyXsiFtkObjects();

private:
	CSLScene* m_pScene;
public:
	fs::path m_fileName; // temp
};

// XSI model template structure quick ref :

/*
SI_Model MDL-cube2 {
	SI_Transform SRT-cube2
	{
		1.0,1.0,1.0,
			0.0,0.0,0.0,
			0.0,0.0,0.0,
	}

	SI_Mesh MSH-cube2
	{
		SI_Shape SHP-cube2-ORG
		{
			2,
				"ORDERED",
				8,
				"POSITION",
				-0.5,-0.5,-0.5, // vertex[0] 
				-0.5,-0.5,0.5,  // vertex[1]
				-0.5,0.5,-0.5,  // vertex[2]
				-0.5,0.5,0.5,   // vertex[3]
				0.5,-0.5,-0.5,  // vertex[4]
				0.5,-0.5,0.5,   // vertex[5]
				0.5,0.5,-0.5,   // vertex[6]
				0.5,0.5,0.5,    // vertex[7]

				6,
				"NORMAL",
				-1.0,0.0,0.0,  // normal[0]
				0.0,0.0,-1.0,  // normal[1]
				0.0,-1.0,0.0,  // normal[2]
				0.0,0.0,1.0,   // normal[3]
				0.0,1.0,0.0,   // normal[4]
				1.0,0.0,0.0,   // normal[5]
		}

		SI_PolygonList
		{
			6,    // 6 polygons 
				"NORMAL",
				"MAT_SURFACE-TERRACOTTA001.1-1",
				24,   // 24 vertices 
				4,     // Each polygon in the cube has 4 vertices 
				4,
				4,
				4,
				4,
				4,

				// Indices of the vertices in the POSITION section of 
				the SI_Shape template 
				0,1,3,2,  // polygon[0] 
				1,5,7,3,
				5,4,6,7,
				4,0,2,6,
				4,5,1,0,
				2,3,7,6,  // polygon[5] 

				// Indices of the normals in the NORMAL section of the 
				SI_Shape template 
				0,0,0,0,  // polygon[0] 
				3,3,3,3,
				5,5,5,5,
				1,1,1,1,
				2,2,2,2,
				4,4,4,4,  // polygon[5] 
		}
	}
} 
*/

} // End ZBPlugin namespace

#endif // __XSIFILE_H__