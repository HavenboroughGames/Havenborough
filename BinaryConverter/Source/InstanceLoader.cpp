#include "InstanceLoader.h"

InstanceLoader::InstanceLoader()
{
	m_Header.m_NumberOfModels = 0;
	m_Header.m_NumberOfLights = 0;
	m_Header.m_NumberOfCheckPoints = 0;
	m_Header.m_NumberOfEffects = 0;
}

InstanceLoader::~InstanceLoader()
{
	clear();
}

void InstanceLoader::clear()
{
	m_ModelList.clear();
	m_ModelList.shrink_to_fit();
	m_CheckPointStart = DirectX::XMFLOAT3(0, 0, 0);
	m_CheckPointEnd = DirectX::XMFLOAT3(0, 0, 0);
	m_EffectList.clear();
	m_EffectList.shrink_to_fit();
	m_LevelCheckPointList.clear();
	m_LevelCheckPointList.shrink_to_fit();
	m_LevelDirectionalLightList.clear();
	m_LevelDirectionalLightList.shrink_to_fit();
	m_LevelPointLightList.clear();
	m_LevelPointLightList.shrink_to_fit();
	m_LevelSpotLightList.clear();
	m_LevelSpotLightList.shrink_to_fit();
	m_Header.m_NumberOfModels = 0;
	m_Header.m_NumberOfLights = 0;
	m_Header.m_NumberOfCheckPoints = 0;
	m_Stringstream.clear();
	m_ModelHeaders.clear();
	m_ModelHeaders.shrink_to_fit();
}

bool InstanceLoader::loadLevel(std::string p_FilePath)
{
	std::ifstream input(p_FilePath, std::ifstream::in);
	if(!input)
	{
		return false;
	}
	clearData();
	startReading(input);
	readModelHeaders(p_FilePath);
	input.close();

	return true;
}

void InstanceLoader::startReading(std::istream& p_Input)
{
	std::string line, key, filler;
	while(!p_Input.eof() && std::getline(p_Input, line))
	{
		key = "";
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> key >> std::ws;
		if(key == "*ObjectHeader*")
		{
			m_Header.m_NumberOfModels = readHeader(p_Input);
			std::getline(p_Input, line);
		}
		else if(key == "*LightHeader*")
		{
			m_Header.m_NumberOfLights = readHeader(p_Input);
			std::getline(p_Input, line);
		}
		else if(key == "*CheckPointHeader*")
		{
			m_Header.m_NumberOfCheckPoints = readHeader(p_Input);
			std::getline(p_Input, line);
		}
		else if(key == "*EffectHeader*")
		{
			m_Header.m_NumberOfEffects = readHeader(p_Input);
			std::getline(p_Input, line);
		}
		else if(key == "#MESH:" || key == "#MESH")
		{
			readMeshList(p_Input);
			std::getline(p_Input, line);
		}
		else if(key == "#Light:")
		{
			readLightList(p_Input);
			std::getline(p_Input, line);
		}
		else if(key == "#Type:")
		{
			readCheckPointList(p_Input);
			std::getline(p_Input, line);
		}
		else if(key == "#Effect:")
		{
			readEffect(p_Input);
			std::getline(p_Input, line);
		}
	}
}

int InstanceLoader::readHeader(std::istream& p_Input)
{
	std::string key, line;
	int result;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> key >> result;
	return result;
}

void InstanceLoader::readModelHeaders(std::string p_FilePath)
{
	std::ifstream headerFile(getPath(p_FilePath), std::ifstream::binary);
	ModelHeader tempHeader;
	headerFile.seekg(0,std::ifstream::end);
	std::streamoff size = headerFile.tellg();
	headerFile.seekg(0,std::ifstream::beg);
	while(headerFile.tellg() < size)
	{
		byteToString(headerFile, tempHeader.m_MeshName);
		byteToInt(headerFile, tempHeader.m_Animated);
		byteToInt(headerFile, tempHeader.m_Transparency);
		byteToInt(headerFile, tempHeader.m_Collidable);
		m_ModelHeaders.push_back(tempHeader);
	}
	headerFile.close();
}

void InstanceLoader::readMeshList(std::istream& p_Input)
{
	std::string key, filler, line;
	ModelStruct tempLevel;
	m_Stringstream >> tempLevel.m_MeshName;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> filler >> tempLevel.m_Translation.x >> tempLevel.m_Translation.y >> tempLevel.m_Translation.z;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> filler >> tempLevel.m_Rotation.x >> tempLevel.m_Rotation.y >> tempLevel.m_Rotation.z;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> filler >> tempLevel.m_Scale.x >> tempLevel.m_Scale.y >> tempLevel.m_Scale.z;

	m_ModelList.push_back(tempLevel);
}

void InstanceLoader::readLightList(std::istream& p_Input)
{
	std::string key, filler, line, tempName;
	LightData tempLight;
	std::string tempString;
	m_Stringstream >> tempName;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> filler >> tempLight.m_Translation.x >> tempLight.m_Translation.y >> tempLight.m_Translation.z;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> filler >> tempLight.m_Color.x >> tempLight.m_Color.y >> tempLight.m_Color.z;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> filler >> tempString;
	if(tempString == "kDirectionalLight")
	{
		tempLight.m_Type = 0;
		DirectionalLight tempDirectional;
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> tempDirectional.m_Intensity;
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> tempDirectional.m_Direction.x >> tempDirectional.m_Direction.y >> tempDirectional.m_Direction.z;
		m_LevelDirectionalLightList.push_back(std::make_pair(tempLight,tempDirectional));
	}
	else if(tempString == "kPointLight")
	{
		tempLight.m_Type = 1;
		PointLight tempDirectional;
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> tempDirectional.m_Intensity;
		m_LevelPointLightList.push_back(std::make_pair(tempLight,tempDirectional));
	}
	else if(tempString == "kSpotLight")
	{
		tempLight.m_Type = 2;
		SpotLight tempSpot;
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> tempSpot.m_Intensity;
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> tempSpot.m_Direction.x >> tempSpot.m_Direction.y >> tempSpot.m_Direction.z;
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> tempSpot.m_ConeAngle >> tempSpot.m_PenumbraAngle;
		m_LevelSpotLightList.push_back(std::make_pair(tempLight,tempSpot));
	}
}

