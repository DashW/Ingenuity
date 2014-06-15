#pragma pack_matrix(row_major)

#include "TextureShader.inl"

static const float2 offsets[16] = {
	float2(-1.5f, -1.5f),
	float2(-0.5f, -1.5f),
	float2( 0.5f, -1.5f),
	float2( 1.5f, -1.5f),
	float2(-1.5f, -0.5f),
	float2(-0.5f, -0.5f),
	float2( 0.5f, -0.5f),
	float2( 1.5f, -0.5f),
	float2(-1.5f,  0.5f),
	float2(-0.5f,  0.5f),
	float2( 0.5f,  0.5f),
	float2( 1.5f,  0.5f),
	float2(-1.5f,  1.5f),
	float2(-0.5f,  1.5f),
	float2( 0.5f,  1.5f),
	float2( 1.5f,  1.5f)
};

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 samp = 0.0f;
	float2 step = float2(1.0f / texWidth, 1.0f / texHeight);
	//vtx.TexCoord -= float2(2.0f * xStep, 2.0f * yStep);

	//int3 coord = int3(int(vtx.TexCoord.x * texWidth) - 2, int(vtx.TexCoord.x * texHeight) - 2, 0);

	for(uint i = 0; i < 16; i++) 
	{
		samp += tex.Sample(texSampler, vtx.TexCoord + (offsets[i] * step));
		//samp += tex.Load(coord + int3(i % 4, i / 4, 0));
	}

	return samp / 16;
}
