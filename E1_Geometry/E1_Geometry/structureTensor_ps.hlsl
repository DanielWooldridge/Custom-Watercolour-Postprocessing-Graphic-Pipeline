Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

struct InputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET {
    float2 uv = input.tex;

    // Compute gradients (Sobel-like filter)
    float2 texelSize = float2(1.0 / 1920.0, 1.0 / 1080.0); // We can just sample the Texture

    // Gradient in X direction
    float3 u = -1.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2(-1, -1)).rgb +
               -2.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2(-1,  0)).rgb +
               -1.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2(-1,  1)).rgb +
                1.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2( 1, -1)).rgb +
                2.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2( 1,  0)).rgb +
                1.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2( 1,  1)).rgb;

    // Gradient in Y direction
    float3 v = -1.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2(-1, -1)).rgb +
               -2.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2( 0, -1)).rgb +
               -1.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2( 1, -1)).rgb +
                1.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2(-1,  1)).rgb +
                2.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2( 0,  1)).rgb +
                1.0 * shaderTexture.Sample(sampleType, uv + texelSize * float2( 1,  1)).rgb;

    // Compute structure tensor components
    float E = dot(u, u); // I_x^2 - Gradient Magnitude in X
    float F = dot(u, v); // I_x * I_y - Cross Product
    float G = dot(v, v); // I_y^2 - Gradient Magnitude in Y

    return float4(E, F, G, 1.0); // Store as RGBA (E, F, G, unused)
}
