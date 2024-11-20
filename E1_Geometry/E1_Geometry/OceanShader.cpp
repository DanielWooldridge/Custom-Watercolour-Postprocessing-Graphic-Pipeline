#include "OceanShader.h"

OceanShader::OceanShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"ocean_vs.cso", L"ocean_ps.cso");
}

OceanShader::~OceanShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void OceanShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{

	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC waveBufferDesc;
	D3D11_BUFFER_DESC directionalBufferDesc;


	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	//WAVE BUFFER
	waveBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waveBufferDesc.ByteWidth = sizeof(WaveParams);
	waveBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waveBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waveBufferDesc.MiscFlags = 0;
	waveBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&waveBufferDesc, NULL, &waveBuffer);


	// Setup directional light buffer
	directionalBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	directionalBufferDesc.ByteWidth = sizeof(LightType);
	directionalBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	directionalBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	directionalBufferDesc.MiscFlags = 0;
	directionalBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&directionalBufferDesc, NULL, &directionalLightBuffer);

}


void OceanShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, 
	float totalTime, float amplitude, float frequency, float speed, float numWaves, float phases, float transparency, Light* dirLight)
{
	// Map the buffer to set wave parameters and time
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(waveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Access the buffer and fill in the wave parameters and time
	WaveParams* waveData = (WaveParams*)mappedResource.pData;

	// Set the time value
	waveData->time = totalTime;
	waveData->speed = speed;
	waveData->amplitude = amplitude;
	waveData->frequency = frequency;
	waveData->numWaves = numWaves;
	waveData->phases = phases;
	waveData->transparency = transparency;


	// Unmap the buffer after updating it
	deviceContext->Unmap(waveBuffer, 0);

	// Set the combined constant buffer to the shaders (vertex and pixel shaders)
	deviceContext->VSSetConstantBuffers(1, 1, &waveBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &waveBuffer);




	// Directional Light Variable declarations to send to pixel shader
	LightType* dirlightPtr;
	deviceContext->Map(directionalLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dirlightPtr = (LightType*)mappedResource.pData;
	dirlightPtr->diffuse = dirLight->getDiffuseColour();
	dirlightPtr->direction = dirLight->getDirection();
	deviceContext->Unmap(directionalLightBuffer, 0);
	deviceContext->PSSetConstantBuffers(2, 1, &directionalLightBuffer);




	// Now set the world, view, and projection matrices in the shader
	// Create a matrix buffer for the transformation matrices (world, view, projection)
	D3D11_MAPPED_SUBRESOURCE matrixMappedResource;
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &matrixMappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Access the matrix buffer and set the world, view, and projection matrices
	MatrixBufferType* matrixData = (MatrixBufferType*)matrixMappedResource.pData;

	// Transpose matrices (needed for HLSL)
	matrixData->world = XMMatrixTranspose(world);
	matrixData->view = XMMatrixTranspose(view);
	matrixData->projection = XMMatrixTranspose(projection);

	// Unmap the matrix buffer
	deviceContext->Unmap(matrixBuffer, 0);

	// Set the matrix buffer in the vertex shader
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Set the texture resource to the pixel shader
	deviceContext->PSSetShaderResources(0, 1, &texture);
}



