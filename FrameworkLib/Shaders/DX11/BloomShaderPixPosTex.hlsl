#pragma pack_matrix(row_major)

#include "TextureShader.inl"

static const int MAX_SAMPLES = 16;    // Maximum texture grabs

cbuffer C1: register(b10)
{
	float4 g_avSampleOffsets[MAX_SAMPLES];
	float4 g_avSampleWeights[MAX_SAMPLES];
};

cbuffer C2: register(b11)
{
	float numSamples;
	float3 filler2;
}

//static float2 g_avSampleOffsets[MAX_SAMPLES] = (float2[MAX_SAMPLES]) g_packedAvSampleOffsets;

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 vSample = 0.0f;
	float4 vColor = 0.0f;

	float2 vSamplePosition;

	const int numSamplesInt = int(numSamples);

	// Sample from eight points along the bloom line
	for(int iSample = 0; iSample < numSamplesInt; iSample++)
	{
		vSamplePosition = vtx.TexCoord + g_avSampleOffsets[iSample].xy;
		vSample = tex.Sample(texSampler, vSamplePosition);
		vColor += g_avSampleWeights[iSample] * vSample;
	}

	return float4(vColor.rgb, 1.0f);
}
