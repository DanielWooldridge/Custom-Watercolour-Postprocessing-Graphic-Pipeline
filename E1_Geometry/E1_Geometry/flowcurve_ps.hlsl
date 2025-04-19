Texture2D flowmap : register(t0);
Texture2D dogImg  : register(t1);
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

struct LIC {
    float2 p;
    float2 t;
    float w;
    float dw;
};

void Step(inout LIC s, float2 texelSize) {
    float2 t = flowmap.Sample(sampleType, s.p).xy;
    if (dot(t, s.t) < 0.0)
        t = -t;
    s.t = t;

    s.dw = (abs(t.x) > abs(t.y)) ?
        abs((frac(s.p.x) - 0.5 - sign(t.x)) / t.x) :
        abs((frac(s.p.y) - 0.5 - sign(t.y)) / t.y);

    s.p += t * s.dw * texelSize;
    s.w += s.dw;
}

float4 main(InputType input) : SV_TARGET {
    float2 uv = clamp(input.tex, 0.0, 1.0);

    // Hardcoded settings
    //float sigma_m = 2.0;
    //float phi = 2;

    uint width, height;
    dogImg.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    float twoSigmaMSquared = 2.0 * sigma_m * sigma_m;
    float halfWidth = 2.0 * sigma_m;

    float H = dogImg.Sample(sampleType, uv).x;
    float w = 1.0;

    // Forward and backward streamline integration
    LIC a, b;
    a.p = b.p = uv;
    a.t = flowmap.Sample(sampleType, uv).xy * texelSize;
    b.t = -a.t;
    a.w = b.w = 0.0;

    [loop]
    while (a.w < halfWidth) {
        Step(a, texelSize);
        float k = a.dw * exp(-a.w * a.w / twoSigmaMSquared);
        H += k * dogImg.Sample(sampleType, clamp(a.p, 0.0, 1.0)).x;
        w += k;
    }

    [loop]
    while (b.w < halfWidth) {
        Step(b, texelSize);
        float k = b.dw * exp(-b.w * b.w / twoSigmaMSquared);
        H += k * dogImg.Sample(sampleType, clamp(b.p, 0.0, 1.0)).x;
        w += k;
    }
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
