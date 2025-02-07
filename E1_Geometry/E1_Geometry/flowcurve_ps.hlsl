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
    float2 pTanL = pTan; 
    tangent = normalize(tangent);


    if (dot(tangent, pTanL) < 0.0)
    {
        tangent = -tangent;
        pTanL = tangent; 
    }
    
    float safeX = max(abs(tangent.x), 0.001f); // Prevents very small numbers
    float safeY = max(abs(tangent.y), 0.001f);

    float cLengthL = (abs(tangent.x) > abs(tangent.y)) ?
        abs((frac(pTanL.x) - 0.5 - sign(tangent.x)) / safeX) :
        abs((frac(pTanL.y) - 0.5 - sign(tangent.y)) / safeY);

   pTanL = clamp(pTanL, -1.0f, 1.0f);



    // Get texture dimensions
    uint width, height;
    flowmap.GetDimensions(width, height);
    
    pTanL += tangent * cLengthL / float2(width, height);
    float tLengthL = tLength + cLengthL; 
    
    //return float4(pTanL.x, pTanL.y, 0.0f, 1.0f);
    //return float4(tangent.x, tangent.y, 0.0f, 1.0f);
   //return float4(cLengthL * 0.5f, cLengthL * 0.5f, cLengthL * 0.5f, 1.0f);
   //return float4(cLengthL, cLengthL, cLengthL, 1.0f);

   return float4(pTanL, 0.0f, 1.0f);

   //return float4(cLengthL * 0.5f, 0, 0, 1);

//cLengthL = cLengthL / (1.0 + cLengthL); // Keep values in range [0,1]
//cLengthL = pow(cLengthL, 0.5); // Apply contrast enhancement
//return float4(cLengthL, cLengthL, cLengthL, 1);













}
