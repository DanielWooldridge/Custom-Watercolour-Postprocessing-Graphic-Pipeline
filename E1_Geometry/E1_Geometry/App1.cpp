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
	
	// Step 1: Structure Tensor and Flow Map generation
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
	CombinePass();

	// Final conversions and presentation
	YCBCRToRGBPass();
	PaperRenderingPass();
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
	if (changeScene)
	{

		// Render Sphere 
		XMMATRIX sphereTranslationMatrix = XMMatrixTranslation(30.0f, 14.0f, 60.0f);
		XMMATRIX sphereScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		XMMATRIX sphereTransformedWorldMatrix = sphereScalingMatrix * sphereTranslationMatrix * worldMatrix;
		sphere->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), sphereTransformedWorldMatrix, viewMatrix, projectionMatrix, totalTime, amplitude, frequency, speed, numWaves, phases, transparency, sine_movement);
		depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

		// Render Cube
		XMMATRIX cubeTranslationMatrix = XMMatrixTranslation(50.0f, 14.0f, 20.0f);
		XMMATRIX cubeScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		XMMATRIX cubeTransformedWorldMatrix = cubeScalingMatrix * cubeTranslationMatrix * worldMatrix;
		cube->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), cubeTransformedWorldMatrix, viewMatrix, projectionMatrix, totalTime, amplitude, frequency, speed, numWaves, phases, transparency, sine_movement);
		depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());


		// Render Ship Model
		movementIndicator = no_movement;
		XMMATRIX shipTranslationMatrix = XMMatrixTranslation(40.0f, 7.0f, 40.0f);
		XMMATRIX shipScalingMatrix = XMMatrixScaling(4.0f, 4.0f, 4.0f);
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

	}
	else
	{
		floor->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, totalTime, amplitude, frequency, speed, numWaves, phases, transparency, no_movement);
		depthShader->render(renderer->getDeviceContext(), floor->getIndexCount());
	}

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




	if (changeScene)
	{
		// Render Sphere 
		XMMATRIX sphereTranslationMatrix = XMMatrixTranslation(30.0f, 14.0f, 60.0f); 
		XMMATRIX sphereScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		XMMATRIX sphereTransformedWorldMatrix = sphereScalingMatrix * sphereTranslationMatrix * worldMatrix;
		sphere->sendData(renderer->getDeviceContext());
		movementShader->setShaderParameters(renderer->getDeviceContext(), sphereTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"), totalTime, 0.0);
		movementShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

		// Render Cube 
		XMMATRIX cubeTranslationMatrix = XMMatrixTranslation(50.0f, 14.0f, 20.0f); 
		XMMATRIX cubeScalingMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		XMMATRIX cubeTransformedWorldMatrix = cubeScalingMatrix * cubeTranslationMatrix * worldMatrix;
		cube->sendData(renderer->getDeviceContext());
		movementShader->setShaderParameters(renderer->getDeviceContext(), cubeTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"), totalTime, 1.0);
		movementShader->render(renderer->getDeviceContext(), cube->getIndexCount());


		 //Ship model
		if (changeShip)
		{
			XMMATRIX shipTranslationMatrix = XMMatrixTranslation(40.0f, -1.0f, 40.0f);
			XMMATRIX shipScalingMatrix = XMMatrixScaling(4.0f, 4.0f, 4.0f);
			XMMATRIX shipRotationMatrix = XMMatrixRotationX(0);
			XMMATRIX shipTransformedWorldMatrix = shipRotationMatrix * shipScalingMatrix * shipTranslationMatrix * worldMatrix;
			ship->sendData(renderer->getDeviceContext());
			oceanShader->setShaderParameters(renderer->getDeviceContext(), shipTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"shipWood"), totalTime, -amplitude / 2, frequency, speed, numWaves, phases, transparency);
			oceanShader->render(renderer->getDeviceContext(), ship->getIndexCount());
		}
		else
		{
			XMMATRIX shipTranslationMatrix = XMMatrixTranslation(40.0f, 4.0f, 40.0f);
			XMMATRIX shipScalingMatrix = XMMatrixScaling(12.0f, 12.0f, 12.0f);
			XMMATRIX shipRotationMatrix = XMMatrixRotationX(0);
			XMMATRIX shipTransformedWorldMatrix = shipRotationMatrix * shipScalingMatrix * shipTranslationMatrix * worldMatrix;
			ship2->sendData(renderer->getDeviceContext());
			oceanShader->setShaderParameters(renderer->getDeviceContext(), shipTransformedWorldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"shipWood2"), totalTime, -amplitude / 10, frequency, speed, numWaves, phases, transparency);
			oceanShader->render(renderer->getDeviceContext(), ship2->getIndexCount());
		}


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
	else
	{
		/* Render Floor*/
		floor->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"));
		textureShader->render(renderer->getDeviceContext(), floor->getIndexCount());
	}


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


void App1::FlowCurvePass()
{

	flowCurveTexture->setRenderTarget(renderer->getDeviceContext());
	flowCurveTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = flowCurveTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	flowCurveShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, verticalBlurTexture->getShaderResourceView(), dogFilterTexture->getShaderResourceView(), flowPhi, flowSigma_m, inversion, polsterize);
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

void App1::CombinePass()
{
	combineRenderTexture->setRenderTarget(renderer->getDeviceContext());
	combineRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = combineRenderTexture->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	combineShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, flowCurveTexture->getShaderResourceView(), colourQuantizationTexture->getShaderResourceView());
	combineShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

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
	ycbcrToRgbShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, orthoMatrix, combineRenderTexture->getShaderResourceView()); 
	ycbcrToRgbShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
	renderer->setBackBufferRenderTarget();
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
	paperShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, paperTexture, rgbTexture->getShaderResourceView(), depthTexture->getShaderResourceView(), paperStrength, depthFactor);
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
	ID3D11ShaderResourceView* selectedResourceView = GetSelectedOutputTexture();

	// Set shader parameters for the comparison shader
	comparisonShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), selectedResourceView, comparisonSliderPosition, visualizeInRGB);
	

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
	combineShader = new CartoonRendering(renderer->getDevice(), hwnd);
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

	ship = new AModel(renderer->getDevice(), "res/models/pShip.obj"); // https://sketchfab.com/3d-models/pirate-ship-lowpoly-15aaf52d00dd4c78a984bf97ed9d7967
	ship2 = new AModel(renderer->getDevice(), "res/models/sShip.obj"); // https://sketchfab.com/3d-models/pirate-ship-lowpoly-15aaf52d00dd4c78a984bf97ed9d7967

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
	sensitivity = 10.0f;
	smoothing = 0.72f;
	tau = 1;
	texelSize = XMFLOAT2(1.0f / screenWidth, 1.0f / screenHeight);

	// Flow Curve Controls
	flowPhi = 2.0f;
	flowSigma_m = 2.0f;
	inversion = true;
	polsterize = true;

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

	visualizeInRGB = false;
	changeScene = true;
	changeShip = true;
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
	combineRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	paperRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	depthTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	blendedTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	previousFrameTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	ycbcrTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	rgbTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	conversionTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	
}

