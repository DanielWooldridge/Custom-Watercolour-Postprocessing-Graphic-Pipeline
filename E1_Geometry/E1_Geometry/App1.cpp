#include "App1.h"
using namespace std;
App1::App1()
{
	floor = nullptr;
	textureShader = nullptr;
	greyscaleToggle = false;
	comparisonSliderPosition = 0.5f;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	camera->setPosition(0, 15, 0);
	camera->setRotation(0, 45, 0);

	// Initialises the shaders that will be used
	InitialiseShaders(hinstance, hwnd, screenWidth, screenHeight);

	// Loads in Textures and assigns a string 
	LoadIntextures();

	// Initialise the Meshs on Scene
	InitialiseMeshs(screenWidth, screenHeight);

	// Initialise Render Textures
	InitialiseRenderTextures(screenWidth, screenHeight);

	// Initialises Variables - GUI related
	InitialiseVariables(screenWidth, screenHeight);

	// Initialises Lights
	InitaliseLights();

	// Rasterizer state for rendering skybox
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));

	// Set the properties for the rasterizer state
	rasterDesc.FillMode = D3D11_FILL_SOLID;  // Solid fill
	rasterDesc.CullMode = D3D11_CULL_NONE;   // No culling for the skybox (so all faces are visible)
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
	rasterDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterDesc.DepthClipEnable = true;        // Depth clipping enabled
	rasterDesc.ScissorEnable = false;         // No scissor test
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;

	// Create the rasterizer state
	renderer->getDevice()->CreateRasterizerState(&rasterDesc, &skyboxRasterizerState);



}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if (floor)
	{
		delete floor;
		floor = 0;
	}

	if (textureShader)
	{
		delete textureShader;
		textureShader = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::Render()
{
	
	FirstPass();

	GreyScalePass();

	StructureTensorPass();

	HorizontalSmoothingPass();

	VerticalSmoothingPass();

	BilateralFilterPass(renderTexture, bilateralFilterTexture, true); 

	BilateralFilterPass(bilateralFilterTexture, finalBilateralTexture, false); 

	DoGFilterPass();

	FlowCurvePass();

	DoGFlowPass();

	ColourQuantizationPass();

	CartoonRenderingPass();

	PaperRenderingPass();

	ComparisonPass();
	
	FinalPass();

	return true;
}


void App1::FirstPass()
{
	// Clear the scene. (default blue colour)
	renderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.5f, 0.8f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Time
	totalTime += timer->getTime();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	//// Save the current rasterizer state and set the skybox-specific one.
 //   ID3D11RasterizerState* originalRasterizerState;
 //   renderer->getDeviceContext()->RSGetState(&originalRasterizerState);
 //   renderer->getDeviceContext()->RSSetState(skyboxRasterizerState);



 //   // Render Skybox
	//XMMATRIX skyboxViewMatrix = XMMatrixIdentity(); // No translation, keep skybox stationary


	//XMMATRIX skyboxTranslationMatrix = XMMatrixTranslation(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
 //   //XMMATRIX skyboxTranslationMatrix = XMMatrixTranslation(25, 10, 25);
 //   XMMATRIX skyboxScalingMatrix = XMMatrixScaling(100.0f, 100.0f, 100.0f);
 //   XMMATRIX skyboxTransformedWorldMatrix = worldMatrix * skyboxScalingMatrix * skyboxTranslationMatrix;

 //   skybox->sendData(renderer->getDeviceContext());
 //   skyboxShader->setShaderParameters(renderer->getDeviceContext(), skyboxTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"skyboxTexture"));
 //   skyboxShader->render(renderer->getDeviceContext(), skybox->getIndexCount());

 //    //Reset rasterizer state back to original for the rest of the scene.

 //   renderer->getDeviceContext()->RSSetState(originalRasterizerState);
 //   if (originalRasterizerState) { originalRasterizerState->Release(); }


	// Render Floor
	//floor->sendData(renderer->getDeviceContext());
	//textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"));
	//textureShader->render(renderer->getDeviceContext(), floor->getIndexCount());

	// Render Sphere
	XMMATRIX sphereTranslationMatrix = XMMatrixTranslation(60.0f, 10.0f, 50.0f); 
	XMMATRIX sphereScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f); 
	XMMATRIX sphereTransformedWorldMatrix = sphereScalingMatrix * sphereTranslationMatrix * worldMatrix;
	sphere->sendData(renderer->getDeviceContext());
	movementShader->setShaderParameters(renderer->getDeviceContext(), sphereTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"), totalTime, 0.0);
	movementShader->render(renderer->getDeviceContext(), floor->getIndexCount());

	// Render Cube
	XMMATRIX cubeTranslationMatrix = XMMatrixTranslation(50.0f, 10.0f, 62.5f);
	XMMATRIX cubeScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX cubeTransformedWorldMatrix = cubeScalingMatrix * cubeTranslationMatrix * worldMatrix;
	cube->sendData(renderer->getDeviceContext());
	movementShader->setShaderParameters(renderer->getDeviceContext(), cubeTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"), totalTime, 1.0);
	movementShader->render(renderer->getDeviceContext(), floor->getIndexCount());


	// Ship model
	XMMATRIX shipTranslationMatrix = XMMatrixTranslation(40.0f, 7.0f, 40.0f);
	XMMATRIX shipScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX shipRotationMatrix = XMMatrixRotationX(160);
	XMMATRIX shipTransformedWorldMatrix = shipRotationMatrix * shipScalingMatrix * shipTranslationMatrix * worldMatrix;
	ship->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), shipTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"shipWood"));
	textureShader->render(renderer->getDeviceContext(), ship->getIndexCount());

	// Render Ocean
	renderer->setAlphaBlending(true);
	XMMATRIX oceanTranslationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMMATRIX oceanScalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX oceanTransformedWorldMatrix = oceanScalingMatrix * oceanTranslationMatrix * worldMatrix;
	ocean->sendData(renderer->getDeviceContext());
	oceanShader->setShaderParameters(renderer->getDeviceContext(), oceanTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"water"), 
		totalTime, amplitude, frequency, speed, numWaves, phases, transparency, directionalLight);
	oceanShader->render(renderer->getDeviceContext(), floor->getIndexCount());
	renderer->setAlphaBlending(false);


}

