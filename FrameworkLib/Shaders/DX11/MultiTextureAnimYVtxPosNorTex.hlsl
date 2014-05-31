#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPosNorTex {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos : SV_Position;
    float3 _Normal : TEXCOORD1;
    float4 _Color : COLOR0;
    float2 _BlendCoord : TEXCOORD2;
    float2 _TexCoord : TEXCOORD3;
};

cbuffer C1: register(b10)
{
    float _yStart;
    float _progress;
    float2 _filler;
}

VertexOutPosNorTex main(in float3 _inputPos : POSITION, in float3 _inputNormal : NORMAL0, in float2 _inputTexCoord : TEXCOORD0)
{
    VertexOutPosNorTex _vout;

    _inputPos.y = _inputPos.y*_progress + _yStart*(1.0f - _progress);
    _inputNormal = _inputNormal*_progress + float3(0.0f, 1.0f, 0.0f)*(1.0f - _progress);

    _vout._WorldPos = mul(float4(_inputPos, 1.0f), _world).xyz;
    _vout._ProjPos = mul(float4(_vout._WorldPos, 1.0f), _viewProjection);
    _vout._Normal = mul(float4(_inputNormal, 0.0f), _worldInverseTranspose).xyz;

    _vout._Color = _materialColor;
    _vout._BlendCoord = _inputTexCoord;
    _vout._TexCoord = _inputTexCoord* 16.0f;

    return _vout;
}
