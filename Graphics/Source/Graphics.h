#pragma once
#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#include <windows.h>
#include <vector>
#include <string>
#include <d3d11.h>
#include <dxgi.h>

#include "../include/IGraphics.h"
#include "GraphicsExceptions.h"
#include "TextureLoader.h"
#include "DeferredRenderer.h"
#include "WrapperFactory.h"
#include "ModelFactory.h"
#include "ModelInstance.h"
#include "VRAMMemInfo.h"

#include "ShaderStructs.h"

using std::string;
using std::vector;
using std::pair;
using std::make_pair;

class Graphics : public IGraphics
{
private:
	ID3D11Device					*m_Device;
	ID3D11DeviceContext				*m_DeviceContext;

	IDXGISwapChain					*m_SwapChain;
	ID3D11RenderTargetView			*m_RenderTargetView;
	
	ID3D11RasterizerState			*m_RasterState;

	ID3D11Texture2D					*m_DepthStencilBuffer;
	ID3D11DepthStencilState			*m_DepthStencilState;
	ID3D11DepthStencilView			*m_DepthStencilView;

	unsigned int					m_Numerator;
	unsigned int					m_Denominator;
	char							m_GraphicsCard[128];
	int								m_GraphicsMemory;
	bool							m_VSyncEnabled;

	XMFLOAT4X4 m_ViewMatrix;
	XMFLOAT4X4 m_ProjectionMatrix;
	XMFLOAT3 m_Eye;

	static const unsigned int m_MaxLightsPerLightInstance;
	TextureLoader					m_TextureLoader;	
	WrapperFactory					*m_WrapperFactory;
	ModelFactory *m_ModelFactory;
	VRAMMemInfo						*m_VRAMMemInfo;

	vector<pair<string, Shader*>>	m_ShaderList;
	vector<pair<string, ModelDefinition>> m_ModelList;
	vector<pair<string, ID3D11ShaderResourceView*>> m_TextureList;
	vector<pair<int, ModelInstance>> m_ModelInstances;
	int m_NextInstanceId;
	
	DeferredRenderer *m_DeferredRender;

	//Lights
	std::vector<Light>			m_SpotLights;
	std::vector<Light>			m_PointLights;
	std::vector<Light>			m_DirectionalLights;

	Shader *m_Shader; //DEBUG
	ID3D11SamplerState *m_Sampler;

	IGraphics::loadModelTextureCallBack m_LoadModelTexture;
	void *m_LoadModelTextureUserdata;
	
	IGraphics::releaseModelTextureCallBack m_ReleaseModelTexture;
	void *m_ReleaseModelTextureUserdata;

public:
	Graphics(void);
	~Graphics(void);

	bool initialize(HWND p_Hwnd, int p_ScreenWidth, int p_ScreenHeight,	bool p_Fullscreen) override;
	bool reInitialize(HWND p_Hwnd, int p_ScreenWidht, int p_ScreenHeight, bool p_Fullscreen) override;
	
	bool createModel(const char *p_ModelId, const char *p_Filename) override;
	bool releaseModel(const char *p_ModelID) override;

	void createShader(const char *p_shaderId, LPCWSTR p_Filename,
		const char *p_EntryPoint, const char *p_ShaderModel, ShaderType p_Type) override;
	void createShader(const char *p_shaderId, LPCWSTR p_Filename,
		const char *p_EntryPoint, const char *p_ShaderModel, ShaderType p_Type,
		ShaderInputElementDescription *p_VertexLayout, unsigned int p_NumOfInputElements) override;
	void linkShaderToModel(const char *p_ShaderId, const char *p_ModelId) override;
	
	bool createTexture(const char *p_TextureId, const char *p_filename) override;
	bool releaseTexture(const char *p_TextureID) override;	

	void addStaticLight(void) override;
	void removeStaticLight(void) override;
	
	
	
	void useFramePointLight(Vector3 p_LightPosition, Vector3 p_LightColor, float p_LightRange) override;
	void useFrameSpotLight(Vector3 p_LightPosition, Vector3 p_LightColor, Vector3 p_LightDirection,
		Vector2 p_SpotLightAngles,	float p_LightRange) override;
	void useFrameDirectionalLight(Vector3 p_LightColor, Vector3 p_LightDirection) override;
	
	void renderModel(int p_ModelId) override;
	void renderText(void) override;
	void renderQuad(void) override;
	void drawFrame(int i) override;

	void updateAnimations(float p_DeltaTime) override;
	void playAnimation(int p_Instance, char* p_ClipName, bool p_Layer, bool p_Crossfade, int p_FadeFrames, float p_ExtraTrackWeight, int p_Track) override;

	int getVRAMMemUsage(void) override;
	
	int createModelInstance(const char *p_ModelId) override;
	void eraseModelInstance(int p_Instance) override;
	void setModelPosition(int p_Instance, Vector3 p_Position) override;
	void setModelRotation(int p_Instance, Vector3 p_YawPitchRoll) override;
	void setModelScale(int p_Instance, Vector3 p_Scale) override;
	void applyIK_ReachPoint(int p_Instance, const char* p_TargetJoint, const char* p_HingeJoint, const char* p_BaseJoint, Vector3 p_Target) override;
	Vector3 getJointPosition(int p_Instance, const char* p_Joint) override;

	void updateCamera(Vector3 p_Position, float p_Yaw, float p_Pitch) override;

	void setLogFunction(clientLogCallback_t p_LogCallback) override;

	void setLoadModelTextureCallBack(loadModelTextureCallBack p_LoadModelTexture, void* p_Userdata) override;
	void setReleaseModelTextureCallBack(releaseModelTextureCallBack p_ReleaseModelTexture, void* p_Userdata) override;

private:
	void shutdown(void) override;

	void setViewPort(int p_ScreenWidth, int p_ScreenHeight);
	HRESULT createDeviceAndSwapChain(HWND p_Hwnd, int p_ScreenWidth, int p_ScreenHeight, bool p_Fullscreen);
	HRESULT createRenderTargetView(void);
	HRESULT createDepthStencilBuffer(int p_ScreenWidth,	int p_ScreenHeight);
	HRESULT createDepthStencilState(void);
	HRESULT createDepthStencilView(void);
	HRESULT createRasterizerState(void);
	void initializeMatrices(int p_ScreenWidth, int p_ScreenHeight);
	
	Shader *getShaderFromList(string p_Identifier);
	ModelDefinition *getModelFromList(string p_Identifier);
	ID3D11ShaderResourceView *getTextureFromList(string p_Identifier);
	int calculateTextureSize(ID3D11ShaderResourceView *p_Texture);
	void Begin(float color[4]);
	void End(void);

	//TODO: Remove later
	void DebugDefferedDraw(void);
};