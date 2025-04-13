#include "App1.h"
using namespace std;
App1::App1()
{
	floor = nullptr;
	textureShader = nullptr;
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

	UpdateCamera(timer->getTime());
	
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
	DepthPass();                 

	FirstPass();                 
	RGBToYCBCRPass();                      

	// Step 1: Structure Tensor
	StructureTensorPass();
	HorizontalSmoothingPass();    
	VerticalSmoothingPass();      

	// Step 2: Bilateral for edge
	for (int i = 0; i < bf_edge; ++i)
	{
		BilateralFilterPass(ycbcrTexture, bilateralFilterTexture, true);  
		BilateralFilterPass(bilateralFilterTexture, finalBilateralTexture, false); 
	}

	// Step 3: DoG
	DoGFilterPass();

	// Step 4: Flow-Guided DoG
	FlowCurvePass();

	// Step 5: Bilateral for abstraction
	for (int i = 0; i < bf_abstraction; ++i)
	{
		BilateralFilterPass(finalBilateralTexture, bilateralFilterTexture, true);  
		BilateralFilterPass(bilateralFilterTexture, finalBilateralTexture, false); 
	}

	// Step 6: CQ 
	ColourQuantizationPass();

	// Step 7: Blending
	CartoonRenderingPass();

	// Step 8: Paper texture overlay
	PaperRenderingPass();

	// Final conversions and presentation
	YCBCRToRGBPass();
	TemporalPass();
	ComparisonPass();
	FinalPass();

	return true;
}



void App1::DepthPass()
{
	// Set the depth render target and clear it
	depthTexture->setRenderTarget(renderer->getDeviceContext());
	depthTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	//Force-clear the depth buffer
	ID3D11DepthStencilView* depthStencil = nullptr;
	renderer->getDeviceContext()->OMGetRenderTargets(0, nullptr, &depthStencil);
	if (depthStencil)
	{
		renderer->getDeviceContext()->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
		depthStencil->Release(); // Release reference
	}

	
	// Generate the view matrix from the camera's perspective
	camera->update();

	// Get world, view, and projection matrices
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// Render Sphere
	movementIndicator = sine_movement;
	XMMATRIX sphereTranslationMatrix = XMMatrixTranslation(60.0f, 10.0f, 50.0f);
	XMMATRIX sphereScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX sphereTransformedWorldMatrix = sphereScalingMatrix * sphereTranslationMatrix * worldMatrix;
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), sphereTransformedWorldMatrix, viewMatrix, projectionMatrix, totalTime, amplitude, frequency, speed, numWaves, phases, transparency, sine_movement);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Render Cube
	movementIndicator = sine_movement;
	XMMATRIX cubeTranslationMatrix = XMMatrixTranslation(50.0f, 10.0f, 62.5f);
	XMMATRIX cubeScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX cubeTransformedWorldMatrix = cubeScalingMatrix * cubeTranslationMatrix * worldMatrix;
	cube->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), cubeTransformedWorldMatrix, viewMatrix, projectionMatrix, totalTime, amplitude, frequency, speed, numWaves, phases, transparency, sine_movement);
	depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	// Render Ship Model
	movementIndicator = no_movement;
	XMMATRIX shipTranslationMatrix = XMMatrixTranslation(40.0f, 7.0f, 40.0f);
	XMMATRIX shipScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX shipRotationMatrix = XMMatrixRotationX(160);
	XMMATRIX shipTransformedWorldMatrix = shipRotationMatrix * shipScalingMatrix * shipTranslationMatrix * worldMatrix;
	ship->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), shipTransformedWorldMatrix, viewMatrix, projectionMatrix, totalTime, amplitude, frequency, speed, numWaves, phases, transparency, no_movement);
	depthShader->render(renderer->getDeviceContext(), ship->getIndexCount());

	// Render Ocean
	movementIndicator = wave_movement;
	XMMATRIX oceanTranslationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMMATRIX oceanScalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX oceanTransformedWorldMatrix = oceanScalingMatrix * oceanTranslationMatrix * worldMatrix;
	ocean->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), oceanTransformedWorldMatrix, viewMatrix, projectionMatrix, totalTime, amplitude, frequency, speed, numWaves, phases, transparency, wave_movement);
	depthShader->render(renderer->getDeviceContext(), ocean->getIndexCount());



	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
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
		totalTime, amplitude, frequency, speed, numWaves, phases, transparency);
	oceanShader->render(renderer->getDeviceContext(), floor->getIndexCount());
	renderer->setAlphaBlending(false);


}

