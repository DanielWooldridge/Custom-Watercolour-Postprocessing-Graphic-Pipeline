Texture2D paperTex : register(t0);
Texture2D renderTex : register(t1);
SamplerState sampleType : register(s0);


cbuffer paperFilter : register(b1)
{
    float blendStrength;
}

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float4 main(InputType input) : SV_TARGET
{
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



