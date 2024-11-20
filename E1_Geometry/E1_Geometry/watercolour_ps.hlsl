Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

struct InputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


const int StrokeSize = 5; // Increase thickness of the stroke for stronger effect
const float Threshold = 0.2f; // Threshold for alpha


// Random Function
float rand(float2 coord) {
    return frac(sin(dot(coord.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float4 main(InputType input) : SV_TARGET {
    float2 uv = input.tex;

    // Sample the base color from the texture
    float4 color = shaderTexture.Sample(sampleType, uv);

    // Quantize the color for a painterly look
    int colorLevels = 9; // Fewer levels for stronger color stepping
    color.rgb = floor(color.rgb * colorLevels) / colorLevels;

    // Define brushstroke direction with added variation
    float2 brushDirection = normalize(float2(0.02, 0.03) + uv * 0.1); 

    // Apply stroke in brush direction with varying stroke widths
    float4 strokeColor = float4(1.0f, 1.0f, 1.0f, 1.0f); // White stroke color for now
    for (int i = 1; i <= StrokeSize; i++) { // Loop through stroke size
        for (int j = -1; j <= 1; j++) { // Sample offsets in X direction
            for (int k = -1; k <= 1; k++) { // Sample offsets in Y direction
                float2 offset = float2(j, k) * brushDirection * float(i) * 0.005; // Variable width strokes
                float alpha = shaderTexture.Sample(sampleType, uv + offset).a; // Sample alpha at offset
                if (alpha >= Threshold) { // Check against alpha threshold
                    strokeColor = float4(1.0f, 1.0f, 1.0f, 1.0f); // Apply stroke color
                    break; // Exit loops once stroke is applied
                }
            }
        }
    }

    // Blend between the base color and stroke effect with a subtle factor
    color = lerp(color, strokeColor, 0.2);

    // Edge detection to simulate thick paint ridges
    float edge = abs(shaderTexture.Sample(sampleType, uv + float2(0.001, 0)).r - 
                     shaderTexture.Sample(sampleType, uv - float2(0.001, 0)).r);
    edge += abs(shaderTexture.Sample(sampleType, uv + float2(0, 0.001)).r - 
                shaderTexture.Sample(sampleType, uv - float2(0, 0.001)).r);

    // Emphasize depth by darkening edges more strongly
    color.rgb = lerp(color.rgb, float3(0.9, 0.9, 0.9), edge * 0.3); 
    return color;
}