void InstanceLoader::readCheckPointList(std::istream& p_Input)
{
	std::string key, filler, line;
	CheckPointStruct tempCheckPoint;
	std::string tempString;
	m_Stringstream >> tempString;
	if(tempString == "Start")
	{
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> m_CheckPointStart.x >> m_CheckPointStart.y >> m_CheckPointStart.z;
	}
	else if(tempString == "End")
	{
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> m_CheckPointEnd.x >> m_CheckPointEnd.y >> m_CheckPointEnd.z;
	}
	else
	{
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> tempCheckPoint.m_Number;
		std::getline(p_Input, line);
		m_Stringstream = std::stringstream(line);
		m_Stringstream >> filler >> tempCheckPoint.m_Translation.x >> tempCheckPoint.m_Translation.y >> tempCheckPoint.m_Translation.z;
		m_LevelCheckPointList.push_back(tempCheckPoint);
	}
}

void InstanceLoader::readEffect(std::istream& p_Input)
{
	std::string key, filler, line;
	EffectStruct tempEffect;
	std::string tempString;
	m_Stringstream >> tempString;
	tempEffect.m_EffectName = tempString;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> filler >> tempEffect.m_Translation.x >> tempEffect.m_Translation.y >> tempEffect.m_Translation.z;
	std::getline(p_Input, line);
	m_Stringstream = std::stringstream(line);
	m_Stringstream >> filler >> tempEffect.m_Rotation.x >> tempEffect.m_Rotation.y >> tempEffect.m_Rotation.z;

	m_EffectList.push_back(tempEffect);
}

void InstanceLoader::byteToString(std::istream& p_Input, std::string& p_Return)
{
	int strLength = 0;
	byteToInt(p_Input, strLength);
	std::vector<char> buffer(strLength);
	p_Input.read( buffer.data(), strLength);
	p_Return = std::string(buffer.data(), strLength);
}

void InstanceLoader::byteToInt(std::istream& p_Input, int& p_Return)
{
	p_Input.read((char*)&p_Return, sizeof(int));
}

std::string InstanceLoader::getPath(std::string p_FilePath)
{
	std::string file("ModelHeader.txx");
	std::vector<char> buffer(p_FilePath.begin(), p_FilePath.end());
	buffer.push_back(0);
	char *tmp, *type = nullptr;
	tmp = strtok(buffer.data(), "\\");
	while(tmp != nullptr)
	{
		type = tmp;
		tmp = strtok(NULL,"\\");
	}
	int length = buffer.size();
	int size;
	if(type == nullptr)
	{
		size = 1;	
	}
	else
	{
		size = strlen(type)+1;
	}
	std::string temp;
	if(length < size)
	{
		return "..\\error\\";
	}
	temp.append(p_FilePath.data(), length-size);
	temp.append(file.data(),file.size());
	temp.push_back(0);
	return temp;
}

void InstanceLoader::clearData()
{
	m_ModelList.clear();
	m_CheckPointStart = DirectX::XMFLOAT3(0, 0, 0);
	m_CheckPointEnd = DirectX::XMFLOAT3(0, 0, 0);
	m_EffectList.clear();
	m_LevelCheckPointList.clear();
	m_LevelDirectionalLightList.clear();
	m_LevelPointLightList.clear();
	m_LevelSpotLightList.clear();
	m_Header.m_NumberOfModels = 0;
	m_Header.m_NumberOfLights = 0;
	m_Header.m_NumberOfCheckPoints = 0;
	m_ModelHeaders.clear();
	m_Stringstream.clear();
}

InstanceLoader::LevelHeader InstanceLoader::getLevelHeader()
{
	return m_Header;
}

const std::vector<InstanceLoader::ModelStruct>& InstanceLoader::getModelList() const
{
	return m_ModelList;
}

const std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::DirectionalLight>>& InstanceLoader::getLevelDirectionalLightList() const
{
	return m_LevelDirectionalLightList;
}

const std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::PointLight>>& InstanceLoader::getLevelPointLightList() const
{
	return m_LevelPointLightList;
}

const std::vector<std::pair<InstanceLoader::LightData, InstanceLoader::SpotLight>>& InstanceLoader::getLevelSpotLightList() const
{
	return m_LevelSpotLightList;
}

const std::vector<InstanceLoader::CheckPointStruct>& InstanceLoader::getLevelCheckPointList() const
{
	return m_LevelCheckPointList;
}

const std::vector<InstanceLoader::EffectStruct>& InstanceLoader::getLevelEffectList() const
{
	return m_EffectList;
}

DirectX::XMFLOAT3 InstanceLoader::getLevelCheckPointStart() const
{
	return m_CheckPointStart;
}

DirectX::XMFLOAT3 InstanceLoader::getLevelCheckPointEnd() const
{
	return m_CheckPointEnd;
}

const std::vector<InstanceLoader::ModelHeader>& InstanceLoader::getModelInformation() const
{
	return m_ModelHeaders;
}