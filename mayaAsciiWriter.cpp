
// Copyright(c) 2005-2006. All Rights Reserved
// By Jean-René Bédard (https://github.com/jrbedard/3d-converter)

#include "StdAfx.h"

#include "windows.h" // TODO : wrap platform code
#include "winbase.h"

#include "mayaAsciiWriter.h"



CMayaAsciiWriter::CMayaAsciiWriter(CFile* pFile, const fs::path& fileName)
{
	CMayaFile* pMayaFile = static_cast<CMayaFile*>(pFile);
	OBJ_ASSERT(pMayaFile);

	m_pMayaFile = pMayaFile;

	m_fileName = fileName;
}


bool CMayaAsciiWriter::Write()
{
	OBJ_ASSERT(m_pMayaFile);
	if(!m_pMayaFile)
		return false;

	std::string fileName(m_fileName.string()); // Extract native file path string

	// Verify here that we have something to actually write
	OBJ_ASSERT(m_pMayaFile->GetMeshVector().size() > 0);

	// Write Maya Ascii file version 6.0

	OBJ_ASSERT(CheckStr(fileName));
	if(!CheckStr(fileName))
		return false;

	m_ofs.open(fileName.c_str());

	MSG_INFO("Writing Maya Ascii file (.MA) : '" << fileName << "'.");
	if( !m_ofs.is_open() )
	{
		MSG_ERROR("Couldn't write to Maya file '" << fileName << "'");
		return false;
	}

	std::string fileNameOnly(fileName);
	if(m_fileName.has_leaf())
	{
		fileNameOnly = m_fileName.leaf();
	}

	// Write the Maya file header
	m_ofs << "//Maya ASCII 6.0 scene" << std::endl;
	m_ofs << "//Name: " << fileNameOnly << std::endl;


	// Windows System Time // TODO : abstract date&time to have it multiplatform
	{
		SYSTEMTIME st;
		GetSystemTime(&st);

		m_ofs << "//Last modified: " << st.wMonth << "/" << st.wDay << "/" << st.wYear << " " <<
									 st.wHour << ":" << st.wMinute << ":" << st.wSecond << std::endl;
	}


	// File information
	m_ofs << "requires maya \"7.0\";" << std::endl;
	m_ofs << "currentUnit -l centimeter -a degree -t film;" << std::endl;
	m_ofs << "fileInfo \"application\" \"ZConverter v1.0\";" << std::endl;
	m_ofs << "fileInfo \"product\" \"Maya Unlimited 7.0\";" << std::endl;
	m_ofs << "fileInfo \"version\" \"7.0\";" << std::endl;
	m_ofs << "fileInfo \"cutIdentifier\" \"200404092210-618567\";" << std::endl;
	m_ofs << "fileInfo \"osv\" \"Microsoft Windows XP Professional Service Pack 2 (Build 2600)\\n\";" << std::endl;

/*
	// Read the maya_viewport file and write it
	{
		std::ifstream ifs("maya_viewport.txt");

		if( !ifs )
		{
			MSG_ERROR("Couldn't read the maya_viewport.txt file'");
			return false;
		}

		const int maxline = 1000;
		char line[maxline];

		while(ifs)
		{
			ifs.getline( line, maxline ); // get the curent line pointer
			m_ofs << line << std::endl;
		}
	}
*/

	// Retreive the group vector
	CMayaFile::MeshVector& meshes = m_pMayaFile->GetMeshVector();
	CMayaFile::MeshVector::iterator meshIt;
	CMayaFile::MeshVector::const_iterator meshEnd = meshes.end();

	StringVector validNodeVector;

	// for every groups
	for(meshIt = meshes.begin(); meshIt != meshEnd; ++meshIt) // For each Maya mesh
	{
		std::string meshName(meshIt->GetMeshName());

		// verify that it is a valid node
		if(meshIt->GetPolyFaceVector().size() <= 0 )
			continue;

		validNodeVector.push_back(meshName);


		// Create the transform node
		{
			m_ofs << "createNode transform -n \"" << meshName << "\";" << std::endl;
		}

		// Create the shape node
		{
			m_ofs << "createNode mesh -n \"" << meshName << "Shape\" -p \"" << meshName << "\";" << std::endl;
			m_ofs << "\tsetAttr -k off \".v\";" << std::endl;

			// what is that ?
			//m_ofs << "\tsetAttr \".vir\" yes;" << std::endl;
			//m_ofs << "\tsetAttr \".vif\" yes;" << std::endl;
		}


		// Texture coordinates
		{
			Vector3DVector& textureCoords = meshIt->GetTextureCoordVector();
			Vector3DVector::iterator textureCoordIt;
			Vector3DVector::const_iterator textureCoordEnd = textureCoords.end();

			if(textureCoords.size() > 0)
			{
				uint lastFaceIndex = meshIt->GetPolyFaceVector().size() - 1;

				// Connection on the shading groups : instObjectGroups
				m_ofs << "\tsetAttr -s 2 \".iog[0].og\";" << std::endl; // I think the 2 is for the initialShadingGroup + assigned shadingGroup
				m_ofs << "\tsetAttr \".iog[0].og[0].gcl\" -type \"componentList\" 0;" << std::endl;
				m_ofs << "\tsetAttr \".iog[0].og[1].gcl\" -type \"componentList\" 1 \"f[0:" << lastFaceIndex << "]\";" << std::endl;

				// Texture coordinates
				m_ofs << "\tsetAttr \".uvst[0].uvsn\" -type \"string\" \"map1\";" << std::endl;

				// for a cube : 30 : 60
				m_ofs << "\tsetAttr -s " << textureCoords.size() << " \".uvst[0].uvsp[0:" << textureCoords.size()-1 << "]\" -type \"float2\" ";

				int i = 0;
				for(textureCoordIt = textureCoords.begin(); textureCoordIt != textureCoordEnd; ++textureCoordIt) // for each UV coord
				{
					m_ofs << (*textureCoordIt)[0] << " " << (*textureCoordIt)[1] << " ";

					if(!(i++%6))
						m_ofs << std::endl << "\t\t";
				}
				m_ofs << ";" << std::endl;
				m_ofs << "\tsetAttr \".cuvs\" -type \"string\" \"map1\";" << std::endl;
				m_ofs << "\tsetAttr \".dcc\" -type \"string\" \"Ambient+Diffuse\";" << std::endl;
			}
		}


		//m_ofs.setf(std::ios::showpoint);
		//m_ofs.precision(6);


		// Vertex positions
		{
			Vector3DVector& vertices = meshIt->GetVertexPositionVector();
			Vector3DVector::iterator vertexIt;
			Vector3DVector::const_iterator vertexEnd = vertices.end();

			OBJ_ASSERT(vertices.size() > 0);
			if(vertices.size() > 0) // if vertex vector is not empty
			{
				m_ofs << "\tsetAttr -s " << vertices.size() << " \".vt[0:" << vertices.size()-1 << "]\"  ";

				int i = 0;
				for(vertexIt = vertices.begin(); vertexIt != vertexEnd; ++vertexIt) // for each vertex position
				{
					m_ofs << (*vertexIt)[0] << " " << (*vertexIt)[1] << " " << (*vertexIt)[2] << " ";

					if(!(i++%2))
						m_ofs << std::endl << "\t\t";
				}
				m_ofs << ";" << std::endl;
			}
		}


		// Edges
		{
			Vector3DVector& edgeVector = meshIt->GetEdgeVector();
			Vector3DVector::iterator edgeIt;
			Vector3DVector::const_iterator edgeEnd = edgeVector.end();

			OBJ_ASSERT(edgeVector.size() > 0);
			if(edgeVector.size() > 0) // if edge vector is not empty
			{
				m_ofs << "\tsetAttr -s " << edgeVector.size() << " \".ed[0:" << edgeVector.size()-1 << "]\" ";

				int i = 0;
				for(edgeIt = edgeVector.begin(); edgeIt != edgeEnd; ++edgeIt) // for each edge (2 index to vertex)
				{
					m_ofs << (*edgeIt)[0] << " " << (*edgeIt)[1] << " " << (*edgeIt)[2] << " ";

					if(!(i++%6))
						m_ofs << std::endl << "\t\t";
				}
				m_ofs << ";" << std::endl;
			}
		}


		// Normals (4 per face - 1 per vertex per face)
		{
			Vector3DVector& normals = meshIt->GetNormalVector();
			Vector3DVector::iterator normalIt;
			Vector3DVector::const_iterator normalEnd = normals.end();

			if(normals.size() > 0)
			{
				m_ofs << "\tsetAttr -s " << normals.size() << " \".n[0:" << normals.size()-1 << "]\" -type \"float3\" " << std::endl;			

				int i = 0;
				for(normalIt = normals.begin(); normalIt != normalEnd; ++normalIt) // for each normal
				{
					m_ofs << (*normalIt)[0] << " " << (*normalIt)[1] << " " << (*normalIt)[2] << " ";

					if(!(i++%6))
						m_ofs << std::endl << "\t\t";
				}
				m_ofs << ";" << std::endl;
			}
		}


		// Faces
		{
			CMayaFile::CMesh::PolyFaceVector& polyFaces = meshIt->GetPolyFaceVector();
			CMayaFile::CMesh::PolyFaceVector::iterator polyFaceIt;
			CMayaFile::CMesh::PolyFaceVector::const_iterator polyFaceEnd = polyFaces.end();

			OBJ_ASSERT(polyFaces.size() > 0);

			m_ofs << "\tsetAttr -s " << polyFaces.size() << " \".fc[0:" << polyFaces.size()-1 << "]\" -type \"polyFaces\" " << std::endl;

			for(polyFaceIt = polyFaces.begin(); polyFaceIt != polyFaceEnd; ++polyFaceIt) // for each face
			{

				CMayaFile::CMesh::CPolyFace::CFace face = polyFaceIt->GetFace();
				CMayaFile::CMesh::CPolyFace::CMu mu = polyFaceIt->GetMu();

				uint edgeCount = face.EdgeCount();
				uint uvCount = mu.UvCount();

				// Face
				std::vector<int>& edgeVector = face.GetEdgeVector();
				std::vector<int>::iterator edgeIt;
				std::vector<int>::const_iterator edgeEnd = edgeVector.end();

				// Mu
				std::vector<uint>& uvVector = mu.GetUvVector();
				std::vector<uint>::iterator uvIt;
				std::vector<uint>::const_iterator uvEnd = uvVector.end();

				// if edge vector not empty
				if(edgeVector.size() > 0)
				{
					m_ofs << "\t\tf " << edgeCount;

					for(edgeIt = edgeVector.begin(); edgeIt != edgeEnd; ++edgeIt ) // for each indexed edge
					{
						m_ofs << " " << (*edgeIt);
					}

					if(polyFaceIt+1 == polyFaceEnd && uvVector.size() == 0) // last face and no "mu"
						m_ofs << " ;" << std::endl;
					else
						m_ofs << std::endl;
				}

				// if uv vector not empty
				if(uvVector.size() > 0)
				{
					m_ofs << "\t\tmu " << "0 " << uvCount;

					for(uvIt = uvVector.begin(); uvIt != uvEnd; ++uvIt ) // for each indexed UV coord
					{
						m_ofs << " " << (*uvIt);
					}

					if(polyFaceIt+1 == polyFaceEnd) // last face and no "mu"
						m_ofs << " ;" << std::endl;
					else
						m_ofs << std::endl;
				}
			}

		}

	}



	// Add shading groups / material / texture here

	CMtlFile* pMtlFile = m_pMayaFile->GetMaterialFile();

	OBJ_ASSERT(pMtlFile);
	if(!pMtlFile)
		return true;

	CMtlFile::MaterialMap& materials = pMtlFile->GetMaterialMap();
	CMtlFile::MaterialMap::iterator materialIt;
	CMtlFile::MaterialMap::const_iterator materialEnd = materials.end();

	uint materialCount = materials.size();
	uint textureCount = 0;
	uint materialID = 0;


	m_ofs << "createNode lightLinker -n \"lightLinker1\";" << std::endl;
	m_ofs << "\tsetAttr -s " << materialCount + 2 << " \".lnk\";" << std::endl;	// number of shading group + initialShadingGroup + initialParticleSE

	m_ofs << "createNode displayLayerManager -n \"layerManager\";" << std::endl;
	m_ofs << "createNode displayLayer -n \"defaultLayer\";" << std::endl;
	m_ofs << "createNode renderLayerManager -n \"renderLayerManager\";" << std::endl;
	m_ofs << "createNode renderLayer -n \"defaultRenderLayer\";" << std::endl;
	m_ofs << "createNode renderLayer -s -n \"globalRender\";" << std::endl;


	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each material : Retreive materials
	{
		std::string shadingGroupName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;
		OBJ_ASSERT(materialIt->first == pMaterial->GetMaterialName());
		//MayaStringCheck(materialName);

		// TODO : look if we already created this material

		std::stringstream materialIDSS;
		materialIDSS << materialID;

		// Create the shading group
		m_ofs << "createNode shadingEngine -n \"" << shadingGroupName << "\";" << std::endl;
		m_ofs << "\tsetAttr \".ihi\" 0;" << std::endl;
		m_ofs << "\tsetAttr \".ro\" yes;" << std::endl;


		// MaterialInfo, 1 per material (shading group)
		materialIDSS.str("");
		materialIDSS << (materialID+1); // starts at 1, the first one is reserved ?
		m_ofs << "createNode materialInfo -n \"materialInfo" << materialIDSS.str() << "\";" << std::endl;


		// what is that ? 2 groupID per mesh
		materialIDSS.str("");
		materialIDSS << (materialID*3+2); // starts at 2
		m_ofs << "createNode groupId -n \"groupId" << materialIDSS.str() << "\";" << std::endl;
		m_ofs << "\tsetAttr \".ihi\" 0;" << std::endl;
		materialIDSS.str("");
		materialIDSS << (materialID*3+3);
		m_ofs << "createNode groupId -n \"groupId" << materialIDSS.str() << "\";" << std::endl;
		m_ofs << "\tsetAttr \".ihi\" 0;" << std::endl;


		int number = 1; // TODO : increment
		std::string materialName(shadingGroupName + "1");

		// TODO : support more type of material : phong, blinn, etc

		// Create the material
		m_ofs << "createNode phong -n \"" << materialName << "\";" << std::endl; // Phong by default
		m_ofs << "\tsetAttr \".cp\" 2.059999942779541;" << std::endl; // what is this number ?


		// Texture Map
		if(pMaterial->HasTexMap())
		{
			// Create the texture file
			std::string textureName(materialName + "F");

			m_ofs << "createNode file -n \"" << textureName << "\";" << std::endl;
			m_ofs << "\tsetAttr \".ftn\" -type \"string\" \"" << pMaterial->GetTexMap() << "\";" << std::endl;

			// Create the place2dTexture placement
			std::string place2dTextureName(materialName + "P2D");
			m_ofs << "createNode place2dTexture -n \"" << place2dTextureName << "\";" << std::endl;

			++textureCount;
		}

		++materialID;
	}


	m_ofs << "select -ne :time1;" << std::endl;
	m_ofs << "\tsetAttr \".o\" 1;" << std::endl;
	m_ofs << "select -ne :renderPartition;" << std::endl;
	m_ofs << "\tsetAttr -s " << materialCount + 2 << " \".st\";" << std::endl; // number of shading group + initialShadingGroup + initialParticleSE

	m_ofs << "select -ne :renderGlobalsList1;" << std::endl;
	m_ofs << "select -ne :defaultShaderList1;" << std::endl;
	m_ofs << "\tsetAttr -s " << materialCount + 2 << " \".s\";" << std::endl; // number of shading group + initialShadingGroup + initialParticleSE

	m_ofs << "select -ne :postProcessList1;" << std::endl;
	m_ofs << "\tsetAttr -s 2 \".p\";" << std::endl;
	m_ofs << "select -ne :defaultRenderUtilityList1;" << std::endl;
	m_ofs << "\tsetAttr -s " << materialCount << " \".u\";" << std::endl; // not sure about that : groups

	m_ofs << "select -ne :lightList1;" << std::endl;
	m_ofs << "select -ne :defaultTextureList1;" << std::endl;
	m_ofs << "\tsetAttr -s " << textureCount << " \".tx\";" << std::endl; // number of texture file

	m_ofs << "select -ne :initialShadingGroup;" << std::endl;
	m_ofs << "\tsetAttr -s " << materialCount << " \".dsm\";" << std::endl;
	m_ofs << "\tsetAttr \".ro\" yes;" << std::endl;
	m_ofs << "\tsetAttr -s " << materialCount << " \".gn\";" << std::endl;

	m_ofs << "select -ne :initialParticleSE;" << std::endl;
	m_ofs << "\tsetAttr \".ro\" yes;" << std::endl;
	m_ofs << "select -ne :hardwareRenderGlobals;" << std::endl;
	m_ofs << "\taddAttr -ci true -sn \"ani\" -ln \"animation\" -bt \"ANIM\" -min 0 -max 1 -at \"bool\";" << std::endl;
	m_ofs << "\tsetAttr \".fn\" -type \"string\" \"%s.%e\";" << std::endl;
	m_ofs << "\tsetAttr \".ctrs\" 256;" << std::endl;
	m_ofs << "\tsetAttr \".btrs\" 512;" << std::endl;
	m_ofs << "\tsetAttr -k on \".ani\";" << std::endl;
	m_ofs << "select -ne :defaultHardwareRenderGlobals;" << std::endl;
	m_ofs << "\tsetAttr \".fn\" -type \"string\" \"im\";" << std::endl;
	m_ofs << "\tsetAttr \".res\" -type \"string\" \"ntsc_4d 646 485 1.333\";" << std::endl;


	// Useless commands :
	//m_ofs << "select -ne :ikSystem;" << std::endl;
	//m_ofs << "\tsetAttr -s 3 \".sol\";" << std::endl;
	//connectAttr "myMaterial.mwc" "swordShape.iog.og[1].gco"; // wireframe color



	StringVector vecShadingGroup;

	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each material : Retreive materials
	{
		std::string shadingGroupName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;

		// hack : Assuming 1 shading group per mesh
		vecShadingGroup.push_back(shadingGroupName);
	}
	StringVector::iterator nodeIt;
	StringVector::const_iterator nodeEnd = validNodeVector.end();

	
	StringVector vecMeshName;

	// for every groups
	for(meshIt = meshes.begin(); meshIt != meshEnd; ++meshIt) // for each mesh
	{
		std::string meshName(meshIt->GetMeshName());
		vecMeshName.push_back(meshName);
	}
	StringVector::iterator meshNameIt;
	StringVector::const_iterator meshNameEnd = vecMeshName.end();



	uint groupID = 2;
	for(nodeIt = validNodeVector.begin(); nodeIt != nodeEnd; ++nodeIt)
	{
		uint sgIndex = nodeIt - validNodeVector.begin();
		if(sgIndex > vecShadingGroup.size()-1)
			continue;

		std::string shadingGroupName = vecShadingGroup[sgIndex];

		// 3 groupID per mesh
		std::stringstream groupIDss;
		groupIDss << groupID+1;
		m_ofs << "connectAttr \"groupId" << groupIDss.str() << ".id\" \"" << (*nodeIt) << "Shape.iog.og[1].gid\";" << std::endl;

		groupIDss.str("");
		groupIDss << groupID;
		m_ofs << "connectAttr \"groupId" << groupIDss.str() << ".id\" \"" << (*nodeIt) << "Shape.ciog.cog[0].cgid\";" << std::endl;

		groupID += 3; // the first one is always hidden
	}



	m_ofs << "connectAttr \":defaultLightSet.msg\" \"lightLinker1.lnk[0].llnk\";" << std::endl;
	m_ofs << "connectAttr \":initialShadingGroup.msg\" \"lightLinker1.lnk[0].olnk\";" << std::endl;
	m_ofs << "connectAttr \":defaultLightSet.msg\" \"lightLinker1.lnk[1].llnk\";" << std::endl;
	m_ofs << "connectAttr \":initialParticleSE.msg\" \"lightLinker1.lnk[1].olnk\";" << std::endl;
	
	
	uint lightLinkerIndex = 2;
	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each shading group
	{
		std::string shadingGroupName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;

		std::stringstream ssLightLinkerIndex;
		ssLightLinkerIndex << lightLinkerIndex;

		m_ofs << "connectAttr \":defaultLightSet.msg\" \"lightLinker1.lnk[" << ssLightLinkerIndex.str() << "].llnk\";" << std::endl;
		m_ofs << "connectAttr \"" << shadingGroupName << ".msg\" \"lightLinker1.lnk[" << ssLightLinkerIndex.str() << "].olnk\";" << std::endl;

		lightLinkerIndex++;
	}


	m_ofs << "connectAttr \"layerManager.dli[0]\" \"defaultLayer.id\";" << std::endl;
	m_ofs << "connectAttr \"renderLayerManager.rlmi[0]\" \"defaultRenderLayer.rlid\";" << std::endl;


	uint sgIndex = 0; // shading group index
	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each material : Retreive materials
	{
		std::string shadingGroupName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;
		//MayaStringCheck(materialName);


		std::string materialName(shadingGroupName + "1");

		// Connection Material.outColor -> ShadingGroup.ss
		m_ofs << "connectAttr \"" << materialName << ".oc\" \"" << shadingGroupName << ".ss\";" << std::endl;


		std::stringstream ssSgIndex;
		ssSgIndex << sgIndex*3+3;

		m_ofs << "connectAttr \"groupId" << ssSgIndex.str() << ".msg\" \"" << shadingGroupName << ".gn\" -na;" << std::endl;


		// Assign Shading group to the Mesh first
		m_ofs << "connectAttr \"" << vecMeshName[sgIndex] << "Shape.iog.og[1]\" \"" << shadingGroupName << ".dsm\" -na;" << std::endl;


		// Material information
		ssSgIndex.str("");
		ssSgIndex << (sgIndex+1); // starts at 1
		m_ofs << "connectAttr \"" << shadingGroupName << ".msg\" \"materialInfo" << ssSgIndex.str() << ".sg\";" << std::endl;
		m_ofs << "connectAttr \"" << materialName << ".msg\" \"materialInfo" << ssSgIndex.str() << ".m\";" << std::endl;


		if(pMaterial->HasTexMap())
		{
			// Connect texture
			std::string textureName(materialName + "F");

			m_ofs << "connectAttr \"" << textureName << ".msg\" \"materialInfo" << ssSgIndex.str() << ".t\" -na;" << std::endl;
			m_ofs << "connectAttr \"" << textureName << ".oc\" \"" << materialName << ".c\";" << std::endl;

			// Connect place2D texture
			std::string place2dTextureName(materialName + "P2D");

			m_ofs << "connectAttr \"" << place2dTextureName << ".c\" \"" << textureName << ".c\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".tf\" \"" << textureName << ".tf\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".rf\" \"" << textureName << ".rf\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".s\" \"" << textureName << ".s\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".wu\" \"" << textureName << ".wu\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".wv\" \"" << textureName << ".wv\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".re\" \"" << textureName << ".re\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".of\" \"" << textureName << ".of\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".r\" \"" << textureName << ".ro\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".o\" \"" << textureName << ".uv\";" << std::endl;
			m_ofs << "connectAttr \"" << place2dTextureName << ".ofs\" \"" << textureName << ".fs\";" << std::endl;

		}

		++sgIndex;
	}


	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each shading group
	{
		std::string shadingGroupName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;

		m_ofs << "connectAttr \"" << shadingGroupName << ".pa\" \":renderPartition.st\" -na;" << std::endl;
	}


	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each shading group
	{
		std::string shadingGroupName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;

		m_ofs << "connectAttr \"" << shadingGroupName << ".msg\" \":defaultShaderList1.s\" -na;" << std::endl;
	}


	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for place texture 2d
	{
		std::string shadingGroupName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;

		std::string place2dTextureName(shadingGroupName + "1P2D" ); // find a better way

		if(pMaterial->HasTexMap())
			m_ofs << "connectAttr \"" << place2dTextureName << ".msg\" \":defaultRenderUtilityList1.u\" -na;" << std::endl;
	}

	// Light linker
	m_ofs << "connectAttr \"lightLinker1.msg\" \":lightList1.ln\" -na;" << std::endl;


	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each textures
	{
		std::string shadingGroupName(materialIt->first);
		CMtlFile::CMaterial* pMaterial = &materialIt->second;

		std::string textureName(shadingGroupName + "1F" ); // find a better way

		if(pMaterial->HasTexMap())
			m_ofs << "connectAttr \"" << textureName << ".msg\" \":defaultTextureList1.tx\" -na;" << std::endl;
	}


	for(meshNameIt = vecMeshName.begin(); meshNameIt != meshNameEnd; ++meshNameIt) // mesh -> initial shading group
	{
		// Assign Initial shading group to the Mesh second
		m_ofs << "connectAttr \"" << (*meshNameIt) << "Shape.ciog.cog[0]\" \":initialShadingGroup.dsm\" -na;" << std::endl;
	}


	groupID = 2;
	for(materialIt = materials.begin(); materialIt != materialEnd; ++materialIt) // for each SG : group ID 2,5,8,...
	{
		std::stringstream groupIDss;
		groupIDss << groupID;

		m_ofs << "connectAttr \"groupId" << groupIDss.str() << ".msg\" \":initialShadingGroup.gn\" -na;" << std::endl;

		groupID += 3;
	}



	m_ofs << "// End of " << fileNameOnly;

	m_ofs.close();

	MSG_DEBUG("Done.");

	return true;
}