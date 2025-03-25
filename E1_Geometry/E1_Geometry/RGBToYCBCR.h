#pragma once
#include "BaseShader.h"
class RGBToYCBCR : public BaseShader
{
public:
    RGBToYCBCR(ID3D11Device* device, HWND hwnd);
    ~RGBToYCBCR();

    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
        ID3D11ShaderResourceView* texture);

private:
    void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

    ID3D11Buffer* matrixBuffer;
    ID3D11SamplerState* sampleState;
};

