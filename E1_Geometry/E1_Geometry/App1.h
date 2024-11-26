// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "TextureShader.h"
#include "ColourTriangle.h"
#include "GreyScale.h"
#include "CompSlider.h"
#include "MovementShader.h"
#include "StructureTensor.h"
#include "Skybox.h"
#include "OceanShader.h"
#include "HorizontalBlur.h"
#include "VerticalBlur.h"
#include "BilateralFilter.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:

	void FirstPass();
	void GreyScalePass();
	void StructureTensorPass();
	void HorizontalSmoothingPass();
	void VerticalSmoothingPass();
	void BilateralFilterPass(RenderTexture* input, RenderTexture* output, bool isHorizontal);
	void ComparisonPass();
	void FinalPass();
	bool Render();
	void GUI();

	void InitialiseShaders(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void LoadIntextures();
	void InitialiseMeshs(int screenWidth, int screenHeight);
	void InitialiseVariables();
	void InitialiseRenderTextures(int screenWidth, int screenHeight);
	void InitaliseLights();

private:

	//Shader Declaration
	TextureShader* textureShader;
	GreyScale* greyscaleShader;
	CompSlider* comparisonShader;
	MovementShader* movementShader;
	Watercolour* structureTensorShader;
	Skybox* skyboxShader;
	OceanShader* oceanShader;
	HorizontalBlur* horizontalBlurShader;
	VerticalBlur* verticalBlurShader;
	BilateralFilter* bilateralFilterShader;

	//Mesh Declaration
	PlaneMesh* floor;
	PlaneMesh* ocean;
	OrthoMesh* orthoMesh; 
	SphereMesh* sphere;
	CubeMesh* cube;
	CubeMesh* skybox;
	AModel* ship;


	//Render To Texture
	RenderTexture* renderTexture;
	RenderTexture* greyscaleTexture;
	RenderTexture* comparisonTexture;
	RenderTexture* structureTensorTexture;
	RenderTexture* horizontalBlurTexture;
	RenderTexture* verticalBlurTexture;
	RenderTexture* bilateralFilterTexture;
	RenderTexture* finalBilateralTexture;


	//Lighting
	Light* directionalLight;
	

	//GUI Variable Declaration
	bool greyscaleToggle;
	float comparisonSliderPosition;

	// Ocean Controls
	float amplitude;
	float frequency;
	float speed;
	float phases;
	float numWaves;
	float transparency;

	// Bilateral Filer Controls
	float range;
	float spatial;

	//Overarching Vairables
	float totalTime;
	ID3D11ShaderResourceView* skyMapTextures[6];
	ID3D11RasterizerState* skyboxRasterizerState;
	ID3D11ShaderResourceView* skyboxTexture;

};

#endif