void App1::RGBToYCBCRPass()
{
	ycbcrTexture->setRenderTarget(renderer->getDeviceContext());
	ycbcrTexture->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 1);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX orthoMatrix = ycbcrTexture->getOrthoMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	rgbToYcbcrShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, orthoMatrix, renderTexture->getShaderResourceView());
	rgbToYcbcrShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
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
	structureTensorShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, ycbcrTexture->getShaderResourceView());

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


void App1::FlowCurvePass()
{

	flowCurveTexture->setRenderTarget(renderer->getDeviceContext());
	flowCurveTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = flowCurveTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	flowCurveShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, verticalBlurTexture->getShaderResourceView(), dogFilterTexture->getShaderResourceView(), currentPosition, previousTan, totLength, curLength);
	flowCurveShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

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
	cartoonShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, flowCurveTexture->getShaderResourceView(), colourQuantizationTexture->getShaderResourceView());
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
	paperShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, paperTexture, colourQuantizationTexture->getShaderResourceView(), depthTexture->getShaderResourceView(), paperStrength, depthFactor);
	paperShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
}


void App1::TemporalPass() 
{
	blendedTexture->setRenderTarget(renderer->getDeviceContext());
	blendedTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);


	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = blendedTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	temporalShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, previousFrameTexture->getShaderResourceView(), paperRenderTexture->getShaderResourceView(), blendStrength);
	temporalShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());


	renderer->getDeviceContext()->CopyResource(
		previousFrameTexture->getTexture(),
		comparisonTexture->getTexture()
	);

	renderer->setZBuffer(true);
}

void App1::YCBCRToRGBPass()
{
	rgbTexture->setRenderTarget(renderer->getDeviceContext());
	rgbTexture->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 1);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX orthoMatrix = rgbTexture->getOrthoMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	ycbcrToRgbShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, orthoMatrix, cartoonRenderTexture->getShaderResourceView()); 
	ycbcrToRgbShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
	renderer->setBackBufferRenderTarget();
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
	ID3D11ShaderResourceView* selectedResourceView = GetSelectedOutputTexture();


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
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	renderer->setZBuffer(false);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	auto* texture = comparisonTexture->getShaderResourceView();

	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, texture);
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
	GUI();
	renderer->endScene();
}




// TEMPORAL COHERENCE - https://onlinelibrary.wiley.com/doi/epdf/10.1111/j.1467-8659.2012.03075.x
// TAA - https://onlinelibrary.wiley.com/doi/epdf/10.1111/cgf.14018
// Temporal Filtering - https://dl.acm.org/doi/pdf/10.1145/3233301
// This is good - https://www.elopezr.com/temporal-aa-and-the-quest-for-the-holy-trail/


void App1::UpdateCamera(float deltaTime)
{
	if (useArcball)
	{
		// Update Arcball Camera
		arcballCamera.UpdateArcballCamera(deltaTime, camera);
	}
	else
	{
		camera->update();
	}
}


