Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// Function of matrix conversion
float3 RGBtoYCbCr(float3 rgb)
{
    float Y  = 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b;
    float Cb = -0.169 * rgb.r - 0.331 * rgb.g + 0.500 * rgb.b;
    float Cr =  0.500 * rgb.r - 0.419 * rgb.g - 0.081 * rgb.b;

  
    return float3(Y, Cb + (128.0 / 255.0), Cr + (128.0 / 255.0));
}


float4 main(InputType input) : SV_TARGET {


    float3 rgb = tex.Sample(samplerState, input.tex).rgb;
    return float4(RGBtoYCbCr(rgb), 1.0);    // convert colour space
}   
