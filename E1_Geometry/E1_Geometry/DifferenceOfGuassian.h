#pragma once
#include "BaseShader.h"
class DifferenceOfGuassian : public BaseShader
{
public:
	DifferenceOfGuassian(ID3D11Device* device, HWND hwnd);
	~DifferenceOfGuassian();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
		ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* flowMapTexture, float sensitivity, float smoothing, float tau, XMFLOAT2 texelSize);

	struct DoGFilterType
	{
		float sensitivty;
		float smoothing;
		float tau;
		XMFLOAT2 texelSize;
		float pad[3];
	};
private:

	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* DoGBuffer;
	ID3D11SamplerState* sampleState;

};

