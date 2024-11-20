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
    float padding[2]; // Padding to ensure 16-byte alignment
};

// Define input and output structures for the vertex shader
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

// Main vertex shader function
OutputType main(InputType input)
{
    OutputType output;
    float4 transformedPosition = input.position;

    //// Apply transformations based on movementType range
    //if (movementType < 1.0f)
    //{
    //    // Sine wave (vertical) movement for values less than 1.0
    //    transformedPosition.y += sin(time) * 5.0f;
    //}
    //else
    //{
    //    // Circular movement for values 1.0 and above
    //    transformedPosition.x += cos(time) * 5.0f;
    //    transformedPosition.z += sin(time) * 5.0f;
    //}

    
    // Create sine wave movement
    float waveAmplitude = 3.0f; // Amplitude of the sine wave
    float waveFrequency = 3.0f; // Frequency of the sine wave

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
