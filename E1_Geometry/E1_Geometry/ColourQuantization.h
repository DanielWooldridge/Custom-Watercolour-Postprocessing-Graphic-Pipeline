#pragma once
#include "BaseShader.h"
class ColourQuantization : public BaseShader
{
public:
	ColourQuantization(ID3D11Device* device, HWND hwnd);
	~ColourQuantization();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
		ID3D11ShaderResourceView* inputTexture, float transitionSmoothing, int quantLevel);

	struct CQFilterType
	{
		float transitionSmoothing;
		int quantLevel;
		XMFLOAT2 padding;
	};
private:

	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* cqBuffer;
	ID3D11SamplerState* sampleState;
};