void App1::GreyScalePass()
{

	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	greyscaleTexture->setRenderTarget(renderer->getDeviceContext());
	greyscaleTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.5f, 0.5f, 1.0f);

	// Get the world matrix, orthographic view matrix, and orthographic projection matrix
	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = greyscaleTexture->getOrthoMatrix();

	// Disable the depth (Z) buffer
	renderer->setZBuffer(false);

	// Send mesh data to the rendering context
	orthoMesh->sendData(renderer->getDeviceContext());

	// Set shader parameters for the horizontal blur shader
	greyscaleShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView());

	// Render using the horizontal blur shader
	greyscaleShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Re-enable the Z buffer
	renderer->setZBuffer(true);


	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}


void App1::StructureTensorPass()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	structureTensorTexture->setRenderTarget(renderer->getDeviceContext());
	structureTensorTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.5f, 0.5f, 1.0f);

	// Get the world matrix, orthographic view matrix, and orthographic projection matrix
	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = structureTensorTexture->getOrthoMatrix();

	// Disable the depth (Z) buffer
	renderer->setZBuffer(false);

	// Send mesh data to the rendering context
	orthoMesh->sendData(renderer->getDeviceContext());

	// Set shader parameters for the horizontal blur shader
	structureTensorShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView());

	// Render using the horizontal blur shader
	structureTensorShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Re-enable the Z buffer
	renderer->setZBuffer(true);


	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::HorizontalSmoothingPass()
{
	horizontalBlurTexture->setRenderTarget(renderer->getDeviceContext());
	horizontalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.5f, 0.5f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = horizontalBlurTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	horizontalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, structureTensorTexture->getShaderResourceView());
	horizontalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
	renderer->setBackBufferRenderTarget();
}

void App1::VerticalSmoothingPass()
{
	// Set the vertical blur render target
	verticalBlurTexture->setRenderTarget(renderer->getDeviceContext());
	verticalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = verticalBlurTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	// Disable Z-buffer for 2D rendering
	renderer->setZBuffer(false);

	// Render the vertical blur
	orthoMesh->sendData(renderer->getDeviceContext());
	verticalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, horizontalBlurTexture->getShaderResourceView());  // Use horizontal blur as input
	verticalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Re-enable Z-buffer
	renderer->setZBuffer(true);

	// Reset the render target
	renderer->setBackBufferRenderTarget();
}

void App1::BilateralFilterPass(RenderTexture* input, RenderTexture* output, bool isHorizontal)
{
	output->setRenderTarget(renderer->getDeviceContext());
	output->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = output->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	bilateralFilterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, input->getShaderResourceView(), verticalBlurTexture->getShaderResourceView(),
		isHorizontal ? 0 : 1, spatial, range);
	bilateralFilterShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);
	renderer->setBackBufferRenderTarget();



}

