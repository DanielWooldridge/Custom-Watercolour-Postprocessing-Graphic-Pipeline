#pragma once
#include "BaseShader.h"
class BilateralFilter : public BaseShader
{

public:
    BilateralFilter(ID3D11Device* device, HWND hwnd);
    ~BilateralFilter();

    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
        ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* flowTexture, int pass, float spatial, float range);

    struct FilterSettingsBufferType
    {
        float spatial;
        float range;
        int pass;
        float padding;
    };

private:
    void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

    ID3D11Buffer* matrixBuffer;
    ID3D11Buffer* filterBuffer;
    ID3D11SamplerState* sampleState;
};

