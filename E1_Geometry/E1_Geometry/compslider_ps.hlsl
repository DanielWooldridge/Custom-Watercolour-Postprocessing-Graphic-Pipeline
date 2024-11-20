Texture2D aTexture : register(t0);
Texture2D bTexture : register(t1);
SamplerState aSampler : register(s0);
SamplerState bSampler : register(s1);

cbuffer SliderBuffer : register(b1) 
{
    float sliderPosition; // Controls line position (0.0 to 1.0)
    float3 padding;       // Padding to ensure 16-byte alignment
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{
    // Use sliderPosition to control blending between textures
    if (input.tex.x < sliderPosition)
    {
        return aTexture.Sample(aSampler, input.tex);
    }
    else
    {
        return bTexture.Sample(bSampler, input.tex);
    }
}
