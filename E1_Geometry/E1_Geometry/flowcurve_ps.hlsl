Texture2D flowmap : register(t0);
SamplerState sampleType : register(s0);

cbuffer cFlowBuffer : register(b0)
{
    float2 cpos;
    float2 pTan; // This is constant and cannot be modified
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
    float2 pTanL = pTan; 

    if (dot(tangent, pTanL) < 0.0)
    {
        tangent = -tangent;
        pTanL = tangent; 
    }
    
    float cLengthL = (abs(tangent.x) > abs(tangent.y)) ?
        abs((frac(pTanL.x) - 0.5 - sign(tangent.x)) / tangent.x) :
        abs((frac(pTanL.y) - 0.5 - sign(tangent.y)) / tangent.y);
    
    // Get texture dimensions
    uint width, height;
    flowmap.GetDimensions(width, height);
    
    pTanL += tangent * cLengthL / float2(width, height);
    float tLengthL = tLength + cLengthL; 
    
    return float4(0, 0, 0, 1);
}
