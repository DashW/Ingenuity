#pragma pack_matrix(row_major)

#define VERTEXSHADER
#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNor {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos1 : SV_Position;
};

VertexOutPosNor main(in VertexPosNor input)
{
    VertexOutPosNor _vout;

    _vout._WorldPos = mul(float4(input.Pos, 1.0f), _world).xyz;
    _vout._ProjPos1 = mul(float4(_vout._WorldPos, 1.0f), _viewProjection);

    return _vout;
} 
