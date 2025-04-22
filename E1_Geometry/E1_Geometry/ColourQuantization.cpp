#include "ColourQuantization.h"

ColourQuantization::ColourQuantization(ID3D11Device* device, HWND hwnd) : BaseShader (device, hwnd)
{
	initShader(L"standard_vs.cso", L"colour_quantization_ps.cso");
}

ColourQuantization::~ColourQuantization()
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

void ColourQuantization::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC cqBufferDesc;

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

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

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

	//CQ BUffer
	cqBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cqBufferDesc.ByteWidth = sizeof(CQFilterType);
	cqBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cqBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cqBufferDesc.MiscFlags = 0;
	cqBufferDesc.StructureByteStride = 0;

	renderer->CreateBuffer(&cqBufferDesc, NULL, &cqBuffer);

}

void ColourQuantization::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, 
	const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* inputTexture, float transitionSmoothing, int quantLevel)
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

	// Create cbuffer
	CQFilterType* cqptr;
	deviceContext->Map(cqBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cqptr = (CQFilterType*)mappedResource.pData;
	cqptr->transitionSmoothing = transitionSmoothing;
	cqptr->quantLevel = quantLevel;
	deviceContext->Unmap(cqBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &cqBuffer);
	deviceContext->PSSetShaderResources(0, 1, &inputTexture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}
