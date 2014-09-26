
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __MAYAFILE_H__
#define __MAYAFILE_H__


#include "file.h"
#include "fileException.h"
#include "mtlFile.h"

namespace ZBPlugin
{

// CMayaFile: Maya file format
class CMayaFile : public CFile
{
public:
	CMayaFile(){}
	~CMayaFile(){}


	// Transform node
	class CTransform
	{
	public:
		std::string m_name;
	};


	// mesh Shape node
	class CMesh
	{
	public:

		CMesh(const std::string& name) :
		  m_name(name)
		{}

		// PolyFace class
		class CPolyFace
		{
		public:

			// Face class
			class CFace
			{
			public:

				inline void AddEdge( const int edgeIndex ) // can be negative = edge reversed
				{
					m_edgeIndex.push_back( edgeIndex );
				}

				inline std::vector< int >& GetEdgeVector()	{return m_edgeIndex;}
				inline uint EdgeCount() const				{return m_edgeIndex.size();}


				inline void AddNormal( const int normalIndex ) // 1 normal per vertex ?
				{
					m_normalIndex.push_back( normalIndex );
				}

				inline std::vector< int >& GetNormalVector()	{return m_normalIndex;}
				inline uint NormalCount() const				{return m_normalIndex.size();}

			private:
				std::vector< int > m_edgeIndex;
				std::vector< int > m_normalIndex;
			};


			// Mu class
			class CMu
			{
			public:

				inline void AddUV( const uint uv )
				{
					m_uvs.push_back( uv );
				}

				inline uint GetUvSet() const	{ return m_uvSet; }
				inline void SetUvSet(uint uv)  { m_uvSet = uv; }

				inline std::vector< uint >& GetUvVector() {return m_uvs;}
				inline uint UvCount() const {return m_uvs.size();}

			private:
				uint m_uvSet;
				std::vector< uint > m_uvs;
			};


			inline void SetFace(const CFace& face)	{m_face = face;}
			inline void SetMu(const CMu& mu)		{m_mu = std::make_pair(true,mu);}

			inline bool HasTextureCoordinate() const {return m_mu.first;}
			inline bool HasNormal() const {return (m_face.NormalCount() > 0);}

			// do some error checking here
			inline const CFace	GetFace() const {return m_face;}
			inline const CMu	GetMu() const	{OBJ_ASSERT(m_mu.first); return m_mu.second;}


		private:
			CFace m_face;
			std::pair< bool, CMu > m_mu;
		};
		typedef std::vector< CPolyFace > PolyFaceVector;


		inline std::string GetMeshName() const	{return m_name;}

		inline Vector3DVector& GetTextureCoordVector()		{return m_textureCoords;}
		inline Vector3DVector& GetVertexPositionVector()	{return m_vertexPositions;}
		inline Vector3DVector& GetEdgeVector()				{return m_edges;}
		inline Vector3DVector& GetNormalVector()			{return m_normals;}
		inline PolyFaceVector& GetPolyFaceVector()			{return m_polyFaces;}

	private:

		CTransform* m_pTransform;

		std::string m_name;

		Vector3DVector  m_textureCoords;
		Vector3DVector  m_vertexPositions;
		Vector3DVector	m_edges;
		Vector3DVector  m_normals;
		PolyFaceVector	m_polyFaces;

	};
	typedef std::vector< CMesh > MeshVector;



	inline void AddMesh( const CMesh& mesh )
	{
		m_meshes.push_back( mesh );
	}


	inline MeshVector& GetMeshVector()					{return m_meshes;}
	
	// material
	//std::string GetMaterialNameFromID(int id)	{return m_materials[id];}
	//CMtlFile* CreateMaterialFile();
	inline CMtlFile* GetMaterialFile()					{return m_pMtlFile;}
	inline void SetMaterialFile(CMtlFile* pMtlFile)		{m_pMtlFile = pMtlFile;} 


public:
	fs::path m_destDir;

private:
	MeshVector m_meshes;
	CMtlFile* m_pMtlFile;

};

} // End ZBPlugin namespace

#endif // __MAYAFILE_H__