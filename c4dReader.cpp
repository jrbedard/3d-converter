
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "c4dReader.h"

using std::ios;



CC4dReader::CC4dReader(CFile* pFile, const fs::path& fileName)
{
	CC4dFile* pC4dFile = static_cast<CC4dFile*>(pFile);

	m_pC4dFile = pC4dFile;

	m_fileName = fileName;
}



bool CC4dReader::Read()
{
	OBJ_ASSERT(m_pC4dFile);
	if(!m_pC4dFile)
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

	FileSize();

	char version; // ?
	ReadBlock((char*)&version, sizeof(version));

	char c4dc4d[6];
	ReadBlock((char*)c4dc4d, sizeof(c4dc4d));


	//int form;

	//if(!ReadBlock((char*) &chunk, sizeof(C3dsFile::SChunk)))
	//	return false;


	return true;
}

/*
bool CC4dReader::LoadC4D(CHAR* name)
{
	file = FOpen(name);
	if (!file)
		return false;

	if (ReadLONG()!=FORM)
		return false;
	
	Seek(file,4); // Skip file length value
	
	if (ReadLONG()!=MC4D)
		return false;

	while (!FileEnd())
	{
		chunkname = ReadLONG();
		chunksize = ReadLONG();

		if (chunkname==PLTF)
			platform = ReadLONG(); // Read platform chunk
		else if (chunkname==OBJ5)
			LoadObjects(); // Read object chunk
		else if (chunkname==MAT4)
			LoadMaterials(); // Read material chunk
		else if (chunkname==UMG4)
			LoadEnvironment(); // Read environment chunk
		else
			FSkip(chunksize); // Skip unknown chunk

		if (Odd(FPos()))) // IFF specification: pad byte at
			FSkip(1); // the end of odd chunk length
	}
	return true;
}
*/


bool CC4dReader::SkipChunk(CC4dFile::SChunkC4d &chunk)
{
	m_pc += (chunk.Size - sizeof(CC4dFile::SChunkC4d));
	m_ifs.seekg(m_pc);
	if(m_ifs.eof())
		return false;
	return true;
}


/*
void C3dsReader::LoadMaterials()
{
	CHAR strlen;
	WORD id=MAT_NEXT,len;
	Material *mat;
	while (id!=MAT_END)
	{
		mat = AllocMaterial();
		do
		{
			id = ReadWORD();
			len = ReadWORD();
			switch (id)
			{
			case MAT_NAME:
				strlen = ReadCHAR();
				mat->name = ReadNBYTES(strlen);
				break;
			case MAT_FACTIVE:
				mat->bf = ReadCHAR();
				break;
			case MAT_LACTIVE:
				mat->bl = ReadCHAR();
				break;
			case MAT_TACTIVE:
				mat->bt = ReadCHAR();
				break;
			case MAT_SACTIVE:
				mat->bs = ReadCHAR();
				break;
			case MAT_UACTIVE:
				mat->bu = ReadCHAR();
				break;
			case MAT_NACTIVE:
				mat->bn = ReadCHAR();
				break;
			case MAT_RACTIVE:
				mat->br = ReadCHAR();
				break;
			case MAT_GACTIVE:
				mat->bg = ReadCHAR();
				break;
			case MAT_HACTIVE:
				mat->bh = ReadCHAR();
				break;
			case MAT_ZACTIVE:
				mat->bz = ReadCHAR();
				break;
			case MAT_FCOLOUR:
				mat->f = ReadVECTOR();
				break;
			case MAT_LCOLOUR:
				mat->l = ReadVECTOR();
				break;
			case MAT_TCOLOUR:
				mat->t = ReadVECTOR();
				break;
			case MAT_SCOLOR:
				mat->f = ReadVECTOR();
				ReadREAL(u,&mat->s,3);
				break;
			case MAT_UCOLOUR:
				mat->u = ReadVECTOR();
				break;
			case MAT_NCOLOUR:
				mat->n = ReadVECTOR();
				break;
			case MAT_GCOLOUR:
				mat->g = ReadVECTOR();
}
*/



