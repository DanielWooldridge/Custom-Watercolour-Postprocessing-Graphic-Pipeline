//Texture2D prevTexture : register(t0);
Texture2D previousTexture : register(t0);
Texture2D currentTexture : register(t1);
SamplerState sampleState : register(s0);

cbuffer TemporalBuffer : register(b0)
{
	float blendStrength;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
	float4 currentTex = currentTexture.Sample(sampleState, input.tex);
	float4 previousTex = previousTexture.Sample(sampleState, input.tex);
	//return previousTex;
	float prevWeight = 1 - blendStrength;

	return currentTex * blendStrength + previousTex * prevWeight;
	//return lerp(currentTex, previousTex, 0.9);
}