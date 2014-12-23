#pragma pack_matrix(row_major)

#define VERTEXSHADER
#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNor {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos1 : SV_Position;
    float3 _Normal : TEXCOORD1;
	float4 _Color1 : COLOR0;
	float4 _ShadowPos : TEXCOORD2;
};

cbuffer C10: register(b10)
{
	uniform float4x4 shadowMatrix;
};

VertexOutPosNor main(in VertexPosNor input)
{
    VertexOutPosNor _vout;

    _vout._WorldPos = mul(float4(input.Pos, 1.0f), _world).xyz;
    _vout._ProjPos1 = mul(float4(_vout._WorldPos, 1.0f), _viewProjection);
    _vout._Normal = mul(float4(input.Normal, 1.0f), _worldInverseTranspose).xyz;
    _vout._Color1 = _materialColor;
	_vout._ShadowPos = mul(float4(_vout._WorldPos, 1.0f), shadowMatrix);

    return _vout;
} 
