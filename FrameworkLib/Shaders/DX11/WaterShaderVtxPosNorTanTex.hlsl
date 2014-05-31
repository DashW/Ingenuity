#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPosNorTanTex {
    float3 _WorldPos2 : TEXCOORD0;
    float4 _ProjPos3 : SV_Position;
    float3 _Normal2 : TEXCOORD1;
    float4 _Tangent : TEXCOORD2;
    float4 _Color3 : COLOR0;
    float2 _NormalCoord1 : TEXCOORD3;
	float2 _NormalCoord2 : TEXCOORD4;
};

cbuffer C1: register(b10)
{
    float2 offset1;
    float2 offset2;
}

VertexOutPosNorTanTex main(in float3 _inputPos : POSITION, in float3 _inputNormal : NORMAL0, in float4 _inputTangent : TANGENT, in float2 _inputTexCoord : TEXCOORD0)
{
	VertexOutPosNorTanTex _vout;

    _vout._WorldPos2 = mul(float4(_inputPos, 1.0f), _world).xyz;
    _vout._ProjPos3 = mul(float4(_vout._WorldPos2, 1.0f), _viewProjection);
    _vout._Normal2 = mul(float4(_inputNormal, 1.0f), _worldInverseTranspose).xyz;
	_vout._Tangent = float4(mul(float4(_inputTangent.xyz, 1.0f), _world).xyz, _inputTangent.w);
    _vout._Color3 = _materialColor;
	_vout._NormalCoord1 = _inputTexCoord + offset1;
	_vout._NormalCoord2 = _inputTexCoord + offset2;

	return _vout;
}