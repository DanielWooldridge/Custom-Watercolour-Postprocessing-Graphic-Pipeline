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

float ApplySobel(float2 uv)
{
    float sampleValues[9];
    float2 offsets[9] = {
        float2(-1, -1), float2(0, -1), float2(1, -1),
        float2(-1,  0), float2(0,  0), float2(1,  0),
        float2(-1,  1), float2(0,  1), float2(1,  1)
    };

    // Sample DoG results
    for (int i = 0; i < 9; i++)
    {
        float2 sampleUV = uv + offsets[i] * texelSize;
        sampleValues[i] = img.Sample(sampleType, sampleUV).r;
    }

    // Sobel kernels
    float SobelX[9] = { -1,  0,  1, -2,  0,  2, -1,  0,  1 };
    float SobelY[9] = { -1, -2, -1,  0,  0,  0,  1,  2,  1 };

    float Gx = 0, Gy = 0;
    for (int i = 0; i < 9; i++)
    {
        Gx += sampleValues[i] * SobelX[i];
        Gy += sampleValues[i] * SobelY[i];
    }

    // Compute final Sobel edge magnitude
    float sobelEdge = sqrt(Gx * Gx + Gy * Gy);
    sobelEdge = clamp(sobelEdge * 2.0, 0.0, 1.0);

    return sobelEdge;
}


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
    //[unroll(62)]
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

    //With Sobel
    //return float4(sobelOperation, sobelOperation, sobelOperation, 1.0);
    // Should show gradient colors


   //float testValue = img.Sample(sampleType, uv).r;
   // return float4(testValue, testValue, testValue, 1.0); // Should show grayscale DoG image

    //float DoG = img.Sample(sampleType, uv).r; 
    //return float4(DoG, DoG, DoG, 1.0);


    //float value = img.Sample(sampleType, uv).r;
    //return float4(value, value, value, 1.0);
}
