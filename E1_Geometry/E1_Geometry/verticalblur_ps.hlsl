Texture2D img : register(t0);
SamplerState sampleType : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
   float2 uv = input.tex;

    // Predefined Gaussian weights 
    float weights[5] = { 1.0 / 16.0, 4.0 / 16.0, 6.0 / 16.0, 4.0 / 16.0, 1.0 / 16.0 };

    // Initialize the smoothed tensor result
    float3 smoothedTensor = float3(0.0, 0.0, 0.0);

    // Get texel size
    uint width, height;
    img.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    // Apply Gaussian weights to sample neighboring texels
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(0, -2)).xyz * weights[0];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(0, -1)).xyz * weights[1];
    smoothedTensor += img.Sample(sampleType, uv).xyz * weights[2];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(0, 1)).xyz * weights[3];
    smoothedTensor += img.Sample(sampleType, uv + texelSize * float2(0, 2)).xyz * weights[4];

    // Extract tensor components
    float E = smoothedTensor.x; // xx component
    float F = smoothedTensor.y; // xy component
    float G = smoothedTensor.z; // yy component 


    // Compute dominant egienvalue
    float dominantEigenvalue  = 0.5 * (F + E + sqrt(F * F - 2.0 * E * F + E * E + 4.0 * G * G));

    // Compute dominant eignvelue direction
    float2 dominantDirection  = float2(E - dominantEigenvalue , G);
    float len = length(dominantDirection);

    // Normalize or fall back if near zero
    dominantDirection  = (len < 1e-5) ? float2(0.0f, 1.0f) : dominantDirection  / len;

    return float4(normalize(dominantDirection), sqrt(dominantEigenvalue ), 1.0f);





}
