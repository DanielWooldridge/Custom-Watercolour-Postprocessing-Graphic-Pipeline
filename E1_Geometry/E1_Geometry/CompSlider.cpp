#include "CompSlider.h"

CompSlider::CompSlider(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"compslider_vs.cso", L"compslider_ps.cso");
}

CompSlider::~CompSlider()
{
	// Release the sampler states.
	if (a_sampleState)
	{
		a_sampleState->Release();
		a_sampleState = 0;
	}
	if (b_sampleState)
	{
		b_sampleState->Release();
		b_sampleState = 0;
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

void CompSlider::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
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

	// Create two sampler states, one for each texture.
	renderer->CreateSamplerState(&samplerDesc, &a_sampleState);
	renderer->CreateSamplerState(&samplerDesc, &b_sampleState);

	D3D11_BUFFER_DESC sliderBufferDesc;
	sliderBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	sliderBufferDesc.ByteWidth = sizeof(SliderBufferType);
	sliderBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sliderBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sliderBufferDesc.MiscFlags = 0;
	sliderBufferDesc.StructureByteStride = 0;

	renderer->CreateBuffer(&sliderBufferDesc, NULL, &sliderBuffer);

}

void CompSlider::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, 
	ID3D11ShaderResourceView* a_texture, ID3D11ShaderResourceView* b_texture, float sliderPosition, int visualize)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Send matrix data
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Map and update the slider buffer
	SliderBufferType* sliderPtr;
	deviceContext->Map(sliderBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	sliderPtr = (SliderBufferType*)mappedResource.pData;
	sliderPtr->sliderPosition = sliderPosition; // Position from 0.0 to 1.0
	sliderPtr->visualizeInRGB = visualize ? 1 : 0;
	deviceContext->Unmap(sliderBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &sliderBuffer);

	// Set shader textures and sampler resources in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &a_texture);
	deviceContext->PSSetShaderResources(1, 1, &b_texture);
	deviceContext->PSSetSamplers(0, 1, &a_sampleState);
	deviceContext->PSSetSamplers(1, 1, &b_sampleState);
}
