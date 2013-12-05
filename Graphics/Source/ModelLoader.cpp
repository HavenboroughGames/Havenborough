#include "ModelLoader.h"

ModelLoader::ModelLoader()
{
}

ModelLoader::~ModelLoader()
{
	m_Index.clear();
	m_Normals.clear();
	m_Vertex.clear();
	m_Tangents.clear();
	m_TextureCoord.clear();	
}

void ModelLoader::clear()
{
	m_Index.clear();
	m_Index.shrink_to_fit();
	m_Normals.clear();
	m_Normals.shrink_to_fit();
	m_Tangents.clear();
	m_Tangents.shrink_to_fit();
	m_TextureCoord.clear();
	m_TextureCoord.shrink_to_fit();
	m_Vertex.clear();
	m_Vertex.shrink_to_fit();
	for(int i = 0; i < m_IndexPerMaterial.size(); i++)
	{
		m_IndexPerMaterial.at(i).clear();
		m_IndexPerMaterial.at(i).shrink_to_fit();
	}
	m_IndexPerMaterial.clear();
	m_IndexPerMaterial.shrink_to_fit();
	m_Material.clear();
	m_Material.shrink_to_fit();
	m_NumberOfMaterials = 0;
	m_MeshName = "";
	m_NumberOfVertices = 0;
	m_NumberOfTriangles = 0;
}

bool ModelLoader::loadFile(std::string p_FilePath)
{
	
	std::ifstream input(p_FilePath.c_str(),std::ifstream::in);
	std::string line, key, filler;
	int readMaterial = 0;
	int tempInt;
	Material tempMaterial;
	Face tempFace;
	DirectX::XMFLOAT3 tempFloat3;
	std::stringstream stringstream;

	if(!input)
	{
		return false;
	}

	while (!input.eof() && std::getline(input, line))
	{

		key = "";
		stringstream = std::stringstream(line);
		stringstream >> key >> std::ws;
		if(strcmp(key.c_str(), "*Header") == 0)
		{
			std::getline(input, line);
			stringstream = std::stringstream(line);
			stringstream >> key >> m_NumberOfMaterials;
			std::getline(input, line);
			stringstream = std::stringstream(line);
			stringstream >> key >> m_MeshName;
			std::getline(input, line);
			stringstream = std::stringstream(line);
			stringstream >> key >> m_NumberOfVertices;
			std::getline(input, line);
			stringstream = std::stringstream(line);
			stringstream >> key >> m_NumberOfTriangles;
		}
		else if(key  == "*Materials")
		{
			std::getline(input, line);
			stringstream = std::stringstream(line);
			for(int i = 0; i < m_NumberOfMaterials; i++)
			{
				stringstream >> key >> tempMaterial.m_MaterialID;
				std::getline(input, line);
				stringstream = std::stringstream(line);
				stringstream >> key >> tempMaterial.m_DiffuseMap;
				std::getline(input, line);
				stringstream = std::stringstream(line);
				stringstream >> key >> tempMaterial.m_NormalMap;
				std::getline(input, line);
				stringstream = std::stringstream(line);
				stringstream >> key >> tempMaterial.m_SpecularMap;
				std::getline(input, line);
				stringstream = std::stringstream(line);
				if(i != m_NumberOfMaterials-1)
				{
					std::getline(input, line);
					stringstream = std::stringstream(line);
				}
				m_Material.push_back(tempMaterial);
			}
		}
		else if(key == "*Vertices")
		{
			for(int i = 0; i < m_NumberOfVertices; i++)
			{	
				std::getline(input, line);
				stringstream = std::stringstream(line);
				stringstream >> key >> tempFloat3.x >> tempFloat3.y >> tempFloat3.z;
				m_Vertex.push_back(tempFloat3);
			}
		}
		else if(key == "*Normals")
		{
			while(std::getline(input, line))
			{
				stringstream = std::stringstream(line);
				if(line == "")
					break;
				stringstream >> key >> tempFloat3.x >> tempFloat3.y >> tempFloat3.z;
				m_Normals.push_back(tempFloat3);
			}
		}
		else if(key == "*UV")
		{
			while(std::getline(input, line))
			{
				stringstream = std::stringstream(line);
				if(line == "")
					break;
				stringstream >> key >> tempFloat3.x >> tempFloat3.y;
				m_TextureCoord.push_back(DirectX::XMFLOAT2(tempFloat3.x, 1 - tempFloat3.y));
			}
		}
		else if(key == "*Tangets")
		{
			while(std::getline(input, line))
			{
				stringstream = std::stringstream(line);
				if(line == "")
					break;
				stringstream >> key >> tempFloat3.x >> tempFloat3.y >> tempFloat3.z;
				m_Tangents.push_back(tempFloat3);
			}
		}
		else if(key == "*FACES")
		{
			for(int i = 0; i < m_NumberOfMaterials; i++)
			{
				m_Index.clear();
				std::getline(input, line);
				stringstream = std::stringstream(line);
				stringstream >> tempFace.m_MaterialID;
				std::getline(input, line);
				stringstream = std::stringstream(line);
				stringstream >> filler >> key;
				while(std::getline(input,line))
				{
					stringstream = std::stringstream(line);
					if(line == "")
						break;
					for(int i = 0; i < atoi(key.c_str()); i++)
					{
						stringstream >> tempInt >> filler;
						tempFace.m_Vertex = tempInt;
						stringstream >> tempInt >> filler;
						tempFace.m_Tangents = tempInt;
						stringstream >> tempInt >> filler;
						tempFace.m_Normals = tempInt;
						stringstream >> tempInt >> filler;	
						tempFace.m_TextureCoord = tempInt;
						m_Index.push_back(tempFace);
					}
					
				}
				m_IndexPerMaterial.push_back(m_Index);
			}
		}
	}
	return true;
}

std::vector<DirectX::XMFLOAT3> ModelLoader::getVertices()
{
	return m_Vertex;
}

std::vector<std::vector<ModelLoader::Face>> ModelLoader::getIndices()
{
	return m_IndexPerMaterial;
}

std::vector<ModelLoader::Material> ModelLoader::getMaterial()
{
	return m_Material;
}

std::vector<DirectX::XMFLOAT3> ModelLoader::getNormals()
{
	return m_Normals;
}

std::vector<DirectX::XMFLOAT3> ModelLoader::getTangents()
{
	return m_Tangents;
}

std::vector<DirectX::XMFLOAT2> ModelLoader::getTextureCoords()
{
	return m_TextureCoord;
}