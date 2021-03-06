#pragma once

#include <fstream>
#include <sstream>
#include <DirectXMath.h>
#include <memory>
#include <vector>

#include <tinyxml2\tinyxml2.h>

class ModelLoader
{
public:

	struct KeyFrame
	{
		DirectX::XMFLOAT3 m_Trans;
		DirectX::XMFLOAT4 m_Rot;
		DirectX::XMFLOAT3 m_Scale;
	};

	struct Joint
	{
		std::string m_JointName;
		int m_ID;
		int m_Parent;
		DirectX::XMFLOAT4X4 m_JointOffsetMatrix;
		std::vector<KeyFrame> m_JointAnimation;
	};

	struct IndexDesc
	{
		std::string m_MaterialID;
		int m_Vertex;
		int m_Tangent;
		int m_Normal;
		int m_TextureCoord;
	};
	
	struct Material
	{
		std::string m_MaterialID;
		std::string m_DiffuseMap;
		std::string m_NormalMap;
		std::string m_SpecularMap;
	};

private:
	
	std::string m_MeshName;
	int m_NumberOfTriangles;
	bool m_Transparent, m_Collidable;
	std::vector<IndexDesc> m_Indices;

	float m_Start, m_End;
	int m_NumberOfFrames, m_NumberOfVertices, m_NumberOfMaterials;
	std::vector<DirectX::XMFLOAT3> m_Vertices, m_Normals, m_Tangents;
	std::vector<DirectX::XMFLOAT2> m_TextureCoord;
	std::vector<Material> m_Material;
	std::vector<std::vector<IndexDesc>> m_IndexPerMaterial;
	std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMINT4>> m_WeightsList;
	std::vector<Joint> m_ListOfJoints;
	
	std::stringstream m_Stringstream;
	
public:
	
	/**
	 * Constructor.
	 */
	ModelLoader();
	
	/**
	 * Destructor.
	 */
	~ModelLoader();
	
	/**
	 * Use this function to release the memory in loader vectors.
	 */
	void clear();
	
	/**
	 * Creates vectors with information from the requested file.
	 *
	 * @param p_FilePath, the absolute path to the requested file.
	 * @return false if something is wrong when loading file.
	 */
	bool loadFile(std::string p_FilePath, std::string p_ResourceListLocation);

	/**
	 * Returns the stored information about vertices as a vector with Float3 values. 
	 *
	 * @returns a vector of vertices.
	 */
	const std::vector<DirectX::XMFLOAT3>& getVertices() const;

	/**
	 * Returns a bool variable that is true if the object is transparent.
	 * 
	 * @returns a bool.
	 */
	bool getTransparent() const;

	/**
	 * Returns a bool variable that is true if the object is Collidable.
	 * 
	 * @returns a bool.
	 */
	bool getCollidable() const;

	/**
	 * Returns the stored information about indices as a vector with Float3 values.
	 *
	 * @returns a vector of indices.
	 */
	const std::vector<std::vector<ModelLoader::IndexDesc>>& getIndices() const;

	/**
	 * Returns the stored information about the materials that are used by the model.
	 *
	 * @returns a vector of material structs.
	 */
	const std::vector<ModelLoader::Material>& getMaterial() const;

	/**
	 * Returns the stored information about normal as a vector with Float3.
	 *
	 * @returns a vector of normals.
	 */
	const std::vector<DirectX::XMFLOAT3>& getNormals() const;

	/**
	 * Returns the stored information about tangents as a vector with Float3. 
	 *
	 * @returns a vector of tangents.
	 */
	const std::vector<DirectX::XMFLOAT3>& getTangents() const;

	/**
	 * Returns the stored information about uv coordinates as a vector with Float2. 
	 *
	 * @returns a vector of texture coords.
	 */
	const std::vector<DirectX::XMFLOAT2>& getTextureCoords() const;
	
	/**
	 * Returns a paired vector with information about weights and to what joint it is weighted.
	 *
	 * @returns a paired vector of weights and joints.
	 */
	const std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMINT4>>& getWeightsList() const;
	
	/**
	 * Returns a list of joint structs that includes information about animation and parents.
	 *
	 * @return a vector of Joint. 
	 */
	const std::vector<Joint>& getListOfJoints() const;

	/**
	 * Returns the start number for the animation. 
	 *
	 * @returns a float with the start value
	 */
	float getAnimationStartValue() const;

	/**
	 * Returns the end number for the animation. 
	 *
	 * @returns a float with the end value
	 */
	float getAnimationEndValue() const;

	/**
	 * Returns the number of frames for the animation.
	 *
	 * @returns an in number of frames.
	 */
	int getNumberOfFrames() const;

	/**
	 * Returns a mesh name.
	 *
	 * @returns a string.
	 */
	std::string getMeshName() const;

protected: 
	void startReading(std::istream& p_Input);

	void readHeader(std::istream& p_Input);

	void readMaterials(std::istream& p_Input);

	void readVertex(std::istream& p_Input); 

	void readNormals(std::istream& p_Input);

	void readUV(std::istream& p_Input);

	void readTangents(std::istream& p_Input);

	void readFaces(std::istream& p_Input);

	void readWeights(std::istream& p_Input);

	void readHierarchy(std::istream& p_Input);

	void readJointOffset(std::istream& p_Input);

	void readAnimation(std::istream& p_Input);
	
	void printOutResourceInfo(std::string p_ResourceListLocation);

private:
	void printPath(tinyxml2::XMLElement* p_Ele, std::string p_Path);
	tinyxml2::XMLElement* searchForElement(tinyxml2::XMLDocument& p_Doc, tinyxml2::XMLElement* p_Parent, std::string p_ElementName, std::string p_Attribute, std::string p_AttributeValue);
	void clearData();

};