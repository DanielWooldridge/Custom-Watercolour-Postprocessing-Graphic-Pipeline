// Application.h
#ifndef _APP1_H
#define _APP1_H
#define STORED_FRAMES 10

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
#include "DifferenceOfGuassian.h"
#include "FlowCurve.h"
#include "DoG_via_FlowCurve.h"
#include "ColourQuantization.h"
#include "CartoonRendering.h"
#include "PaperShader.h"
#include "DepthShader.h"
#include "ArcBallCamera.h"
#include "TemporalCoherence.h"
#include "RGBToYCBCR.h"
#include "YCBCRToRGB.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:

	void DepthPass();
	void FirstPass();
	void RGBToYCBCRPass();
	void GreyScalePass();
	void StructureTensorPass();
	void HorizontalSmoothingPass();
	void VerticalSmoothingPass();
	void BilateralFilterPass(RenderTexture* input, RenderTexture* output, bool isHorizontal);
	void DoGFilterPass();
	void FlowCurvePass();
	void DoGFlowPass();
	void ColourQuantizationPass();
	void CartoonRenderingPass();
	void PaperRenderingPass();
	void ComparisonPass();
	void TemporalPass();
	void YCBCRToRGBPass();
	void FinalPass();
	bool Render();


	void UpdateCamera(float deltaTime);
	void GUI();

	void InitialiseShaders(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void LoadIntextures();
	void InitialiseMeshs(int screenWidth, int screenHeight);
	void InitialiseVariables(int screenWidth, int screenHeight);
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
	DifferenceOfGuassian* dogFilterShader;
	FlowCurve* flowCurveShader;
	DoG_via_FlowCurve* dogFlowShader;
	ColourQuantization* cqShader;
	CartoonRendering* cartoonShader;
	PaperShader* paperShader;
	DepthShader* depthShader;
	TemporalCoherence* temporalShader;
	RGBToYCBCR* rgbToYcbcrShader;
	YCBCRToRGB* ycbcrToRgbShader;

	//Mesh Declaration
	PlaneMesh* floor;
	PlaneMesh* ocean;
	OrthoMesh* orthoMesh; 
	SphereMesh* sphere;
	CubeMesh* cube;
	CubeMesh* skybox;
	AModel* ship;


	//Render To Texture
	RenderTexture* depthTexture;
	RenderTexture* renderTexture;
	RenderTexture* greyscaleTexture;
	RenderTexture* comparisonTexture;
	RenderTexture* structureTensorTexture;
	RenderTexture* horizontalBlurTexture;
	RenderTexture* verticalBlurTexture;
	RenderTexture* bilateralFilterTexture;
	RenderTexture* finalBilateralTexture;
	RenderTexture* dogFilterTexture;
	RenderTexture* flowCurveTexture;
	RenderTexture* dogFlowTexture;
	RenderTexture* colourQuantizationTexture;
	RenderTexture* cartoonRenderTexture;
	RenderTexture* paperRenderTexture;
	RenderTexture* blendedTexture;
	RenderTexture* previousFrameTexture;
	RenderTexture* ycbcrTexture;
	RenderTexture* rgbTexture;



	//Lighting
	Light* directionalLight;
	

	//GUI Variable Declaration
	bool greyscaleToggle;
	float comparisonSliderPosition;
	int selectedTexture = 0; 


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

	// Dog Controls
	float sensitivity;
	float smoothing;
	float tau;
	XMFLOAT2 texelSize;

	// Flow Curve Controls
	XMFLOAT2 currentPosition;
	XMFLOAT2 previousTan;
	float totLength;
	float curLength;

	// dog flow Controls
	float dogFlowSmoothing;
	float dogFlowThreshold;

	// Colour quantization controls
	float transitionSmoothing;
	float quantLevel;

	// Paper Shader Controls
	float paperStrength;
	float depthFactor; 

	// Temporal Coherence
	float blendStrength;
	int frameIndex;
	RenderTexture* frameBuffer[STORED_FRAMES];

	ArcBallCamera arcballCamera;
	bool useArcball = false;  // Toggle for ImGui
	DirectX::XMFLOAT3 lastFreeCameraPosition; // Store previous free camera position

	float movementIndicator;
	const float MOVEMENT_NONE = 0;   // No movement 
	const float MOVEMENT_WAVE = 1;   // Ocean waves
	const float MOVEMENT_SINE = 2;   // Floating movement 


	int frameCount = 0;

	//Overarching Vairables
	float totalTime;
	ID3D11ShaderResourceView* skyMapTextures[6];
	ID3D11RasterizerState* skyboxRasterizerState;
	ID3D11ShaderResourceView* skyboxTexture;

	ID3D11ShaderResourceView* paperTexture;

};

#endif