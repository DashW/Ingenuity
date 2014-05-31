#define VERTEXSHADER
#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNor {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos1 : SV_Position;
    float3 _Normal : TEXCOORD1;
    float4 _Color1 : COLOR0;
	float4 _ViewNor : TEXCOORD2;
};

VertexOutPosNor main(in VertexPosNorTex input)
{
    VertexOutPosNor _vout;

    _vout._WorldPos = mul(float4(input.Pos, 1.0f), _world).xyz;
    _vout._ProjPos1 = mul(float4(_vout._WorldPos, 1.0f), _viewProjection);
    _vout._Normal = mul(float4(input.Normal, 1.0f), _worldInverseTranspose).xyz;
    _vout._Color1 = _materialColor;
	_vout._ViewNor = float4(mul(_vout._Normal, (float3x3) _viewProjection), 1.0f);

    return _vout;
} 