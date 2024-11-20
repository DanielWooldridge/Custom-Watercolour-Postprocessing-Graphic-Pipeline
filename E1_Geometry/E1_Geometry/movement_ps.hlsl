Texture2D shaderTexture;
SamplerState sampleType;

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
    // Sample the texture
    float4 textureColor = shaderTexture.Sample(sampleType, input.tex);
    return textureColor;
}
