#pragma pack_matrix(row_major)

#include "TextureShader.inl"

cbuffer C10: register(b10)
{
	float middleGrey;
	float3 filler1;
};

Texture2D<float> adaptedTex : register(t1);
SamplerState adaptedSampler : register(s1);

// The per-color weighting to be used for luminance calculations in RGB order.
static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

// The per-color weighting to be used for blue shift under low light.
static const float3 BLUE_SHIFT_VECTOR = float3(1.05f, 0.97f, 1.27f);

static const float2 texSize = float2(texWidth, texHeight);

static const float adaptedLum = adaptedTex.Load(uint3(0, 0, 0));

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 vSample = tex.SampleLevel(texSampler, vtx.TexCoord, 0.0f);

	// For very low light conditions, the rods will dominate the perception
	// of light, and therefore color will be desaturated and shifted
	// towards blue.
	//if(g_bEnableBlueShift)
	//{
	// Define a linear blending from -1.5 to 2.6 (log scale) which
	// determines the lerp amount for blue shift
	float fBlueShiftCoefficient = 1.0f - (adaptedLum + 1.5) / 4.1;
	fBlueShiftCoefficient = saturate(fBlueShiftCoefficient);

	// Lerp between current color and blue, desaturated copy
	float3 vRodColor = dot(vSample.rgb, LUMINANCE_VECTOR) * BLUE_SHIFT_VECTOR;
	vSample.rgb = lerp(vSample.rgb, vRodColor, fBlueShiftCoefficient);
	//}

	// Map the high range of color values into a range appropriate for
	// display, taking into account the user's adaptation level, and selected
	// values for for middle gray and white cutoff.
	//if(g_bEnableToneMap)
	//{
	vSample *= middleGrey / (adaptedLum + 0.001f);
	vSample /= (1.0f + vSample);
	vSample.a = 1.0f;
	//}

	return vSample;
}