void App1::DoGFilterPass()
{
	dogFilterTexture->setRenderTarget(renderer->getDeviceContext());
	dogFilterTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = dogFilterTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	dogFilterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, finalBilateralTexture->getShaderResourceView(), verticalBlurTexture->getShaderResourceView(),
	sensitivity, smoothing, tau, texelSize);
	dogFilterShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
	renderer->setBackBufferRenderTarget();
}

// WE CAN ADD THIS INTO THE SECOND DOG PASS FOR OPTIMIZATION?
void App1::FlowCurvePass()
{

	flowCurveTexture->setRenderTarget(renderer->getDeviceContext());
	flowCurveTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = flowCurveTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	//std::cout << "Previous Tan: (" << previousTan.x << ", " << previousTan.y << ")\n";
	flowCurveShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, verticalBlurTexture->getShaderResourceView(), currentPosition, previousTan, totLength, curLength);
	flowCurveShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
}

void App1::DoGFlowPass()
{
	dogFlowTexture->setRenderTarget(renderer->getDeviceContext());
	dogFlowTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = dogFlowTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	
	dogFlowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, dogFilterTexture->getShaderResourceView(),  flowCurveTexture->getShaderResourceView(), // Flow Map Texture
	dogFlowSmoothing, dogFlowThreshold);
	dogFlowShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);


}

void App1::ColourQuantizationPass()
{

	colourQuantizationTexture->setRenderTarget(renderer->getDeviceContext());
	colourQuantizationTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = colourQuantizationTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());

	cqShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, finalBilateralTexture->getShaderResourceView(), transitionSmoothing, quantLevel);
	cqShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
}

void App1::CartoonRenderingPass()
{
	cartoonRenderTexture->setRenderTarget(renderer->getDeviceContext());
	cartoonRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = cartoonRenderTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	cartoonShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, dogFilterTexture->getShaderResourceView(), colourQuantizationTexture->getShaderResourceView());
	cartoonShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
}

void App1::PaperRenderingPass()
{
	paperRenderTexture->setRenderTarget(renderer->getDeviceContext());
	paperRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = paperRenderTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	paperShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, paperTexture, colourQuantizationTexture->getShaderResourceView());
	paperShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
}

void App1::ComparisonPass()
{
	// Set the comparison texture as the render target
	comparisonTexture->setRenderTarget(renderer->getDeviceContext());
	comparisonTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.5f, 0.5f, 1.0f);

	// Get orthographic matrices for rendering
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = comparisonTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	// Disable the depth buffer for 2D rendering
	renderer->setZBuffer(false);

	// Send ortho mesh data to the rendering context
	orthoMesh->sendData(renderer->getDeviceContext());

	// Determine which texture to pass based on user selection
	ID3D11ShaderResourceView* selectedResourceView = nullptr;
	switch (selectedTexture)
	{
	case 0: // Original Render Texture
		selectedResourceView = renderTexture->getShaderResourceView();
		break;
	case 1: // Bilateral Filter Texture
		selectedResourceView = bilateralFilterTexture->getShaderResourceView();
		break;
	case 2: // Final Bilateral Texture
		selectedResourceView = finalBilateralTexture->getShaderResourceView();
		break;
	case 3: // structure Tensor Texture
		selectedResourceView = structureTensorTexture->getShaderResourceView();
		break;
	case 4: // smoothed flow map Texture
		selectedResourceView = verticalBlurTexture->getShaderResourceView();
		break;
	case 5: //smoothed structure tensor Horizontally
		selectedResourceView = horizontalBlurTexture->getShaderResourceView();
		break;
	case 6: //smoothed structure tensor Horizontally
		selectedResourceView = dogFilterTexture->getShaderResourceView();
		break;
	case 7: // Flow Curve Texture
		selectedResourceView = flowCurveTexture->getShaderResourceView();
		break;
	case 8: // DoG Flow Texture
		selectedResourceView = dogFlowTexture->getShaderResourceView();
		break;
	case 9: // CQ Texture
		selectedResourceView = colourQuantizationTexture->getShaderResourceView();
		break;
	case 10:
		selectedResourceView = cartoonRenderTexture->getShaderResourceView();
		break;
	case 11:
		selectedResourceView = paperRenderTexture->getShaderResourceView();
		break;
	default:
		selectedResourceView = renderTexture->getShaderResourceView();
		break;
	}

	// Set shader parameters for the comparison shader
	comparisonShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), selectedResourceView, comparisonSliderPosition);

	// Render the comparison using the CompSlider shader
	comparisonShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Re-enable the depth buffer
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer
	renderer->setBackBufferRenderTarget();
}



