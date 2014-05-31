#pragma pack_matrix(row_major)

#include "TextureShader.inl"

float4 main(VertexOut vtx) : SV_TARGET
{
	return tex.Sample(texSampler, vtx.TexCoord);
}
