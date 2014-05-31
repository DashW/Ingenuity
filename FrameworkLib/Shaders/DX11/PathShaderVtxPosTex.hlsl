#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

struct VertexOut
{
	float4 Pos : SV_Position;
	float2 Pos2D : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

VertexOut main( in VertexPosTex vin )
{
	VertexOut vout;
	vout.Pos = mul(mul(float4(vin.Pos.x,vin.Pos.y,vin.Pos.z,1.0f),_world),_viewProjection);
	vout.Pos2D = vin.Pos.xy;
	vout.Tex = vin.TexCoord;
	return vout;
}