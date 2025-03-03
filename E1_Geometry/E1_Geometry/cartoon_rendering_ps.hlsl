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

    //float edgeThreshold = 0.5;  
    //float4 edges = smoothstep(edgeThreshold, 1.0, dogFlow);  // Sharpen edges
    //float4 result = colourQuant * edges; // Screen multiplication

    //float edgeThreshold = 0.5;  // Helps to sharpen the edges
    //float4 edges = smoothstep(edgeThreshold, 1.0, dogFlow);  // Converts DoG to binary-like edges

    //// Blending factor for overlaying the edge map
    //float blendFactor = 0.7;

    //// Overlay-style blending, ensuring that the edges influence but do not darken the color too much
    //float4 result = lerp(colourQuant, colourQuant * (1.0 - edges), blendFactor);

    //return saturate(result);


    return result;
}
