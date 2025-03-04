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
	D3D11_BUFFER_DESC waveBufferDesc;

	// Load shaders
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	//  MATRIX BUFFER
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	//  WAVE BUFFER (AFTER MATRIX BUFFER)
	waveBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waveBufferDesc.ByteWidth = sizeof(WaveParams);
	waveBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waveBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waveBufferDesc.MiscFlags = 0;
	waveBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&waveBufferDesc, NULL, &waveBuffer);
}

void DepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, float time,
	float amplitude, float frequency, float speed, float numWaves, float phases, float transparency, float movementIndicator)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	WaveParams* waveData;

	// Transpose matrices for HLSL compatibility
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// 1. MAP MATRIX BUFFER
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// 2. MAP WAVE BUFFER (DO NOT REUSE mappedResource from above)
	deviceContext->Map(waveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	waveData = (WaveParams*)mappedResource.pData;

	// 3. CORRECTLY ASSIGN VALUES
	waveData->time = time;
	waveData->speed = speed;
	waveData->amplitude = amplitude;
	waveData->frequency = frequency;
	waveData->numWaves = numWaves;
	waveData->phases = phases;
	waveData->transparency = transparency;
	waveData->movementType = movementIndicator; // Now safe to add back

	// 4. UNMAP WAVE BUFFER
	deviceContext->Unmap(waveBuffer, 0);

	// 5. BIND TO SHADERS
	deviceContext->VSSetConstantBuffers(1, 1, &waveBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &waveBuffer);
}



