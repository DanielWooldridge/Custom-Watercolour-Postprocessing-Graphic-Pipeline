Texture2D img : register(t0);   
Texture2D flowmap : register(t1);   // Flow map
SamplerState sampleType : register(s0);

cbuffer dogBuffer
{
    float sensitivity;    // Edge spatial smoothing
    float smoothing;      // Edge range smoothing
    float tau;            // DoG contrast threshold
    float2 texelSize;     // Texel size
    float pad[3];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

// Standard luminance coefficients
float luminance(float3 color) {
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

float4 main(InputType input) : SV_TARGET
{
    float2 uv = clamp(input.tex, 0.0, 1.0);

    float twoSigmaESquared = 2.0 * sensitivity * sensitivity;
    float twoSigmaRSquared = 2.0 * smoothing * smoothing;

    float2 t = normalize(flowmap.Sample(sampleType, uv).xy);
    if (any(isnan(t))) return float4(1, 0, 0, 1); // Debug: red = broken

    float2 n = normalize(float2(t.y, -t.x)); // Perpendicular to tangent
    float2 nabs = abs(n);
    float ds = 1.0 / max(nabs.x, nabs.y);
    n = n * ds / texelSize;

    float center = luminance(img.Sample(sampleType, uv).rgb);
    float2 sum = float2(center, center);
    float2 norm = float2(1.0, 1.0);

    float halfWidth = 2.0 * smoothing;

    [loop]
    for (float d = ds; d <= halfWidth; d += ds)
    {
        float2 kernel = float2(
            exp(-d * d / twoSigmaESquared),
            exp(-d * d / twoSigmaRSquared)
        );

        float2 offset = d * n;

        float2 sampleUV0 = clamp(uv + offset, 0.0, 1.0);
        float2 sampleUV1 = clamp(uv - offset, 0.0, 1.0);

        float value0 = luminance(img.Sample(sampleType, sampleUV0).rgb);
        float value1 = luminance(img.Sample(sampleType, sampleUV1).rgb);

        sum += kernel * (value0 + value1);
        norm += 2.0 * kernel;
    }

    // Normalize
    sum /= norm;

    // Compute edge difference
    float diff = 10.0 * (sum.x - tau * sum.y) * smoothing;
    diff = clamp(diff, 0.0, 1.0);

    // Posterization (optional but nice)
    float levels = 5.0; 
    diff = floor(diff * (levels - 1.0) + 0.5) / (levels - 1.0);

    return float4(diff.xxx, 1.0); // Output grayscale
}
