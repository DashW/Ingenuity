#pragma pack_matrix(row_major)

#include "TextureShader.inl"

cbuffer C1: register(b10)
{
	float sampleWeight;
	float3 _filler;
};

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 vColor = 0.0f;

	float4 texColor = tex.Sample(texSampler, vtx.TexCoord);

	vColor += float4(texColor.rgb, sampleWeight);

	return vColor;
}
