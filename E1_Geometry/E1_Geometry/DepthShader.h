#pragma once
#include "BaseShader.h"
class DepthShader : public BaseShader
{
public:
    DepthShader(ID3D11Device* device, HWND hwnd);
    ~DepthShader();

    struct WaveParams
    {
        float time;                     // 4 bytes
        float amplitude;                 // 4 bytes
        float speed;                     // 4 bytes
        float frequency;                 // 4 bytes 

        float numWaves;                   // 4 bytes
        float phases;                     // 4 bytes
        float transparency;               // 4 bytes
        float movementType;               // 4 bytes 

        float padding[4];                 // Extra padding for alignment
    };


	

    void setShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, float time,
		float amplitude, float frequency, float speed, float numWaves, float phases, float transparency, float movementIndicator);
   

private:
    void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

    ID3D11Buffer* matrixBuffer;
    ID3D11SamplerState* sampleState;
	ID3D11Buffer* waveBuffer;
};