void App1::FinalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// RENDER THE RENDER TEXTURE SCENE
	// Requires 2D rendering and an ortho mesh.
	renderer->setZBuffer(false);
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	// Select the appropriate texture based on ImGui preferences
	//auto* texture = greyscaleToggle ? greyscaleTexture->getShaderResourceView() : renderTexture->getShaderResourceView();
	auto* texture = comparisonTexture->getShaderResourceView();
	// Send mesh data once
	orthoMesh->sendData(renderer->getDeviceContext());

	// Set shader parameters and render with the selected texture
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, texture);
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);

	// Render GUI
	GUI();

	// Present the rendered scene to the screen.
	renderer->endScene();
}

void App1::InitialiseShaders(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	greyscaleShader = new GreyScale(renderer->getDevice(), hwnd);
	comparisonShader = new CompSlider(renderer->getDevice(), hwnd);
	movementShader = new MovementShader(renderer->getDevice(), hwnd);
	structureTensorShader = new Watercolour(renderer->getDevice(), hwnd);
	skyboxShader = new Skybox(renderer->getDevice(), hwnd);
	oceanShader = new OceanShader(renderer->getDevice(), hwnd);
	horizontalBlurShader = new HorizontalBlur(renderer->getDevice(), hwnd);
	verticalBlurShader = new VerticalBlur(renderer->getDevice(), hwnd);
	bilateralFilterShader = new BilateralFilter(renderer->getDevice(), hwnd);
	dogFilterShader = new DifferenceOfGuassian(renderer->getDevice(), hwnd);
	flowCurveShader = new FlowCurve(renderer->getDevice(), hwnd);
	dogFlowShader = new DoG_via_FlowCurve(renderer->getDevice(), hwnd);
	cqShader = new ColourQuantization(renderer->getDevice(), hwnd);
	cartoonShader = new CartoonRendering(renderer->getDevice(), hwnd);
	paperShader = new PaperShader(renderer->getDevice(), hwnd);
}

void App1::InitialiseMeshs(int screenWidth, int screenHeight)
{
	floor = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());				// Create Floor Mesh 
	ocean = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());				// Create Ocean Mesh 
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Ortho mesh
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());  // Sphere mesh
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());  // Cube mesh	
	skybox = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext(), 1); // Skybox mesh

	ship = new AModel(renderer->getDevice(), "res/models/ship.obj"); // https://sketchfab.com/3d-models/ship-g-4249f44c9f334432bd026a7dd7787058#download

}

void App1::InitialiseVariables(int screenWidth, int screenHeight)
{
	amplitude = 1;
	frequency = 0.1f;
	speed = 1;
	numWaves = 1;
	phases = 1;
	transparency = 1.0f;

	comparisonSliderPosition = 1.f;

	range = 0.1f; //maybe 0.2?
	spatial = 5.0f; //maybe 7.0?

	sensitivity = 5.6f;
	smoothing = 0.6f;
	tau = 1;
	texelSize = XMFLOAT2(1.0f / screenWidth, 1.0f / screenHeight);

	currentPosition = XMFLOAT2(0.5f, 0.5f);
	previousTan = XMFLOAT2(1.0f, -0.3f);
	totLength = 0.0f;
	curLength = 1.0f;

	dogFlowThreshold = 1.0f;
	dogFlowSmoothing = 1.5f;

	transitionSmoothing = 3.4f;
	quantLevel = 10.0f;
}

void App1::InitialiseRenderTextures(int screenWidth, int screenHeight)
{
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	greyscaleTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	comparisonTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	structureTensorTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	horizontalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	verticalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	bilateralFilterTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	finalBilateralTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	dogFilterTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	flowCurveTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	dogFlowTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	colourQuantizationTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	cartoonRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	paperRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
}

void App1::InitaliseLights()
{

	//Directional Light
	directionalLight = new Light();
	directionalLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight->setDirection(-1.0f, 0.0f, 0.0f);
	directionalLight->setPosition(40.0f, 30.0f, 40.0f);
	
}

