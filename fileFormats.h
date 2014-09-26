
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#ifndef __FILEFORMATS_H__
#define __FILEFORMATS_H__

namespace ZBPlugin
{

enum EFileFormat
{
	OBJ = 0,  // .obj (OBJ file)
	MTL = 1,  // .mtl (OBJ's material file)
	MA  = 2,  // .ma  (Maya Ascii)
	MB  = 3,  // .mb  (Maya Binary)
	M3DS= 4, // .3ds (3D Studio Max mesh)
	MAX = 5,  // .max (3D Studio Max scene file)
	FBX = 6,  // .fbx (Kaydara, now Alias, now Autodesk)
	XSI = 7,  // .xsi (XSI|Softimage)
	C4D = 8,  // .c4d (Cinema4D)
	DAE = 9,  // .dae (Collada)
	LWO = 10, // .lwo (LightWave6)
	STL = 11, // .stl (Stereolithography)
	XYZ = 12, // .xyz (Point cloud)
	W3D = 13, // .w3d (Shockwave3D)
	FILE_FORMAT_COUNT
};

static const char* FileFormatString[EFileFormat::FILE_FORMAT_COUNT] = {".obj", ".mtl", ".ma", ".mb", ".3ds", ".max", ".fbx", ".xsi", ".c4d", ".dae", ".lwo", ".stl", ".xyz", ".w3d"};


static EFileFormat GetFileFormat(const fs::path& fileName)
{
	EFileFormat fileFormat = FILE_FORMAT_COUNT;

	std::string fileExtension = fs::extension(fileName);
	std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower); // put extension in lowercase

	for(uint ft=0; ft < FILE_FORMAT_COUNT; ++ft)
	{
		if(fileExtension == FileFormatString[ft])
		{
			return (EFileFormat)ft;
		}
	}
	return fileFormat;
}

} // End ZBPlugin namespace

#endif // __FILEFORMATS_H__