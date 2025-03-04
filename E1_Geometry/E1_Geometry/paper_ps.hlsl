Texture2D paperTex : register(t0);
Texture2D renderTex : register(t1);
Texture2D depthTex : register(t2);
SamplerState sampleType : register(s0);

cbuffer PaperFilter : register(b1)
{
    float blendStrength;  // Minimum blend strength
    float depthFactor;        // How much depth affects blend strength
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    //// Sample the textures
    //float4 paperTexture = paperTex.Sample(sampleType, input.tex);
    //float4 renderTexture = renderTex.Sample(sampleType, input.tex);
    //float depthValue = depthTex.Sample(sampleType, input.tex).r; // Read depth (usually red channel)

    //// Convert paper texture to grayscale for better blending
    //float paperDetail = dot(paperTexture.rgb, float3(0.3, 0.59, 0.11));

    //// Adjust blend strength based on depth (Closer = Stronger Paper Effect)
    //float dynamicBlend = blendStrength + ((1.0 - depthValue) * depthFactor);
    //dynamicBlend = saturate(dynamicBlend); // Keep in range [0,1]

    //// Ensure skybox or undefined depth areas get minimal paper effect
    //if (depthValue > 0.99) dynamicBlend = blendStrength;

    //// Blend the render texture with the paper texture based on adjusted blend strength
    //float4 result = lerp(renderTexture, renderTexture * (paperDetail + 0.5), dynamicBlend);

    //return result;



     // Sample the texture
    float4 paperTexture = paperTex.Sample(sampleType, input.tex);
    //paperTexture.a = 0.4f;

    float4 renderTexture = renderTex.Sample(sampleType, input.tex);


    //float4 result = 1 - (1 - renderTexture) * (1 - paperTexture);
    //float4 result = max(0, renderTex + paperTex - 1);

    // Return the greyscale color while retaining the original alpha
    //return (renderTexture + paperTexture - 0.4);
    //return result;



    float4 paperDetail = dot(paperTexture.rgb, float3(0.3, 0.59, 0.11)); 
    //float blendStrength = 0.6;  // Adjust for stronger or weaker texture presence
    float4 result = lerp(renderTexture, renderTexture * (paperDetail + 0.5), blendStrength);
    return result;

}
