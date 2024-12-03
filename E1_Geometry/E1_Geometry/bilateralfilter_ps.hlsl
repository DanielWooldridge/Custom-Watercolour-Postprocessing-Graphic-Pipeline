Texture2D inputImage : register(t0);    // Input image texture
Texture2D flowMap : register(t1);      // Flow map texture
SamplerState samplerState : register(s0);

cbuffer FilterSettings : register(b0) {
    float spatial;  // Spatial kernel width 
    float range;    // Range kernel width
    int passing;            // 0 = horizontal, 1 = vertical
    float padding;
};

struct InputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET {
    float2 texCoords = input.tex;

    // Constants for Gaussian weights
    float twoSpatialSigmaSquared = 2.0 * spatial * spatial;
    float twoRangeSigmaSquared = 2.0 * range * range;

    // Sample the flow map for tangent direction
    float2 tangentFlow = flowMap.Sample(samplerState, texCoords).xy;

    // Adjust the direction based on pass (horizontal or vertical)
    float2 direction = (passing == 0)
        ? float2(tangentFlow.y, -tangentFlow.x)  // Rotate tangent 90° for horizontal pass
        : tangentFlow;                         // Use tangent as-is for vertical pass

    // Normalize the sampling step size
    float2 absoluteDirection = abs(direction);
    float stepLength = 1.0 / max(absoluteDirection.x, absoluteDirection.y);
    direction *= stepLength;

    // Center pixel color for range weighting
    float3 centerColor = inputImage.Sample(samplerState, texCoords).rgb;

    // Initialize accumulators for weighted color and normalization factor
    float3 weightedColorSum = centerColor;
    float normalizationFactor = 1.0;

    // Define kernel radius (range of sampling)
    float kernelRadius = 2.0 * spatial;

    // Bilateral filtering loop
    for (int i = 1; i <= kernelRadius; i++) {

        float offset = i * stepLength;

        // Positive direction sample
        float2 positiveOffset = texCoords + direction * offset;
        float3 positiveColor = inputImage.Sample(samplerState, positiveOffset).rgb;

        // Negative direction sample
        float2 negativeOffset = texCoords - direction * offset;
        float3 negativeColor = inputImage.Sample(samplerState, negativeOffset).rgb;

        // Range weight based on color similarity
        float positiveRangeWeight = exp(-dot(positiveColor - centerColor, positiveColor - centerColor) / twoRangeSigmaSquared);
        float negativeRangeWeight = exp(-dot(negativeColor - centerColor, negativeColor - centerColor) / twoRangeSigmaSquared);

        // Spatial weight based on distance from center
        float spatialWeight = exp(-offset * offset / twoSpatialSigmaSquared);

        // Combine weights
        float positiveWeight = spatialWeight * positiveRangeWeight;
        float negativeWeight = spatialWeight * negativeRangeWeight;

        // Accumulate weighted colors
        weightedColorSum += positiveWeight * positiveColor;
        weightedColorSum += negativeWeight * negativeColor;

        // Accumulate normalization factor
        normalizationFactor += positiveWeight + negativeWeight;
    }

    // Compute final color by normalizing
    float3 finalColor = weightedColorSum / normalizationFactor;

    return float4(finalColor, 1.0);  // Output smoothed color
}
