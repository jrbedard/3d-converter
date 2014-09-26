
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __STLFILE_H__
#define __STLFILE_H__


#include "file.h"

namespace ZBPlugin
{

// CStlFile: STL file format
class CStlFile : public CFile
{
public:
	CStlFile(){};
	~CStlFile(){};

	// STL object
	class CSolid
	{
	public:
		CSolid(const std::string& solidName);
		~CSolid();

		// STL object face
		class CFacet
		{
		public:
			CFacet();
			~CFacet();

			Vector3D m_normal;
			Vector3DVector m_vertices;
		};
		typedef std::vector< CFacet > FacetVector;

	public:
		inline const std::string GetSolidName() {return m_solidName;}
		inline FacetVector& GetFacetVector() {return m_facets;}
		inline CFacet& GetCurrentFacet() {return m_facets.back();}

	private:
		std::string m_solidName;
		FacetVector m_facets;
	};
	typedef std::vector< CSolid > SolidVector;

	inline SolidVector& GetSolidVector() {return m_solids;}
	inline CSolid& GetCurrentSolid() {return m_solids.back();}

private:
	SolidVector m_solids;

};

} // End ZBPlugin namespace

#endif // __STLFILE_H__
