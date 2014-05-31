#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPosNorTex {
    float3 _WorldPos1 : TEXCOORD0;
    float4 _ProjPos2 : SV_Position;
    float3 _Normal1 : TEXCOORD1;
    float4 _Color2 : COLOR0;
    float2 _TexCoord : TEXCOORD2;
};

VertexOutPosNorTex main(in float3 _inputPos : POSITION, in float3 _inputNormal : NORMAL0, in float2 _inputTexCoord : TEXCOORD0)
{
    VertexOutPosNorTex _vout;

    _vout._WorldPos1 = mul(float4(_inputPos, 1.0f), _world).xyz;
    _vout._ProjPos2 = mul(float4(_vout._WorldPos1, 1.0f), _viewProjection);
    _vout._Normal1 = mul(float4(_inputNormal, 1.0f), _worldInverseTranspose).xyz;
    _vout._Color2 = _materialColor;
    _vout._TexCoord = _inputTexCoord;

    return _vout;
}
