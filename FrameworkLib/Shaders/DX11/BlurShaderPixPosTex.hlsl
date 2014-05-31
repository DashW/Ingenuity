#pragma pack_matrix(row_major)

#include "TextureShader.inl"

static const float2 sampleOffsets[13] =
{
	float2(0.0f, -2.0f),
	float2(-1.0f, -1.0f),
	float2(0.0f, -1.0f),
	float2(1.0f, -1.0f),
	float2(-2.0f, 0.0f),
	float2(-1.0f, 0.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(2.0f, 0.0f),
	float2(-1.0f, 1.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 1.0f),
	float2(0.0f, 2.0f)
};

static const float gaussianWeights[13] =
{
	0.0248824656f,
	0.0676375553f,
	0.111515477f,
	0.0676375553f,
	0.0248824656f,
	0.111515477f,
	0.183857948f,
	0.111515477f,
	0.0248824656f,
	0.0676375553f,
	0.111515477f,
	0.0676375553f,
	0.0248824656f,
};

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 vSample = 0.0f;
	float2 texUnit = float2(1.0f / texWidth, 1.0f / texHeight);

	float2 vSamplePosition;

	for(int i = 0; i < 13; i++)
	{
		vSamplePosition = vtx.TexCoord + (sampleOffsets[i] * texUnit);
		vSample += gaussianWeights[i] * tex.Sample(texSampler, vSamplePosition);
	}

	return vSample;
}
