#include "BilateralFilter.h"

BilateralFilter::BilateralFilter(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
    initShader(L"standard_vs.cso", L"bilateralfilter_ps.cso");
}

BilateralFilter::~BilateralFilter()
{
}

void BilateralFilter::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_BUFFER_DESC filterBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;

    // Load and compile the vertex and pixel shader files
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Setup the matrix constant buffer description (for world, view, projection matrices)
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create the matrix constant buffer
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Setup the filter settings buffer description (for sigma values and pass flag)
    filterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    filterBufferDesc.ByteWidth = sizeof(FilterSettingsBufferType);
    filterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    filterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    filterBufferDesc.MiscFlags = 0;
    filterBufferDesc.StructureByteStride = 0;

    // Create the filter settings constant buffer
    renderer->CreateBuffer(&filterBufferDesc, NULL, &filterBuffer);

    // Setup the texture sampler state description
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state
    renderer->CreateSamplerState(&samplerDesc, &sampleState);
}

void BilateralFilter::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, 
    ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* flowTexture, int pass, float spatial, float range)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    // Map the matrix buffer
    MatrixBufferType* matrixPtr;
    result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    matrixPtr = (MatrixBufferType*)mappedResource.pData;
    matrixPtr->world = XMMatrixTranspose(worldMatrix);
    matrixPtr->view = XMMatrixTranspose(viewMatrix);
    matrixPtr->projection = XMMatrixTranspose(projectionMatrix);
    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

    // Create cbuffer
    FilterSettingsBufferType* filterPtr;
    result = deviceContext->Map(filterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    filterPtr = (FilterSettingsBufferType*)mappedResource.pData;
    filterPtr->spatial = spatial;
    filterPtr->range = range;
    filterPtr->pass = pass;

    deviceContext->Unmap(filterBuffer, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &filterBuffer);

    // Bind textures and samplers
    deviceContext->PSSetShaderResources(0, 1, &inputTexture);
    deviceContext->PSSetShaderResources(1, 1, &flowTexture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}


