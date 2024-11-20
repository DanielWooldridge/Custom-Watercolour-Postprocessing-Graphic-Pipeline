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

cbuffer DirectionalLightBuffer : register(b2)
{
    float4 diffuseLight;        // Diffuse light color
	float3 direction;      // Direction of the light
	float pad;
}; 


struct InputType
{
    float4 position : SV_POSITION;  
    float2 tex : TEXCOORD0;        
    float3 normal : NORMAL;         
};



// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 main(InputType input) : SV_TARGET
{

 
    float4 textureColor = mtexture.Sample(msampler, input.tex);
    textureColor.a *= transparency;
    return textureColor;

    //float4 textureColour;
    //float4 lightColour;

    //// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    //textureColour = mtexture.Sample(msampler, input.tex);
    //lightColour = calculateLighting(-direction, input.normal, diffuseLight);


    ////lightColour = calcSpotlight(direction, input.normal, diffuse, input.worldPosition, lightPosition);

    //return lightColour;








}
