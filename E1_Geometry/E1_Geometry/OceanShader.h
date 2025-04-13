#pragma once
#include "BaseShader.h"

#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;

class OceanShader : public BaseShader
{
public:

	// Creating a Struct to hold values for the pixel and vertex shader
	struct WaveParams
	{
		float time;                     // Time for the wave animation
		float amplitude;             // Amplitudes for the waves
		float speed;                 // Speeds for the waves
		float frequency;
		float numWaves;
		float phases;
		float transparency;
		float padding;
	};




	OceanShader(ID3D11Device* device, HWND hwnd);
	~OceanShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float time,
		float amplitude, float frequency, float speed, float numWaves, float phases, float transparency);
private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

	ID3D11SamplerState* sampleState;
	ID3D11Buffer* waveBuffer;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* directionalLightBuffer;
};

