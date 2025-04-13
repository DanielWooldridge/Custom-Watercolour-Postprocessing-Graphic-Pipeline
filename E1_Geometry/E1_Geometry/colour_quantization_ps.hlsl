Texture2D img : register(t0);
SamplerState sampleType : register(s0);

struct InputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

cbuffer ColourFilter : register(b0)
{
    float transitionSmoothing;
	int quantLevel;
}

// HARD-CODED PARAMS
static const int NUM_BINS = 8;           // Number of quantization levels
static const float PHI_Q = 3.4f;         // Smooth transition factor

float4 main(InputType input) : SV_TARGET {
    // Sample the YCbCr color
    float3 ycbcr = img.Sample(sampleType, input.tex).rgb;

    // Extract Y (luma)
    float y = ycbcr.x;

    // Hard quantization step
    float qY = floor(y * quantLevel + 0.5) / quantLevel;

    // Smoothstep softens banding
    float smoothY = smoothstep(-2.0, 2.0, transitionSmoothing * (y - qY) * 100.0) - 0.5;
    float quantY = qY + smoothY / quantLevel;

    // Combine quantized Y with original CbCr
    float3 quantYCbCr = float3(quantY, ycbcr.yz);

    // Optional: saturate just in case
    return float4(saturate(quantYCbCr), 1.0);
}
