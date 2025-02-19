Texture2D img : register(t0);
SamplerState sampleType : register(s0);

cbuffer cqBuffer
{
	float transitionSmoothing;
	float3 padding;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};




float4 main(InputType input) : SV_TARGET
{
	
	float3 colour = img.Sample(sampleType, input.tex).rgb;



	float qn = colour.r * 10 + 0.5f / 10;
	float qs = smoothstep(-2, 2, transitionSmoothing*(colour.r, qn) * 100) - 0.5f;
	float qc = qn + qs / 10;
	
	return float4(qc, qc, qc, 1.0f);
}

//https://dl.acm.org/doi/pdf/10.1145/1141911.1142018 -> Winnemoller06 "Real Time Video Abstraction"