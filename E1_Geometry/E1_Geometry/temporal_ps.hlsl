//Texture2D prevTexture : register(t0);
Texture2D previousTexture : register(t0);
Texture2D currentTexture : register(t1);
SamplerState sampleState : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
	float4 currentTex = currentTexture.Sample(sampleState, input.tex);
	float4 previousTex = currentTexture.Sample(sampleState, input.tex);
	
	return currentTex * 0.1 + previousTex * 0.9;
}