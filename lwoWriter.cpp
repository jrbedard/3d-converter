// LWO File format writer
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter), 2005

#include "StdAfx.h"
#include "lwoWriter.h"


CLwoWriter::CLwoWriter(CFile* pFile, const fs::path& fileName):
m_curLayer(0)
{
	CLwoFile* pLwoFile = static_cast<CLwoFile*>(pFile);
	OBJ_ASSERT(pLwoFile);

	m_pLwoFile = pLwoFile;

	m_fileName = fileName;

	m_bWriteBinary = true;

	IsBigEndian(); // Make the base class call this instead
}



bool CLwoWriter::Write()
{
	OBJ_ASSERT(m_pLwoFile);
	if(!m_pLwoFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	m_ofs.open(fileName.c_str(), ios::out | ios::binary | std::ios::trunc);

	MSG_INFO("Writing LWO file : '" << fileName << "'.");
	if( !m_ofs.is_open() )
	{
		MSG_ERROR("Couldn't write to .LWO file '" << fileName << "'");
		return false;
	}

	// TODO iterate through layers here:
	m_curLayer = m_pLwoFile->GetLayerVector()[0]; // first layer

	// Build the chunk lengths
	BuildChunkLengths();

	// Write the LWO file header
	WriteHeader();

	{
		// Write TAGS strings
		WriteTagStrings();

		// Write LAYR : Support for only one layer
		WriteLayer();

		// Write PNTS
		WritePoints();

		// Write VMAP | TXUV
		WriteVertexMapping();

		// Write POLS | FACE
		WritePolygons();

		// Write PTAG
		WritePolygonTagMapping();

		// Write VMAD | TXUV
		WriteDiscVertexMapping();

		// Write CLIP
		WriteImageDefinitions();

		// Write SURF
		WriteSurfaces();
	}

	MSG_DEBUG("Done.");

	// Close file
	m_ofs.close();

	return true;
}


bool CLwoWriter::WriteHeader()
{
	MSG_DEBUG("FORM | LWO2");

	// "FORM" + size
	WriteChunk(CLwoFile::CHUNK_FORM);

	// "LWO2"
	WriteTag(CLwoFile::CHUNK_LWO2);

	return true;
}


bool CLwoWriter::WriteTagStrings()
{
	MSG_DEBUG("TAGS");

	// "TAGS" + size
	WriteChunk(CLwoFile::CHUNK_TAGS);

	// Surface tag names
	CLwoFile::CLayer::SurfaceMap& surfaces = m_curLayer.GetSurfaceMap();
	CLwoFile::CLayer::SurfaceMap::iterator surfaceIt, surfaceEnd;
	surfaceEnd = surfaces.end();

	for(surfaceIt = surfaces.begin(); surfaceIt != surfaceEnd; ++surfaceIt) // For each surface
	{
		WriteString(surfaceIt->first); // Write surface name
	}

	// Part tag names (OBJ groups)
	CLwoFile::CLayer::PartVector& parts = m_curLayer.GetPartVector();
	CLwoFile::CLayer::PartVector::iterator partIt, partEnd;
	partEnd = parts.end();

	for(partIt = parts.begin(); partIt != partEnd; ++partIt) // For each part name
	{
		WriteString(*partIt); // Write part name
	}

	// Smoothing groups tag names
	CLwoFile::CLayer::SGVector& sgs = m_curLayer.GetSGVector();
	CLwoFile::CLayer::SGVector::iterator sgIt, sgEnd;
	sgEnd = sgs.end();
	std::stringstream ssSG;

	for(sgIt = sgs.begin(); sgIt != sgEnd; ++sgIt) // For each sg name
	{
		ssSG.str("");
		ssSG << "sg";
		if(*sgIt < 10)
			ssSG << "0";

		ssSG << *sgIt;
		WriteString(ssSG.str()); // Write smoothing group name
	}

	return true;
}


// Layer
bool CLwoWriter::WriteLayer()
{
	MSG_DEBUG("LAYR");

	// "LAYR" + size
	WriteChunk(CLwoFile::CHUNK_LAYR);

	// layer ID
	ushort layerID = 0; // hack, only support first layer
	WriteShort(layerID);

	// layer Flag
	ushort layerFlag = 0; // hack
	WriteShort(layerFlag);

	// layer Pivot
	Vector3D pivot(3); // Pivot
	pivot[0] = 0.0f;
	pivot[1] = 0.0f;
	pivot[2] = 0.0f;
	WriteVector3D(pivot);

	// layer Name
	WriteString(""); // Layer Name

	// no parent

	return true;
}


// Point list
bool CLwoWriter::WritePoints()
{
	MSG_DEBUG("PNTS");

	// "PNTS" + size
	WriteChunk(CLwoFile::CHUNK_PNTS);

	Vector3DVector& vertices = m_curLayer.GetVertexPositionVector();
	Vector3DVector::iterator vertexIt, vertexBegin, VertexEnd;
	vertexBegin = vertices.begin();
	VertexEnd = vertices.end();

	for(vertexIt = vertexBegin; vertexIt != VertexEnd; ++vertexIt) // For each LWO point
	{
		WriteVector3D(*vertexIt);
	}

	return true;
}


// Vertex Mapping, We use it in association with VMADs, see WriteDiscVertexMapping below
bool CLwoWriter::WriteVertexMapping()
{
	CLwoFile::CLayer::TexCoordMap& texCoords = m_curLayer.GetTexCoordMap();
	if(texCoords.size() == 0)
		return true;

	MSG_DEBUG("VMAP | TXUV");

	// "VMAP" + size
	WriteChunk(CLwoFile::CHUNK_VMAP);

	// type : "TXUV"
	WriteTag(CLwoFile::CHUNK_TXUV);

	ushort dimension = 2; // UVs are 2D
	WriteShort(dimension);

	std::string name("txuv00");
	WriteString(name);

	CLwoFile::CLayer::TexCoordMap::iterator texCoordIt, texCoordEnd;
	texCoordEnd = texCoords.end();

	ushort pointIndex;
	Vector3D uv(3);

	for(texCoordIt = texCoords.begin(); texCoordIt != texCoordEnd; ++texCoordIt) // For each LWO texCoord
	{
		pointIndex = texCoordIt->first;
		WriteShort(pointIndex); // vertex position index

		uv = (texCoordIt->second)[0].m_texCoord; // Retreive the first UV coordinate for this control point (the one assigned to the first face)
		WriteVector2D(uv); // Write UV
	}

	return true;
}


// Discontinuous Vertex Mapping
bool CLwoWriter::WriteDiscVertexMapping()
{
	CLwoFile::CLayer::TexCoordMap& texCoords = m_curLayer.GetTexCoordMap();
	if(texCoords.size() == 0)
		return true;

	MSG_DEBUG("VMAD | TXUV");

	// "VMAD" + size
	WriteChunk(CLwoFile::CHUNK_VMAD);

	// type : "TXUV"
	WriteTag(CLwoFile::CHUNK_TXUV);

	ushort dimension = 2; // UVs are 2D
	WriteShort(dimension);

	std::string name("txuv00");
	WriteString(name);

	CLwoFile::CLayer::TexCoordMap::iterator texCoordIt, texCoordEnd;
	texCoordEnd = texCoords.end();

	std::vector< CLwoFile::CLayer::CTexCoord > uvVector;
	ushort pointIndex = 0;
	ushort polyIndex = 0;
	Vector3D uv(3);

	for(texCoordIt = texCoords.begin(); texCoordIt != texCoordEnd; ++texCoordIt) // For each LWO texCoord
	{
		uvVector = texCoordIt->second;
		std::vector< CLwoFile::CLayer::CTexCoord >::iterator uvVectorIt, uvVectorEnd;
		uvVectorEnd = uvVector.end();

		for(uvVectorIt = uvVector.begin(); uvVectorIt != uvVectorEnd; ++uvVectorIt) // For each LWO face
		{
			if(uvVectorIt == uvVector.begin()) // skip the first one since it was written in the VMAP
				continue;

			pointIndex = texCoordIt->first;
			WriteShort(pointIndex); // vertex position index

			ushort polyIndex = uvVectorIt->m_faceIndex;
			WriteShort(polyIndex); // face index

			uv = uvVectorIt->m_texCoord;
			WriteVector2D(uv); // Write UV
		}
	}

	return true;
}



// Polygon list
bool CLwoWriter::WritePolygons()
{
	MSG_DEBUG("POLS | FACE");

	// "POLS" + size
	WriteChunk(CLwoFile::CHUNK_POLS);

	// type : "FACE"
	WriteTag(CLwoFile::CHUNK_FACE);

	CLwoFile::CLayer::FaceVector& faces = m_curLayer.GetFaceVector();
	CLwoFile::CLayer::FaceVector::iterator faceIt, faceBegin, faceEnd;
	faceBegin = faces.begin();
	faceEnd = faces.end();

	for(faceIt = faceBegin; faceIt != faceEnd; ++faceIt) // For each LWO face
	{
		ushort vertexCount = faceIt->VertexCount();
		WriteShort(vertexCount);

		for(ushort v=0; v<vertexCount; ++v) // For each vertex in LWO face
		{
			ushort vertexIndex = faceIt->GetVertexIndexVector()[v]; // Retreive vertex index
			WriteShort(vertexIndex);
		}
	}

	return true;
}


// Polygon Tag mapping
bool CLwoWriter::WritePolygonTagMapping()
{
	CLwoFile::CLayer::FaceVector& faces = m_curLayer.GetFaceVector();
	CLwoFile::CLayer::FaceVector::iterator faceIt, faceBegin, faceEnd;
	faceBegin = faces.begin();
	faceEnd = faces.end();

	// Surfaces
	ushort surfaceCount = m_curLayer.GetSurfaceMap().size();
	{
		MSG_DEBUG("PTAG | SURF");

		// "PTAG" + size for surface indices
		WriteChunk(CLwoFile::CHUNK_PTAG);

		// type : "SURF"
		WriteTag(CLwoFile::CHUNK_SURF);

		for(faceIt = faceBegin; faceIt != faceEnd; ++faceIt) // For each LWO face
		{
			// polygon index
			ushort faceIndex = (ushort)(faceIt - faceBegin);
			WriteShort(faceIndex);

			// 0-based index into TAGS chunk
			ushort surfIndex = (ushort)(faceIt->m_surface);
			OBJ_ASSERT(surfIndex == 0 || surfIndex < surfaceCount);

			WriteShort(surfIndex); // Write surface index, refers to TAGS
		}
	}


	// Parts, export this by default?, if only one part?
	ushort partCount = m_curLayer.GetPartVector().size();
	{
		MSG_DEBUG("PTAG | PART");

		// "PTAG" + size for surface indices
		WriteChunk(CLwoFile::CHUNK_PTAG);

		// type : "SURF"
		WriteTag(CLwoFile::CHUNK_PART);

		for(faceIt = faceBegin; faceIt != faceEnd; ++faceIt) // for each LWO face
		{
			// polygon index
			ushort faceIndex = (ushort)(faceIt - faceBegin);
			WriteShort(faceIndex);

			// 0-based index of the part
			ushort partIndex = (ushort)(faceIt->m_part) + surfaceCount;
			OBJ_ASSERT(partIndex == 0 || partIndex < surfaceCount + partCount);

			WriteShort(partIndex); // Write part index, refers to TAGS
		}
	}


	// Smoothing groups
	{
		MSG_DEBUG("PTAG | SMGP");

		// "PTAG" + size for smoothing groups
		WriteChunk(CLwoFile::CHUNK_PTAG);

		// type : "SMGP"
		WriteTag(CLwoFile::CHUNK_SMGP);
		std::stringstream ssSG;

		CLwoFile::CLayer::SGVector& sgs = m_curLayer.GetSGVector();

		for(faceIt = faceBegin; faceIt != faceEnd; ++faceIt) // For each LWO face
		{
			// polygon index
			ushort faceIndex = (ushort)(faceIt - faceBegin);
			WriteShort(faceIndex);

			// write Smoothing group index
			ushort sgIndex = (ushort)(faceIt->m_smoothingGroup.second);

			CLwoFile::CLayer::SGVector::iterator sgIt;
			CLwoFile::CLayer::SGVector::iterator sgEnd = sgs.end();
			ushort sgSetPos = 0;
			for(sgIt = sgs.begin(); sgIt != sgEnd; ++sgIt) // For each SG
			{
				if(sgIndex == *sgIt)
				{
					sgIndex = sgSetPos + surfaceCount + partCount;
					WriteShort(sgIndex); // Write smoothing group, refers to TAGS
					break;
				}
				++sgSetPos;
			}
		}
	}

	return true;
}


// Image definition
bool CLwoWriter::WriteImageDefinitions()
{
	CLwoFile::CLayer::ImageVector& images = m_curLayer.GetImageVector();
	CLwoFile::CLayer::ImageVector::iterator imageBegin, imageIt, imageEnd;
	imageBegin = images.begin();
	imageEnd = images.end();

	for(imageIt = imageBegin; imageIt != imageEnd; ++imageIt) // For each LWO face
	{
		std::string imagePath(*imageIt);
		ushort still_size = (ushort)GetStringSize(imagePath);

		MSG_DEBUG("CLIP | STIL: '" << imagePath << "'");

		// "CLIP" + size
		WriteTag(CLwoFile::CHUNK_CLIP);
		WriteLong(4 + 4 + 2 + still_size); // imageIndex + "STIL" + stilSize + pathString 

		ulong imageIndex = (imageIt - imageBegin);
		WriteLong(imageIndex); // image index : 0-based index

		WriteTag(CLwoFile::CHUNK_STIL); // STIL
		WriteShort(still_size);

		// Write the image path
		WriteString(imagePath);
	}

	return true;
}


// Surface
bool CLwoWriter::WriteSurfaces()
{
	// surface tag names
	CLwoFile::CLayer::SurfaceMap& surfaces = m_curLayer.GetSurfaceMap();
	CLwoFile::CLayer::SurfaceMap::iterator surfaceIt, surfaceEnd;
	surfaceEnd = surfaces.end();


	for(surfaceIt = surfaces.begin(); surfaceIt != surfaceEnd; ++surfaceIt) // For each surface
	{
		std::string surfaceName(surfaceIt->first);
		CLwoFile::CLayer::CSurface& surface = surfaceIt->second;

		MSG_DEBUG("SURF: '" << surfaceName << "'");

		// "SURF" + size
		WriteTag(CLwoFile::CHUNK_SURF);
		WriteLong(surface.m_size); // hack for surface, in fact, it should be like this for everything

		// surface name
		OBJ_ASSERT(surfaceName == surface.GetSurfaceName());
		WriteString(surfaceName);

		// parent/source name
		std::string parentLayerName("");
		WriteString(parentLayerName);


		// SURF | COLR
		{
			MSG_DEBUG("SURF | COLR");

			WriteTag(CLwoFile::CHUNK_COLR);
			ushort colr_size = (ushort)(m_chunkLengthMap[CLwoFile::CHUNK_COLR]);
			WriteShort(colr_size);

			// Color
			Vector3D color(3);
			color[0] = 0.78431f;
			color[1] = 0.78431f;
			color[2] = 0.78431f;
			WriteVector3D(color);

			// Envl
			ushort envl = 0;
			WriteShort(envl);
		}

		// SURF | DIFF
		{
			MSG_DEBUG("SURF | DIFF");

			WriteTag(CLwoFile::CHUNK_DIFF);
			ushort diff_size = (ushort)(m_chunkLengthMap[CLwoFile::CHUNK_DIFF]);
			WriteShort(diff_size);

			float diffuse = 1.0f;
			WriteFloat(diffuse);

			ushort lumi = 0;
			WriteShort(lumi);
		}


		// SURF | BLOK
		if(surface.m_imageIndex != -1) // there is an image index
		{
			MSG_DEBUG("SURF | BLOK");

			// BLOK
			WriteTag(CLwoFile::CHUNK_BLOK);
			WriteShort((ushort)m_chunkLengthMap[CLwoFile::CHUNK_BLOK]);


			// SURF | BLOK | IMAP
			{
				MSG_DEBUG("SURF | BLOK | IMAP");

				// IMAP
				WriteTag(CLwoFile::CHUNK_IMAP);
				m_chunkLengthMap[CLwoFile::CHUNK_IMAP] = 50; 
				WriteShort((ushort)m_chunkLengthMap[CLwoFile::CHUNK_IMAP]); // IMAP size

				{
					ushort ordStrCmp = 0x8000;
					WriteShort(ordStrCmp); // Ordinal string compare function

					// CHAN
					WriteTag(CLwoFile::CHUNK_CHAN);
					WriteShort(4); // CHAN size
					WriteTag(CLwoFile::CHUNK_COLR);

					// OPAC
					WriteTag(CLwoFile::CHUNK_OPAC);
					WriteShort(8); // OPAC size
					WriteShort(0);
					WriteFloat(1.0f);
					WriteShort(0);

					// ENAB
					WriteTag(CLwoFile::CHUNK_ENAB);
					WriteShort(2); // ENAB size
					WriteShort(1);

					// NEGA
					WriteTag(CLwoFile::CHUNK_NEGA);
					WriteShort(2); // NEGA size
					WriteShort(0);

					// AXIS
					WriteTag(CLwoFile::CHUNK_AXIS);
					WriteShort(2); // AXIS size
					WriteShort(1);
				}
			}

			// SURF | BLOK | TMAP
			{
				MSG_DEBUG("SURF | BLOK | TMAP");

				// TMAP
				WriteTag(CLwoFile::CHUNK_TMAP);
				ushort imap_size = 104;
				WriteShort(imap_size);

				{
					// CNTR
					WriteTag(CLwoFile::CHUNK_CNTR);
					WriteShort(14); // CHAN size
					Vector3D center(3);
					WriteVector3D(center);
					WriteShort(0);

					// SIZE
					WriteTag(CLwoFile::CHUNK_SIZE);
					WriteShort(14); // SIZE size
					Vector3D size(3);
					size[0] = 1.0f;
					size[1] = 1.0f;
					size[2] = 1.0f;
					WriteVector3D(size);
					WriteShort(0);

					// ROTA
					WriteTag(CLwoFile::CHUNK_ROTA);
					WriteShort(14); // ROTA size
					Vector3D rota(3);
					WriteVector3D(rota);
					WriteShort(0);

					// FALL
					WriteTag(CLwoFile::CHUNK_FALL);
					WriteShort(16); // FALL size
					WriteShort(0);
					Vector3D fall(3);
					WriteVector3D(fall);
					WriteShort(0);

					// OREF
					WriteTag(CLwoFile::CHUNK_OREF);
					WriteShort(8); // OREF size
					WriteString(std::string("(none)"));

					// CSYS
					WriteTag(CLwoFile::CHUNK_CSYS);
					WriteShort(2); // CSYS size
					WriteShort(0);
				}
			}

			// PROJ : projection
			WriteTag(CLwoFile::CHUNK_PROJ);
			WriteShort(2); // PROJ size
			WriteShort(5); // UV projection mapping

			// AXIS
			WriteTag(CLwoFile::CHUNK_AXIS);
			WriteShort(2); // AXIS size
			WriteShort(2);

			// IMAG
			WriteTag(CLwoFile::CHUNK_IMAG);
			WriteShort(2); // IMAG size
			WriteShort(surface.m_imageIndex); // Write the image index corresponding to imageVector

			// WRAP
			WriteTag(CLwoFile::CHUNK_WRAP);
			WriteShort(4); // WRAP size
			WriteShort(1);
			WriteShort(1);

			// WRPW
			WriteTag(CLwoFile::CHUNK_WRPW);
			WriteShort(6); // WRPW size
			WriteFloat(1.0f);
			WriteShort(0);

			// WRPH
			WriteTag(CLwoFile::CHUNK_WRPH);
			WriteShort(6); // WRPH size
			WriteFloat(1.0f);
			WriteShort(0);

			// VMAP
			WriteTag(CLwoFile::CHUNK_VMAP);
			WriteShort(8); // VMAP size
			WriteString("txuv00"); // TODO : increment for multiple meshes?

			// AAST
			WriteTag(CLwoFile::CHUNK_AAST);
			WriteShort(6); // AAST size
			WriteShort(1);
			WriteFloat(1.0f);

			// PIXB
			WriteTag(CLwoFile::CHUNK_PIXB);
			WriteShort(2); // PIXB size
			WriteShort(1);
		}
	}

	return true;
}


// Chunk doesnt inlcude its tag and size size
void CLwoWriter::BuildChunkLengths()
{
	MSG_DEBUG("Building LWO chunks length...");

	ulong tagsSize = 0;
	{
		// Surfaces
		CLwoFile::CLayer::SurfaceMap& surfaces = m_curLayer.GetSurfaceMap();
		CLwoFile::CLayer::SurfaceMap::iterator surfaceIt, surfaceEnd;
		surfaceEnd = surfaces.end();

		for(surfaceIt = surfaces.begin(); surfaceIt != surfaceEnd; ++surfaceIt) // For each part name
			tagsSize += GetStringSize(surfaceIt->first);

		// Parts
		CLwoFile::CLayer::PartVector& parts = m_curLayer.GetPartVector();
		CLwoFile::CLayer::PartVector::iterator partIt, partEnd;
		partEnd = parts.end();

		for(partIt = parts.begin(); partIt != partEnd; ++partIt) // For each part name
			tagsSize += GetStringSize(*partIt);

		// Smoothing groups
		CLwoFile::CLayer::SGVector& sgs = m_curLayer.GetSGVector();
		tagsSize += sgs.size() * GetStringSize("sgXX");
	}

	ulong vertexPosCount = m_curLayer.GetVertexPositionVector().size();
	ulong vmapTexCount = 0;
	ulong vmadTexCount = 0;
	{
		CLwoFile::CLayer::TexCoordMap& texCoords = m_curLayer.GetTexCoordMap();
		CLwoFile::CLayer::TexCoordMap::iterator texCoordIt, texCoordEnd;
		texCoordEnd = texCoords.end();

		std::vector< CLwoFile::CLayer::CTexCoord > texCoordVector;

		for(texCoordIt = texCoords.begin(); texCoordIt != texCoordEnd; ++texCoordIt) // For each LWO texCoord
		{
			texCoordVector = texCoordIt->second;
			std::vector< CLwoFile::CLayer::CTexCoord >::iterator texCoordVectorIt, texCoordVectorEnd;
			texCoordVectorEnd = texCoordVector.end();

			for(texCoordVectorIt = texCoordVector.begin(); texCoordVectorIt != texCoordVectorEnd; ++texCoordVectorIt) // For each LWO face
			{
				if(texCoordVectorIt == texCoordVector.begin()) // skip the first one since its in the VMAP
				{
					++vmapTexCount;
					continue;
				}

				++vmadTexCount;
			}
		}
	}


	// Indexed vertices count
	ulong faceCount = 0;
	ulong vertexPolyCount = 0;
	{
		CLwoFile::CLayer::FaceVector& faces =  m_curLayer.GetFaceVector();
		faceCount = faces.size();
		CLwoFile::CLayer::FaceVector::iterator faceIt, faceBegin, faceEnd;
		faceBegin = faces.begin();
		faceEnd = faces.end();

		for(faceIt = faceBegin; faceIt != faceEnd; ++faceIt) // for each LWO point
			vertexPolyCount += faceIt->VertexCount();
	}



	//Images
	ulong imageCount = 0;
	ulong imagesSize = 0;
	{
		CLwoFile::CLayer::ImageVector& images = m_curLayer.GetImageVector();
		imageCount = images.size();
		CLwoFile::CLayer::ImageVector::iterator imageIt, imageEnd;
		imageEnd = images.end();

		imageIt = images.begin(); // first image only
		for(imageIt = images.begin(); imageIt != imageEnd; ++imageIt) // for each LWO image
		{
			imagesSize += 4; // index of the image
			imagesSize += 6; // "STIL" + size of still
			imagesSize += GetStringSize(*imageIt); // size of path
		}
	}


	// TAGS (every surface names + part names + sg names)
	m_chunkLengthMap[CLwoFile::CHUNK_TAGS] = tagsSize;

	// LAYR
	m_chunkLengthMap[CLwoFile::CHUNK_LAYR] = 18; // blank layer

	// PNTS
	m_chunkLengthMap[CLwoFile::CHUNK_PNTS] = vertexPosCount * 12; // number of vertex in this mesh * 12

	// VMAP | TXUV
	m_chunkLengthMap[CLwoFile::CHUNK_VMAP] = 4 + 2 + 8 + vmapTexCount * (2+8); // "TXUV" + dimension(short) + "txuv00\0\0" + vertexPos*(vIndex+UV)

	// VMAD | TXUV
	m_chunkLengthMap[CLwoFile::CHUNK_VMAD] = 4 + 2 + 8 + vmadTexCount * (2+2+8); // "TXUV" + dimension(short) + "txuv00\0\0" + texCoordCount*(pointIndex+polyIndex+UV)

	// POLS
	m_chunkLengthMap[CLwoFile::CHUNK_POLS] = 4 + vertexPolyCount * 2 + faceCount * 2; // "FACE" + for each face : vertex Number(short) + index per vertex(short) 

	// PTAG
	m_chunkLengthMap[CLwoFile::CHUNK_PTAG] = 4 + faceCount * 4;  // "SURF" or "PART" or "SMGP" + face number * 4

	// CLIP
	m_chunkLengthMap[CLwoFile::CHUNK_CLIP] = imagesSize; // "STIL" sub-chunk

	// SURF constant sub-chunks
	{
		m_chunkLengthMap[CLwoFile::CHUNK_BLOK] = (6+50) + (6+104) + (92); // IMAP + TMAP + rest = 244
		m_chunkLengthMap[CLwoFile::CHUNK_COLR] = 14;
		m_chunkLengthMap[CLwoFile::CHUNK_DIFF] = 6;
	}

	// Surfaces
	ulong surfacesSize = 0;
	CLwoFile::CLayer::SurfaceMap& surfaces = m_curLayer.GetSurfaceMap();
	ulong surfaceCount = surfaces.size();
	CLwoFile::CLayer::SurfaceMap::iterator surfaceIt, surfaceEnd;
	surfaceEnd = surfaces.end();

	for(surfaceIt = surfaces.begin(); surfaceIt != surfaceEnd; ++surfaceIt) // For each surface
	{
		std::string surfaceName(surfaceIt->first);
		CLwoFile::CLayer::CSurface& surface = surfaceIt->second;

		bool bHasTex = (surface.m_imageIndex != -1);

		m_chunkLengthMap[CLwoFile::CHUNK_SURF] = GetStringSize(surfaceName) + 2 + // surface name + parent name "\0\0" 
									  (bHasTex ? (6 + m_chunkLengthMap[CLwoFile::CHUNK_BLOK]):0) +
												 (6 + m_chunkLengthMap[CLwoFile::CHUNK_COLR]) +
												 (6 + m_chunkLengthMap[CLwoFile::CHUNK_DIFF]);

		surface.m_size = m_chunkLengthMap[CLwoFile::CHUNK_SURF];
		surfacesSize += m_chunkLengthMap[CLwoFile::CHUNK_SURF];
	}



	// FORM
	m_chunkLengthMap[CLwoFile::CHUNK_FORM] = (4) + // LWO2
											 (8 + m_chunkLengthMap[CLwoFile::CHUNK_TAGS]) + // "TAGS" + size + chunk...
											 (8 + m_chunkLengthMap[CLwoFile::CHUNK_LAYR]) + // "LAYR"
											 (8 + m_chunkLengthMap[CLwoFile::CHUNK_PNTS]) + // "PNTS"

					    (vmapTexCount > 0 ? (8 + m_chunkLengthMap[CLwoFile::CHUNK_VMAP]):0) + // "VMAP"
					    (vmapTexCount > 0 ? (8 + m_chunkLengthMap[CLwoFile::CHUNK_VMAD]):0) + // "VMAD" // If VMAP, sure to have VMAD!

											 (8 + m_chunkLengthMap[CLwoFile::CHUNK_POLS]) + // "POLS"

											 (8 + m_chunkLengthMap[CLwoFile::CHUNK_PTAG]) + // ("PTAG" + size) + "SURF" + data
											 (8 + m_chunkLengthMap[CLwoFile::CHUNK_PTAG]) + // "PART"
											 (8 + m_chunkLengthMap[CLwoFile::CHUNK_PTAG]) + // "SMGP"

											 imageCount*8 + m_chunkLengthMap[CLwoFile::CHUNK_CLIP] + // "CLIP"

											 surfaceCount*8 + surfacesSize; // "SURF"
}



void CLwoWriter::WriteChunk(ulong chunkName)
{
	ulong chunkTag = chunkName;
	ulong chunkLength = m_chunkLengthMap[chunkName];
	WriteLong(chunkTag);
	WriteLong(chunkLength);
}

void CLwoWriter::WriteTag(ulong tagName)
{
	ulong tag = tagName;
	WriteLong(tag);
}



