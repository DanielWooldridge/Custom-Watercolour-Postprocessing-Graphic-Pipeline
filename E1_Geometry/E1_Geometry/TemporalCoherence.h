#pragma once
#include "BaseShader.h"
class TemporalCoherence : public BaseShader
{
public:
	TemporalCoherence(ID3D11Device* device, HWND hwnd);
	~TemporalCoherence();

	struct TemporalFilterType
	{
		float blendStrength;
		XMFLOAT3 pad;
	};

	void setShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* previousFrame,
		ID3D11ShaderResourceView* currentFrame, float blendStrength);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* temporalBuffer;
};

