#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNor {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos : SV_Position;
    float3 _Normal : TEXCOORD1;
	float4 _Color : COLOR0;
	float4 _ShadowPos : TEXCOORD2;
};

struct PixelOut {
    float4 color : SV_Target;
};

cbuffer C10: register(b10)
{
	// TODO: Investigate Slope Scaled Depth Bias (Rasterizer State)
	uniform float shadowBias;
	uniform float3 padding;
};

Texture2D<float4> shadowMap : register(t3);
SamplerComparisonState shadowSampler : register(s3);

//---------------------------------------------------------------------------------------
// Performs shadowmap test to determine if a pixel is in shadow.
//---------------------------------------------------------------------------------------

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow,
	Texture2D shadowMap,
	float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z - shadowBias;

	// Texel size.
	const float dx = SMAP_DX;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
	};

	[unroll]
	for(int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit /= 9.0f;
}

PixelOut main(in VertexOutPosNor _vtx)
{
    PixelOut output;

	float3 normalizedNormal = normalize(_vtx._Normal);
	float3 cameraVector = normalize(_cameraPosition - _vtx._WorldPos);

	float4 inputColor = _vtx._Color;
	inputColor = cubeReflect(normalizedNormal, cameraVector, inputColor);

	output.color = inputColor * _ambient;

	[loop]
	for(uint i = 0; i < _numLights; ++i)
	{
		float shadowFactor = (i == 0 ? CalcShadowFactor(shadowSampler, shadowMap, _vtx._ShadowPos) : 1.0f);
		if(shadowFactor < 0.001f) break;
		float3 lightDirection = normalize(lightPositionSpecs[i].xyz - _vtx._WorldPos);
		float  lightDistance = distance(lightPositionSpecs[i].xyz, _vtx._WorldPos);
		float4 resultColor = inputColor;

		resultColor = diffuse(lightDirection, lightDistance, lightColorAttens[i], normalizedNormal, resultColor);
		resultColor = specular(lightDirection, lightColorAttens[i], lightPositionSpecs[i].w, normalizedNormal, cameraVector, resultColor);
		resultColor = spot(lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor);

		output.color += (resultColor * shadowFactor);
	}

	output.color = float4(output.color.rgb, inputColor.a);

    return output;
}
