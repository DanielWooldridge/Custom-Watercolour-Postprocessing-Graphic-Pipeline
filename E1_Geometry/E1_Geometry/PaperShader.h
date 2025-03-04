#pragma once
#include "BaseShader.h"
class PaperShader : public BaseShader
{
public:
	PaperShader(ID3D11Device* device, HWND hwnd);
	~PaperShader();

	struct paperTextureBuffer
	{
		float strength;
		float depthFactor;
		XMFLOAT2 padding;
	};
	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
		ID3D11ShaderResourceView* paperTex, ID3D11ShaderResourceView* renderTex, ID3D11ShaderResourceView* depthTex, float paperStrength, float depthFactor );

private:

	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* paperBuffer;
	ID3D11SamplerState* sampleState;
};

