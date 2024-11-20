// Skybox Vertex Shader

cbuffer MatrixBuffer : register(b0) {
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}
struct InputType {
    float4 position : POSITION;
    float3 texCoord : TEXCOORD0; // Cubemap coordinates
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

 OutputType main(InputType input) {

    OutputType output;

    float4 worldPosition = mul(input.position, worldMatrix);
    float4 viewPosition = mul(worldPosition, viewMatrix);
    output.position = mul(viewPosition, projectionMatrix);

    output.tex = input.texCoord;

    return output;
}
