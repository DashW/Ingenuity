#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosTex {
	float4 _WorldPos : TEXCOORD0;
	float4 _ProjPos : SV_Position;
	float2 _TexCoord : TEXCOORD2;
};

static const float epsilon = 0.00001f;

float4 main(VertexOutPosTex vtx) : SV_TARGET
{
	float4 texColor = tex.Sample(_textureSampler, vtx._TexCoord);
	clip(texColor.a < epsilon ? -1 : 1);
	return float4(texColor.rgb, texColor.a);
}
