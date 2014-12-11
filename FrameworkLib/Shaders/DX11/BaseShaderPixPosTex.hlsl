#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosTex {
	float4 _WorldPos : TEXCOORD0;
	float4 _ProjPos : SV_Position;
	float2 _TexCoord : TEXCOORD2;
};

float4 main(VertexOutPosTex vtx) : SV_TARGET
{
	return tex.Sample(_textureSampler, vtx._TexCoord);
}
