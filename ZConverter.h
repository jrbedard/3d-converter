
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __ZCONVERTER_H__
#define __ZCONVERTER_H__


#include "fileFormats.h"


namespace ZBPlugin
{

class CObjMayaApi;

// CConverter: converter main class
class DLLEXPORT CZConverter
{
public:
	CZConverter();
	~CZConverter();

public:
	bool Convert(fs::path& sourceFileName, fs::path& destinationFileName, const std::string& optionString);

private:
	// tests for Maya
	void RetreiveMayaVersion(std::string& mayaVersion);
	bool DetectMayaModule(std::string& mayaVersion);
	bool GetMayaEnvPath(std::string& latestMayaPath);
	bool LoadObjMayaApiDll(std::string& mayaVersion);

private:
	CObjMayaApi* m_pObjMayaApi;
};

} // End ZBPlugin namespace

#endif // __ZCONVERTER_H__