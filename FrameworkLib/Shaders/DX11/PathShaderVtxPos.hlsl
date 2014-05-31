#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPos
{
	float4 Pos : SV_Position;
	float2 Pos2D : TEXCOORD0;
	float4 Color : COLOR;
};

VertexOutPos main( in VertexPos vin )
{
	VertexOutPos vout;
	vout.Pos = mul(mul(float4(vin.Pos.x,vin.Pos.y,vin.Pos.z,1.0f),_world),_viewProjection);
	vout.Pos2D = vin.Pos.xy;
	vout.Color = _materialColor;
	return vout;
}