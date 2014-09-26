
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __XYZFILE_H__
#define __XYZFILE_H__


#include "file.h"

namespace ZBPlugin
{

// CXyzFile: XYZ file format
class CXyzFile : public CFile
{
public:
	CXyzFile(){};
	~CXyzFile(){};

	inline Vector3DVector&	GetVertexPositionVector()	{return m_vertices;}
	inline Vector3DVector&	GetTextureCoordVector()		{return m_texCoords;}	

private:
	Vector3DVector m_vertices;
	Vector3DVector m_texCoords;
	

// PowerCrust stuff
public:

	// PolyData
	class CPolyData
	{
	public:

		class CPoly
		{
		public:
			std::vector<ulong> m_vertIndices;
		};
		typedef std::vector<CPoly> PolyVector;

		inline PolyVector& GetPolys() {return m_polys;}
		inline Vector3DVector& GetPoints() {return m_points;}
		inline std::vector<double>& GetWeights() {return m_weights;}

	private:
		PolyVector m_polys;
		Vector3DVector m_points;
		std::vector<double> m_weights;
	};

	inline CPolyData& GetMedialSurface() {return m_medialSurface;}
	inline CPolyData& GetFinalSurface() {return m_finalSurface;}

private:
	CPolyData m_medialSurface;
	CPolyData m_finalSurface;
};

} // End ZBPlugin namespace

#endif // __XYZFILE_H__
