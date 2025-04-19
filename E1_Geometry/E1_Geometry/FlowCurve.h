#pragma once
#include "BaseShader.h"
class FlowCurve : public BaseShader
{
public:
	FlowCurve(ID3D11Device* device, HWND hwnd);
	~FlowCurve();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
		ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* dogTexture, float phi, float sigma_m, bool inveretd, bool polsterize);

	struct FlowCurveFilterType
	{
		float phi;
		float sigma_m;
		int invertedLines;
		int polsterize;
	};
private:

	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* fCurveBuffer;
	ID3D11SamplerState* sampleState;
};

