Texture2D flowMap : register(t0);
Texture2D dogImage : register(t1);
SamplerState sampleType : register(s0);

struct InputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

cbuffer filterSettings : register(b0)
{
    float phi;
    float sigma_m;
    int invertedLines;
    int polsterize;
}

struct StepState {
    float2 uv;
    float2 flowDir;
    float totalLength;
    float stepDistance;
};

// Step Function to move along flow field
void Step(inout StepState s, float2 texelSize) {
    float2 sampledFlow = flowMap.Sample(sampleType, s.uv).xy;

    // Flip direction to maintain consistent flow
    if (dot(sampledFlow, s.flowDir) < 0.0)
        sampledFlow = -sampledFlow;
    s.flowDir = sampledFlow;

    // Compute distance to next texel 
    s.stepDistance = (abs(sampledFlow.x) > abs(sampledFlow.y)) ?
        abs((frac(s.uv.x) - 0.5 - sign(sampledFlow.x)) / sampledFlow.x) :
        abs((frac(s.uv.y) - 0.5 - sign(sampledFlow.y)) / sampledFlow.y);

    // Move along flow direction
    s.uv += sampledFlow * s.stepDistance * texelSize;
    s.totalLength += s.stepDistance;
}

float4 main(InputType input) : SV_TARGET {
    float2 uv = clamp(input.tex, 0.0, 1.0);

    // Get texture size
    uint width, height;
    dogImage.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    float twoSigmaMSquared = 2.0 * sigma_m * sigma_m;
    float halfWidth = 2.0 * sigma_m;

    float H = dogImage.Sample(sampleType, uv).x;
    float w = 1.0;

    // Set up struct values
    StepState forward, backward;
    forward.uv = backward.uv = uv;
    forward.flowDir = flowMap.Sample(sampleType, uv).xy * texelSize;
    backward.flowDir = -forward.flowDir;
    forward.totalLength = backward.totalLength = 0.0;

    [loop]
    // Foward step
    while (forward.totalLength < halfWidth) {
        Step(forward, texelSize);
        float k = forward.stepDistance * exp(-forward.totalLength * forward.totalLength / twoSigmaMSquared);
        H += k * dogImage.Sample(sampleType, clamp(forward.uv, 0.0, 1.0)).x;
        w += k;
    }

    [loop]
    // backward step
    while (backward.totalLength < halfWidth) {
        Step(backward, texelSize);
        float k = backward.stepDistance * exp(-backward.totalLength * backward.totalLength / twoSigmaMSquared);
        H += k * dogImage.Sample(sampleType, clamp(backward.uv, 0.0, 1.0)).x;
        w += k;
    }

    // Normalize
    H /= w;

    // Smooth edge thresholding
    float edge = (H > 0.0) ? 1.0 : 2.0 * smoothstep(-2.0, 2.0, phi * H);

    // Invert lines if enabled
    if (invertedLines == 0) {
        edge = saturate(1.0 - edge);
    }

    // Posterize if enabled
    if (polsterize == 0) {
        float levels = 5.0; // tweak if needed
        edge = floor(edge * (levels - 1.0) + 0.5) / (levels - 1.0);
    }

    return float4(edge.xxx, 1.0);
}
