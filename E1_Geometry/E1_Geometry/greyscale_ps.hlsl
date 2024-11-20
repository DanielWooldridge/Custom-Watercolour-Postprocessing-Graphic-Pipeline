Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float4 main(InputType input) : SV_TARGET
{
    //// Sample the texture
    float4 color = shaderTexture.Sample(sampleType, input.tex);
    
    // Convert the sampled color to greyscale using weighted RGB values
    float greyValue = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
    
    // Return the greyscale color while retaining the original alpha
    return float4(greyValue, greyValue, greyValue, color.a);




}
