#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPosNor {
    float3 _WorldPos1 : TEXCOORD0;
    float4 _ProjPos2 : SV_Position;
    float3 _Normal1 : TEXCOORD1;
    float4 _Color2 : COLOR0;
};

VertexOutPosNor main(in float3 _inputPos : POSITION, in float3 _inputNormal : NORMAL0, in float3 worldPos : COLOR0, in float3 worldScale : COLOR1)
{
    VertexOutPosNor _vout;

    _vout._WorldPos1 = mul(float4(
		_inputPos.x * worldScale.x, 
		_inputPos.y * worldScale.y,
		_inputPos.z * worldScale.z, 
		1.0f), _world).xyz + worldPos;
    _vout._ProjPos2 = mul(float4(_vout._WorldPos1, 1.0f), _viewProjection);
    _vout._Normal1 = mul(float4(_inputNormal, 1.0f), _worldInverseTranspose).xyz;
    _vout._Color2 = _materialColor;

    return _vout;
}
