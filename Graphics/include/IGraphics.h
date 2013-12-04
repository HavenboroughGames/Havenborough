#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class IGraphics
{
public:
	enum class ShaderType
	{
		VERTEX_SHADER = 1,
		PIXEL_SHADER = 2,
		GEOMETRY_SHADER = 4,
		HULL_SHADER = 8,
		DOMAIN_SHADER = 16
	};
	inline friend ShaderType operator|(ShaderType a, ShaderType b)
	{
		return static_cast<ShaderType>(static_cast<int>(a) | static_cast<int>(b)); 
	};

	inline friend bool operator&(ShaderType a, ShaderType b)
	{
		return static_cast<int>(a) & static_cast<int>(b) ? true : false; 
	};

	enum class Format
	{
		R32G32B32A32_FLOAT = 2,
		R32G32B32A32_UINT = 3,
		R32G32B32_FLOAT = 6,
		R32G32B32_UINT = 7,
		R32G32_FLOAT = 16,
		R32G32_UINT = 17,
		R8G8B8A8_UNORM = 28,
	};

	struct ShaderInputElementDescription
	{
		LPCSTR semanticName;
		UINT semanticIndex; 
		Format format;
		UINT inputSlot;
		UINT alignedByteOffset;
		UINT inputSlotClass;
		UINT instanceDataStepRate;
	};

	virtual ~IGraphics(void)
	{}

	/**
	* Create a pointer from which the graphics library can be accessed.
	* @return the graphics pointer
	*/
	__declspec(dllexport) static IGraphics *createGraphics(void);
	
	/**
	* Initialize the graphics API.
	* @param p_Hwnd the handle to the window which the graphics should be connected to
	* @param p_ScreenWidth input of the window's width
	* @param p_ScreenHeight input of the window's height
	* @param p_Fullscreen input whether the program should run in fullscreen or not
	* @return true if successful, otherwise false
	*/
	virtual bool initialize(HWND p_Hwnd, int p_ScreenWidth, int p_ScreenHeight,	bool p_Fullscreen) = 0;

	/**
	* Reinitialize parts of the graphics API when switching fullscreen on/off or changing resoluton.
	* @param p_Hwnd the handle to the window which the graphics should be connected to
	* @param p_ScreenWidth input of the window's width
	* @param p_ScreenHeight input of the window's height
	* @param p_Fullscreen input whether the program should run in fullscreen or not
	* @return true if successful, otherwise false
	*/
	virtual bool reInitialize(HWND p_Hwnd, int p_ScreenWidth, int p_ScreenHeight, bool p_Fullscreen) = 0;

	
	/**
	* Clear sub resources allocated by the graphics API and delete the pointer. 
	*/
	__declspec(dllexport) static void deleteGraphics(IGraphics *p_Graphics);

	/**
	* Creates a new model and stores in a vector connected with an ID.
	* @param p_ModelId the ID of the model
	* @param p_Filename the filename of the model
	*/
	virtual bool createModel(const char *p_ModelId, const char *p_Filename) = 0;

	/**
	* Automatically creates a shader based on layout in the shader file and stores in a vector connected with and ID.
	* @param p_ShaderId the ID of the shader
	* @param p_Filename the file where the shader code is located
	* @param p_EntryPoint the main entry point in the shader file, can be combined as e.g.
	*		 "mainVS,mainPS,mainGS,mainHS,mainDS", note this order is important to be kept but all steps are not necessary,
	*		 note the ',' is the separator
	* @param p_ShaderModel the shader model version to be used, e.g. "5_0" 
	* @param p_ShaderType the shader types to be created, can be combined as
	*		 ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER | ShaderType::GEOMETRY_SHADER | ShaderType::HULL_SHADER | ShaderType::DOMAIN_SHADER
	*/
	virtual void createShader(const char *p_shaderId, LPCWSTR p_Filename,
		const char *p_EntryPoint, const char *p_ShaderModel, ShaderType p_Type) = 0;

	/**
	* Creates a new shader object with user defined vertex layout. If shader ID already exists or no vertex shader type 
	* is added an exception is thrown.
	* @param p_ShaderId the ID of the shader that should be created, note if the ID already exists an exception will be thrown
	* @param p_Filename the file where the shader code is located
	* @param p_EntryPoint the main entry point in the shader file, can be combined as e.g.
	*		 "mainVS,mainPS,mainGS,mainHS,mainDS", note this order is important to be kept but all steps are not necessary,
	*		 note the ',' is the separator
	* @param p_ShaderModel the shader model version to be used, e.g. "5_0" 
	* @param p_ShaderType the shader types to be created, can be combined as
	*		 ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER | ShaderType::GEOMETRY_SHADER | ShaderType::HULL_SHADER | ShaderType::DOMAIN_SHADER,
	*		 note that vertex shader needs to be included or an exception will be thrown
	* @param p_VertexLayout the user defined vertex layout
	* @param p_NumOfElement the number of elements in the layout
	*/
	virtual void createShader(const char *p_shaderId, LPCWSTR p_Filename,
		const char *p_EntryPoint, const char *p_ShaderModel, ShaderType p_Type,
		ShaderInputElementDescription *p_VertexLayout, unsigned int p_NumOfElements) = 0;

	/**
	* Establish a map of shader name to a model name.
	* @param p_ShaderId name of the shader
	* @param p_ModelId name of the model
	*/
	virtual void linkShaderToModel(const char *p_ShaderId, const char *p_ModelId) = 0;

	/**
	* Creates a new texture and stores in a vector connected with an ID.
	* @param p_TextureId the ID of the texture
	* @param p_Filename the filename of the texture
	*/
	virtual bool createTexture(const char *p_TextureId, const char *p_FilePath) = 0;
	
	/**
	* 
	*/
	virtual void addStaticLight(void) = 0;

	/**
	* 
	*/
	virtual void removeStaticLight(void) = 0;

	/**
	* 
	*/
	virtual void useFrameLight(void) = 0;

	/**
	* Renders a model specified with an ID.
	* @param p_ModelId the ID of the model to be rendered
	*/
	virtual void renderModel(char *p_ModelId) = 0;

	/**
	* 
	*/
	virtual void renderText(void) = 0;

	/**
	* 
	*/
	virtual void renderQuad(void) = 0;
	
	/**
	* Draw the current frame.
	*/
	virtual void drawFrame(void) = 0;

private:

	/**
	* Release the sub resources allocated by the graphics API.
	*/
	virtual void shutdown(void) = 0;
};

