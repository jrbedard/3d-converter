
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __C4DREADER_H__
#define __C4DREADER_H__

#include <stdio.h>

#include "binaryReader.h"
#include "c4dFile.h"


namespace ZBPlugin
{

class CFile;

// CC4dReader: C4D file format reader
class CC4dReader : public CBinaryReader
{
public:
	CC4dReader(CFile* pFile, const fs::path& fileName);

private:
	CC4dReader(const CC4dReader&); // Copy constructor
	void operator=(const CC4dReader&); // Assignment operator

public:
	bool Read();
	bool ReadDebug(){return true;}

private:
	bool LoadC4D(char* name);

protected:
	// should be in binaryReader
	bool SkipChunk(CC4dFile::SChunkC4d &chunk);

private:
/*
	File* file;
	File* FOpen(CHAR* name); // Open a file
	void FSkip(LONG relskip); // Skip 'relskip' bytes
	void FClose(File* file); // Close a file
	LONG FPos(); // Return absolute position in file
	CHAR ReadCHAR(); // Read 1 byte
	WORD ReadWORD(); // Read 2 bytes
	LONG ReadLONG(); // Read 4 bytes
	Real ReadREAL(); // Read 4 bytes
	Vector ReadVECTOR(); // Read 12 bytes

	void* ReadNBytes(LONG n); // Read n bytes
	void LoadMaterials(void);
	void LoadObjects(void);
	void LoadEnvironment(void);
*/
private:
	long	m_fileSize;
	long	m_pc;	// File Pointer

	CC4dFile*	m_pC4dFile;
};

} // End ZBPlugin namespace

#endif // __C4DREADER_H__