ID3D11ShaderResourceView* App1::GetSelectedOutputTexture()
{
	switch (selectedTexture)
	{
	case 0:
		return renderTexture->getShaderResourceView(); // Original Scene
	case 1:
		return ycbcrTexture->getShaderResourceView(); // Colour conevrsion
	case 2:
		return structureTensorTexture->getShaderResourceView(); // Structure Tensor
	case 3:
		return horizontalBlurTexture->getShaderResourceView(); // Horizontal Smoothing
	case 4:
		return verticalBlurTexture->getShaderResourceView(); // Vertical Smoothing (Flow Map)
	case 5:
		return bilateralFilterTexture->getShaderResourceView(); // BF1
	case 6:
		return finalBilateralTexture->getShaderResourceView(); // BF2
	case 7:
		return dogFilterTexture->getShaderResourceView(); // DoG
	case 8:
		return flowCurveTexture->getShaderResourceView(); // Flow Curve DoG
	case 9:
		return colourQuantizationTexture->getShaderResourceView(); // CQ
	case 10:
		return combineRenderTexture->getShaderResourceView(); // Cartoon Pass
	case 11:
		return rgbTexture->getShaderResourceView(); // Colour conversion
	case 12:
		return paperRenderTexture->getShaderResourceView(); // Paper Overlay
	case 13:
		return blendedTexture->getShaderResourceView(); // Temporal Blend
	case 14:
		return previousFrameTexture->getShaderResourceView(); // Previous Frame
	case 15:
		return depthTexture->getShaderResourceView(); // Depth
	default:
		return renderTexture->getShaderResourceView(); // default
	}



}



void App1::LoadIntextures()
{
	textureMgr->loadTexture(L"grass", L"res/funny.jpg"); // Grass Texture
	textureMgr->loadTexture(L"wood", L"res/sand.jpg"); // Wood Texture
	textureMgr->loadTexture(L"water", L"res/water.jpg"); // water Texture
	textureMgr->loadTexture(L"shipWood", L"res/Shiptexnew.png"); // ship Texture
	textureMgr->loadTexture(L"shipWood2", L"res/sShipTex.png"); // ship2 Texture
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
				"01 - Original Scene (Base Color Pass)", "02 - RGB to YCbCr", "03 - Structure Tensor (Raw)",
				"04 - Structure Tensor (Smoothed Horizontally)", "05 - Flow Map (Smoothed Vertically)",
				"06 - Bilateral Filter (first)", "07 - Bilateral Filter (second)",
				"08 - DoG Filter (Edge Detection)", "09 - DoG via Flow Curve",
				"10 - Colour Quantization", "11 - Combined Texture",
				"12 - YCbCr to RGB Output", "13 - Paper Overlay",
				"14 - Temporal Blended Frame", "15 - Previous Frame", "16 - Depth Map"
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
			ImGui::SliderFloat("Edge Sensitivity", &sensitivity, 1.0f, 2.0f);
			ImGui::SliderFloat("Smoothing", &smoothing, 0.72f, 5.0f);
			ImGui::SliderFloat("Edge Threshold (Tau)", &tau, 0.7f, 1.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Flow Curve"))
		{
			ImGui::SliderFloat("phi", &flowPhi, 0.01f, 10.0f);
			ImGui::SliderFloat("simga_m", &flowSigma_m, 0.0f, 5.0f);
			ImGui::Checkbox("Invert Lines", &inversion);
			ImGui::Checkbox("Polsterize", &polsterize);
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
		if (ImGui::CollapsingHeader("Colour Space")) {
			ImGui::Checkbox("Visualize Intermediate in RGB", &visualizeInRGB);
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
	if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Change Scene", &changeScene);
		ImGui::Checkbox("Change Ship", &changeShip);
	}

	// === Final UI render ===
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}





