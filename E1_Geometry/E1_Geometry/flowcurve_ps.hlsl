Texture2D flowmap : register(t0);
SamplerState sampleType : register(s0);

cbuffer cFlowBuffer
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

    if (dot(tangent, pTan) < 0.0)
    {
        tangent = -tangent;
        pTan = tangent;
    }
    
    cLength = (abs(tangent.x) > abs(tangent.y)) ?
            abs((frac(pTan.x) - 0.5 - sign(tangent.x)) / tangent.x) :
            abs((frac(pTan.y) - 0.5 - sign(tangent.y)) / tangent.y);
    
    pTan += tangent * cLength / flowmap.GetDimensions(0, 0); // change this;
    tLength += cLength;
    
    return float4(0, 0, 0, 1);
}
