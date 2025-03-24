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
    // Compute the dominant eigenvector v2
  
    //float disc = sqrt((E - G) * (E - G) + 4.0 * F * F);

    //float lambda1 = 0.5 * (E + G + sqrt(disc));
    //float lambda2 = 0.5 * (E + G - sqrt(disc));

    //	float lambda1 = 0.5 * (g.y + g.x +\n"
    //"			  sqrt(g.y*g.y - 2.0*g.x*g.y + g.x*g.x + 4.0*g.z*g.z));\n"
    //"	vec2 v = vec2(g.x - lambda1, g.z);\n"

    // g.y = F,  g.x = E, g.z = G

    float lambda1 = 0.5 * (F + E + sqrt(F * F - 2.0 * E * F + E * E + 4.0 * G * G));
    float2 v = float2(E - lambda1, G);

    return float4(normalize(v), sqrt(lambda1), 1.0f);


    //float lambda1 = 0.5 * (E + G + sqrt((E - G) * (E - G) + 4.0 * F * F));
    //float lambda2 = 0.5 * (E + G - sqrt((E - G) * (E - G) + 4.0 * F * F));

    //float2 v2 = normalize(float2(lambda1 - G, F)); // Dominant eigenvector
    //float2 v1 = normalize(lambda2 - G)

    //float2 v1 = normalize(float2(F, lambda1 - E)); // Gradient direction
    //float2 v2 = normalize(float2(F, lambda2 - E)); // Tangent direction


    //return float4(v1.xy, v2.xy);

     //Output the flow direction (v2) and magnitude of λ1
    //return float4(v2, sqrt(lambda1), 1.0);

    // This could also be the issue


}
