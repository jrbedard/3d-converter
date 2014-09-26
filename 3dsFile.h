
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __3DSFILE_H__
#define __3DSFILE_H__


#include "file.h"
#include "fileException.h"


namespace ZBPlugin
{

// C3dsFile: 3DS file
class C3dsFile : public CFile
{
public:

	// TODO : to ease the endianess make it private and provide an accessor....
	enum CHUNK
	{
		CHUNK_VERSION	= 0x0002,
	    CHUNK_RGBF      = 0x0010,
	    CHUNK_RGBB      = 0x0011,

		CHUNK_PERCENTW	= 0x0030,
		CHUNK_PERCENTF	= 0x0031,

	    CHUNK_PRJ       = 0xC23D,
	    CHUNK_MLI       = 0x3DAA,

	    CHUNK_MAIN      = 0x4D4D,
	        CHUNK_EDITOR  = 0x3D3D,
				CHUNK_ONEUNIT	= 0x0100,
				CHUNK_CONFIG    = 0x3D3E,  
	            CHUNK_BKGCOLOR  = 0x1200,
	            CHUNK_AMBCOLOR  = 0x2100,
	            CHUNK_OBJBLOCK  = 0x4000,
	                CHUNK_TRIMESH   = 0x4100,
	                    CHUNK_VERTLIST  = 0x4110, // Vertex positions
	                    CHUNK_VERTFLAGS = 0x4111,
						CHUNK_MAPLIST   = 0x4140, // UV coords
	                    CHUNK_FACELIST  = 0x4120,
							CHUNK_FACEMAT   = 0x4130,
							CHUNK_SMOOLIST  = 0x4150,
	                    CHUNK_TRMATRIX  = 0x4160,
	                    CHUNK_MESHCOLOR = 0x4165,
	                    CHUNK_TXTINFO   = 0x4170,
	                CHUNK_LIGHT     = 0x4600,
	                    CHUNK_SPOTLIGHT = 0x4610,
	                CHUNK_CAMERA    = 0x4700,
	                CHUNK_HIERARCHY = 0x4F00,

			CHUNK_MATERIAL  = 0xAFFF,
	            CHUNK_MATNAME   = 0xA000,
	            CHUNK_AMBIENT   = 0xA010,
	            CHUNK_DIFFUSE   = 0xA020,
	            CHUNK_SPECULAR  = 0xA030,
	            CHUNK_TEXTURE   = 0xA200,
	            CHUNK_BUMPMAP   = 0xA230,
	            CHUNK_MAPFILE   = 0xA300,
			
			CHUNK_KEYFRAMER = 0xB000,
	            CHUNK_AMBIENTKEY    = 0xB001,
	            CHUNK_TRACKINFO = 0xB002,
	                CHUNK_TRACKOBJNAME  = 0xB010,
	                CHUNK_TRACKPIVOT    = 0xB013,
	                CHUNK_TRACKPOS      = 0xB020,
	                CHUNK_TRACKROTATE   = 0xB021,
	                CHUNK_TRACKSCALE    = 0xB022,
	                CHUNK_TRACKMORPH    = 0xB026,
	                CHUNK_TRACKHIDE     = 0xB029,
	                CHUNK_OBJNUMBER     = 0xB030,
	            CHUNK_TRACKCAMERA = 0xB003,
	                CHUNK_TRACKFOV  = 0xB023,
	                CHUNK_TRACKROLL = 0xB024,
	            CHUNK_TRACKCAMTGT = 0xB004,
	            CHUNK_TRACKLIGHT  = 0xB005,
	            CHUNK_TRACKLIGTGT = 0xB006,
	            CHUNK_TRACKSPOTL  = 0xB007,
	            CHUNK_FRAMES	  = 0xB008,
				CHUNK_KEYFUNKN2	  = 0xB009,
				CHUNK_KEYFUNKN1	  = 0xB00A,
	};


	#pragma pack(2)
	struct SChunk
	{
		ushort	Flag;
		ulong	Size;
	};
	#pragma pack()


	struct STransform3dsMatrix
	{
		float _11, _12, _13;
		float _21, _22, _23;
		float _31, _32, _33;
	};

	struct STranslate3dsMatrix
	{
		float _11, _12, _13;
	};


	typedef std::pair< bool, uint > SmoothingGroupPair;


	// 3DS Material Class
	class CMaterial
	{
	public:
		CMaterial(){}
		CMaterial(const std::string& materialName):
		m_materialName(materialName),
		m_TexMapName(false,"")
		{}

		inline const std::string& GetMaterialName()	{return m_materialName;}

		inline void SetTexMap(const std::string& texMap)	{m_TexMapName  = std::make_pair(true, texMap);}
		inline bool HasTexMap()								{return m_TexMapName.first;}
		inline std::string& GetTexMap()						{OBJ_ASSERT(m_TexMapName.first);  return m_TexMapName.second;}

	private:
		std::string m_materialName;
		std::pair< bool, std::string > m_TexMapName;  // texture map
	};
	typedef std::map< uint, CMaterial > MaterialMap;


	
	// TODO : look at CLwoFile for re-organization, un-nesting, etc
	// Class Object
	class CObject
	{
	public:

		CObject(const std::string& objectName):
		m_objName(objectName)
		{}
		~CObject(){}

		 // triangular mesh
		class CMesh
		{
		public:

			// face (triangle)
			class CFace
			{
			public:
				CFace();
				~CFace();

				inline uint VertexCount(){return m_vertexIndices.size();}

				std::vector< short > m_vertexIndices;
				ushort m_flag;
				SmoothingGroupPair m_smoothingGroup;

				uint m_materialID;
			};
			typedef std::vector< CFace > FaceVector;

			inline uint FaceCount() const		{return m_faces.size();}
			inline uint VertexCount() const		{return m_vertices.size();}
			inline uint MapCoordCount() const	{return m_mapCoords.size();}

			inline void AddFace( const CFace& face )
			{
				m_faces.push_back( face );
			}

			inline FaceVector&	   GetFaceVector()			{return m_faces;}
			inline Vector3DVector& GetVertexPositionVector(){return m_vertices;}
			inline Vector3DVector& GetMapCoordVector()		{return m_mapCoords;}

		private:
			FaceVector m_faces;
			Vector3DVector m_vertices;
			Vector3DVector m_mapCoords; // UVs
		};
		typedef std::vector< CMesh > MeshVector;


		inline const std::string& GetObjectName() const {return m_objName;}

		inline void AddMesh( const CMesh& mesh )
		{
			m_meshes.push_back( mesh );
		}

		inline MeshVector& GetMeshVector() {return m_meshes;}

	private:
		std::string m_objName;
		MeshVector m_meshes;

	};
	typedef std::vector< CObject > ObjectVector;


	inline void AddObject( const CObject& object )
	{
		m_objects.push_back( object );
	}

	uint AddMaterial(const std::string& materialName);
	uint FindMaterial(const std::string& materialName);

	inline ObjectVector&	GetObjectVector()	{return m_objects;}
	inline CObject&			GetLastObject()		{return m_objects.back();}
	inline CObject::CMesh&	GetLastMesh()		{return m_objects.back().GetMeshVector().back();}
	
	inline MaterialMap&		GetMaterialMap()	{return m_materials;}

private:
	ObjectVector m_objects;
	MaterialMap m_materials;
};

} // End ZBPlugin namespace

#endif // __3DSFILE_H__