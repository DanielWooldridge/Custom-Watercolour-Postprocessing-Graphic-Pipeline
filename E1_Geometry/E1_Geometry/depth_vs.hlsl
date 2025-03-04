// Constant Buffers
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer WaveBuffer : register(b1)
{
    float time;                     
    float amplitude;             
    float speed;                 
    float frequency;
    float numWaves;
    float phases;
    float transparency;
    float movementType;
};

// Input and Output Structures
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};

// Main Vertex Shader Function
OutputType main(InputType input)
{
    OutputType output;
    float4 transformedPosition = input.position;

    // Apply movementType-based transformations
    if (abs(movementType - 2.0f) < 0.001f) 
    {
        // Floating object movement (Sine wave)
        float waveAmplitude = 3.0f; 
        float waveFrequency = 3.0f;
        transformedPosition.x += sin(time * waveFrequency) * waveAmplitude; 
        transformedPosition.y += sin(time) * waveAmplitude; 
    }
    else if (abs(movementType - 1.0f) < 0.001f) // Ocean waves
    {
        float waveHeight = 0.0f;
        for (int i = 0; i < numWaves; ++i)
        {
            float waveFrequency = frequency * (i + 1); 
            float waveAmplitude = amplitude * (1.0f / (i + 1)); 
            float waveSpeed = speed * (1.0f / (i + 1));  

            float directionX = cos(i * 3.14159f / numWaves); 
            float directionZ = sin(i * 3.14159f / numWaves);

            float wave = sin((input.position.x * waveFrequency + input.position.z * waveFrequency) + time * waveSpeed + phases * i);
            wave *= waveAmplitude;

            waveHeight += wave;
        }
        transformedPosition.y += waveHeight;
    }

    // Apply world, view, and projection transformations
    transformedPosition = mul(transformedPosition, worldMatrix);
    transformedPosition = mul(transformedPosition, viewMatrix);
    transformedPosition = mul(transformedPosition, projectionMatrix);

    output.position = transformedPosition;

    // Store depth
    output.depthPosition = transformedPosition.z / transformedPosition.w;

    return output;
}
