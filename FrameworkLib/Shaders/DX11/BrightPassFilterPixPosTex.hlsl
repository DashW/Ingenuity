#pragma pack_matrix(row_major)

#include "TextureShader.inl"

//static const float  BRIGHT_PASS_THRESHOLD = 5.0f;  // Threshold for BrightPass filter
//static const float  BRIGHT_PASS_OFFSET = 10.0f; // Offset for BrightPass filter

cbuffer C10: register(b10)
{
	float threshold;
	float offset;
	float2 filler1;
};

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 texSample = tex.SampleLevel(texSampler, vtx.TexCoord, 0);

	texSample.rgb /= (1.0f - texSample.rgb);

	// Subtract out dark pixels
	texSample.rgb -= threshold;

	// Clamp to 0
	texSample = max(texSample, 0.0f);

	// Map the resulting value into the 0 to 1 range. Higher values for
	// BRIGHT_PASS_OFFSET will isolate lights from illuminated scene 
	// objects.
	texSample.rgb /= (offset + texSample.rgb);

	return texSample;
}
