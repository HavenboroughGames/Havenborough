#pragma once

#include <sstream>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "ModelLoader.h"

class ModelConverter
{
public:
	struct VertexBufferAnimation
	{
		DirectX::XMFLOAT4 m_Position;
		DirectX::XMFLOAT3 m_Normal;
		DirectX::XMFLOAT2 m_UV;
		DirectX::XMFLOAT3 m_Tangent;
		DirectX::XMFLOAT3 m_Binormal;
		DirectX::XMFLOAT3 m_Weight;
		DirectX::XMINT4 m_Joint;
	};
	
	struct VertexBuffer
	{
		DirectX::XMFLOAT4 m_Position;
		DirectX::XMFLOAT3 m_Normal;
		DirectX::XMFLOAT2 m_UV;
		DirectX::XMFLOAT3 m_Tangent;
		DirectX::XMFLOAT3 m_Binormal;
	};

	struct BoundingVolume
	{
		DirectX::XMFLOAT4 m_Position;
	};

	struct MaterialBuffer
	{
		std::string material;
		int start;
		int length;
	};

private:
	std::string m_MeshName;
	std::vector<ModelLoader::IndexDesc>* m_Indices;

	int m_NumberOfFrames;
	int m_MaterialSize, m_IndexPerMaterialSize, m_ListOfJointsSize, m_WeightsListSize;
	bool m_Transparency, m_Collidable, m_Animated;
	const std::vector<DirectX::XMFLOAT3> *m_Vertices, *m_Normals, *m_Tangents;
	const std::vector<DirectX::XMFLOAT2>* m_TextureCoord;
	const std::vector<ModelLoader::Material>* m_Material;
	const std::vector<std::vector<ModelLoader::IndexDesc>>* m_IndexPerMaterial;
	const std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMINT4>>* m_WeightsList;
	const std::vector<ModelLoader::Joint>* m_ListOfJoints;

	int m_VertexCount;
public:
	
	/**
	 * Constructor.
	 */
	ModelConverter();
	
	/**
	 * Destructor.
	 */
	~ModelConverter();
	
	/**
	 * Use this function to release the memory in loader vectors.
	 */
	void clear();

	/**
	 * This function converts the loaded file to a binary file. If you are using a left handed model this will make it right handed.
	 * This converter is for DirectX and if you whant to use it to convert files to an OpenGL project or any other left handed API:s
	 * you have to change the direction of the vertex loop and the animated vertex loop, and remove the *=-1 on X-axis.
	 *
	 * @param p_FilePath is the complete filepath to the source file.
	 * @return false if something is wrong when writing the file.
	 */
	bool writeFile(std::string p_FilePath);
	
	/**
	 * This whants a pointer to the source information about vertices. 
	 *
	 * @param p_Vertices is a vector pointer that contains DirectX::XMFLOAT3.
	 */
	void setVertices(const std::vector<DirectX::XMFLOAT3>* p_Vertices);

	/**
	 * This whants a bool value about the transparency. 
	 *
	 * @param p_Transparent is a bool variable from source file.
	 */
	void setTransparent(bool p_Transparent);

	/**
	 * This whants a bool value if the object is collide-able or not. 
	 *
	 * @param p_Collidable is a bool variable from source file.
	 */
	void setCollidable(bool p_Collidable);

	/**
	 * This whants a pointer to the source information about indices. 
	 *
	 * @param p_Indices is a vector pointer that contains vectors of ModelLoader::IndexDesc.
	 */
	void setIndices(const std::vector<std::vector<ModelLoader::IndexDesc>>* p_Indices);

	/**
	 * This whants a pointer to the source information about material. 
	 *
	 * @param p_Material is a vector pointer that contains ModelLoader::Material.
	 */
	void setMaterial(const std::vector<ModelLoader::Material>* p_Material);

	/**
	 * This whants a pointer to the source information about normals. 
	 *
	 * @param p_Normals is a vector pointer that contains DirectX::XMFLOAT3.
	 */
	void setNormals(const std::vector<DirectX::XMFLOAT3>* p_Normals);

	/**
	 * This whants a pointer to the source information about tangent. 
	 *
	 * @param p_Tangents is a vector pointer that contains DirectX::XMFLOAT3.
	 */
	void setTangents(const std::vector<DirectX::XMFLOAT3>* p_Tangents);

	/**
	 * This whants a pointer to the source information about texture coordinates. 
	 *
	 * @param p_TextureCoord is a vector pointer that contains DirectX::XMFLOAT2.
	 */
	void setTextureCoords(const std::vector<DirectX::XMFLOAT2>* p_TextureCoord);

	/**
	 * This whants a pointer to the source information about weights and joint per vertex. 
	 *
	 * @param p_WeightsList is a vector pointer that contains a pair of DirectX::XMFLOAT3 and DirectX::XMFLOAT4.
	 */
	void setWeightsList(const std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMINT4>>* p_WeightsList);

	/**
	 * This whants a pointer to the source information about each joint. 
	 *
	 * @param p_ListOfJoints is a vector pointer that contains ModelLoader::Joint.
	 */
	void setListOfJoints(const std::vector<ModelLoader::Joint>* p_ListOfJoints);

	/**
	 * This whants the number of frames in the animation. 
	 *
	 * @param p_NumberOfFrames is an int.
	 */
	void setNumberOfFrames(int p_NumberOfFrames);

	/**
	 * This whants the mesh name. 
	 *
	 * @param p_MeshName is a std::string.
	 */
	void setMeshName(std::string p_MeshName);

protected:
	void intToByte(int p_Int, std::ostream* p_Output);
	void stringToByte(std::string p_String, std::ostream* p_Output);

	void createHeader(std::ostream* p_Output);
	bool createModelHeaderFile(std::string p_FilePath);
	void createAnimationHeader(std::ostream* p_AnimationOutput);
	void createMaterial(std::ostream* p_Output);
	void createMaterialBuffer(std::ostream* p_Output);
	void createVertexBuffer(std::ostream* p_Output);
	void createVertexBufferAnimation(std::ostream* p_Output);
	void createJointBuffer(std::ostream* p_Output);
private:
	void clearData();
	void byteToString(std::istream& p_Input, std::string& p_Return);
	void byteToInt(std::istream& p_Input, int& p_Return);
	std::string getPath(std::string p_FilePath);
};