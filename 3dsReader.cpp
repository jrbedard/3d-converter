
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "3dsReader.h"

using std::ios;


C3dsReader::C3dsReader(CFile* pFile, const fs::path& fileName):
m_curMaterial(0)
{
	C3dsFile* p3dsFile = static_cast<C3dsFile*>(pFile);
	OBJ_ASSERT(p3dsFile);

	m_p3dsFile = p3dsFile;

	m_fileName = fileName;
}


bool C3dsReader::Read()
{
	OBJ_ASSERT(m_p3dsFile);
	if(!m_p3dsFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	m_ifs.open(fileName.c_str(), ios::binary);

	MSG_INFO("Reading .3DS File '" << fileName << "'");
	OBJ_ASSERT(m_ifs.is_open());
	if(!m_ifs.is_open())
	{
		MSG_ERROR("Couldn't open .3DS file '" << fileName << "'");
		return false;
	}

	// Compute the file size
	FileSize();

	// Read the 3DS header here
	if(!ReadHeader())
	{
		OBJ_ASSERT(false);
		return false;
	}

	// Chunk parsing main loop
	ParseChunks();

	// Finish the reading
	EndReading();

	m_ifs.close();

	return true;
}

bool C3dsReader::ReadHeader()
{
	OBJ_ASSERT(m_ifs.is_open());
	if(!m_ifs.is_open())
		return false;

	C3dsFile::SChunk chunk;

	if(!ReadBlock((char*)&chunk, sizeof(C3dsFile::SChunk)))
		return false;

	if(chunk.Flag != C3dsFile::CHUNK_MAIN)
	{
		MSG_ERROR("Invalid 3DS file");
		return false;
	}

	return true;
}


bool C3dsReader::ParseChunks()
{
	OBJ_ASSERT(m_ifs.is_open());
	if(!m_ifs.is_open())
		return false;
	if(m_ifs.eof())
		return false;

	C3dsFile::SChunk chunk;

	if(!ReadBlock((char*)&chunk, sizeof(C3dsFile::SChunk)))
		return false;

	switch(chunk.Flag)
	{

	case C3dsFile::CHUNK_RGBF:
		MSG_DEBUG("Chunk RGBF detected");
		ReadRGBF();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_RGBB:
		MSG_DEBUG("Chunk RGBB detected");
		ReadRGBB();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_MAIN:
		MSG_DEBUG("Chunk Main detected");
		ParseChunks();
		break;
	case C3dsFile::CHUNK_EDITOR:
		MSG_DEBUG("Chunk Editor detected");
		ReadEditorChunk();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_OBJBLOCK:
		MSG_DEBUG("Chunk Object Block detected");
		ReadObjBlock();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_TRIMESH:
		MSG_DEBUG("Chunk Trimesh detected");
		ReadTriMesh();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_VERTLIST:
		MSG_DEBUG("Chunk VertList detected at " << (m_pc - sizeof(C3dsFile::SChunk)));
		ReadVertList();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_FACELIST:
		MSG_DEBUG("Chunk FaceList detected");
		ReadFaceList();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_FACEMAT:
		MSG_DEBUG("Chunk FaceMat detected");
		ReadFaceMat();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_MAPLIST:
		MSG_DEBUG("Chunk MapList detected");
		ReadMapList();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_SMOOLIST:
		MSG_DEBUG("Chunk SmoothList detected");
		ReadSmoothList(chunk.Size);
		ParseChunks();
		break;
	case C3dsFile::CHUNK_TRMATRIX:
		MSG_DEBUG("Chunk TRMatrix detected");
		ReadTrMatrix();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_MATERIAL:
		MSG_DEBUG("Chunk Material detected");
		ParseChunks();
		break;
	case C3dsFile::CHUNK_MATNAME:
		MSG_DEBUG("Chunk MatName detected");
		ReadMatName();
		ParseChunks();
		break;
	case C3dsFile::CHUNK_MAPFILE:
		MSG_DEBUG("Chunk MapFile detected");
		ReadMapFile();
		ParseChunks();
		break;
	default:
		MSG_DEBUG("Unknown Chunk at " << m_ifs.tellg() << " offset");
		SkipChunk(chunk);
		ParseChunks();
		break;
	}
	return true;
}



bool C3dsReader::ReadObjBlock()
{
	char objName[255];
	ReadASCIIZ(objName);

	MSG_DEBUG("Object Name: " << objName);

	C3dsFile::CObject object(objName);
	m_p3dsFile->AddObject(object);
	return true;
}

bool C3dsReader::ReadFaceMat()
{
	short n, nf;
	char szMaterialName[255];

	ReadASCIIZ(szMaterialName);

	if(!ReadBlock((char*) &n, sizeof(n))) 
		return false;

	std::string materialName(szMaterialName);
	UserFaceMaterial(materialName, (int)n);
	uint materialID = m_p3dsFile->FindMaterial(materialName);

	while(n-- > 0) 
	{
		// Material number assigned to any face
		if (!ReadBlock((char*) &nf, sizeof(nf)))
			return false;

		m_p3dsFile->GetLastMesh().GetFaceVector()[nf].m_materialID = materialID;
	}
	return true;
}

bool C3dsReader::ReadSmoothList(long chunkSize)
{
	MSG_DEBUG("Smoothing group...");
	ulong faceCount = (chunkSize - 6) / 4; // minus 6 bytes for the header, 4 bytes per face

	OBJ_ASSERT(faceCount == m_p3dsFile->GetLastMesh().FaceCount());
	C3dsFile::CObject::CMesh::FaceVector& faces = m_p3dsFile->GetLastMesh().GetFaceVector();

	for(uint faceIndex=0; faceIndex < faceCount; ++faceIndex)
	{
		long sg;
		if(!ReadBlock((char*)&sg, sizeof(sg)))
			return false;

		uint smoothingGroup = 0;
		while(sg >>= 1)
		{++smoothingGroup;}

		faces[faceIndex].m_smoothingGroup = std::make_pair(true, smoothingGroup);
	}
	return true;
}


bool C3dsReader::ReadMapFile()
{
	char fileName[255];

	ReadASCIIZ(fileName);
	std::string mapFileName(fileName);
	UserMapFile(mapFileName);

	m_p3dsFile->GetMaterialMap()[m_curMaterial].SetTexMap(mapFileName);

	return true;
}

bool C3dsReader::ReadMatName()
{
	char matName[255];

	ReadASCIIZ(matName);
	std::string materialName(matName);
	UserMatName(materialName);

	m_curMaterial = m_p3dsFile->AddMaterial(materialName);
	return true;
}

bool C3dsReader::ReadTrMatrix()
{
	float rot[9];
	float trans[3];
	if(!ReadBlock((char*) rot, sizeof(rot)))
		return false;
	if(!ReadBlock((char*) trans, sizeof(trans)))
		return false;
	C3dsFile::STransform3dsMatrix Transform;
	C3dsFile::STranslate3dsMatrix Translate;
	Transform._11 = rot[0]; Transform._12 = rot[1]; Transform._13 = rot[2];
	Transform._21 = rot[3]; Transform._22 = rot[4]; Transform._23 = rot[5];
	Transform._31 = rot[6]; Transform._32 = rot[7]; Transform._33 = rot[8];
	Translate._11 = trans[0]; Translate._12 = trans[1]; Translate._13 = trans[2];
	UserTransformMatrix(Transform, Translate);
	return true;
}

bool C3dsReader::ReadTriMesh()
{
	C3dsFile::CObject::CMesh mesh;
	m_p3dsFile->GetLastObject().AddMesh(mesh); // add the mesh to to current object
	return true;
}

bool C3dsReader::ReadVertList()
{
	short nv;
	float v[3];

	if(!ReadBlock((char*) &nv, 2))
		return false;

	while(nv-- > 0) 
	{
		if(!ReadBlock((char*) v, sizeof(v)))
			return false;

		Vector3D vert(3);
		vert[0] = v[0];
		vert[1] = v[1];
		vert[2] = v[2];

		//MSG_DEBUG("X: " << vert[0] << " Y: " << vert[1] << " Z:" << vert[2]);

		m_p3dsFile->GetLastMesh().GetVertexPositionVector().push_back(vert);
	}
	return true;
}

bool C3dsReader::ReadMapList()
{
	short nv;
	float c[2];

	if(!ReadBlock((char*) &nv, sizeof(nv))) 
		return false;

	while(nv-- > 0)
	{
		if(!ReadBlock((char*) c, sizeof(c)))
			return false;

		Vector3D uv(3);
		uv[0] = c[0];
		uv[1] = c[1];
		uv[2] = c[2];

		//MSG_DEBUG("U: " << uv[0] << " V: " << uv[1]);

		m_p3dsFile->GetLastMesh().GetMapCoordVector().push_back(uv);
	}
	return true;
}

bool C3dsReader::ReadFaceList()
{
	ushort nv;
	ushort c[4];

	if(!ReadBlock((char*) &nv, sizeof(nv)))
		return false;

	while(nv-- > 0)
	{
		if(!ReadBlock((char*) c, sizeof(c)))
			return false;

		//MSG_DEBUG("A: " << c[0] << " B: " << c[1] << " C:" << c[2]);
		//MSG_DEBUG(" Flags: " << c[3]);

		C3dsFile::CObject::CMesh::CFace face;
		face.m_vertexIndices.push_back(c[0]);
		face.m_vertexIndices.push_back(c[1]);
		face.m_vertexIndices.push_back(c[2]);
		face.m_flag = c[3];

		if(face.m_flag && 0x0004)
		{
			// mapping for this face
		}

		m_p3dsFile->GetLastMesh().AddFace(face);
	}
	return true;
}


bool C3dsReader::ReadEditorChunk()
{
	return true;
}





bool C3dsReader::ReadRGBF()
{
	float c[3];
	if(!ReadBlock((char*) c, sizeof(c))) 
		return false;

	return true;
}

bool C3dsReader::ReadRGBB()
{
	byte c[3];
	if(!ReadBlock((char*) c, sizeof(c)))
		return false;

	return true;
}

bool C3dsReader::ReadOneUnit()
{
	float OneUnit;
	if(!ReadBlock((char*) &OneUnit, sizeof(OneUnit)))
		return false;

	UserOneUnit(OneUnit);
	return true;
}


bool C3dsReader::SkipChunk(C3dsFile::SChunk& chunk)
{
	m_pc += (chunk.Size - sizeof(C3dsFile::SChunk));
	m_ifs.seekg(m_pc);
	if(m_ifs.eof())
		return false;
	return true;
}





//////////////////////////////////////////////////////////////////////
// DEBUG
//////////////////////////////////////////////////////////////////////



void C3dsReader::UserFaceMaterial(std::string Name, int Number)
{
	MSG_DEBUG("Name: " << Name);
	MSG_DEBUG("Face Number: " << Number);
}

void C3dsReader::UserTransformMatrix(const C3dsFile::STransform3dsMatrix &Transform, const C3dsFile::STranslate3dsMatrix &Translate)
{
	MSG_DEBUG("Transformation Matrix:");
	MSG_DEBUG(Transform._11 << " " << Transform._12 << " " << Transform._13 << " ");
	MSG_DEBUG(Transform._21 << " " << Transform._22 << " " << Transform._23 << " ");
	MSG_DEBUG(Transform._31 << " " << Transform._32 << " " << Transform._33 << " ");
	MSG_DEBUG("Translation Matrix:");
	MSG_DEBUG(Translate._11 << " " << Translate._12 << " " << Translate._13 << " ");
}

void C3dsReader::UserMatName(const std::string Name)
{
	MSG_DEBUG("Material Name: " << Name);
}

void C3dsReader::UserMapFile(const std::string FileName)
{
	MSG_DEBUG("Texture FileName: " << FileName);
}

void C3dsReader::UserEndOfFile()
{
	MSG_DEBUG("EndOfFile");
}

void C3dsReader::UserOneUnit(float Unit)
{
	MSG_DEBUG("One Unit is: " << Unit);
}

