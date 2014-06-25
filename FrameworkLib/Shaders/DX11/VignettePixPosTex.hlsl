#pragma pack_matrix(row_major)

#include "TextureShader.inl"

static const float pi = 3.14159;

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 pixColor = tex.Sample(texSampler, vtx.TexCoord) * sin(vtx.TexCoord.x * pi) * sin(vtx.TexCoord.y * pi);
	return float4(pixColor.rgb, 1.0f);
}
