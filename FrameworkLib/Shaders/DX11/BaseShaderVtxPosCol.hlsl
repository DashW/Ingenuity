#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPosCol {
    float4 _ProjPos : SV_Position;
    float4 _Color : COLOR0;
};

VertexOutPosCol main(in float3 _pos : POSITION, in float4 _Color : COLOR0)
{
    VertexOutPosCol vout;
    vout._ProjPos = mul(mul(float4(_pos, 1.0f), _world), _viewProjection);
    vout._Color = _Color;
    return vout;
} 
