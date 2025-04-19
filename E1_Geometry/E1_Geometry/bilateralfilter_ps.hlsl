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
    float twoSigmaD2 = 2.0 * spatial * spatial;
    float twoSigmaR2 = 2.0 * rangeK * rangeK;
    float2 uv = input.tex;

    float2 t = flowMap.Sample(sampleType, uv).xy;
    float2 dir = (passing == 0) ? float2(t.y, -t.x) : t;

    float2 dabs = abs(dir);
    float ds = 1.0 / max(dabs.x, dabs.y);

    // Get texel size
    uint width, height;
    inputImage.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    dir *= texelSize;

    float3 center = inputImage.Sample(sampleType, uv).rgb;
    float3 sum = center;
    float norm = 1.0;

    float halfWidth = 2.0 * spatial;
    [loop]
    for (float d = ds; d <= halfWidth; d += ds) {
        float3 c0 = inputImage.Sample(sampleType, uv + d * dir).rgb;
        float3 c1 = inputImage.Sample(sampleType, uv - d * dir).rgb;
        float e0 = length(c0 - center);
        float e1 = length(c1 - center);

        float kerneld = exp(-d * d / twoSigmaD2);
        float kernele0 = (e0 < rangeK) ? 1.0 : 0.0;
        float kernele1 = (e1 < rangeK) ? 1.0 : 0.0;

        norm += kerneld * kernele0;
        norm += kerneld * kernele1;

        sum += kerneld * kernele0 * c0;
        sum += kerneld * kernele1 * c1;
    }

    sum /= norm;
    return float4(sum, 1.0);
}
