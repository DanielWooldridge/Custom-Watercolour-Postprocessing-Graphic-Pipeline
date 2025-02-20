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

    
    //https://stackoverflow.com/questions/5308961/hlsl-combining-textures
    //https://www.reddit.com/r/shaders/comments/162tj6n/help_with_blending_two_textures/
    // fixed4 result = 1 - (1 - color_rt) * (1 - color_main); // Screen blending

    float4 dogFlow = dfImg.Sample(sampleType, input.tex);
    float4 colourQuant = cqImg.Sample(sampleType, input.tex);
    float4 result = 1 - (1 - colourQuant) * (1 - dogFlow); // Screen blending

    return result;
}
