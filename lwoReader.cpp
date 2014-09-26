// LWO File format reader
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter), 2005

#include "StdAfx.h"
#include "lwoReader.h"


// LWO reader Constructor
CLwoReader::CLwoReader(CFile* pFile, const fs::path& fileName):
m_pCurLayer(NULL)
{
	CLwoFile* pLwoFile = static_cast<CLwoFile*>(pFile);
	OBJ_ASSERT(pLwoFile);

	m_pLwoFile = pLwoFile;

	m_fileName = fileName;
}


// LWO Global Read
bool CLwoReader::Read()
{
	OBJ_ASSERT(m_pLwoFile);
	if(!m_pLwoFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	m_ifs.open(fileName.c_str(), ios::binary);

	MSG_INFO("Reading .LWO File '" << fileName << "'");
	OBJ_ASSERT(m_ifs.is_open());
	if(!m_ifs.is_open())
	{
		MSG_ERROR("Couldn't open .LWO file '" << fileName << "'");
		return false;
	}

	bool bStatus = false;

	// Compute the file size
	FileSize();

	// Read the header of the LWO file
	bStatus = ReadHeader();
	if(!bStatus)
	{
		MSG_ERROR("Invalid LWO Header");
		m_ifs.close();
		return false;
	}

	// Parse the LWO file chunks
	bStatus = ParseChunks();
	if(!bStatus)
	{
		MSG_ERROR("Error parsing LWO chunks");
		m_ifs.close();
		return false;
	}

	// End reading
	EndReading();

	m_ifs.close();

	return true;
}


// LWO Read Header
bool CLwoReader::ReadHeader()
{
	// FORM chunk
	CLwoFile::SChunk formChunk = ReadChunk();

	if(formChunk.Flag != CLwoFile::CHUNK_FORM) // FORM check
		return false;

	m_curChunk = formChunk;

	// LWO2 tag
	ulong lwo2 = ReadLong();

	if(lwo2 != CLwoFile::CHUNK_LWO2) // LWO check
		return false;

	return true;
}


// LWO Chunks Parsing
bool CLwoReader::ParseChunks()
{
	if(!m_ifs.is_open())
		return false;

	if(m_ifs.eof())
		return true;

	CLwoFile::SChunk chunk = ReadChunk();
	m_curChunk = chunk;

	if(chunk.Flag == CLwoFile::CHUNK_TAGS) // TAGS
	{
		MSG_DEBUG("Chunk TAGS detected");
		ReadTagStrings();
		ParseChunks();
	}
	else if(chunk.Flag == CLwoFile::CHUNK_LAYR) // LAYR
	{
		MSG_DEBUG("Chunk LAYR detected");
		ReadLayer();
		ParseChunks();
	}
	else if(chunk.Flag == CLwoFile::CHUNK_PNTS) // PNTS
	{
		MSG_DEBUG("Chunk PNTS detected");
		ReadPoints();
		ParseChunks();
	}
	else if(chunk.Flag == CLwoFile::CHUNK_VMAP) // VMAP
	{
		MSG_DEBUG("Chunk VMAP detected");
		ReadVertexMapping();
		ParseChunks();
	}
	else if(chunk.Flag == CLwoFile::CHUNK_POLS) // POLS
	{
		MSG_DEBUG("Chunk POLS detected");
		ReadPolygons();
		ParseChunks();
	}
	else if(chunk.Flag == CLwoFile::CHUNK_PTAG) // PTAG
	{
		MSG_DEBUG("Chunk PTAG detected");
		ReadPolygonTagMapping();
		ParseChunks();
	}
	else if(chunk.Flag == CLwoFile::CHUNK_VMAD) // VMAD
	{
		MSG_DEBUG("Chunk VMAD detected");
		ReadDiscVertexMapping();
		ParseChunks();
	}
	else if(chunk.Flag == CLwoFile::CHUNK_CLIP) // CLIP
	{
		MSG_DEBUG("Chunk CLIP detected");
		ReadImageDefinition();
		ParseChunks();
	}
	else if(chunk.Flag == CLwoFile::CHUNK_SURF) // SURF
	{
		MSG_DEBUG("Chunk SURF detected");
		ReadSurface();
		ParseChunks();
	}
	else
	{
		MSG_DEBUG("Unknown Chunk '" << GetTagString(chunk.Flag) << "' at " << m_ifs.tellg() << " offset");
		SkipChunk(chunk.Size);
		ParseChunks();;
	}

	return true;
}


// Read LWO Tags
bool CLwoReader::ReadTagStrings()
{
	OBJ_ASSERT(m_curChunk.Flag == CLwoFile::CHUNK_TAGS);
	while(m_curChunk.Size > 0) // for each tag
	{
		std::string name;
		ReadString(name);
		m_curChunk.Size -= GetStringSize(name);

		MSG_DEBUG("  tag name: '" << name << "'");
		m_tagVector.push_back(name);
	}

	return true;
}


// Read LWO Layers
bool CLwoReader::ReadLayer()
{
	OBJ_ASSERT(m_curChunk.Flag == CLwoFile::CHUNK_LAYR);

	ushort layerNumber = ReadShort();
	m_curChunk.Size -= 2;

	CLwoFile::CLayer layer(layerNumber);

	layer.m_flag = ReadShort();
	m_curChunk.Size -= 2;

	Vector3D pivot = ReadVector3D();
	layer.m_pivot = pivot;
	m_curChunk.Size -= 3*4;

	std::string layerName;
	ReadString(layerName);
	layer.SetLayerName(layerName);
	m_curChunk.Size -= GetStringSize(layerName);

	if(m_curChunk.Size > 2)
	{
		layer.m_parent = ReadShort();
		m_curChunk.Size -= 2;
	}

	MSG_DEBUG("LAYER '" << layer.GetLayerID() << "', Name: '" << layer.GetLayerName() << "'");
	SkipChunk(m_curChunk.Size);

	// remember current layer for parse methods
	m_pCurLayer = m_pLwoFile->AddLayer(layer); // Store current layer

	return true;
}


// Read LWO Point List
bool CLwoReader::ReadPoints()
{
	OBJ_ASSERT(m_pCurLayer && m_curChunk.Flag == CLwoFile::CHUNK_PNTS);
	int vertexCount = m_curChunk.Size / 12;

	MSG_DEBUG( vertexCount << " vertices");

	while(vertexCount--) // for each vertex position
	{
		Vector3D vert = ReadVector3D();
		m_pCurLayer->GetVertexPositionVector().push_back(vert);

		//MSG_DEBUG("X: " << vert[0] << " Y: " << vert[1] << " Z:" << vert[2]);		
	}

	return true;
}


// Read LWO Vertex Mapping
bool CLwoReader::ReadVertexMapping()
{
	OBJ_ASSERT(m_pCurLayer && m_curChunk.Flag == CLwoFile::CHUNK_VMAP); // VMAP

	ulong type = ReadLong();
	m_curChunk.Size -= 4;

	ushort dimension = ReadShort();
	m_curChunk.Size -= 2;

	std::string name;
	ReadString(name); // TODO : merge multiple UV sets
	m_curChunk.Size -= GetStringSize(name);

	MSG_DEBUG("VMAP: '" << GetTypeString(type) << "', name: '" << name << "' dimension: " << dimension);

	if(type == CLwoFile::CHUNK_TXUV && dimension == 2) // TXUV
	{
		ulong uvCount = m_curChunk.Size / 10;

		Vector3DVector& vertPos = m_pCurLayer->GetVertexPositionVector();
		CLwoFile::CLayer::TexCoordMap& texCoords = m_pCurLayer->GetTexCoordMap();

		ushort vertexIndex = 0;
		Vector3D uv(3);

		while(uvCount--) // For each UV coordinate
		{
			ushort vertexIndex = ReadShort();
			uv = ReadVector2D();

			CLwoFile::CLayer::CTexCoord lwoTexCoord;
			lwoTexCoord.m_faceIndex = -1; // TODO : what do we do with this
			lwoTexCoord.m_texCoord = uv;
			m_pCurLayer->GetTexCoordMap()[vertexIndex].push_back(lwoTexCoord); // push CTexCoord at the vertex position index
		}
	}
	else
	{
		// not recognized yet
		MSG_WARNING("VMAP type '" << GetTypeString(type) << "' not recognized" );
		SkipChunk(m_curChunk.Size);
	}

	return true;
}


// Read LWO Discontinuous Vertex Mapping
bool CLwoReader::ReadDiscVertexMapping()
{
	OBJ_ASSERT(m_pCurLayer && m_curChunk.Flag == CLwoFile::CHUNK_VMAD);

	ulong type = ReadLong();
	m_curChunk.Size -= 4;

	ushort dimension = ReadShort();
	m_curChunk.Size -= 2;

	std::string name;
	ReadString(name); // TODO : merge multiple UV sets
	m_curChunk.Size -= GetStringSize(name);

	MSG_DEBUG("VMAD: '" << name << "', type: '" << GetTypeString(type) << "', dimension: " << dimension);

	if(type == CLwoFile::CHUNK_TXUV && dimension == 2) 
	{
		int count = m_curChunk.Size / 12;
		ushort point_index = 0;
		ushort polygon_index = 0;
		Vector3D uv(3);

		ushort uv_index = 0;
		CLwoFile::CLayer::TexCoordMap& texCoords = m_pCurLayer->GetTexCoordMap();

		while(count--) // For each UV mapping
		{
			point_index = ReadShort();
			polygon_index = ReadShort();
			uv = ReadVector2D();

			//MSG_DEBUG("point: " << point_index << ", polygon: " << polygon_index << ", uv: (" << uv[0] << "," << uv[1] << ")"); 

			CLwoFile::CLayer::CTexCoord lwoTexCoord;
			lwoTexCoord.m_faceIndex = polygon_index;
			lwoTexCoord.m_texCoord = uv;
			m_pCurLayer->GetTexCoordMap()[point_index].push_back(lwoTexCoord); // push CTexCoord at the vertex position index
		}
	}
	else
	{
		// not recognized yet
		MSG_WARNING("VMAD type '" << GetTypeString(type) << "' not recognized" );
		SkipChunk(m_curChunk.Size);
	}

	return true;
}


// Read LWO Polygon List
bool CLwoReader::ReadPolygons()
{
	OBJ_ASSERT(m_pCurLayer && m_curChunk.Flag == CLwoFile::CHUNK_POLS);

	ulong type = ReadLong();
	m_curChunk.Size -= 4;

	MSG_DEBUG("POLS: '" << GetTypeString(type) << "'");

	if(type == CLwoFile::CHUNK_FACE)
	{
		ushort vertexCount;

		while(m_curChunk.Size > 0) // For each face
		{
			vertexCount = ReadShort() & 0x03FF;
			m_curChunk.Size -= 2;

			CLwoFile::CLayer::CFace face;

			while(vertexCount--) // For each vertex
			{
				ushort vertexIndex = ReadShort();
				face.GetVertexIndexVector().push_back(vertexIndex);
				m_curChunk.Size -= 2;
			}

			m_pCurLayer->AddFace(face);
		}
	}
	else if(type == CLwoFile::CHUNK_PTCH)
	{
		ushort vertexCount;

		while(m_curChunk.Size > 0) // For each face
		{
			vertexCount = ReadShort() & 0x03FF;
			m_curChunk.Size -= 2;

			CLwoFile::CLayer::CFace face;

			while(vertexCount--) // For each vertex
			{
				ushort vertexIndex = ReadShort();
				face.GetVertexIndexVector().push_back(vertexIndex);
				m_curChunk.Size -= 2;
			}

			m_pCurLayer->AddFace(face);
		}
	}
	else
	{
		// not recognized
		MSG_WARNING("POLS type '" << GetTypeString(type) << "' not recognized" );
		SkipChunk(m_curChunk.Size);
	}
	return true;
}


// Read LWO Polygon Tag Mapping
bool CLwoReader::ReadPolygonTagMapping()
{
	OBJ_ASSERT(m_pCurLayer && m_curChunk.Flag == CLwoFile::CHUNK_PTAG);

	ulong type = ReadLong();
	m_curChunk.Size -= 4;

	MSG_DEBUG("PTAG: '" << GetTypeString(type) << "'");

	CLwoFile::CLayer::FaceVector& faces = m_pCurLayer->GetFaceVector();
	uint faceSize = faces.size();

	int ptagCount = m_curChunk.Size / 4;
	ushort polygon_index;

	// Note: the "&& faces.size()" below are to skip the subchunk if there is no faces in the layer.
	// That can be caused by a non-"FACE" "POLS" list. Like for subpatches "PTCH", "MBAL", "BONE" that we skipped.

	if(type == CLwoFile::CHUNK_SURF && faces.size()) // Surface Tag
	{
		ushort surf_index;
		while(ptagCount--) // for each polygon tag
		{
			polygon_index = ReadShort();
			surf_index = ReadShort();

			OBJ_ASSERT(polygon_index < (int)faces.size());
			if(polygon_index < (int)faces.size())
			{
				faces[polygon_index].m_surface = surf_index;
			}
		}
	}
	else if(type == CLwoFile::CHUNK_PART && faces.size()) // Part Tag
	{
		ushort part_index;
		while(ptagCount--) // for each polygon tag
		{
			polygon_index = ReadShort();
			part_index = ReadShort();

			OBJ_ASSERT(polygon_index < (int)faces.size());
			if(polygon_index < (int)faces.size())
			{
				faces[polygon_index].m_part = part_index;
			}
		}
	}
	else if(type == CLwoFile::CHUNK_SMGP && faces.size()) // Smoothing Group tag
	{
		ushort smoothingGroup;
		while(ptagCount--) // for each polygon tag
		{
			polygon_index = ReadShort();
			smoothingGroup = ReadShort();

			OBJ_ASSERT(polygon_index < (int)faces.size());
			if(polygon_index < (int)faces.size())
			{
				faces[polygon_index].m_smoothingGroup = std::make_pair(true, smoothingGroup);
			}
		}
	}
	else
	{
		// not existing or POLS is not of the FACE type 
		MSG_WARNING("PTAG type '" << GetTypeString(type) << "' not recognized" );
		SkipChunk(m_curChunk.Size);
	}

	return true;
}


// Read LWO Image Definition
bool CLwoReader::ReadImageDefinition()
{
	OBJ_ASSERT(m_pCurLayer && m_curChunk.Flag == CLwoFile::CHUNK_CLIP);

	ulong index = ReadLong();
	m_curChunk.Size -= 4;

	MSG_DEBUG("Image index: " << index);

	while(m_curChunk.Size > 0)
	{
		ulong type = ReadLong();
		m_curChunk.Size -= 4;

		// size of still
		ushort current_tag_size = ReadShort();
		m_curChunk.Size -= 2;

		if(type == CLwoFile::CHUNK_STIL)
		{
			std::string imagePath;
			ReadString(imagePath);
			m_curChunk.Size -= GetStringSize(imagePath);

			m_pCurLayer->AddImage(index, imagePath);
			MSG_DEBUG("\t type: '" << GetTypeString(type) << "', image name: '" << imagePath << "'");
		}
		else
		{
			m_curChunk.Size -= SkipChunk(current_tag_size);
			MSG_DEBUG("\t type: '" << GetTypeString(type) << "', not processed...");
		}
	}

	return true;
}


// Read LWO Surface
bool CLwoReader::ReadSurface()
{
	OBJ_ASSERT(m_curChunk.Flag == CLwoFile::CHUNK_SURF);

	std::string surfaceName;
	ReadString(surfaceName);
	m_curChunk.Size -= GetStringSize(surfaceName); 

	// Create new surface
	CLwoFile::CLayer::CSurface surface;
	surface.SetSurfaceName(surfaceName);

	std::string source;
	ReadString(source);
	m_curChunk.Size -= GetStringSize(source); 

	MSG_DEBUG("SURF: '" << surfaceName << "', " << "source: '" << source << "'");

	long current_tag_name;
	ushort current_tag_size;

	while(m_curChunk.Size > 0 && !m_ifs.eof()) 
	{
		current_tag_name = ReadLong();
		m_curChunk.Size -= 4;
		current_tag_size = ReadShort();
		m_curChunk.Size -= 2;

		MSG_DEBUG("\ttag: '" << GetTagString(current_tag_name) << "', size: " << current_tag_size);

		if(current_tag_name == CLwoFile::CHUNK_BLOK) // BLOK
		{
			int blok_size = current_tag_size;
			m_curChunk.Size -= blok_size;

			while(blok_size > 0 && !m_ifs.eof())
			{
				current_tag_name = ReadLong();
				blok_size -= 4;
				current_tag_size = ReadShort();
				blok_size -= 2;

				MSG_DEBUG("\t\ttag: '" << GetTagString(current_tag_name) << "', size: " << current_tag_size);

				if(current_tag_name == CLwoFile::CHUNK_IMAG) // BLOK | IMAG
				{
					surface.m_imageIndex = ReadShort();
					MSG_DEBUG("\t\t\timage index: " << surface.m_imageIndex);
					blok_size -= 2;
				}
				else if(current_tag_name == CLwoFile::CHUNK_IMAP) // BLOK | IMAP
				{
					int imap_size = current_tag_size;
					blok_size -= imap_size;

					std::string ordinal;
					ReadString(ordinal);
					imap_size -= GetStringSize(ordinal);

					MSG_DEBUG("\t\t\tordinal: '" << ordinal << "'");

					while(imap_size > 0)
					{
						current_tag_name = ReadLong();
						imap_size -= 4;
						current_tag_size = ReadShort();
						imap_size -= 2;

						MSG_DEBUG("\t\t\ttag: '" << GetTagString(current_tag_name) << "', size: " << current_tag_size);
						imap_size -= SkipChunk(current_tag_size);
					}
				}
				else
				{
					MSG_WARNING("Unknown tag in BLOK: '" << GetTagString(current_tag_name) << "'");
					blok_size -= SkipChunk(current_tag_size );
				}
			}
		}
		else if(current_tag_name == CLwoFile::CHUNK_COLR) // COLR
		{
			Vector3D color = ReadVector3D();

			surface.m_vertexColor = color;
			current_tag_size -= 12;
			m_curChunk.Size -= 12;

			MSG_DEBUG("\tcolor: (" << color[0] << "," << color[1] << "," << color[2] << ")" );

			// envelope
			ushort envl = ReadShort();
			current_tag_size -= 2;
			m_curChunk.Size -= 2;
		}
		else
		{
			m_curChunk.Size -= SkipChunk(current_tag_size);
		}
	}

	m_pCurLayer->GetSurfaceMap()[surfaceName] = surface; // Add new surface

	return true;
}



// print 4-char tag to debug out
std::string CLwoReader::GetTagString(uint tag)
{
	std::string sTag;
	sTag += char(tag >> 24);
	sTag += char(tag >> 16);
	sTag += char(tag >> 8);
	sTag += char(tag);
	return sTag;
}

// print 4-char type
std::string CLwoReader::GetTypeString(uint type)
{
	std::string sType;
	sType += char(type >> 24);
	sType += char(type >> 16);
	sType += char(type >> 8);
	sType += char(type);
	return sType;
}


ulong CLwoReader::SkipChunk(ulong size)
{
	ulong newPos = size + size % 2; 
	m_ifs.seekg(newPos, ios::cur);
	return newPos;
}


// Change endianess
CLwoFile::SChunk CLwoReader::ReadChunk()
{
	CLwoFile::SChunk chunk;
	chunk.Flag = ReadLong();
	chunk.Size = ReadLong();
	return chunk;
}


