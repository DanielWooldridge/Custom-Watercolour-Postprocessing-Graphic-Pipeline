#include "DoG_via_FlowCurve.h"


DoG_via_FlowCurve::DoG_via_FlowCurve(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
    initShader(L"dogflow_vs.cso", L"dogflow_ps.cso");
}

DoG_via_FlowCurve::~DoG_via_FlowCurve()
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

void DoG_via_FlowCurve::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc, dogBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;

    // Load shader files
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Matrix buffer
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // DoG Flow Buffer
    dogBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    dogBufferDesc.ByteWidth = sizeof(DoGFlowBufferType);
    dogBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    dogBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dogBufferDesc.MiscFlags = 0;
    dogBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&dogBufferDesc, NULL, &dogBuffer);

    // Sampler state
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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

void DoG_via_FlowCurve::setShaderParameters(ID3D11DeviceContext* deviceContext,
    const XMMATRIX& worldMatrix,
    const XMMATRIX& viewMatrix,
    const XMMATRIX& projectionMatrix,
    ID3D11ShaderResourceView* dogTexture,
    ID3D11ShaderResourceView* flowTexture,
    float sigma_m, float phi)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    // Matrix buffer
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

    // DoG flow parameters
    DoGFlowBufferType* dogPtr;
    result = deviceContext->Map(dogBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dogPtr = (DoGFlowBufferType*)mappedResource.pData;
    dogPtr->sigma_m = sigma_m;
    dogPtr->phi = phi;
    deviceContext->Unmap(dogBuffer, 0);
    deviceContext->PSSetConstantBuffers(1, 1, &dogBuffer);

    // Set textures
    deviceContext->PSSetShaderResources(0, 1, &dogTexture);
    deviceContext->PSSetShaderResources(1, 1, &flowTexture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}