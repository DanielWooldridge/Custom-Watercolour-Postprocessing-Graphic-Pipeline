Texture2D img : register(t0);
SamplerState sampleType : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float2 uv = input.tex;

    // Predefined Gaussian weights (5-tap kernel)
    float weights[5] = { 1.0 / 16.0, 4.0 / 16.0, 6.0 / 16.0, 4.0 / 16.0, 1.0 / 16.0 };

    // Initialize the smoothed tensor result
    float3 smoothedTensor = float3(0.0, 0.0, 0.0);

    // Texel size for horizontal smoothing
    float2 texelSize = float2(1.0 / 1920.0, 0.0); // Horizontal only

    // Apply Gaussian weights to sample neighboring texels
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(-2, 0)).xyz * weights[0];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(-1, 0)).xyz * weights[1];
    smoothedTensor += img.Sample(sampleType, uv).xyz * weights[2];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(1, 0)).xyz * weights[3];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(2, 0)).xyz * weights[4];

    // Output the smoothed tensor
    return float4(smoothedTensor, 1.0);




   //return shaderTexture.Sample(sampleType, input.tex); // Directly output the input texture

}
