Texture2D inputImage : register(t0);    // Input image texture
Texture2D flowMap : register(t1);      // Flow map texture
SamplerState sampleType : register(s0);

cbuffer FilterSettings : register(b0) {
    float spatial;  // Spatial kernel width 
    float rangeK;    // Range kernel width
    int passing;    // 0 = horizontal, 1 = vertical
    float padding;
};

struct InputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET {
    float2 texCoords = input.tex;

    // Compute Gaussian parameters
    float twoSpatialSigmaSquared = 2.0 * spatial * spatial;
    float twoRangeSigmaSquared = 2.0 * rangeK * rangeK;

    // Sample the flow map for tangent direction
    float2 tangentFlow = flowMap.Sample(sampleType, texCoords).xy;


    float2 direction = (passing == 0)
        ? float2(tangentFlow.y, -tangentFlow.x)  // Gradient direction (perpendicular to flow)
        : tangentFlow;                           // Tangent direction (aligned with flow)

    // Normalize sampling step size
    float2 absoluteDirection = abs(direction);
    float stepLength = 1.0 / max(absoluteDirection.x, absoluteDirection.y); // Prevent zero division
    direction /= stepLength;

    // Get center pixel color for range weighting
    float3 centerColor = inputImage.Sample(sampleType, texCoords).rgb;

    // Accumulators
    float3 weightedColorSum = centerColor;
    float normalizationFactor = 1.0;

    // Define integer kernel radius
    float halfWidth = 2.0 * spatial;
    int numIterations = int(ceil(halfWidth / stepLength)); // Ensure finite iterations

    // Bilateral filtering loop with forced loop handling
    [loop] 
    for (int i = 1; i <= numIterations; i++) 
    {
        float off = i * stepLength;

        // Compute offset coordinates
        float2 posOffset = texCoords + direction * off;
        float2 negOffset = texCoords - direction * off;

        // Sample colors
        float3 posColor = inputImage.Sample(sampleType, posOffset).rgb;
        float3 negColor = inputImage.Sample(sampleType, negOffset).rgb;

         //Compute range weights (color similarity)
        float3 posDiff = posColor - centerColor;
        float3 negDiff = negColor - centerColor;
        float posRangeWeight = exp(-dot(posDiff, posDiff) / twoRangeSigmaSquared);
        float negRangeWeight = exp(-dot(negDiff, negDiff) / twoRangeSigmaSquared);

        //// LUminance
                
        //float centerLuminance = dot(centerColor, float3(0.2126, 0.7152, 0.0722));
        //float posLuminance = dot(posColor, float3(0.2126, 0.7152, 0.0722));
        //float negLuminance = dot(negColor, float3(0.2126, 0.7152, 0.0722));

        //// Compute luminance-based range weights
        //float posRangeWeight = exp(-pow(posLuminance - centerLuminance, 2) / twoRangeSigmaSquared);
        //float negRangeWeight = exp(-pow(negLuminance - centerLuminance, 2) / twoRangeSigmaSquared);


        // Compute spatial weight (distance-based)
        float spatialWeight = exp(-off * off / twoSpatialSigmaSquared);

        // Compute final weights
        float posWeight = spatialWeight * posRangeWeight;
        float negWeight = spatialWeight * negRangeWeight;

        // Accumulate weighted colors
        weightedColorSum += posWeight * posColor;
        weightedColorSum += negWeight * negColor;

        // Accumulate normalization factor
        normalizationFactor += posWeight + negWeight;
    }

    // Compute final color with normalization
    float3 finalColor = weightedColorSum / max(normalizationFactor, 1e-6); // Prevent zero division

    return float4(finalColor, 1.0); // Output smoothed color
}
