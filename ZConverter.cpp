
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard

#include "StdAfx.h"

// File formats
#include "3dsFile.h"
#include "fbxFile.h"
#include "lwoFile.h"
#include "mtlFile.h"
#include "mayaFile.h"
#include "../ZConverterMaxSdk/maxFile.h" // ZConverterMaxApi.dll
#include "objFile.h"
#include "w3dFile.h"
#include "stlFile.h"
#include "xsiFile.h"
#include "xyzFile.h"

// X -> OBJ
#include "3dsObjConverter.h"
#include "fbxObjConverter.h"
#include "lwoObjConverter.h"
#include "mayaObjConverter.h"
#include "../ZConverterMayaApi/mayaApiObjConverter.h" // ZConverterMayaApi.dll
#include "../ZConverterMaxSdk/maxSdkObjConverter.h" // ZConverterMaxSdk.dll
#include "extObjConverter.h"
#include "stlObjConverter.h"
#include "xsiObjConverter.h"
#include "xyzObjConverter.h"
#include "w3dObjConverter.h"

// OBJ -> X
#include "obj3dsConverter.h"
#include "objFbxConverter.h"
#include "objLwoConverter.h"
#include "objMayaConverter.h"
#include "../ZConverterMayaApi/objMayaApiConverter.h" // ZConverterMayaApi.dll
#include "../ZConverterMaxSdk/objMaxSdkConverter.h" // ZConverterMaxSdk.dll
#include "objStlConverter.h"
#include "objXsiConverter.h"
#include "objW3dConverter.h"

// Readers
#include "objReader.h"
#include "objReaderFbx.h"
#include "mtlReader.h"
#include "mayaAsciiReader.h"
#include "mayaBinaryReader.h"
#include "../ZConverterMayaApi/mayaApiReader.h" // ZConverterMayaApi.dll
#include "../ZConverterMaxSdk/maxSdkReader.h" // ZConverterMaxSdk.dll
#include "3dsReader.h"
#include "3dsReaderFbx.h"
#include "fbxReader.h"
#include "xsiReader.h"
#include "c4dReader.h"
#include "lwoReader.h"
#include "stlAsciiReader.h"
#include "w3dReader.h"
#include "xyzReader.h"

// Writers
#include "objWriter.h"
#include "objWriterFbx.h"
#include "mtlWriter.h"
#include "mayaAsciiWriter.h"
#include "mayaBinaryWriter.h"
#include "../ZConverterMayaApi/mayaApiWriter.h" // ZConverterMayaApi.dll
#include "../ZConverterMaxSdk/maxSdkWriter.h" // ZConverterMaxSdk.dll
#include "3dsWriter.h"
#include "3dsWriterFbx.h"
#include "fbxWriter.h"
#include "xsiWriter.h"
#include "daeWriter.h"
#include "lwoWriter.h"
#include "stlAsciiWriter.h"
#include "w3dWriter.h"

#include "ZConverter.h"


// test
#include "../ZConverterMayaApi/objMayaApi.h" // objMayaApi.dll
typedef void   (WINAPI* PCObjMayaApi)();
typedef bool   (WINAPI* PCreateMayaApiReader)(CFile*, const fs::path&);
typedef bool   (WINAPI* PCreateMayaObjApiConverter)();
typedef bool   (WINAPI* PCreateMayaApiWriter)(CFile*, const fs::path&, bool);
typedef bool   (WINAPI* PReadMayaApi)();
typedef bool   (WINAPI* PConvertFromObj)(CFile* pObj, CFile* pOther);
typedef bool   (WINAPI* PConvertToObj)(CFile* pOther, CFile* pObj);
typedef bool   (WINAPI* PWriteMayaApi)();
typedef std::string  (WINAPI* PGetMayaDllVersion)();


//#pragma comment(linker, "/delayload:objMayaApi60.dll")
//#pragma comment(linker, "/delayload:objMayaApi65.dll")
//#pragma comment(linker, "/delayload:objMayaApi70.dll")



// windows stuff
#include "DelayImp.h"


