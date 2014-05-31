#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOutPos
{
	float4 Pos : SV_Position;
	float2 Pos2D : TEXCOORD0;
	float4 Color : COLOR;
};

VertexOutPos main( in VertexPos vin, in InstancePosCol instance )
{
	VertexOutPos vout;
	vout.Pos = mul(float4(
		 vin.Pos.x + instance.Pos.x,
		 -vin.Pos.y - instance.Pos.y,
		 vin.Pos.z + instance.Pos.z,
		1.0f),_viewProjection);
	vout.Pos2D = vin.Pos.xy;
	vout.Color = instance.Color;
	return vout;
}