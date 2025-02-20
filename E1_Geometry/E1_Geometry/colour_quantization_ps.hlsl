Texture2D img : register(t0);
SamplerState sampleType : register(s0);

cbuffer cqBuffer : register(b1)
{
    float transitionSmoothing;
    float3 padding;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    
    // Sample color from input texture
    float4 colour = img.Sample(sampleType, input.tex);

    // Number of quantization levels (e.g., 10)
    float numLevels = 2.0;

    // Floor the color to the nearest level
    float3 qn = floor(colour * numLevels + 0.5) / numLevels;

    // Apply smoothstep to soften transitions
    float3 qs = smoothstep(-2.0, 2.0, transitionSmoothing * (colour - qn) * 100.0) - 0.5;
    float3 qc = qn + qs / numLevels;

  

    //return float4(1, 0, 0, 1);  // Red output
    return colour;

    //return img.Sample(sampleType, input.tex); // Directly output the texture color
    //return float4(qn, 1.0);  // This should give you blocky colors

}
