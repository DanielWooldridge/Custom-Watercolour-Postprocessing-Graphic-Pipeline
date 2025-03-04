#include "DepthShader.h"

DepthShader::DepthShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"depth_vs.cso", L"depth_ps.cso");
}

DepthShader::~DepthShader()
{
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

	BaseShader::~BaseShader();
}

void DepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	//D3D11_BUFFER_DESC waveBufferDesc;


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

	////WAVE BUFFER
	//waveBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//waveBufferDesc.ByteWidth = sizeof(WaveParams);
	//waveBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//waveBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//waveBufferDesc.MiscFlags = 0;
	//waveBufferDesc.StructureByteStride = 0;
	//renderer->CreateBuffer(&waveBufferDesc, NULL, &waveBuffer);
}

void DepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix/*, float time,
	float amplitude, float frequency, float speed, float numWaves, float phases, float transparency, float movementIndicator*/)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//// Access the buffer and fill in the wave parameters and time
	//WaveParams* waveData = (WaveParams*)mappedResource.pData;

	//// Set the time value
	//waveData->time = time;
	//waveData->speed = speed;
	//waveData->amplitude = amplitude;
	//waveData->frequency = frequency;
	//waveData->numWaves = numWaves;
	//waveData->phases = phases;
	//waveData->transparency = transparency;
	//waveData->movementType = movementIndicator;


	//// Unmap the buffer after updating it
	//deviceContext->Unmap(waveBuffer, 0);

	//// Set the combined constant buffer to the shaders (vertex and pixel shaders)
	//deviceContext->VSSetConstantBuffers(1, 1, &waveBuffer);
	//deviceContext->PSSetConstantBuffers(1, 1, &waveBuffer);
}


