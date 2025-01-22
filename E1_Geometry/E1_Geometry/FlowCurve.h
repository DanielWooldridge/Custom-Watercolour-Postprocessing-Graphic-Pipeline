#pragma once
#include "BaseShader.h"
class FlowCurve : public BaseShader
{
public:
	FlowCurve(ID3D11Device* device, HWND hwnd);
	~FlowCurve();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
		ID3D11ShaderResourceView* inputTexture, XMFLOAT2 currentPosition, XMFLOAT2 previousTan, float totLength, float curLength);

	struct FlowCurveFilterType
	{
		XMFLOAT2 cPos;
		XMFLOAT2 pTan;
		float tLength;
		float cLength;
		float pad[2];
	};
private:

	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* fCurveBuffer;
	ID3D11SamplerState* sampleState;
};

