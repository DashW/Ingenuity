#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPosTex {
	float4 _WorldPos : TEXCOORD0;
    float4 _ProjPos : SV_Position;
	float2 _TexCoord : TEXCOORD2;
};

VertexOutPosTex main(in float3 _pos : POSITION, in float2 _TexCoord : TEXCOORD0)
{
    VertexOutPosTex vout;
	vout._WorldPos = mul(float4(_pos, 1.0f), _world);
    vout._ProjPos = mul(vout._WorldPos, _viewProjection);
    vout._TexCoord = _TexCoord;
    return vout;
} 
