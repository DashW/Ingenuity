#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPos
{
	float4 _WorldPos : TEXCOORD0;
    float4 _ProjPos : SV_Position;
};

VertexOutPos main(in float3 _pos : POSITION)
{
    VertexOutPos vout;
	vout._WorldPos = mul(float4(_pos, 1.0f), _world);
    vout._ProjPos = mul(vout._WorldPos, _viewProjection);
    return vout;
} 
