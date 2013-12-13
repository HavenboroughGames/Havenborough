#include "ModelFactory.h"


#include <boost/filesystem.hpp>

ModelFactory *ModelFactory::m_Instance = nullptr;

ModelFactory *ModelFactory::getInstance(void)
{
	if(!m_Instance)
		m_Instance = new ModelFactory();

	return m_Instance;
}

void ModelFactory::initialize(vector<pair<string, ID3D11ShaderResourceView*>> *p_TextureList)
{
	if(!m_Instance)
		throw ModelFactoryException("Error when initializing ModelFactory, no instance exists", __LINE__, __FILE__);

	m_TextureList = p_TextureList;
}

void ModelFactory::shutdown(void)
{
	SAFE_DELETE(m_Instance);
}

ModelDefinition ModelFactory::createModel(const char *p_Filename, bool p_IsAnimated)
{
	ModelBinaryLoader modelLoader;
	modelLoader.loadBinaryFile(p_Filename);

	ModelDefinition model;
	Buffer::Description bufferDescription;
	const vector<Material> &materialData = modelLoader.getMaterial();
	const vector<MaterialBuffer> &materialBufferData = modelLoader.getMaterialBuffer();
	
	if(!p_IsAnimated)
	{
		const vector<Vertex> &vertexData = modelLoader.getVertexBuffer();
		bufferDescription = createBufferDescription(vertexData, Buffer::Usage::USAGE_IMMUTABLE); //Change to default when needed to change data.
	}
	else
	{
		const vector<VertexAnimation> &vertexData = modelLoader.getAnimationVertexBuffer();
		bufferDescription = createBufferDescription(vertexData, Buffer::Usage::USAGE_IMMUTABLE); //Change to default when needed to change data.
	}
	std::unique_ptr<Buffer> vertexBuffer(WrapperFactory::getInstance()->createBuffer(bufferDescription));

	boost::filesystem::path modelPath(p_Filename);
	boost::filesystem::path parentDir(modelPath.parent_path());

	vector<pair<int, int>> tempInterval(materialBufferData.size());
	for(unsigned int i = 0; i < materialBufferData.size(); i++)
	{
		tempInterval.at(i).first = materialBufferData.at(i).start;
		tempInterval.at(i).second = materialBufferData.at(i).length;
	}
	loadTextures(model, p_Filename, materialData.size() , materialData);

	model.vertexBuffer.swap(vertexBuffer);
	model.drawInterval = tempInterval;
	model.numOfMaterials = materialData.size();

	modelLoader.clear();

	return model;
}

void ModelFactory::setLoadModelTextureCallBack(loadModelTextureCallBack p_LoadModelTexture, void* p_Userdata)
{
	m_LoadModelTexture = p_LoadModelTexture;
	m_LoadModelTextureUserdata = p_Userdata;
}

ModelFactory::ModelFactory(void)
{
}

ModelFactory::~ModelFactory(void)
{
}

Buffer::Description ModelFactory::createBufferDescription(const vector<Vertex> &p_VertexData, Buffer::Usage p_Usage)
{
	Buffer::Description bufferDescription;
	bufferDescription.initData = p_VertexData.data();
	bufferDescription.numOfElements = p_VertexData.size();
	bufferDescription.sizeOfElement = sizeof(Vertex);
	bufferDescription.type = Buffer::Type::VERTEX_BUFFER;
	bufferDescription.usage = p_Usage;
	
	return bufferDescription;
}

Buffer::Description ModelFactory::createBufferDescription(const vector<VertexAnimation> &p_VertexData,
	Buffer::Usage p_Usage)
{
	Buffer::Description bufferDescription;
	bufferDescription.initData = p_VertexData.data();
	bufferDescription.numOfElements = p_VertexData.size();
	bufferDescription.sizeOfElement = sizeof(VertexAnimation);
	bufferDescription.type = Buffer::Type::VERTEX_BUFFER;
	bufferDescription.usage = p_Usage;

	return bufferDescription;
}

void ModelFactory::loadTextures(ModelDefinition &p_Model, const char *p_Filename, unsigned int p_NumOfMaterials,
	const vector<Material> &p_Materials)
{
	using std::pair;

	boost::filesystem::path modelPath(p_Filename);
	boost::filesystem::path parentDir(modelPath.parent_path());

	vector<pair<string, ID3D11ShaderResourceView*>> diffuse;
	vector<pair<string, ID3D11ShaderResourceView*>> normal;
	vector<pair<string, ID3D11ShaderResourceView*>> specular;

	for(unsigned int i = 0; i < p_NumOfMaterials; i++)
	{
		const Material &material = p_Materials.at(i);
		boost::filesystem::path diff = (material.m_DiffuseMap == "NONE") ?
			"assets/Default/Default_COLOR.jpg" : parentDir / material.m_DiffuseMap;
		boost::filesystem::path norm = (material.m_NormalMap == "NONE" || material.m_NormalMap == "Default_NRM.jpg") ?
			"assets/Default/Default_COLOR.jpg" : parentDir / material.m_NormalMap;
		boost::filesystem::path spec = (material.m_SpecularMap == "NONE" || material.m_SpecularMap == "Default_SPEC.jpg") ?
			"assets/Default/Default_SPEC.jpg" : parentDir / material.m_SpecularMap;

		m_LoadModelTexture(material.m_DiffuseMap.c_str(), diff.string().c_str(), m_LoadModelTextureUserdata);
		m_LoadModelTexture(material.m_NormalMap.c_str(), norm.string().c_str(), m_LoadModelTextureUserdata);
		m_LoadModelTexture(material.m_SpecularMap.c_str(), spec.string().c_str(), m_LoadModelTextureUserdata);

		diffuse.push_back(std::make_pair(material.m_DiffuseMap, getTextureFromList(material.m_DiffuseMap.c_str())));
		normal.push_back(std::make_pair(material.m_NormalMap, getTextureFromList(material.m_NormalMap.c_str())));
		specular.push_back(std::make_pair(material.m_SpecularMap, getTextureFromList(material.m_SpecularMap.c_str())));

	}

	p_Model.diffuseTexture = diffuse;
	p_Model.normalTexture = normal;
	p_Model.specularTexture = specular;
}

ID3D11ShaderResourceView *ModelFactory::getTextureFromList(string p_Identifier)
{
	for(auto it = m_TextureList->begin(); it != m_TextureList->end(); ++it)
	{
		if(it->first == p_Identifier)
		{
			return it->second;
		}
	}

	return nullptr;
}