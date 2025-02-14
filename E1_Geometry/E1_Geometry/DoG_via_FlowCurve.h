#pragma once
#include "BaseShader.h"

class DoG_via_FlowCurve : public BaseShader
{
public:
    DoG_via_FlowCurve(ID3D11Device* device, HWND hwnd);
    ~DoG_via_FlowCurve();

    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* dogTexture,
        ID3D11ShaderResourceView* flowTexture, float sigma_m, float phi);

    struct DoGFlowBufferType
    {
        float sigma_m;
        float phi;
        float pad[2]; 
    };

private:
    void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

    ID3D11Buffer* matrixBuffer;
    ID3D11Buffer* dogBuffer;
    ID3D11SamplerState* sampleState;
};
