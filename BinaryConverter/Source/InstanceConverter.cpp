#include "InstanceConverter.h"

InstanceConverter::InstanceConverter()
{
	m_Header.m_NumberOfModels = 0;
	m_Header.m_NumberOfLights = 0;
	m_Header.m_NumberOfCheckPoints = 0;
	m_LevelDataSize = 0;
	m_LevelCheckPointStart = DirectX::XMFLOAT3(0, 0, 0);
	m_LevelCheckPointEnd = DirectX::XMFLOAT3(0, 0, 0);
}

InstanceConverter::~InstanceConverter()
{

}

void InstanceConverter::clear()
{
	m_Header.m_NumberOfModels = 0;
	m_Header.m_NumberOfLights = 0;
	m_Header.m_NumberOfCheckPoints = 0;
	m_LevelDataSize = 0;
	m_LevelCheckPointStart = DirectX::XMFLOAT3(0, 0, 0);
	m_LevelCheckPointEnd = DirectX::XMFLOAT3(0, 0, 0);
}

bool InstanceConverter::writeFile(std::string p_FilePath)
{
	std::ofstream output(p_FilePath, std::ostream::out | std::ostream::binary);
	if(!output)
	{
		return false;
	}
	if(m_Header.m_NumberOfModels != 0)
	{
		createHeader(&output);
		createLevel(&output);
		createLighting(&output);
		createCheckPoints(&output);
	}
	else
	{
		return false;
	}
	output.close();
	return true;
}

void InstanceConverter::createHeader(std::ostream* p_Output)
{
	intToByte(m_Header.m_NumberOfModels, p_Output);
	intToByte(m_Header.m_NumberOfLights, p_Output);
	intToByte(m_Header.m_NumberOfCheckPoints, p_Output);
}

void InstanceConverter::createLevel(std::ostream* p_Output)
{
	std::vector<ModelData> level;
	ModelData model;
	bool written = false;
	
	for(int i = 0; i < m_LevelDataSize; i++)
	{
		model = ModelData();
		for(unsigned int j = 0; j < level.size(); j++)
		{
			if(level.at(j).m_MeshName == m_Data->at(i).m_MeshName)
			{
				DirectX::XMFLOAT3 translation = m_Data->at(i).m_Translation;
				translation.x *= -1.f;
				level.at(j).m_Translation.push_back(translation);

				DirectX::XMFLOAT3 rotation = m_Data->at(i).m_Rotation;
				rotation.x *= -1.f;
				rotation.z *= -1.f;
				level.at(j).m_Rotation.push_back(rotation);
				level.at(j).m_Scale.push_back(m_Data->at(i).m_Scale);
				written = true;
				break;
			}	
		}
		if(written != true)
		{
			DirectX::XMFLOAT3 translation = m_Data->at(i).m_Translation;
			translation.x *= -1.f;
			model.m_Translation.push_back(translation);

			DirectX::XMFLOAT3 rotation = m_Data->at(i).m_Rotation;
			rotation.x *= -1.f;
			rotation.z *= -1.f;
			model.m_Rotation.push_back(rotation);
			model.m_Scale.push_back(m_Data->at(i).m_Scale);
			model.m_MeshName = m_Data->at(i).m_MeshName;
			level.push_back(model);
		}
		written = false;
	}
	intToByte(level.size(), p_Output);
	for(unsigned int i = 0; i < level.size(); i++)
	{
		stringToByte(level.at(i).m_MeshName, p_Output);
		for(unsigned int j = 0; j < m_ModelInformation->size(); j++)
		{
			if(strcmp(level.at(i).m_MeshName.c_str(), m_ModelInformation->at(j).m_MeshName.c_str()) == 0)
			{
				intToByte(m_ModelInformation->at(j).m_Animated, p_Output);
				intToByte(m_ModelInformation->at(j).m_Transparency, p_Output);
				intToByte(m_ModelInformation->at(j).m_Collidable, p_Output);
				break;
			}
			else if(j == m_ModelInformation->size()-1)
			{
				intToByte(0, p_Output);
				intToByte(0, p_Output);
				intToByte(0, p_Output);
			}
		}
		if(m_ModelInformation->size() == 0)
		{
			intToByte(0, p_Output);
			intToByte(0, p_Output);
			intToByte(0, p_Output);
		}
		intToByte(level.at(i).m_Translation.size(), p_Output);
		p_Output->write(reinterpret_cast<const char*>(level.at(i).m_Translation.data()), sizeof(DirectX::XMFLOAT3) * level.at(i).m_Translation.size());
		intToByte(level.at(i).m_Rotation.size(), p_Output);
		p_Output->write(reinterpret_cast<const char*>(level.at(i).m_Rotation.data()), sizeof(DirectX::XMFLOAT3) * level.at(i).m_Rotation.size());
		intToByte(level.at(i).m_Scale.size(), p_Output);
		p_Output->write(reinterpret_cast<const char*>(level.at(i).m_Scale.data()), sizeof(DirectX::XMFLOAT3) * level.at(i).m_Scale.size());
	}
}

