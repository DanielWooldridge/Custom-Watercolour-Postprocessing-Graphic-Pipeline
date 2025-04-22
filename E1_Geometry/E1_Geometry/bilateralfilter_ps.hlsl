Texture2D inputImage : register(t0);    // Input image texture
Texture2D flowMap : register(t1);       // Flow map texture
SamplerState sampleType : register(s0);

cbuffer FilterSettings : register(b0) {
    float spatial;     
    float rangeK;      
    int passing;       
};

struct InputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET {

    // Compute constants for Gaussian weighting
    float twoSigmaD2 = 2.0 * spatial * spatial;
    float twoSigmaR2 = 2.0 * rangeK * rangeK;
    float2 uv = input.tex;

    // Sample Flow map
    float2 flowDirection = flowMap.Sample(sampleType, uv).xy;

    // Get Sampling direction based on passing variable and normalize step
    float2 samplingDirection = (passing == 0) ? float2(flowDirection.y, -flowDirection.x) : flowDirection;
    float2 absDirection = abs(samplingDirection);
    float stepIncrement = 1.0 / max(absDirection.x, absDirection.y);

    // Get texel size
    int width, height;
    inputImage.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    // Scale direction by texelSize
    samplingDirection *= texelSize;

    // Read center pixel colour
    float3 centerColor = inputImage.Sample(sampleType, uv).rgb;
    float3 blendedColor = centerColor; 
    float weightSum = 1.0;    

    float halfWidth = 2.0 * spatial;
  
    [loop]
    for (float d = stepIncrement; d <= halfWidth; d += stepIncrement) {

        // Sample forward and backward along direction 
        float3 sampleColorForward = inputImage.Sample(sampleType, uv + d * samplingDirection).rgb;
        float3 sampleColorBackward = inputImage.Sample(sampleType, uv - d * samplingDirection).rgb;

        // COmpute differences for weight
        float colorDiffForward = length(sampleColorForward - centerColor);
        float colorDiffBackward = length(sampleColorBackward - centerColor);

        // Spatial weight
        float spatialWeight = exp(-d * d / twoSigmaD2);

        // Range weight
        float rangeWeightForward = (colorDiffForward < rangeK) ? 1.0 : 0.0;
        float rangeWeightBackward = (colorDiffBackward < rangeK) ? 1.0 : 0.0;

        // Noramlize and accumulate values
        weightSum += spatialWeight * rangeWeightForward;
        weightSum += spatialWeight * rangeWeightBackward;

        blendedColor += spatialWeight * rangeWeightForward * sampleColorForward;
        blendedColor += spatialWeight * rangeWeightBackward * sampleColorBackward;
    }

    // Normalize final colour
    blendedColor /= weightSum;
    return float4(blendedColor, 1.0);
}
