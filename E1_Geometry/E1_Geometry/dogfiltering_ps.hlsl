Texture2D img : register(t0);   
Texture2D flowmap : register(t1);   
SamplerState sampleType : register(s0);

cbuffer dogBuffer : register(b0)
{
    float sensitivity;
    float smoothing;
    float tau;
    float2 texelize;
    float pad[3];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float2 uv = clamp(input.tex, 0.0, 1.0);

    // Compute constants for Gaussian weighting
    float sigmaEdgeSquared = 2.0 * sensitivity * sensitivity;
    float sigmaRangeSquared = 2.0 * smoothing * smoothing;

    // Sample flow direction and get perpendicular for edge 
    float2 flowDir = flowmap.Sample(sampleType, uv).xy;
    float2 edgeDir = float2(flowDir.y, -flowDir.x);
    float2 edgeDirAbs = abs(edgeDir);
    float stepSize = 1.0 / max(edgeDirAbs.x, edgeDirAbs.y);

    // Get texel size
    int width, height;
    img.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    edgeDir *= texelSize;

    // Sample center luminance value
    float centerLuminance = img.Sample(sampleType, uv).r;

    float2 blurredSum = float2(centerLuminance, centerLuminance);
    float2 normalization = float2(1.0, 1.0);

    float kernelRadius = 2.0 * smoothing;

    [loop]
    for (float d = stepSize; d <= kernelRadius; d += stepSize)
    {
        // Guassian kernel weights for offset
        float2 kernel = float2(
            exp(-d * d / sigmaEdgeSquared),
            exp(-d * d / sigmaRangeSquared)
        );

        // Offset positions
        float2 offset = d * edgeDir;
        float2 uvPos = clamp(uv + offset, 0.0, 1.0);
        float2 uvNeg = clamp(uv - offset, 0.0, 1.0);

        // Sample Luminance values in both directions
        float luminancePos = img.Sample(sampleType, uvPos).r;
        float luminanceNeg = img.Sample(sampleType, uvNeg).r;

        // Accumulate weighted values for both narrow and wide filters
        blurredSum += kernel * (luminancePos + luminanceNeg);
        normalization += 2.0 * kernel;
    }

    // Normalize
    blurredSum /= normalization;

    // Final DoG value 
    float edgeDifference = (blurredSum.x - tau * blurredSum.y);

    // Add contrast boost
    edgeDifference *= 100.0;

    return float4(edgeDifference.xxx, 1.0); // Output greyscale
}
