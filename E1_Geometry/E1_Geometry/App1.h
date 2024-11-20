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
#include "Watercolour.h"
#include "Skybox.h"
#include "OceanShader.h"

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
	void WatercolourPass();
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
	Watercolour* watercolourShader;
	Skybox* skyboxShader;
	OceanShader* oceanShader;

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
	RenderTexture* watercolourTexture;

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

	//Overarching Vairables
	float totalTime;
	ID3D11ShaderResourceView* skyMapTextures[6];
	ID3D11RasterizerState* skyboxRasterizerState;
	ID3D11ShaderResourceView* skyboxTexture;

};

#endif