void App1::LoadIntextures()
{
	textureMgr->loadTexture(L"grass", L"res/grass.jpg"); // Grass Texture
	textureMgr->loadTexture(L"wood", L"res/sand.jpg"); // Wood Texture
	textureMgr->loadTexture(L"water", L"res/water2.jpg"); // water Texture
	textureMgr->loadTexture(L"shipWood", L"res/shipWood.jpg"); // water Texture
	textureMgr->loadTexture(L"canvas", L"res/canvas.jpg");



	paperTexture = textureMgr->getTexture(L"canvas");


	textureMgr->loadTexture(L"skyboxTexture", L"res/Askymap.dds"); // CubeMap
	skyboxTexture = textureMgr->getTexture(L"skyboxTexture");
	//HRESULT hr = DirectX::CreateDDSTextureFromFile(
	//	renderer->getDevice(),
	//	renderer->getDeviceContext(),
	//	L"res/skymap.dds",
	//	nullptr, // No resource is needed, only the shader resource view
	//	&skyboxTexture
	//);
	//if (FAILED(hr)) {
	//	OutputDebugString(L"Failed to load cubemap texture!\n");
	//}


}

void App1::GUI()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	if (ImGui::TreeNode("Post-Processing"))
	{
		ImGui::SliderFloat("Slider Position", &comparisonSliderPosition, 0.0f, 1.0f);

		if (ImGui::TreeNode("Texture Selection"))
		{
			const char* textureOptions[] = { "Original Scene", "Bilateral Filter Texture", "Final Bilateral Texture", "Structure Tensor Texture", "Smoothed Flow Map Texture", 
				"Smooth Structure Tensor (Horiz)", "DoG Filter", "Flow Curve Calc", "Dog Flow Texture", "Colour Quantization Texture", "Cartoon Rendering Texture", 
				"Paper Texure"};
			ImGui::Combo("Output Texture", &selectedTexture, textureOptions, IM_ARRAYSIZE(textureOptions));
			ImGui::TreePop();
		}

		//ImGui::Checkbox("Greyscale mode", &greyscaleToggle);

		if (ImGui::TreeNode("Bilateral Filter Settings")) {
			ImGui::SliderFloat("Spatial Sigma", &spatial, 1.0f, 20.0f);  
			ImGui::SliderFloat("Range Sigma", &range, 0.01f, 1.0f);     
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("DoG Filter Settings")) {
			ImGui::SliderFloat("Edge Sensitivity", &spatial, 1.0f, 20.0f);  // Adjust spatial sigma
			ImGui::SliderFloat("Smoothing", &range, 0.01f, 5.0f);           // Adjust range sigma
			ImGui::SliderFloat("Edge Threshold (tau)", &tau, 0.0f, 2.0f);             // Adjust tau
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Flow Curve Settings"))
		{
			ImGui::SliderFloat2("Start Position", &currentPosition.x, 0.0f, 1.0f);
			ImGui::SliderFloat2("Initial Tangent", &previousTan.x, -1.0f, 1.0f);
			ImGui::SliderFloat("Step Length", &curLength, 0.01f, 10.0f);
			ImGui::SliderFloat("Total Traversal Length", &totLength, 0.0f, 100.0f);

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("DoG Curve Settings"))
		{
			ImGui::SliderFloat("Smoothing", &dogFlowSmoothing, 0.0f, 5.0f);
			ImGui::SliderFloat("Threshold", &dogFlowThreshold, -1.0f, 1.5f);

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Colour Quantization Settings"))
		{
			ImGui::SliderFloat("Smoothing", &transitionSmoothing, 0.0f, 5.0f);
			ImGui::SliderFloat("Quantization Level", &quantLevel, 0.0f, 20.0f);

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	

	// Ocean controller
	if (ImGui::TreeNode("Ocean"))
	{
		// Speed of the waves
		if (ImGui::TreeNode("Speed of Waves"))
		{
			ImGui::SliderFloat("Speed value", &speed, 0.1, 20);
			ImGui::TreePop();
		}

		// Frequency of the waves
		if (ImGui::TreeNode("Frequency of Waves"))
		{
			ImGui::SliderFloat("Frequency", &frequency, 0.1, 5);
			ImGui::TreePop();
		}

		// Amplitude of the waves
		if (ImGui::TreeNode("Amplitude of Waves"))
		{
			ImGui::SliderFloat("Amplitude", &amplitude, 0.1, 20);
			ImGui::TreePop();
		}
		// Amplitude of the waves
		if (ImGui::TreeNode("Number of Waves"))
		{
			ImGui::SliderFloat("WaveNumber", &numWaves, 0.1, 20);
			ImGui::TreePop();
		}
		// Amplitude of the waves
		if (ImGui::TreeNode("Phases"))
		{
			ImGui::SliderFloat("Phases", &phases, 0.1, 20);
			ImGui::TreePop();
		}
		// Amplitude of the waves
		if (ImGui::TreeNode("Wave Transparency"))
		{
			ImGui::SliderFloat("Transparency", &transparency, 0.1, 1);
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}





