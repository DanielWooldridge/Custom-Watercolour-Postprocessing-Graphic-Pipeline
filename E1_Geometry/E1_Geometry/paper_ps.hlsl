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

    // Clamp color values to [0, 1] to avoid weird artifacts
    float3 baseColor = saturate(renderTexture.rgb);
    float3 paperColor = saturate(paperTexture.rgb);

    // Adjust depth for blending
    depthValue = pow(depthValue, 2.2);  
    depthValue = smoothstep(0.2, 0.8, depthValue);  

    float dynamicBlend = saturate(blendStrength + (depthValue * depthFactor));

    // Overlay blend
    float3 blendOverlay = (baseColor < 0.5) 
        ? (2.0 * baseColor * paperColor)  
        : (1.0 - 2.0 * (1.0 - baseColor) * (1.0 - paperColor));

    float3 finalColor = lerp(baseColor, blendOverlay, dynamicBlend);

    return float4(finalColor, renderTexture.a);
}
