Texture2D img : register(t0);       // First DoG pass texture (grayscale edge map)
Texture2D tfm : register(t1);       // Tangent flow map (XY direction)
SamplerState sampleType : register(s0);

cbuffer DoGFlowBuffer : register(b1)
{
    float sigma_m;    // Flow integration scale
    float phi;        // Threshold shaping
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float2 uv = input.tex;

    // Read and normalize flow direction
    float2 tangent = tfm.Sample(sampleType, uv).xy;
    tangent = normalize(tangent);

    // Early out if tangent is invalid
    if (dot(tangent, tangent) < 0.0001)
        return float4(1.0, 0.0, 0.0, 1.0); // Debug: red = bad flow

    // Parameters
    float sigmaSq = 2.0 * sigma_m * sigma_m;
    float stepSize = 1.0 / 200.0;
    int maxSteps = 20;
    float halfWidth = stepSize * maxSteps;

    // Base sample
    float H = img.Sample(sampleType, uv).r;
    float sum = H;
    float weight = 1.0;

    // Integrate along the flow in both directions
    [loop]
    for (int i = 1; i <= maxSteps; ++i)
    {
        float t = i * stepSize;
        float w = exp(-t * t / sigmaSq);

        float2 offset = tangent * t;
        float2 pos1 = clamp(uv + offset, 0.0, 1.0);
        float2 pos2 = clamp(uv - offset, 0.0, 1.0);

        float h1 = img.Sample(sampleType, pos1).r;
        float h2 = img.Sample(sampleType, pos2).r;

        sum += w * (h1 + h2);
        weight += 2.0 * w;
    }

    H = sum / weight;

    // Thresholding (in GLSL it's contrast mapped)
    float edge = smoothstep(-phi, phi, H);

    return float4(edge.xxx, 1.0);
}
