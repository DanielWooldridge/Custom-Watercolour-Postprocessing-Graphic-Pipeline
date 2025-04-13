Texture2D inputImage : register(t0);    // Input image texture
Texture2D flowMap : register(t1);       // Flow map texture
SamplerState sampleType : register(s0);

cbuffer FilterSettings : register(b0) {
    float spatial;     
    float rangeK;      
    int passing;       
    float padding;
};

struct InputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET {
    float2 texCoords = input.tex;

    // Sigma terms
    float twoSpatialSigmaSquared = 2.0 * spatial * spatial;
    float twoRangeSigmaSquared = 2.0 * rangeK * rangeK;

    // Sample flow direction
    float2 tangentFlow = flowMap.Sample(sampleType, texCoords).xy;

    float2 direction = (passing == 0)
        ? float2(tangentFlow.y, -tangentFlow.x)  // Gradient direction
        : tangentFlow;                           // Flow direction

    // Compute step size based on max axis
    float2 absDir = abs(direction);
    float ds = 1.0 / max(absDir.x, absDir.y); // Stable step size
    direction = normalize(direction);         // We normalize and scale with d later

    float halfWidth = 2.0 * spatial;

    float3 centerColor = inputImage.Sample(sampleType, texCoords).rgb;
    float3 sumColor = centerColor;
    float normFactor = 1.0;

    // Iterate over kernel using float d
    [loop]
    for (float d = ds; d <= halfWidth; d += ds) {
        float2 offsetVec = direction * d;

        float3 samplePos = inputImage.Sample(sampleType, clamp(texCoords + offsetVec, 0.0, 1.0)).rgb;
        float3 sampleNeg = inputImage.Sample(sampleType, clamp(texCoords - offsetVec, 0.0, 1.0)).rgb;

        float colorDistPos = length(samplePos - centerColor);
        float colorDistNeg = length(sampleNeg - centerColor);

        float spatialWeight = exp(-d * d / twoSpatialSigmaSquared);
        float rangeWeightPos = exp(-colorDistPos * colorDistPos / twoRangeSigmaSquared);
        float rangeWeightNeg = exp(-colorDistNeg * colorDistNeg / twoRangeSigmaSquared);

        float weightPos = spatialWeight * rangeWeightPos;
        float weightNeg = spatialWeight * rangeWeightNeg;

        sumColor += samplePos * weightPos;
        sumColor += sampleNeg * weightNeg;
        normFactor += weightPos + weightNeg;
    }

    float3 finalColor = sumColor / max(normFactor, 1e-6);
    return float4(finalColor, 1.0);
}
