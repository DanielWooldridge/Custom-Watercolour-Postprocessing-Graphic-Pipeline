#pragma once
#include "BaseShader.h"
class CompSlider : public BaseShader
{
public:
	CompSlider(ID3D11Device* device, HWND hwnd);
	~CompSlider();

	void setShaderParameters(ID3D11DeviceContext* deviceContex, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, 
		ID3D11ShaderResourceView* a_texture, ID3D11ShaderResourceView* b_texture, float sliderPosition);

	struct SliderBufferType
	{
		float sliderPosition;
		XMFLOAT3 padding; // has to equal 16-byte
	};

private:
	void initShader(const wchar_t* vs_filenamem, const wchar_t* psFilename);

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* sliderBuffer;
	ID3D11SamplerState* a_sampleState;
	ID3D11SamplerState* b_sampleState;


};

