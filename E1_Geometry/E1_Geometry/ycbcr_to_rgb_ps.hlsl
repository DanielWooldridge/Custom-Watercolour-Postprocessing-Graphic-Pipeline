Texture2D tex : register(t0);
SamplerState samplerState : register(s0);


struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float3 YCbCrtoRGB(float3 ycc)
{
    float3 c = ycc - float3(0.0, 128.0 / 255.0, 128.0 / 255.0);
   float R = c.x + 1.402 * c.z;
    float G = c.x - 0.344136 * c.y - 0.714136 * c.z;
    float B = c.x + 1.772 * c.y;

    return float3(R, G, B);
}

float4 main(InputType input) : SV_TARGET
{
    float3 ycbcr = saturate(tex.Sample(samplerState, input.tex).rgb);
    float3 rgb = YCbCrtoRGB(ycbcr);
    return float4(saturate(rgb), 1.0);
}
