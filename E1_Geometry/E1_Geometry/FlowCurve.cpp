#include "FlowCurve.h"

FlowCurve::FlowCurve(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"flowcurve_vs.cso", L"flowcurve_ps.cso");
}

FlowCurve::~FlowCurve()
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

void FlowCurve::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC fCurveBufferDesc;

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

	//DoG BUFFER
	fCurveBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	fCurveBufferDesc.ByteWidth = sizeof(FlowCurveFilterType);
	fCurveBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	fCurveBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	fCurveBufferDesc.MiscFlags = 0;
	fCurveBufferDesc.StructureByteStride = 0;

	renderer->CreateBuffer(&fCurveBufferDesc, NULL, &fCurveBuffer);
}


void FlowCurve::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, 
	ID3D11ShaderResourceView* inputTexture, XMFLOAT2 currentPosition, XMFLOAT2 previousTan, float totLength, float curLength)
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

	// Send DoG parameters to pixel shader
	FlowCurveFilterType* flowptr;
	deviceContext->Map(fCurveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	flowptr = (FlowCurveFilterType*)mappedResource.pData;
	flowptr->cPos = currentPosition;
	flowptr->pTan = previousTan;
	flowptr->tLength = totLength;
	flowptr->cLength = curLength;
	deviceContext->Unmap(fCurveBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &fCurveBuffer);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

