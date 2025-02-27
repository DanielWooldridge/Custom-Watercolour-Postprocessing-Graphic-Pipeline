Texture2D img : register(t0);  // First DoG pass texture
Texture2D tfm : register(t1);  // Flow Curve texture 
SamplerState sampleType : register(s0);

cbuffer DoGFlowBuffer : register(b1)
{
    float sigma_m;
    float phi;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float2 uv = input.tex;

    // Get direction from flow curve texture
    float2 tangent = normalize(tfm.Sample(sampleType, uv).xy);
   // if (dot(tangent, tangent) == 0.0) return float4(0, 0, 0, 1); 

    // 1st Dog Pass edge intensity
    float H = img.Sample(sampleType, uv).x;
    float weight = 1.0;
    
    // RAnge of integration along the flow curve
    // Mess with these numbers
    float sigmaSq = 2.0 * sigma_m * sigma_m;
    float stepSize = 1.0 / 50.0;  
    float halfWidth = 2.0 * sigma_m;

    float sum = H;

    [loop]  // Makes it loop
    for (int i = 0; i < 30 && i * stepSize < halfWidth; i++)
    {
        float t = i * stepSize;

        // Both directions along the flow curve
        float2 samplePos1 = uv + tangent * t;
        float2 samplePos2 = uv - tangent * t; 

        float sampleH1 = img.Sample(sampleType, samplePos1).x;
        float sampleH2 = img.Sample(sampleType, samplePos2).x;

        // Create weight based on Gaussians
        float weightFactor = exp(-t * t / sigmaSq);
        sum += weightFactor * (sampleH1 + sampleH2);
        weight += 2.0 * weightFactor;
    }

    
    H = sum / weight;

    // Thresholding
    float edge = smoothstep(-phi, phi, H);
    
    return float4(edge, edge, edge, 1.0);
}
