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

    uint width, height;
    img.GetDimensions(width, height);

    float2 texelSize = 1.0 / float2(width, height);

    // Apply Gaussian weights to sample neighboring texels
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(-2, 0)).xyz * weights[0];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(-1, 0)).xyz * weights[1];
    smoothedTensor += img.Sample(sampleType, uv).xyz * weights[2];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(1, 0)).xyz * weights[3];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(2, 0)).xyz * weights[4];

    // Output the smoothed tensor
    return float4(smoothedTensor, 1.0);

    // Highly doubt this is the issue


   //return shaderTexture.Sample(sampleType, input.tex); // Directly output the input texture

}
