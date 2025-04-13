Texture2D img : register(t0);   
Texture2D flowmap : register(t1);   
SamplerState sampleType : register(s0);

cbuffer dogBuffer : register(b0)
{
    float sensitivity;
    float smoothing;
    float tau;
    float2 texelSize;
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

    float twoSigmaESquared = 2.0 * sensitivity * sensitivity;
    float twoSigmaRSquared = 2.0 * smoothing * smoothing;

    float2 t = normalize(flowmap.Sample(sampleType, uv).xy);
    if (any(isnan(t))) return float4(1, 0, 0, 1); 

    float2 n = normalize(float2(t.y, -t.x));
    float2 nabs = abs(n);
    float ds = 1.0 / max(nabs.x, nabs.y);
    n = n * ds / texelSize;

    float centerY = img.Sample(sampleType, uv).r;
    float2 sum = float2(centerY, centerY);
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

        float y0 = img.Sample(sampleType, sampleUV0).r;
        float y1 = img.Sample(sampleType, sampleUV1).r;

        sum += kernel * (y0 + y1);
        norm += 2.0 * kernel;
    }

    sum /= norm;

    // DoG diff calculation
    float diff = 10.0 * (sum.x - tau * sum.y) * smoothing;
    diff = saturate(1.0 - diff);  // Invert for dark lines

    // Posterization
    float levels = 5.0; 
    diff = floor(diff * (levels - 1.0) + 0.5) / (levels - 1.0);

    // Preserve CbCr, modulate only Y
    float3 ycbcr = img.Sample(sampleType, uv).rgb;
    float Y = diff;
    float Cb = ycbcr.g;
    float Cr = ycbcr.b;

    return float4(Y, Cb, Cr, 1.0); // Full YCbCr output
}