// delayHookFunc – Delay load hooking function
FARPROC WINAPI delayHookFailureFunc (unsigned dliNotify, PDelayLoadInfo pdli) 
{
	FARPROC fp = NULL;   // Default return value

	// NOTE: The members of the DelayLoadInfo structure pointed
	// to by pdli shows the results of progress made so far. 

	switch (dliNotify) 
	{
	case dliFailLoadLib:
		// LoadLibrary failed.
		// In here a second attempt could be made to load the dll somehow. 
		// If fp is still NULL, the ERROR_MOD_NOT_FOUND exception will be raised.
		fp = NULL;
		break;

	case dliFailGetProc:
		// GetProcAddress failed.
		// A second attempt could be made to get the function pointer somehow. 
		// We can override and give our own function pointer in fp.
		// Ofcourse, fp is still going to be NULL, 
		// the ERROR_PROC_NOT_FOUND exception will be raised.
		fp = (FARPROC) NULL;
		break;
	}

	return(fp);
}


void CZConverter::RetreiveMayaVersion(std::string& mayaVersion)
{
	mayaVersion = CObjMayaApi::GetMayaDllVersion();
}

PfnDliHook __pfnDliFailureHook2 = delayHookFailureFunc;

bool CZConverter::LoadObjMayaApiDll(std::string& mayaVersion)
{
	__try
	{
		RetreiveMayaVersion(mayaVersion);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// Handle the error. Errors will reach here only if
		// the hook function could not fix it.
		mayaVersion = "";
		return false;
	}

	return true;
}

// TODO : make this better
bool CZConverter::GetMayaEnvPath(std::string& mayaPath)
{
	char* descr = getenv("PATH");
	std::string envPaths(descr);

	size_t pos = envPaths.find("Alias\\Maya");

	if(pos != std::string::npos)
	{
		size_t pathBegin = envPaths.rfind(";", pos);
		size_t pathEnd = envPaths.find_first_of(";", pos);
		mayaPath = envPaths.substr(pathBegin+1, pathEnd-pathBegin-1);
	}
	else
	{
		MSG_ERROR("Make sure that the path to Maya's 'bin' folder is in the System environement variable 'Path'");
		mayaPath = "unknown";
	}

	return true;
}

bool CZConverter::DetectMayaModule(std::string& mayaVersion)
{
	std::string mayaPath;
	std::string dllString;

	GetMayaEnvPath(mayaPath);

	// TODO : make this generic to handle any version of Maya
	// TODO : the user should create a env path variable to hold the path to Maya if not in the default variable path

	if(mayaPath.find("6.0") != std::string::npos)
	{
		MSG_DEBUG("Maya 6.0 path: " << mayaPath);
		dllString = "objMayaApi60.dll";
	}
	else if(mayaPath.find("6.5") != std::string::npos)
	{
		MSG_DEBUG("Maya 6.5 path: " << mayaPath);
		dllString = "objMayaApi65.dll";
	}
	else if(mayaPath.find("7.0") != std::string::npos)
	{
		MSG_DEBUG("Maya 7.0 path: " << mayaPath);
		dllString = "objMayaApi70.dll";
	}
	else
	{
		MSG_WARNING("Maya " << mayaPath << " is not supported by this version of the ObjConverter, only Maya 6.0 and 7.0 are supported");
		return false;
	}

	// test
	//HINSTANCE hDLL;
	//hDLL = LoadLibrary(dllString.c_str());

	LoadObjMayaApiDll(mayaVersion);

	return true;
}


CZConverter::CZConverter():
m_pObjMayaApi(NULL)
{

}

CZConverter::~CZConverter()
{
	OBJ_SAFE_DELETE(m_pObjMayaApi);
}