void App1::InitialiseShaders(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
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
	cqShader = new ColourQuantization(renderer->getDevice(), hwnd);
	cartoonShader = new CartoonRendering(renderer->getDevice(), hwnd);
	paperShader = new PaperShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	temporalShader = new TemporalCoherence(renderer->getDevice(), hwnd);
	rgbToYcbcrShader = new RGBToYCBCR(renderer->getDevice(), hwnd);
	ycbcrToRgbShader = new YCBCRToRGB(renderer->getDevice(), hwnd);
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

	// Ocean Controls
	amplitude = 1;
	frequency = 0.1f;
	speed = 1;
	numWaves = 1;
	phases = 1;
	transparency = 1.0f;
	no_movement = 0;   
	wave_movement = 1;   
	sine_movement = 2;   

	
	// Comparison Controls
	comparisonSliderPosition = 1.f;

	// Bilateral Filter controls
	range = 0.075f; 
	spatial = 1.0f; 

	// DoG Controls
	sensitivity = 5.6f;
	smoothing = 0.6f;
	tau = 1;
	texelSize = XMFLOAT2(1.0f / screenWidth, 1.0f / screenHeight);

	// Flow Curve Controls
	currentPosition = XMFLOAT2(0.5f, 0.5f);
	previousTan = XMFLOAT2(0.5f, -0.3f);
	totLength = 0.0f;
	curLength = 0.5f;

	// Colour Quantisation Controls
	transitionSmoothing = 3.4f;
	quantLevel = 8;

	// Paper Controls
	paperStrength = 0.2f;
	depthFactor = 0.5f;

	movementIndicator = 0.0f;

	// Temporal Coherence Controls
	blendStrength = 0.1f;
	frameIndex = 0;

	// Camera Controls
	useArcball = false;

	// Number of BF controls
	bf_edge = 1.f;
	bf_abstraction = 3.f;

}

void App1::InitialiseRenderTextures(int screenWidth, int screenHeight)
{
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	comparisonTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	structureTensorTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	horizontalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	verticalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	bilateralFilterTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	finalBilateralTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	dogFilterTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	flowCurveTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	colourQuantizationTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	cartoonRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	paperRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	depthTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	blendedTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	previousFrameTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	ycbcrTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	rgbTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	
}

