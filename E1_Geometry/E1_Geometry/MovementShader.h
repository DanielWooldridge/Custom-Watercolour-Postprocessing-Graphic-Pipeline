#pragma once
#include "BaseShader.h"
class MovementShader : public BaseShader
{
public:
	MovementShader(ID3D11Device* device, HWND hwnd);
	~MovementShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, 
		ID3D11ShaderResourceView* texture, float totalTime, float movementIndicator);
	
	struct TimeBufferType
	{
		float time;
		int movementType;
		XMFLOAT2 padding;
	};
private:

	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* timeBuffer;
	ID3D11SamplerState* sampleState;


	
};

