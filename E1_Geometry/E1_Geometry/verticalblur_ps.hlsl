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

    // Predefined Gaussian weights (5-tap kernel)
    float weights[5] = { 1.0 / 16.0, 4.0 / 16.0, 6.0 / 16.0, 4.0 / 16.0, 1.0 / 16.0 };

    // Initialize the smoothed tensor result
    float3 smoothedTensor = float3(0.0, 0.0, 0.0);

    // Texel size for vertical smoothing
    float2 texelSize = float2(0.0, 1.0 / 1080.0); // Vertical only - sample for the .y

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
    // Compute the dominant eigenvector v2
    float lambda1 = 0.5 * (E + G + sqrt((E - G) * (E - G) + 4.0 * F * F));
    float lambda2 = 0.5 * (E + G - sqrt((E - G) * (E - G) + 4.0 * F * F));

    float2 v2 = normalize(float2(lambda1 - G, F)); // Dominant eigenvector

    // Output the flow direction (v2) and magnitude of λ1
    return float4(v2, sqrt(lambda1), 1.0);



   //return shaderTexture.Sample(sampleType, input.tex); // Directly output the input texture


   // This version of the code allows the successful rendering of the skybox but also depletes the overall look of the bilateral filters

    //float2 uv = input.tex;

    //// Predefined Gaussian weights (5-tap kernel)
    //float weights[5] = { 1.0 / 16.0, 4.0 / 16.0, 6.0 / 16.0, 4.0 / 16.0, 1.0 / 16.0 };

    //// Initialize the smoothed tensor result
    //float3 smoothedTensor = float3(0.0, 0.0, 0.0);

    //// Texel size for vertical smoothing
    //float2 texelSize = float2(0.0, 1.0 / 1080.0); // Adjust for resolution

    //// Sample neighboring texels
    //float3 sample0 = img.Sample(sampleType, uv + texelSize * float2(0, -2)).xyz;
    //float3 sample1 = img.Sample(sampleType, uv + texelSize * float2(0, -1)).xyz;
    //float3 sample2 = img.Sample(sampleType, uv).xyz; // Ensure sample2 is properly defined
    //float3 sample3 = img.Sample(sampleType, uv + texelSize * float2(0, 1)).xyz;
    //float3 sample4 = img.Sample(sampleType, uv + texelSize * float2(0, 2)).xyz;

    //// Handle undefined flow values (assumes invalid areas are near white)
    //float threshold = 0.99;
    //if (length(sample2.rgb) > threshold) sample2 = float3(0,0,0); 

    //// Accumulate weighted values
    //smoothedTensor += sample0 * weights[0];
    //smoothedTensor += sample1 * weights[1];
    //smoothedTensor += sample2 * weights[2];
    //smoothedTensor += sample3 * weights[3];
    //smoothedTensor += sample4 * weights[4];

    //// Extract tensor components
    //float E = smoothedTensor.x;
    //float F = smoothedTensor.y;
    //float G = smoothedTensor.z;

    //// Compute the dominant eigenvector v2
    //float discriminant = (E - G) * (E - G) + 4.0 * F * F;
    //float lambda1 = 0.5 * (E + G + sqrt(max(discriminant, 0.0001))); 
    //float lambda2 = 0.5 * (E + G - sqrt(max(discriminant, 0.0001)));

    //// Avoid division by zero when normalizing
    //float2 v2 = normalize(float2(lambda1 - G, F));
    //if (isnan(v2.x) || isnan(v2.y)) v2 = float2(0, 0); 

    //// Output the flow direction (v2) and magnitude of λ1
    //return float4(v2, sqrt(max(lambda1, 0.0)), 1.0);

}
