
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "3dsWriter.h"


C3dsWriter::C3dsWriter(CFile* pFile, const fs::path& fileName)
{
	C3dsFile* p3dsFile = static_cast<C3dsFile*>(pFile);
	OBJ_ASSERT(p3dsFile);

	m_p3dsFile = p3dsFile;

	m_fileName = fileName;

	m_bWriteBinary = true;
}



void C3dsWriter::WriteChunk(C3dsFile::CHUNK chunkName)
{
	ushort chunkID = chunkName;
	WriteShort(chunkID);

	ulong chunkLength = m_chunkLengthMap[chunkName];
	WriteLong(chunkLength);
}



bool C3dsWriter::Write()
{
	OBJ_ASSERT(m_p3dsFile);
	if(!m_p3dsFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	m_ofs.open(fileName.c_str(), ios::out | ios::binary);

	MSG_INFO("Writing 3DS file : '" << fileName << "'.");
	if( !m_ofs.is_open() )
	{
		MSG_ERROR("Couldn't write to .3DS file '" << fileName << "'");
		return false;
	}

	// Build the chunk lengths
	BuildChunkLengths();

	// Main
	WriteChunk(C3dsFile::CHUNK_MAIN);

	// Version
	WriteChunk(C3dsFile::CHUNK_VERSION);
	ulong version1 = 3;
	WriteLong(version1);
	
	// Editor
	WriteChunk(C3dsFile::CHUNK_EDITOR);

	// Config
	WriteChunk(C3dsFile::CHUNK_CONFIG);
	ulong version2 = 3;
	WriteLong(version2);

	// One unit
	WriteChunk(C3dsFile::CHUNK_ONEUNIT);
	float unit = 1.0f;
	WriteFloat(unit);


	// Get the 3DS object vector
	C3dsFile::ObjectVector& objects = m_p3dsFile->GetObjectVector();
	C3dsFile::ObjectVector::iterator objectIt;
	C3dsFile::ObjectVector::const_iterator objectEnd = objects.end();

	for(objectIt = objects.begin(); objectIt != objectEnd; ++objectIt) // for each 3DS object
	{
		// valid object?
		if(objectIt->GetMeshVector().size() <= 0)
			continue;

		std::string objectName(objectIt->GetObjectName());

		WriteChunk(C3dsFile::CHUNK_OBJBLOCK);
		WriteString(objectName);

		C3dsFile::CObject::MeshVector& meshes = objectIt->GetMeshVector();
		C3dsFile::CObject::MeshVector::iterator meshIt;
		C3dsFile::CObject::MeshVector::const_iterator meshEnd = meshes.end();

		for( meshIt = meshes.begin(); meshIt != meshEnd; ++meshIt) // For each 3DS mesh
		{
			WriteChunk(C3dsFile::CHUNK_TRIMESH);

			WriteChunk(C3dsFile::CHUNK_VERTLIST);

			ushort vertexCount = meshIt->VertexCount();
			OBJ_ASSERT( vertexCount < SIZE_USHORT); // max number of vertices
			WriteShort(vertexCount);


			for(ushort v=0; v < vertexCount; ++v) // For each vertex in 3DS mesh
			{
				Vector3D vertex = meshIt->GetVertexPositionVector()[v];
				WriteVector3D(vertex);
			}


			WriteChunk(C3dsFile::CHUNK_MAPLIST);
			WriteShort(vertexCount); // Number of UV coords = vertex count

			for(ushort v=0; v < vertexCount; ++v) // 1 to 1 vertices - UV ?
			{
				Vector3D uv = meshIt->GetMapCoordVector()[v];
				WriteVector2D(uv);
			}


			WriteChunk(C3dsFile::CHUNK_TRMATRIX);

			float TransformMatrix[4][4];
			TransformMatrix[0][0] = 1.0f;	TransformMatrix[0][1] = 0.0f;	TransformMatrix[0][2] = 0.0f;
			TransformMatrix[1][0] = 0.0f;	TransformMatrix[1][1] = 1.0f;	TransformMatrix[1][2] = 0.0f;
			TransformMatrix[2][0] = 0.0f;	TransformMatrix[2][1] = 0.0f;	TransformMatrix[2][2] = 1.0f;
			TransformMatrix[3][0] = 0.001f; TransformMatrix[3][1] = 0.001f; TransformMatrix[3][2] = 0.001f;

			for(uint i = 0; i < 4; ++i)
			{
				for(uint j = 0; j < 3; ++j)
				{
					float coord = TransformMatrix[i][j];
					WriteFloat(coord);
				}
			}

			WriteChunk(C3dsFile::CHUNK_FACELIST);

			ushort faceCount = meshIt->FaceCount();
			OBJ_ASSERT( faceCount < SIZE_USHORT); // max number of vertices
			WriteShort(faceCount);

			for(ushort f=0; f < faceCount; ++f) // for each 3DS face
			{
				C3dsFile::CObject::CMesh::CFace face = meshIt->GetFaceVector()[f];

				ushort vertexCount = face.VertexCount();
				OBJ_ASSERT(vertexCount == 3); // triangle only in 3DS files

				for(ushort v=0; v < vertexCount; ++v) // for each 3DS vertex
				{
					ushort vertexIndex = face.m_vertexIndices[v];
					WriteShort(vertexIndex);
				}

				ushort faceFlag = face.m_flag;
				WriteShort(faceFlag);
			}

			WriteChunk(C3dsFile::CHUNK_SMOOLIST);

			for(ushort f=0; f < faceCount; ++f) // for each 3DS face
			{
				ulong smoothingGroup = 1; // hacked smoothing group
				WriteLong(smoothingGroup);
			}
		}


		WriteChunk(C3dsFile::CHUNK_KEYFRAMER);

		// KEYFUNKN1
		{
			WriteChunk(C3dsFile::CHUNK_KEYFUNKN1);
			WriteShort(5);
			WriteString(std::string("MAXSCENE")); // will write a '\0' ??
			WriteShort(100);
			WriteShort(0);
		}

		// FRAMES
		{
			WriteChunk(C3dsFile::CHUNK_FRAMES);
			WriteLong(0);
			WriteLong(100);
		}

		// KEYFUNKN2
		{
			WriteChunk(C3dsFile::CHUNK_KEYFUNKN2);
			WriteLong(0);
		}

		// TRACKINFO
		{
			WriteChunk(C3dsFile::CHUNK_TRACKINFO);
		}

		// OBJNUMBER
		{
			WriteChunk(C3dsFile::CHUNK_OBJNUMBER);
			WriteShort(0);
		}

		// TRACKOBJNAME
		{
			WriteChunk(C3dsFile::CHUNK_TRACKOBJNAME);
			WriteString(objectName);
			WriteShort(16384);
			WriteShort(0);
			WriteShort(-1);
		}

		// TRACKPIVOT
		{
			WriteChunk(C3dsFile::CHUNK_TRACKPIVOT);
			for(uint i = 0; i < 3; ++i)
			{
				float pivot = 0.0f;
				WriteFloat(pivot);
			}
		}

		// TRACKPOS
		{
			WriteChunk(C3dsFile::CHUNK_TRACKPOS);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(1);
			WriteShort(0);
			WriteShort(0);
			WriteLong(0);

			for( uint i = 0; i < 3; i++ )
			{
				float origin = 0.0f;
				WriteFloat(origin);
			}
		}

		// TRACKROTATE
		{
			WriteChunk(C3dsFile::CHUNK_TRACKROTATE);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(1);
			WriteShort(0);
			WriteShort(0);
			WriteLong(0);

			WriteFloat(0.0f);
			WriteFloat(0.0f);
			WriteFloat(0.0f);
			WriteFloat(0.0f);
		}

		// TRACKSCALE
		{
			WriteChunk(C3dsFile::CHUNK_TRACKSCALE);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(0);
			WriteShort(1);
			WriteShort(0);
			WriteShort(0);
			WriteLong(0);

			WriteFloat(1.0f);
			WriteFloat(1.0f);
			WriteFloat(1.0f);
		}

	}

	m_ofs.close();

	MSG_DEBUG("Done.");

	return true;
}



void C3dsWriter::BuildChunkLengths()
{
	uint name_length = m_p3dsFile->GetLastObject().GetObjectName().size()+1; // + 1 for the '\0'
	uint face_num = m_p3dsFile->GetLastMesh().FaceCount();
	uint vertex_num = m_p3dsFile->GetLastMesh().VertexCount();

	// TODO : iterate objects!

	m_chunkLengthMap[C3dsFile::CHUNK_VERSION] = 10;


	m_chunkLengthMap[C3dsFile::CHUNK_SMOOLIST] = 2 + 4 + face_num * 4;
	//m_L4130 = 2 + 4 + face_num * ?; // material

	m_chunkLengthMap[C3dsFile::CHUNK_FACELIST] = 2 + 4 + 2 + 4 * face_num * 2 + m_chunkLengthMap[C3dsFile::CHUNK_SMOOLIST];
	m_chunkLengthMap[C3dsFile::CHUNK_TRMATRIX] = 2 + 4 + 4 * 12;
	m_chunkLengthMap[C3dsFile::CHUNK_VERTLIST] = 2 + 4 + 2 + vertex_num * 3 * 4;
	m_chunkLengthMap[C3dsFile::CHUNK_MAPLIST]  = 2 + 4 + 2 + vertex_num * 2 * 4;
	m_chunkLengthMap[C3dsFile::CHUNK_TRIMESH]  = 2 + 4 + m_chunkLengthMap[C3dsFile::CHUNK_VERTLIST] + m_chunkLengthMap[C3dsFile::CHUNK_TRMATRIX] + m_chunkLengthMap[C3dsFile::CHUNK_MAPLIST] + m_chunkLengthMap[C3dsFile::CHUNK_FACELIST];
	m_chunkLengthMap[C3dsFile::CHUNK_OBJBLOCK] = 2 + 4 + ( name_length ) + m_chunkLengthMap[C3dsFile::CHUNK_TRIMESH];

	m_chunkLengthMap[C3dsFile::CHUNK_ONEUNIT] = 2 + 4 + 4;
	m_chunkLengthMap[C3dsFile::CHUNK_CONFIG]  = 2 + 4 + 4;
	m_chunkLengthMap[C3dsFile::CHUNK_EDITOR]  = 2 + 4 + m_chunkLengthMap[C3dsFile::CHUNK_CONFIG] + m_chunkLengthMap[C3dsFile::CHUNK_ONEUNIT] + m_chunkLengthMap[C3dsFile::CHUNK_OBJBLOCK];

	m_chunkLengthMap[C3dsFile::CHUNK_TRACKSCALE]   = 2 + 4 + 32;
	m_chunkLengthMap[C3dsFile::CHUNK_TRACKROTATE]  = 2 + 4 + 9 * 4;
	m_chunkLengthMap[C3dsFile::CHUNK_TRACKPOS]     = 2 + 4 + 8 * 4;
	m_chunkLengthMap[C3dsFile::CHUNK_TRACKPIVOT]   = 2 + 4 + 6 * 2;
	m_chunkLengthMap[C3dsFile::CHUNK_TRACKOBJNAME] = 2 + 4 + ( name_length ) + 3 * 2;
	m_chunkLengthMap[C3dsFile::CHUNK_OBJNUMBER]    = 2 + 4 + 2;
	m_chunkLengthMap[C3dsFile::CHUNK_TRACKINFO]    = 2 + 4 + m_chunkLengthMap[C3dsFile::CHUNK_OBJNUMBER] + m_chunkLengthMap[C3dsFile::CHUNK_TRACKOBJNAME] + m_chunkLengthMap[C3dsFile::CHUNK_TRACKPIVOT] + m_chunkLengthMap[C3dsFile::CHUNK_TRACKPOS] + m_chunkLengthMap[C3dsFile::CHUNK_TRACKROTATE] + m_chunkLengthMap[C3dsFile::CHUNK_TRACKSCALE];

	m_chunkLengthMap[C3dsFile::CHUNK_KEYFUNKN2] = 2 + 4 + 4;
	m_chunkLengthMap[C3dsFile::CHUNK_FRAMES]    = 2 + 4 + 2 * 4;
	m_chunkLengthMap[C3dsFile::CHUNK_KEYFUNKN1] = 2 + 4 + 2 + 9 + 2 * 2;
	m_chunkLengthMap[C3dsFile::CHUNK_KEYFRAMER] = 2 + 4 + m_chunkLengthMap[C3dsFile::CHUNK_KEYFUNKN1] + m_chunkLengthMap[C3dsFile::CHUNK_FRAMES] + m_chunkLengthMap[C3dsFile::CHUNK_KEYFUNKN2] + m_chunkLengthMap[C3dsFile::CHUNK_TRACKINFO];

	m_chunkLengthMap[C3dsFile::CHUNK_MAIN] = 2 + 4 + m_chunkLengthMap[C3dsFile::CHUNK_EDITOR] + m_chunkLengthMap[C3dsFile::CHUNK_VERSION] + m_chunkLengthMap[C3dsFile::CHUNK_KEYFRAMER];
}



