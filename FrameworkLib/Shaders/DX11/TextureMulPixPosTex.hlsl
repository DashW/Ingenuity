#pragma pack_matrix(row_major)

#include "TextureShader.inl"

cbuffer C1: register(b10)
{
	float4 multiplierColor;
};

float4 main(VertexOut vtx) : SV_TARGET
{
	return tex.Sample(texSampler, vtx.TexCoord) * multiplierColor;
}
