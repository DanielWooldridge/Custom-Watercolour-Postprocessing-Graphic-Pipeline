// Constant Buffers
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer WaveParamsBuffer : register(b1)
{
    float time;                     // Time for the wave animation
	float amplitude;             // Amplitudes for the waves
	float speed;                 // Speeds for the waves
	float frequency;
    float numWaves;
    float phases;
    float transparency;
    float padding;
};

cbuffer DirectionalLightBuffer : register(b2)
{
    float4 diffuse;        // Diffuse light color
	float3 direction;      // Direction of the light
	float pad;
};

// Input Structure
struct InputType
{
    float4 position : POSITION;    
    float2 tex : TEXCOORD0;      
    float3 normal : NORMAL;      
};

// Output Structure
struct OutputType
{
    float4 position : SV_POSITION;  
    float2 tex : TEXCOORD0;        
    float3 normal : NORMAL;        
    float3 worldPosition : TEXCOORD1;
    float4 colour : COLOR;          
};

// Vertex Shader
OutputType main(InputType input)
{
    OutputType output;

   
    float waveHeight = 0.0f;

   
    for (int i = 0; i < numWaves; ++i)
    {
        // Calculate a different frequency, amplitude, and direction for each wave
        float waveFrequency = frequency * (i + 1); 
        float waveAmplitude = amplitude * (1.0f / (i + 1)); 
        float waveSpeed = speed * (1.0f / (i + 1));  

        // Randomize the direction of each wave using a simple directional vector
        float directionX = cos(i * 3.14159f / numWaves); 
        float directionZ = sin(i * 3.14159f / numWaves);

        // Calculate the sine wave for this particular wave using both X and Z coordinates
        float wave = sin((input.position.x * waveFrequency + input.position.z * waveFrequency) + time * waveSpeed + phases * i);
        wave *= waveAmplitude;

      
        waveHeight += wave;
    }

    // Modify the y-position of the vertex based on the sum of the waves
    input.position.y += waveHeight;


    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);


	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
	output.worldPosition = mul(input.position, worldMatrix).xyz;

    output.tex = input.tex;

    return output;
}