void InstanceConverter::createLighting(std::ostream* p_Output)
{
	if(m_Header.m_NumberOfLights != 0)
	{
		int numberOfDifferentLights = 0;
		if(m_LevelDirectionalLightList->size() != 0)++numberOfDifferentLights;
		if(m_LevelPointLightList->size() != 0)++numberOfDifferentLights;
		if(m_LevelSpotLightList->size() != 0)++numberOfDifferentLights;
		intToByte(numberOfDifferentLights, p_Output);
		if(m_LevelDirectionalLightList->size() != 0)
		{
			intToByte(m_LevelDirectionalLightList->at(0).first.m_Type, p_Output);
			intToByte(m_LevelDirectionalLightList->size(), p_Output);
			std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::DirectionalLight>> tempDirectional = *m_LevelDirectionalLightList;
			for(auto& tempObj : tempDirectional)
			{
				tempObj.first.m_Translation.x *= -1;
				tempObj.second.m_Direction.x *= -1;
			}
			p_Output->write(reinterpret_cast<const char*>(tempDirectional.data()),
				sizeof(std::pair<InstanceLoader::LightData, InstanceLoader::DirectionalLight>) * m_LevelDirectionalLightList->size());
		}
		if(m_LevelPointLightList->size() != 0)
		{
			intToByte(m_LevelPointLightList->at(0).first.m_Type, p_Output);
			intToByte(m_LevelPointLightList->size(), p_Output);
			std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::PointLight>> tempPoint = *m_LevelPointLightList;
			for(auto& tempObj : tempPoint)
			{
				tempObj.first.m_Translation.x *= -1;
			}
			p_Output->write(reinterpret_cast<const char*>(tempPoint.data()),
				sizeof(std::pair<InstanceLoader::LightData, InstanceLoader::PointLight>) * m_LevelPointLightList->size());
		
		}
		if(m_LevelSpotLightList->size() != 0)
		{
			intToByte(m_LevelSpotLightList->at(0).first.m_Type, p_Output);
			intToByte(m_LevelSpotLightList->size(), p_Output);
			std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::SpotLight>> tempSpot = *m_LevelSpotLightList;
			for(auto& tempObj : tempSpot)
			{
				tempObj.first.m_Translation.x *= -1;
				tempObj.second.m_Direction.x *= -1;
			}
			p_Output->write(reinterpret_cast<const char*>(tempSpot.data()),
				sizeof(std::pair<InstanceLoader::LightData, InstanceLoader::SpotLight>) * m_LevelSpotLightList->size());
		}
	}
}

void InstanceConverter::createCheckPoints(std::ostream* p_Output)
{
	if(m_Header.m_NumberOfCheckPoints != 0)	
	{
		DirectX::XMFLOAT3* tempStart,*tempEnd;
		tempStart = &m_LevelCheckPointStart;
		tempEnd = &m_LevelCheckPointEnd;
		tempStart->x *= -1;
		tempEnd->x *= -1; 
		p_Output->write(reinterpret_cast<const char*>(tempStart), sizeof(DirectX::XMFLOAT3));
		p_Output->write(reinterpret_cast<const char*>(tempEnd), sizeof(DirectX::XMFLOAT3));
		int size = m_LevelCheckPointList->size();
		intToByte(size, p_Output);
		std::vector<InstanceLoader::CheckPointStruct> tempList =  *m_LevelCheckPointList;
		for(auto& tempObj : tempList)
		{
			tempObj.m_Translation.x *= -1;
		}
		p_Output->write(reinterpret_cast<const char*>(tempList.data()), sizeof(InstanceLoader::CheckPointStruct) * size);
	}
}

void InstanceConverter::stringToByte(std::string p_String, std::ostream* p_Output)
{
	unsigned int size = p_String.size();
	p_Output->write(reinterpret_cast<const char*>(&size), sizeof(unsigned int));
	p_Output->write(p_String.data(), p_String.size());
}

void InstanceConverter::intToByte(int p_Int, std::ostream* p_Output)
{
	p_Output->write(reinterpret_cast<const char*>(&p_Int), sizeof(int));
}

void InstanceConverter::setLevelHead(InstanceLoader::LevelHeader p_Header)
{
	m_Header = p_Header;
}

void InstanceConverter::setModelList(const std::vector<InstanceLoader::ModelStruct>* p_LevelModelList)
{
	m_Data = p_LevelModelList;
	m_LevelDataSize = m_Data->size();
}

void InstanceConverter::setLevelDirectionalLightList(const std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::DirectionalLight>>* p_LevelDirectionalLightList)
{
	m_LevelDirectionalLightList = p_LevelDirectionalLightList;
}

void InstanceConverter::setLevelPointLightList(const std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::PointLight>>* p_LevelPointLightList)
{
	m_LevelPointLightList = p_LevelPointLightList;
}

void InstanceConverter::setLevelSpotLightList(const std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::SpotLight>>* p_LevelSpotLightList)
{
	m_LevelSpotLightList = p_LevelSpotLightList;
}

void InstanceConverter::setLevelCheckPointList(const std::vector<InstanceLoader::CheckPointStruct>* p_LevelCheckPointList)
{
	m_LevelCheckPointList = p_LevelCheckPointList;
}

void InstanceConverter::setLevelCheckPointStart(DirectX::XMFLOAT3 p_LevelCheckPointStart)
{
	m_LevelCheckPointStart = p_LevelCheckPointStart;
}

void InstanceConverter::setLevelCheckPointEnd(DirectX::XMFLOAT3 p_LevelCheckPointEnd)
{
	m_LevelCheckPointEnd = p_LevelCheckPointEnd;
}

void InstanceConverter::setModelInformation(const std::vector<InstanceLoader::ModelHeader>* p_ModelInformation)
{
	m_ModelInformation = p_ModelInformation;
}