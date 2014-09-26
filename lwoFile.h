
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __LWOFILE_H__
#define __LWOFILE_H__


#include "binaryFile.h"
#include "fileException.h"


// NOTE : LWO format is limited to 65535 polygons

namespace ZBPlugin
{

#define MAKE_ID(a,b,c,d)	\
	((ulong) (a)<<24 | (ulong) (b)<<16 | \
	(ulong) (c)<<8 | (ulong) (d))

// CLwoFile: LWO file format
class CLwoFile : public CBinaryFile
{
public:
	// TODO : not all of the below are really chunks, some are Types... sort this out
	static const ulong CHUNK_FORM = MAKE_ID('F','O','R','M');
	static const ulong CHUNK_LWO2 = MAKE_ID('L','W','O','2');
	static const ulong CHUNK_LAYR = MAKE_ID('L','A','Y','R');
	static const ulong CHUNK_TAGS = MAKE_ID('T','A','G','S');
	static const ulong CHUNK_PNTS = MAKE_ID('P','N','T','S');
	static const ulong CHUNK_VMAP = MAKE_ID('V','M','A','P');
	static const ulong CHUNK_VMAD = MAKE_ID('V','M','A','D');
	static const ulong CHUNK_TXUV = MAKE_ID('T','X','U','V');
	static const ulong CHUNK_POLS = MAKE_ID('P','O','L','S');
	static const ulong CHUNK_FACE = MAKE_ID('F','A','C','E');
	static const ulong CHUNK_PTCH = MAKE_ID('P','T','C','H');
	static const ulong CHUNK_PTAG = MAKE_ID('P','T','A','G');
	static const ulong CHUNK_SMGP = MAKE_ID('S','M','G','P');
	static const ulong CHUNK_PART = MAKE_ID('P','A','R','T');
	static const ulong CHUNK_CLIP = MAKE_ID('C','L','I','P');
	static const ulong CHUNK_STIL = MAKE_ID('S','T','I','L');
	
	// Surface
	static const ulong CHUNK_SURF = MAKE_ID('S','U','R','F');
		static const ulong CHUNK_COLR = MAKE_ID('C','O','L','R');
		static const ulong CHUNK_DIFF = MAKE_ID('D','I','F','F');
		static const ulong CHUNK_BLOK = MAKE_ID('B','L','O','K');
			static const ulong CHUNK_IMAP = MAKE_ID('I','M','A','P');
				static const ulong CHUNK_CHAN = MAKE_ID('C','H','A','N');
				static const ulong CHUNK_OPAC = MAKE_ID('O','P','A','C');
				static const ulong CHUNK_ENAB = MAKE_ID('E','N','A','B');
				static const ulong CHUNK_NEGA = MAKE_ID('N','E','G','A');
				static const ulong CHUNK_AXIS = MAKE_ID('A','X','I','S');
			static const ulong CHUNK_TMAP = MAKE_ID('T','M','A','P');
				static const ulong CHUNK_CNTR = MAKE_ID('C','N','T','R');
				static const ulong CHUNK_SIZE = MAKE_ID('S','I','Z','E');
				static const ulong CHUNK_ROTA = MAKE_ID('R','O','T','A');
				static const ulong CHUNK_FALL = MAKE_ID('F','A','L','L');
				static const ulong CHUNK_OREF = MAKE_ID('O','R','E','F');
				static const ulong CHUNK_CSYS = MAKE_ID('C','S','Y','S');
		static const ulong CHUNK_PROJ = MAKE_ID('P','R','O','J');
		static const ulong CHUNK_IMAG = MAKE_ID('I','M','A','G');
		static const ulong CHUNK_WRAP = MAKE_ID('W','R','A','P');
		static const ulong CHUNK_WRPW = MAKE_ID('W','R','P','W');
		static const ulong CHUNK_WRPH = MAKE_ID('W','R','P','H');
		static const ulong CHUNK_AAST = MAKE_ID('A','A','S','T');
		static const ulong CHUNK_PIXB = MAKE_ID('P','I','X','B');


