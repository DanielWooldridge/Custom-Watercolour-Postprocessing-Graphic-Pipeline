// Skybox Pixel Shader

Texture2D skyboxTexture : register(t0); // Cubemap texture
SamplerState sampleState : register(s0);  // Sampler state

struct InputType
{
    float4 position : POSITION;
    float3 tex : TEXCOORD0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float3 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET {
    // Sample the cubemap texture
    float4 color = skyboxTexture.Sample(sampleState, input.tex);
    
    return color;
}
