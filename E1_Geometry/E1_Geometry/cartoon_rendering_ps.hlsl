Texture2D dfImg : register(t0);
Texture2D cqImg : register(t1);
SamplerState sampleType : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float4 main(InputType input) : SV_TARGET
{

    // Sample textures
    float edge = dfImg.Sample(sampleType, input.tex).r;      
    float3 ycbcr = cqImg.Sample(sampleType, input.tex).rgb;   

    // lerp to prevent harsh black edges
    edge = lerp(0.5, 1.0, edge);  

    // enhance edge contrast
    float y = saturate(edge * ycbcr.r);
    y = max(y, 0.1);  

    return float4(y, ycbcr.g, ycbcr.b, 1.0);


}
