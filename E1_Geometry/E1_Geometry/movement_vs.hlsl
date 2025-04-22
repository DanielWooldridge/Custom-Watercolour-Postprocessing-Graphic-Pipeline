cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer TimeBuffer : register(b1)
{
    float time;
    float movementType;
    float padding[2]; 
};


struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


OutputType main(InputType input)
{
    OutputType output;
    float4 transformedPosition = input.position;

    
    // Create sine wave movement
    float waveAmplitude = 3.0f; 
    float waveFrequency = 3.0f; 

    // Adjust x and y positions based on time
    transformedPosition.x += sin(time * waveFrequency) * waveAmplitude; // Side to side
    transformedPosition.y += sin(time) * waveAmplitude; // Up and down
 

    // Apply world, view, and projection transformations
    transformedPosition = mul(transformedPosition, worldMatrix);
    transformedPosition = mul(transformedPosition, viewMatrix);
    transformedPosition = mul(transformedPosition, projectionMatrix);

    output.position = transformedPosition;
    output.tex = input.tex;

    return output;
}