ID3D11ShaderResourceView* App1::GetSelectedOutputTexture()
{
	switch (selectedTexture)
	{
	case 0: // Original Render Texture
		return renderTexture->getShaderResourceView();
		break;
	case 1: // Bilateral Filter Texture
		return bilateralFilterTexture->getShaderResourceView();
		break;
	case 2: // Final Bilateral Texture
		return finalBilateralTexture->getShaderResourceView();
		break;
	case 3: // structure Tensor Texture
		return structureTensorTexture->getShaderResourceView();
		break;
	case 4: // smoothed flow map Texture
		return verticalBlurTexture->getShaderResourceView();
		break;
	case 5: //smoothed structure tensor Horizontally
		return horizontalBlurTexture->getShaderResourceView();
		break;
	case 6: //smoothed structure tensor Horizontally
		return dogFilterTexture->getShaderResourceView();
		break;
	case 7: // Flow Curve Texture
		return flowCurveTexture->getShaderResourceView();
		break;
	case 8: // CQ Texture
		return colourQuantizationTexture->getShaderResourceView();
		break;
	case 9: // Blended Texture
		return cartoonRenderTexture->getShaderResourceView();
		break;
	case 10: // Paper Overlay Texture
		return paperRenderTexture->getShaderResourceView();
		break;
	case 11: // Depth texture
		return depthTexture->getShaderResourceView();
		break;
	case 12: // Temporal Coherence Texture
		return blendedTexture->getShaderResourceView();
		break;
	case 13: // Previous Frame Texture
		return previousFrameTexture->getShaderResourceView();
		break;
	case 14: // Final putput in YCBCR Texture
		return ycbcrTexture->getShaderResourceView();
		break;
	case 15: // Final Ouput in RGB Texture
		return rgbTexture->getShaderResourceView();
		break;
	default: // Original Render Texture
		return renderTexture->getShaderResourceView();
		break;
	}
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

	// === Performance and Debug Info ===
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe Mode", &wireframeToggle);
	ImGui::Separator();

	// === Post-Processing ===
	if (ImGui::CollapsingHeader("Post-Processing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SliderFloat("Comparison Slider", &comparisonSliderPosition, 0.0f, 1.0f);

		if (ImGui::TreeNode("Output Texture"))
		{
			const char* textureOptions[] = {
				"01 - Original Scene (Base Color Pass)",
				"02 - Bilateral Filter (Edge Pass)",
				"03 - Bilateral Filter (Final Output)",
				"04 - Structure Tensor (Raw)",
				"05 - Flow Map (Smoothed Vertically)",
				"06 - Structure Tensor (Smoothed Horizontally)",
				"07 - DoG Filter (Edge Detection)",
				"08 - DoG via Flow Curve",
				"09 - Colour Quantization",
				"10 - Cartoon Rendering",
				"11 - Paper Overlay",
				"12 - Depth Map",
				"13 - Temporal Blend Result",
				"14 - Previous Frame",
				"15 - Final YCbCr Output",
				"16 - Final RGB Output"
			};

			ImGui::Combo("Texture", &selectedTexture, textureOptions, IM_ARRAYSIZE(textureOptions));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Bilateral Filter"))
		{
			ImGui::SliderFloat("Spatial Sigma", &spatial, 0.0f, 4.0f);
			ImGui::SliderFloat("Range Sigma", &range, 0.01f, 0.1f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("DoG Filter"))
		{
			ImGui::SliderFloat("Edge Sensitivity", &sensitivity, 1.0f, 20.0f);
			ImGui::SliderFloat("Smoothing", &smoothing, 0.01f, 5.0f);
			ImGui::SliderFloat("Edge Threshold (Tau)", &tau, 0.0f, 2.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Flow Curve"))
		{
			ImGui::SliderFloat2("Start Position", &currentPosition.x, 0.0f, 1.0f);
			ImGui::SliderFloat2("Initial Tangent", &previousTan.x, -1.0f, 1.0f);
			ImGui::SliderFloat("Step Length", &curLength, 0.01f, 10.0f);
			ImGui::SliderFloat("Total Length", &totLength, 0.0f, 100.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Colour Quantization"))
		{
			ImGui::SliderFloat("Smoothing", &transitionSmoothing, 0.0f, 5.0f);
			ImGui::SliderInt("Quantization Level", &quantLevel, 0, 20);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Canvas Overlay"))
		{
			ImGui::SliderFloat("Canvas Strength", &paperStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("Depth Influence", &depthFactor, 0.0f, 1.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Number of Filters"))
		{
			ImGui::SliderFloat("Bilateral - Edge", &bf_edge, 1.0f, 3.0f);
			ImGui::SliderFloat("Bilateral - Abstraction", &bf_abstraction, 1.0f, 5.0f);
			ImGui::TreePop();
		}
	}

	ImGui::Separator();

	// === Temporal Coherence ===
	if (ImGui::CollapsingHeader("Temporal Coherence", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SliderFloat("Blend Strength", &blendStrength, 0.1f, 0.9f);
	}

	ImGui::Separator();

	// === Ocean Settings ===
	if (ImGui::CollapsingHeader("Ocean", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::TreeNode("Speed"))
		{
			ImGui::SliderFloat("Speed", &speed, 0.0f, 20.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Frequency"))
		{
			ImGui::SliderFloat("Frequency", &frequency, 0.0f, 5.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Amplitude"))
		{
			ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 20.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Number of Waves"))
		{
			ImGui::SliderFloat("Wave Count", &numWaves, 0.0f, 20.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Phases"))
		{
			ImGui::SliderFloat("Phases", &phases, 0.0f, 20.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Transparency"))
		{
			ImGui::SliderFloat("Transparency", &transparency, 0.0f, 1.0f);
			ImGui::TreePop();
		}
	}

	ImGui::Separator();

	// === Camera Settings ===
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Use Arcball Camera", &useArcball);

		if (useArcball)
		{
			static float arcballRadius = 100.0f;
			static float arcballSpeed = 0.5f;
			static XMFLOAT3 arcballTarget = XMFLOAT3(40.0f, 10.0f, 40.0f);
			static float verticalAngle = 20.0f;

			ImGui::SliderFloat("Arcball Radius", &arcballRadius, 5.0f, 100.0f);
			ImGui::SliderFloat("Arcball Speed", &arcballSpeed, 0.1f, 2.0f);
			ImGui::SliderFloat("Arcball Target Y", &arcballTarget.y, 0.0f, 100.0f);
			ImGui::SliderFloat("Vertical Angle", &verticalAngle, 0.0f, 45.0f);

			arcballCamera.SetRadius(arcballRadius);
			arcballCamera.SetSpeed(arcballSpeed);
			arcballCamera.SetTarget(arcballTarget);
			arcballCamera.SetAngle(verticalAngle);
		}
	}

	// === Final UI render ===
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}