	//#pragma pack(2)
	struct SChunk
	{
		ulong Flag;
		ulong Size;
	};
	//#pragma pack()


	typedef std::pair< bool, ushort > SmoothingGroupPair;


	// LWO Layer
	class CLayer
	{
	public:

		CLayer(ushort layerID);
		~CLayer();

		inline ushort GetLayerID() const						{return m_layerID;}
		inline std::string GetLayerName() const					{return m_layerName;}
		inline void SetLayerName(const std::string layerName)   {m_layerName = layerName;}


		// LWO Surface 
		class CSurface
		{
		public:
			CSurface();
			~CSurface();

			inline void SetSurfaceName(const std::string& name){m_surfaceName = name;}
			inline std::string GetSurfaceName(){return m_surfaceName;}

		private:
			std::string m_surfaceName;

		public:
			short m_imageIndex;
			int m_stateSet;
			Vector3D m_vertexColor;

			ulong m_size;
			// TODO : other surface attributes
		};
		typedef std::map< std::string, CSurface > SurfaceMap;
		typedef StringVector ImageVector;


		// LWO part vector
		typedef StringVector PartVector;

		// LWO smoothing group vector
		typedef std::set< int > SGVector;


		// LWO texCoord for VMAPs and VMADs
		class CTexCoord
		{
		public:
			int m_faceIndex; // face index into the face vector, can be -1
			Vector3D m_texCoord;
		};
		typedef std::map< ushort, std::vector< CTexCoord > > TexCoordMap; // < point index , <texCoords> >


		// LWO Face
		class CFace
		{
		public:
			CFace();
			~CFace();

			inline uint VertexCount() {return m_vertexIndices.size();}
			inline std::vector< ushort >& GetVertexIndexVector() {return m_vertexIndices;}
			//inline std::vector< ushort >& GetTexCoordIndexVector()	{return m_texCoordIndices;}

			SmoothingGroupPair m_smoothingGroup; // smoothing group
			ushort m_surface; // "material" reference
			ushort m_part; // "mesh" reference

		private:
			std::vector< ushort > m_vertexIndices;
			//std::vector< ushort > m_texCoordIndices;
		};
		typedef std::vector< CFace > FaceVector;


		// Face
		inline uint FaceCount() const {return m_faces.size();}
		inline void AddFace( const CFace& face )
		{
			m_faces.push_back( face );
		}

		// Image
		bool AddImage(int index, const std::string& imagePath);
		std::string GetImagePath(int imageIndex);

		// Vectors
		inline Vector3DVector&	GetVertexPositionVector(){return m_vertices;}
		inline TexCoordMap&		GetTexCoordMap()		 {return m_texCoords;}

		inline FaceVector&		GetFaceVector()			 {return m_faces;}
		inline SurfaceMap&		GetSurfaceMap()			 {return m_surfaces;}
		inline ImageVector&		GetImageVector()		 {return m_images;}
		inline PartVector&		GetPartVector()			 {return m_parts;}
		inline SGVector&		GetSGVector()			 {return m_sgs;}

	public:
		ushort		m_flag;
		Vector3D	m_pivot;
		ushort		m_parent;

	private:
		ushort		m_layerID;
		std::string m_layerName;

		Vector3DVector m_vertices;
		TexCoordMap    m_texCoords;
		FaceVector	   m_faces;
		ImageVector	   m_images;
		SurfaceMap	   m_surfaces;
		PartVector	   m_parts;
		SGVector	   m_sgs;
	};
	typedef std::vector< CLayer > LayerVector;


	// Layers
	inline CLayer* AddLayer( const CLayer& layer )
	{
		m_layers.push_back( layer );
		return &m_layers.back();
	}
	inline LayerVector&	GetLayerVector() {return m_layers;}

private:
	LayerVector m_layers;
};

} // End ZBPlugin namespace

#endif // __LWOFILE_H__