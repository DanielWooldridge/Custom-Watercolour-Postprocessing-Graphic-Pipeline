Texture2D flowmap : register(t0);
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

float4 main(InputType input) : SV_TARGET
{
    float2 tangent = flowmap.Sample(sampleType, input.tex).xy;
    float2 localTan = pTan;

    if (dot(tangent, localTan) < 0.0)
    {
        tangent = -tangent;
        localTan = tangent;
    }

    float safeX = max(abs(tangent.x), 0.001);
    float safeY = max(abs(tangent.y), 0.001);

    float cLengthL = (abs(tangent.x) > abs(tangent.y)) ?
        abs((frac(localTan.x) - 0.5 - sign(tangent.x)) / safeX) :
        abs((frac(localTan.y) - 0.5 - sign(tangent.y)) / safeY);

    localTan = clamp(localTan, -1.0f, 1.0f);

    uint width, height;
    flowmap.GetDimensions(width, height);

    localTan += tangent * cLengthL / float2(width, height);

    return float4(localTan, 0.0f, 1.0f);
}