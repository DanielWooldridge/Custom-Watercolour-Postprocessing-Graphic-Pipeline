#include "PaperShader.h"

PaperShader::PaperShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"paper_vs.cso", L"paper_ps.cso");
}

PaperShader::~PaperShader()
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

void PaperShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC paperBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

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

	// Setup Paper Buffer
	paperBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paperBufferDesc.ByteWidth = sizeof(paperTextureBuffer);
	paperBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	paperBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	paperBufferDesc.MiscFlags = 0;
	paperBufferDesc.StructureByteStride = 0;

	renderer->CreateBuffer(&paperBufferDesc, NULL, &paperBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	renderer->CreateSamplerState(&samplerDesc, &sampleState);
}

void PaperShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, 
	ID3D11ShaderResourceView* paperTex, ID3D11ShaderResourceView* renderTex, ID3D11ShaderResourceView* depthTex, float paperStrength, float depthFactor)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Send matrix data (if needed by the vertex shader)
	MatrixBufferType* dataPtr;
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	paperTextureBuffer* paperptr;
	result = deviceContext->Map(paperBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	paperptr = (paperTextureBuffer*)mappedResource.pData;
	paperptr->strength = paperStrength;
	paperptr->depthFactor = depthFactor;
	deviceContext->Unmap(paperBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &paperBuffer);

	deviceContext->PSSetShaderResources(0, 1, &paperTex); // Texture slot t0
	deviceContext->PSSetShaderResources(1, 1, &renderTex); // Texture slot t1
	deviceContext->PSSetShaderResources(2, 1, &depthTex); // Texture slot t1
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}


