Texture2D flowmap : register(t0);
Texture2D dogImg  : register(t1);
SamplerState sampleType : register(s0);

cbuffer cFlowBuffer : register(b0)
{
    float2 cpos;
    float2 pTan; 
    float tLength;
    float cLength;
    float pad[2];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};


void Step(inout float2 cpos, inout float2 pTan, inout float tLength, inout float cLength, float2 texelSize)
{
    float2 tangent = flowmap.Sample(sampleType, cpos).xy;
    if (dot(tangent, pTan) < 0.0)
        tangent = -tangent;

    pTan = tangent;

    float safeX = max(abs(tangent.x), 0.001);
    float safeY = max(abs(tangent.y), 0.001);

    cLength = (abs(tangent.x) > abs(tangent.y)) ?
        abs((frac(pTan.x) - 0.5 - sign(tangent.x)) / safeX) :
        abs((frac(pTan.y) - 0.5 - sign(tangent.y)) / safeY);

    cpos += tangent * cLength / texelSize;
    tLength += cLength;
}


float4 main(InputType input) : SV_TARGET {
    float2 uv = input.tex;

    // Hardcoded parameters
    float sigma_m = 2.0;
    float phi = 1.5;

    uint width, height;
    dogImg.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    float twoSigmaMSquared = 2.0 * sigma_m * sigma_m;
    float halfWidth = 2.0 * sigma_m;

    float H = dogImg.Sample(sampleType, uv).x;
    float w = 1.0;

    // Struct values manually handled
    float2 posA = uv;
    float2 posB = uv;
    float2 tanA = flowmap.Sample(sampleType, uv).xy / texelSize;
    float2 tanB = -tanA;
    float lenA = 0.0;
    float lenB = 0.0;
    float stepLen;

    


    // Forward direction
    [loop]
    while (lenA < halfWidth) {
        Step(posA, tanA, lenA, stepLen, texelSize);
        posA = clamp(posA, 0.0, 1.0);
        posB = clamp(posB, 0.0, 1.0);
        float k = stepLen * exp(-lenA * lenA / twoSigmaMSquared);
        H += k * dogImg.Sample(sampleType, posA).x;
        w += k;
    }

    // Backward direction
    [loop]
    while (lenB < halfWidth) {
        Step(posB, tanB, lenB, stepLen, texelSize);
        posA = clamp(posA, 0.0, 1.0);
        posB = clamp(posB, 0.0, 1.0);
        float k = stepLen * exp(-lenB * lenB / twoSigmaMSquared);
        H += k * dogImg.Sample(sampleType, posB).x;
        w += k;
    }

    H /= w;

    // GLSL-style contrast thresholding
    // Thresholding like in the GLSL version
    //float edge = (H > 0.0) ? 1.0 : 2.0 * smoothstep(-2.0, 2.0, 1.5 * H);

    //float edge = smoothstep(0.2, 0.6, H); // Adjust 0.2 and 0.6 as needed
    //return float4(edge.xxx, 1.0);


    float3 ycbcr = dogImg.Sample(sampleType, uv).rgb;
    ycbcr.r = H; // your flow-integrated DoG luminance result
    return float4(ycbcr, 1.0);


   
}
 