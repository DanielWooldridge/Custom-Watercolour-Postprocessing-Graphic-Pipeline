Texture2D shaderTexture : register(t0);
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

void step() // smooth step function, can i combine this all into one dog shader??? using the esisting smooth step
{
    
    
}

float4 main(InputType input) : SV_TARGET
{
    
    // Mathematical calculation = C(t) = C0 + DeltaC . f(t)
    
    
    
    
    return float4(0, 0, 0, 1);
}
