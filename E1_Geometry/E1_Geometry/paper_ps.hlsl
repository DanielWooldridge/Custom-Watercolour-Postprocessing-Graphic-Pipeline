Texture2D paperTex : register(t0);
Texture2D renderTex : register(t1);
Texture2D depthTex : register(t2);
SamplerState sampleType : register(s0);

cbuffer PaperFilter : register(b1)
{
    float blendStrength;  
    float depthFactor;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    // Sample Textures
    float4 paperTexture = paperTex.Sample(sampleType, input.tex);
    float4 renderTexture = renderTex.Sample(sampleType, input.tex);
    float depthValue = depthTex.Sample(sampleType, input.tex).r;

    // Convert depth to enhance contrast and control blending
    depthValue = pow(depthValue, 2.2);  
    depthValue = smoothstep(0.2, 0.8, depthValue);  

    // dynamic blend due to distance
    float dynamicBlend = blendStrength + (depthValue * depthFactor);
    dynamicBlend = saturate(dynamicBlend); 

    // Blend
    float3 blendOverlay = (renderTexture.rgb < 0.5) 
        ? (2.0 * renderTexture.rgb * paperTexture.rgb)  
        : (1.0 - 2.0 * (1.0 - renderTexture.rgb) * (1.0 - paperTexture.rgb));

    
    float3 finalColor = lerp(renderTexture.rgb, blendOverlay, dynamicBlend);

    return float4(finalColor, renderTexture.a);
}

// Randomize UV via depth to make it look 3D?
// get tex
// 
