#pragma pack_matrix(row_major)

#include "TextureShader.inl"

cbuffer C1: register(b10)
{
	float4 multiplierColor;
};

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 color = tex.SampleLevel(texSampler, vtx.TexCoord, 0) * multiplierColor;
	clip(max(color.r, max(color.g, color.b)) - 0.001f);
	return color;// ;
}
