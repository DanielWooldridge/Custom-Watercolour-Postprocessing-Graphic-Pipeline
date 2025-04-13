// Texture and Sampler
Texture2D mtexture : register(t0);      
SamplerState msampler : register(s0);    

cbuffer WaveParamsBuffer : register(b1)
{
    float time;                     // Time for the wave animation
    float amplitude;             // Amplitudes for the waves
	float speed;                 // Speeds for the waves
	float frequency; 
    float numWaves;
    float phases;
    float transparency;
    float padding;
};




struct InputType
{
    float4 position : SV_POSITION;  
    float2 tex : TEXCOORD0;        
    float3 normal : NORMAL;         
};



float4 main(InputType input) : SV_TARGET
{

 
    float4 textureColor = mtexture.Sample(msampler, input.tex);
    textureColor.a *= transparency;
    return textureColor;

}
