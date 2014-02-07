#pragma once

#include <sstream>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "LevelLoader.h"

class LevelConverter
{
private:
	struct ModelData
	{
		std::string m_MeshName;
		std::vector<DirectX::XMFLOAT3> m_Translation;
		std::vector<DirectX::XMFLOAT3> m_Rotation;
		std::vector<DirectX::XMFLOAT3> m_Scale;
	};

	DirectX::XMFLOAT3 m_LevelCheckPointStart;
	DirectX::XMFLOAT3 m_LevelCheckPointEnd;
	const std::vector<LevelLoader::CheckPointStruct>* m_LevelCheckPointList;
	const std::vector<LevelLoader::ModelStruct>* m_LevelData;
	const std::vector<std::pair<LevelLoader::LightData, LevelLoader::DirectionalLight>>* m_LevelDirectionalLightList;
	const std::vector<std::pair<LevelLoader::LightData, LevelLoader::PointLight>>* m_LevelPointLightList;
	const std::vector<std::pair<LevelLoader::LightData, LevelLoader::SpotLight>>* m_LevelSpotLightList;
	const std::vector<LevelLoader::ModelHeader>* m_ModelInformation;
	LevelLoader::LevelHeader m_Header;
	int m_LevelDataSize;
public:
	/**
	 * Constructor
	 */
	LevelConverter();
	
	/**
	 * Deconstructor
	 */
	~LevelConverter();

	/**
	 * Call this function after setLevelHead and setLevelModelList
	 * or the file will be empty. 
	 *
	 * @param p_FileName, is the name on the file you whant to create. 
	 */
	bool writeFile(std::string p_FileName);

	/**
	 * Give information about the header. If this is empty the writeFile function will return false.
	 *
	 * @param p_Header, send in information about a header object.
	 */
	void setLevelHead(LevelLoader::LevelHeader p_Header);

	/**
	 * Sets information about the models used in the level. If this information is not set the file will be empty.
	 *
	 * @param p_LevelModelList, is a vector with models that is unsorted. It expects it to be in .txl format.
	 */
	void setLevelModelList(const std::vector<LevelLoader::ModelStruct>* p_LevelModelList);

	/**
	 * Sets information about the directional lighting used in the level.
	 *
	 * @param p_LevelDirectionalLightList, is a vector with directional lights. It expects it to be in .txl format.
	 */
	void setLevelDirectionalLightList(const std::vector<std::pair<LevelLoader::LightData, LevelLoader::DirectionalLight>>* p_LevelDirectionalLightList);

	/**
	 * Sets information about the point lighting used in the level.
	 *
	 * @param p_LevelPointLightList, is a vector with point lights. It expects it to be in .txl format.
	 */
	void setLevelPointLightList(const std::vector<std::pair<LevelLoader::LightData, LevelLoader::PointLight>>* p_LevelPointLightList);

	/**
	 * Sets information about the spot lighting used in the level.
	 *
	 * @param p_LevelSpotLightList, is a vector with spot lights. It expects it to be in .txl format.
	 */
	void setLevelSpotLightList(const std::vector<std::pair<LevelLoader::LightData, LevelLoader::SpotLight>>* p_LevelSpotLightList);

	/**
	 * Sets the list of checkpoints for the track.
	 *
	 * @param p_LevelCheckPointList, is a list of checkpoints structs that contains positions.
	 */
	void setLevelCheckPointList(const std::vector<LevelLoader::CheckPointStruct>* p_LevelCheckPointList);

	/**
	 * Set the checkpoint start value.
	 *
	 * @param p_LevelCheckPointStart, is a DirectX::XMFLOAT3.
	 */
	void setLevelCheckPointStart(DirectX::XMFLOAT3 p_LevelCheckPointStart);

	/**
	 * Set the checkpoint end value.
	 *
	 * @param p_LevelCheckPointEnd, is a DirectX::XMFLOAT3.
	 */
	void setLevelCheckPointEnd(DirectX::XMFLOAT3 p_LevelCheckPointEnd);

	/** 
	 * Set information about the models header.
	 *
	 * @param p_ModelInformation, is a vector of LevelLoader ModelHeader
	 */
	void setModelInformation(const std::vector<LevelLoader::ModelHeader>* p_ModelInformation);

	/**
	 * Clears the writer.
	 */
	void clear();
protected:
	void stringToByte(std::string p_String, std::ostream* p_Output);
	void intToByte(int p_Int, std::ostream* p_Output);

	void createHeader(std::ostream* p_Output);
	void createLevel(std::ostream* p_Output);
	void createLighting(std::ostream* p_Output);
	void createCheckPoints(std::ostream* p_Output);
};