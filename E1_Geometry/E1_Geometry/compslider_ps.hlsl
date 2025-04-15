Texture2D aTexture : register(t0);
Texture2D bTexture : register(t1);
SamplerState aSampler : register(s0);
SamplerState bSampler : register(s1);

cbuffer SliderBuffer : register(b1) 
{
    float sliderPosition;
    int visualizeInRGB;
    float2 padding; // 16-byte alignment for cbuffer
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

// YCbCr to RGB conversion function
float3 YCbCrToRGB(float3 ycbcr)
{
    float Y = ycbcr.r;
    float Cb = ycbcr.g - 0.5;
    float Cr = ycbcr.b - 0.5;

    float R = Y + 1.402 * Cr;
    float G = Y - 0.344136 * Cb - 0.714136 * Cr;
    float B = Y + 1.772 * Cb;

    return float3(R, G, B);
}

float4 main(InputType input) : SV_TARGET
{
    float2 uv = input.tex;
    float4 aColor = aTexture.Sample(aSampler, uv);
    float4 bColor = bTexture.Sample(bSampler, uv);

    if (visualizeInRGB)
    {
        bColor.rgb = YCbCrToRGB(bColor.rgb);
    }

    return (uv.x < sliderPosition) ? aColor : bColor;
}
