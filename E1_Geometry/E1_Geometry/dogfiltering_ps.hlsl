Texture2D img : register(t0);   
Texture2D flowmap : register(t1);   // Flow map
SamplerState sampleType : register(s0);

cbuffer dogBuffer
{
    float sensitivity;  // Sensitivity
    float smoothing;  // Smoothing
    float tau;      // Edge thresholding parameter
    float2 texelSize; // Texel size 
    float pad[3];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float2 uv = input.tex;
    uv = clamp(uv, 0.0, 1.0);


    // Precompute sigma values
    float twoSigmaESquared = 2.0 * sensitivity * sensitivity;
    float twoSigmaRSquared = 2.0 * smoothing * smoothing;

    // Load tangent field from texture
    float2 t = flowmap.Sample(sampleType, uv).xy;
    t = normalize(t); // Normalize tangent vector

    // Initialize sums
    float2 sum = 0.0;
    float2 norm = 0.0;

    // Define the kernel half-width
    float halfWidth = 2.0 * smoothing;

    // Perform filtering in 1D along the tangent direction
    for (float d = -halfWidth; d <= halfWidth; d += 1.0)
    {
       float2 kernel = float2(exp(-d * d / twoSigmaESquared), exp(-d * d / twoSigmaRSquared));

        // Sample the image along the offset
        float2 offset = d * t * texelSize;
        float value = img.Sample(sampleType, uv + offset).r; // Luminance channel

        // Update the sums using the Gaussian weights
        sum.x += value * kernel.x; // Spatial Gaussian weight
        sum.y += value * kernel.y; // Range Gaussian weight
        norm.x += kernel.x;
        norm.y += kernel.y;
    }

    sum /= norm; // Normalize the accumulated values

    // Compute the Difference of Gaussians
    float diff = 100.0 * (sum.x - tau * sum.y);
    diff = clamp(diff, 0.0, 1.0); // Clamp to range [0, 1]

    // Output the result
    return float4(diff, diff, diff, 1.0);



    //float value = img.Sample(sampleType, uv).r;
    //return float4(value, value, value, 1.0);
}