bool CZConverter::Convert(fs::path& sourceFileName, fs::path& destinationFileName, const std::string& optionString)
{
	bool bStatus = false;

	if(!exists(sourceFileName))
	{
		MSG_ERROR("The source file: '" << sourceFileName.string() << "' does'nt exist.");
		return false;
	}

	bool bRead3dsUsingFbxSdk = true; // Using FBX SDK for 3DS reading
	bool bWrite3dsUsingFbxSdk = true; // Using FBX SDK for 3DS writing


	// Put the fileNames in lower case.
	//std::transform(sourceFileName.begin(), sourceFileName.end(), sourceFileName.begin(), tolower);
	//std::transform(destinationFileName.begin(), destinationFileName.end(), destinationFileName.begin(), tolower);

	EFileFormat sourceFileFormat = GetFileFormat(sourceFileName);
	EFileFormat destinationFileFormat = GetFileFormat(destinationFileName);

	if(sourceFileFormat == FILE_FORMAT_COUNT) // TODO : real file format error
	{
		MSG_ERROR("Unsupported Source file format : '" << sourceFileName.string() << "'");
		return false;
	}
	if(destinationFileFormat == FILE_FORMAT_COUNT) // TODO : real file format error
	{
		MSG_ERROR("Unsupported Destination file format : '" << destinationFileName.string() << "'");
		return false;
	}

	// If we convert to or from the Maya file format, we must verify if we have Maya installed
	bool bMayaInstalled = false;
	if(sourceFileFormat == MA || sourceFileFormat == MB || destinationFileFormat == MA || destinationFileFormat == MB)
	{
		std::string mayaVersion;
		bMayaInstalled = DetectMayaModule(mayaVersion); // maybe retreive the version of Maya installed

		if(bMayaInstalled)
		{
			MSG_DEBUG("Maya is installed : Using Maya API version " << mayaVersion);
			m_pObjMayaApi = new CObjMayaApi();
		}
		else
		{
			MSG_DEBUG("Maya is NOT installed : Using Maya ascii converter");
		}
	}

	// If we convert to or from the Max file format, we must verify if we have Max installed
	bool bMaxInstalled = false;
	if(sourceFileFormat == MAX || destinationFileFormat == MAX)
	{
		bMaxInstalled = true; // hack, todo: detect this!
	}


	// Source file
	CFile* pSrcFile = NULL;

	// File reader
	CReader* pReader = NULL;



	if(sourceFileFormat == OBJ)
	{
		pSrcFile = new CObjFile();
		pReader = new CObjReader(pSrcFile, sourceFileName);
	}
	else if(sourceFileFormat == MA)
	{
		pSrcFile = new CMayaFile();
		if(bMayaInstalled)
		{
			pReader = new CMayaApiReader(pSrcFile, sourceFileName); // pSrcFile is not used for this reader
		}
		else
		{
			pReader = new CMayaAsciiReader(pSrcFile, sourceFileName);
		}
	}
	else if(sourceFileFormat == MB ) // Require Maya API
	{
		if(bMayaInstalled)
		{
			pSrcFile = new CMayaFile();
			pReader = new CMayaApiReader(pSrcFile, sourceFileName); // pSrcFile is not used for this reader
		}
		else
		{
			MSG_ERROR("Can't convert Maya Binary file if Maya is not installed on the local machine");
			return false;
		}
	}
	else if(sourceFileFormat == M3DS) // 3DS file
	{
		if(bRead3dsUsingFbxSdk)
		{
			pSrcFile = new CFbxFile();
			pReader = new C3dsReaderFbx(pSrcFile, sourceFileName);
		}
		else
		{
			pSrcFile = new C3dsFile();
			pReader = new C3dsReader(pSrcFile, sourceFileName);
		}
	}
	else if(sourceFileFormat == MAX) // MAX file
	{
		if(bMaxInstalled)
		{
			pSrcFile = new CMaxFile(sourceFileName);
			pReader = new CMaxSdkReader(pSrcFile, sourceFileName);
		}
		else
		{
			MSG_ERROR("Can't convert MAX files if 3D Studio Max is not installed on the local machine");
			return false;
		}
	}
	else if(sourceFileFormat == FBX)
	{
		pSrcFile = new CFbxFile();
		pReader = new CFbxReader(pSrcFile, sourceFileName);
	}
	else if(sourceFileFormat == XSI)
	{
		pSrcFile = new CXsiFile(sourceFileName);
		pReader = new CXsiReader(pSrcFile, sourceFileName);
	}
	else if(sourceFileFormat == C4D)
	{
		pSrcFile = new CC4dFile();
		pReader = new CC4dReader(pSrcFile, sourceFileName);
	}
	else if(sourceFileFormat == LWO)
	{
		pSrcFile = new CLwoFile();
		pReader = new CLwoReader(pSrcFile, sourceFileName);
	}
	else if(sourceFileFormat == STL)
	{
		pSrcFile = new CStlFile();
		pReader = new CStlAsciiReader(pSrcFile, sourceFileName);
	}
	else if(sourceFileFormat == XYZ)
	{
		pSrcFile = new CXyzFile();
		pReader = new CXyzReader(pSrcFile, sourceFileName, 0); // TODO: redo that right
	}
	else
	{
		MSG_ERROR("Unsupported Source file format : '" << sourceFileName.string() << "'");
		return false;
	}

	OBJ_ASSERT(pReader);
	if(!pReader)
		return false;

	bStatus = pReader->Read(); // Read the file
	OBJ_ASSERT(bStatus);



	if(!bStatus)
	{
		MSG_ERROR("Error while reading the source file.");
		return false;
	}


	// Destination file
	CFile* pDestFile = NULL;

	// Parse option string and Convert to/from OBJ
	CObjConverter* pObjConverter = NULL;

	// Writer
	CWriter* pWriter = NULL;


	if(destinationFileFormat == OBJ) // Output OBJ
	{
		pDestFile = new CObjFile();

		if(sourceFileFormat == OBJ) // extOBJ -> OBJ or OBJ -> extOBJ  TODO: Detect this!
		{
			if(optionString.find("/import:") != std::string::npos) // "Import OBJ"
			{
				pObjConverter = new CExtObjToObjConverter();
			}
			else if(optionString.find("/export:") != std::string::npos) // "Export OBJ"
			{
				pObjConverter = new CObjToExtObjConverter();
			}
			else
			{ OBJ_ASSERT(false); }
		}
		else if((sourceFileFormat == MB || sourceFileFormat == MA) && bMayaInstalled) // MB,MA -> OBJ
		{
			pObjConverter = new CMayaApiObjConverter();
		}
		else if(sourceFileFormat == MA && !bMayaInstalled) // MA -> OBJ
		{
			pObjConverter = new CMayaObjConverter();
		}
		else if(sourceFileFormat == M3DS) // 3DS -> OBJ
		{
			if(bRead3dsUsingFbxSdk) // Using FBX SDK to read 3DS -> FBX internal -> OBJ
			{
				pObjConverter = new CFbxObjConverter();
			}
			else
			{
				pObjConverter = new C3dsObjConverter();
			}
		}
		else if(sourceFileFormat == MAX) // MAX -> OBJ
		{
			if(bMaxInstalled)
			{
				pObjConverter = new CMaxSdkObjConverter(destinationFileName);
			}
			else
			{
				MSG_ERROR("Can't convert MAX files if 3D Studio Max is not installed on the local machine");
				return false;
			}
		}
		else if(sourceFileFormat == FBX) // FBX -> OBJ
		{
			pObjConverter = new CFbxObjConverter();
		}
		else if(sourceFileFormat == XSI) // XSI -> OBJ
 		{
			pObjConverter = new CXsiObjConverter();
		}
		else if(sourceFileFormat == LWO) // LWO -> OBJ
		{
			pObjConverter = new CLwoObjConverter();
		}
		else if(sourceFileFormat == STL) // STL -> OBJ
		{
			pObjConverter = new CStlObjConverter();
		}
		else if(sourceFileFormat == XYZ) // XYZ -> OBJ
		{
			pObjConverter = new CXyzObjConverter();
		}
		else
		{
			MSG_ERROR("Unsupported Source file format : '" << sourceFileName.string() << "'");
			OBJ_SAFE_DELETE(pSrcFile);
			OBJ_SAFE_DELETE(pDestFile);
			return false;
		}

		// Create OBJ Writer
		pWriter = new CObjWriter( pDestFile, destinationFileName );


	}
	else // Destination is NOT OBJ, OBJ must be the source :) !!!!!
	{
		if(sourceFileFormat != OBJ)
		{
			MSG_ERROR("Cannot convert '" << sourceFileName.string() << "' to '" << destinationFileName.string() << "'");
			OBJ_SAFE_DELETE(pSrcFile);
			OBJ_SAFE_DELETE(pDestFile);
			return false;
		}

		if(destinationFileFormat == MA && !bMayaInstalled) // OBJ -> MA, myParser 
		{
			pDestFile = new CMayaFile();
			pObjConverter = new CObjMayaConverter();
			pWriter = new CMayaAsciiWriter( pDestFile, destinationFileName );
		}
		else if((destinationFileFormat == MB || destinationFileFormat == MA) && bMayaInstalled) // OBJ -> MA|MB, Maya API 
		{
			pDestFile = new CMayaFile();
			CMayaFile* pMayaFile = static_cast<CMayaFile*>(pDestFile); 
			if(pMayaFile)
				pMayaFile->m_destDir = destinationFileName;

			pObjConverter = new CObjMayaApiConverter();
			pWriter = new CMayaApiWriter( pDestFile, destinationFileName);
		}
		else if(destinationFileFormat == M3DS) // OBJ -> 3DS
		{
			if(bWrite3dsUsingFbxSdk)
			{
				// FBX SDK 3DS writer
				pDestFile = new CFbxFile();
				pObjConverter = new CObjFbxConverter();
				pWriter = new C3dsWriterFbx(pDestFile, destinationFileName);

				// hack
				//fs::path ObjHackedFileName = fs::change_extension(destinationFileName, ".obj"); // change ".3ds" to ".obj" to debug
				//pWriter = new CObjWriterFbx( pDestFile, ObjHackedFileName );
			}
			else
			{
				// My 3DS Writer
				pDestFile = new C3dsFile();
				pObjConverter = new CObj3dsConverter();
				pWriter = new C3dsWriter(pDestFile, destinationFileName);
			}
		}
		else if(destinationFileFormat == MAX) // OBJ -> MAX
		{
			if(bMaxInstalled)
			{
				pDestFile = new CMaxFile(destinationFileName);
				pObjConverter = new CObjMaxSdkConverter(sourceFileName);
				pWriter = new CMaxSdkWriter(pDestFile, destinationFileName);
			}
			else
			{
				MSG_ERROR("Can't convert MAX files if 3D Studio Max is not installed on the local machine");
				return false;
			}
		}
		else if(destinationFileFormat == FBX) // OBJ -> FBX 
		{
			pDestFile = new CFbxFile();
			pObjConverter = new CObjFbxConverter();
			pWriter = new CFbxWriter( pDestFile, destinationFileName );
		}
		else if(destinationFileFormat == XSI) // OBJ -> XSI
		{
			pDestFile = new CXsiFile(destinationFileName);
			pObjConverter = new CObjXsiConverter();
			pWriter = new CXsiWriter( pDestFile, destinationFileName );
		}
		else if(destinationFileFormat == DAE) // OBJ -> DAE (Collada)
		{
			OBJ_ASSERT(false);
		}
		else if(destinationFileFormat == LWO) // OBJ -> LWO
		{
			pDestFile = new CLwoFile();
			pObjConverter = new CObjLwoConverter();
			pWriter = new CLwoWriter( pDestFile, destinationFileName );
		}
		else if(destinationFileFormat == STL) // OBJ -> STL
		{
			pDestFile = new CStlFile();
			pObjConverter = new CObjStlConverter();
			pWriter = new CStlAsciiWriter( pDestFile, destinationFileName, false );
		}
		else
		{
			OBJ_SAFE_DELETE(pSrcFile);
			MSG_ERROR("Unsupported Destination file format : '" << destinationFileName.string() << "'");
			return false;
		}
	}


	OBJ_ASSERT(pObjConverter);
	if(pObjConverter)
	{
		MSG_INFO("Parsing Options...");
		bStatus = pObjConverter->ParseOptions(optionString); // Parse option string
		OBJ_ASSERT(bStatus);

		// Pre-Conversion
		{
			CPreConvertData preConvertData;
			MSG_INFO("Pre-Conversion...");
			bStatus = pObjConverter->PreConvert(preConvertData); // Insert normal maps, displacement maps into MTL, etc
			OBJ_ASSERT(bStatus);
		}

		// Conversion
		{
			MSG_INFO("Conversion...");
			bStatus = pObjConverter->Convert(pSrcFile, pDestFile); // Convert the source file into the destination file
			OBJ_ASSERT(bStatus);
		}

		// Post-Conversion
		{
			CPostConvertData postConvertData;
			postConvertData.m_sourceFileName = sourceFileName;
			postConvertData.m_destinationFileName = destinationFileName;

			MSG_INFO("Post-Conversion...");
			bStatus = pObjConverter->PostConvert(postConvertData); // Delete temporary files, copy maps, load texture return, etc...
			OBJ_ASSERT(bStatus);
		}
	}

	// Write
	OBJ_ASSERT(pWriter);
	if(pWriter)
	{
		bStatus = pWriter->ParseOptions(optionString);
		OBJ_ASSERT(bStatus);

		// hack
		if(sourceFileFormat != MAX)
		{
			bStatus = pWriter->Write(); // Write to the file
			OBJ_ASSERT(bStatus);
		}
	}

	// Safe delete of allocated objects
	OBJ_SAFE_DELETE(pReader);
	OBJ_SAFE_DELETE(pSrcFile);
	OBJ_SAFE_DELETE(pDestFile);
	OBJ_SAFE_DELETE(pObjConverter);
	OBJ_SAFE_DELETE(pWriter);

	if((sourceFileFormat == MB || sourceFileFormat == MA) && bMayaInstalled) // MA|MB -> OBJ using Maya API
	{
		// Maya API library Cleanup
		CMayaApiObjConverter::CleanUpLibrary();
	}

	return true;
}
