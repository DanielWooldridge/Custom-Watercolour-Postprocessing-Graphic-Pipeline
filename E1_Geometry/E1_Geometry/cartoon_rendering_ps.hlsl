Texture2D dfImg : register(t0);
Texture2D cqImg : register(t1);
SamplerState sampleType : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float4 main(InputType input) : SV_TARGET
{

    
    //https://stackoverflow.com/questions/5308961/hlsl-combining-textures
    //https://www.reddit.com/r/shaders/comments/162tj6n/help_with_blending_two_textures/
    // fixed4 result = 1 - (1 - color_rt) * (1 - color_main); // Screen blending

    //float4 dogFlow = dfImg.Sample(sampleType, input.tex);
    //float4 colourQuant = cqImg.Sample(sampleType, input.tex);
    //float4 result = 1 - (1 - colourQuant) * (1 - dogFlow); // Screen blending

    float edge = dfImg.Sample(sampleType, input.tex).r;      // edge strength
    float3 ycbcr = cqImg.Sample(sampleType, input.tex).rgb;   // original quantized YCbCr

    // Boost or clamp the edge so it doesn’t kill all luminance
    edge = lerp(0.5, 1.0, edge);  // prevent too-dark shading

    float y = saturate(edge * ycbcr.r);
    y = max(y, 0.1);  // ensure Y doesn’t collapse

    return float4(y, ycbcr.g, ycbcr.b, 1.0);






    //float edgeThreshold = 0.5;  
    //float4 edges = smoothstep(edgeThreshold, 1.0, dogFlow);  // Sharpen edges
    //float4 result = colourQuant * edges; // Screen multiplication

    //float edgeThreshold = 0.5;  // Helps to sharpen the edges
    //float4 edges = smoothstep(edgeThreshold, 1.0, dogFlow);  // Converts DoG to binary-like edges

    //// Blending factor for overlaying the edge map
    //float blendFactor = 0.7;

    //// Overlay-style blending, ensuring that the edges influence but do not darken the color too much
    //float4 result = lerp(colourQuant, colourQuant * (1.0 - edges), blendFactor);

    //return saturate(result);

   // float2 uv = input.tex;

   //float edge = dfImg.Sample(sampleType, uv).r;
   // float4 colourQuant = cqImg.Sample(sampleType, uv);

   // // Soft edge ramp
   // edge = smoothstep(0.3, 0.8, edge); // adjust to control sharpness

   // // Instead of full black, blend to a soft gray
   // float4 edgeTarget = float4(0.3, 0.3, 0.3, 1.0);

   // // Blend between color and softened edge
   // float4 result = lerp(colourQuant, edgeTarget, edge * 0.5); // 0.5 = blending strength

   // return saturate(result);


  //float2 uv = input.tex;

  //  float4 cqColor = cqImg.Sample(sampleType, uv);
  //  float edge = dfImg.Sample(sampleType, uv).r;

  //  // --- Remove the normalization to keep natural colors ---
  //  // float maxChannel = max(cqColor.r, max(cqColor.g, cqColor.b));
  //  // cqColor.rgb /= max(maxChannel, 0.0001);

  //  // --- Edge blending (soft fade) ---
  //  float fade = 1.0 - smoothstep(0.2, 0.9, edge);

  //  // Optional: ease the fade curve for smoother transitions
  //  fade = pow(fade, 0.8);  // 0.8 = softer edges, try 1.0 if you want linear

  //  float4 result = cqColor * fade;

  //  return float4(result.rgb, 1.0);

}
