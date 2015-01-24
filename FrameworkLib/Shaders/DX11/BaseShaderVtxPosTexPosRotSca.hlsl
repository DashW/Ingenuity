#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "ModelShader.inl"

static const float epsilon = 0.00000001f;

struct VertexOutPosTex {
	float4 _WorldPos : TEXCOORD0;
	float4 _ProjPos : SV_Position;
	float2 _TexCoord : TEXCOORD2;
};

void rotate(inout float x, inout float y, float angle)
{
	if(abs(x+y) < epsilon) return;
	float length = sqrt(x*x + y*y);
	float newAngle = atan2(y,x) + angle;
	x = length * cos(newAngle);
	y = length * sin(newAngle);
}

VertexOutPosTex main(
	in float3 _pos : POSITION,
	in float2 _TexCoord : TEXCOORD0,
	in float3 instPos : COLOR0,
	in float3 instRot : COLOR1,
	in float3 instScale : COLOR2)
{
	VertexOutPosTex vout;
	float3 vtxPos = _pos * instScale;
	rotate(vtxPos.x, vtxPos.z, instRot.y);
	rotate(vtxPos.y, vtxPos.z, instRot.x);
	rotate(vtxPos.x, vtxPos.y, instRot.z);
	vout._WorldPos = mul(float4(vtxPos, 1.0f), _world) + float4(instPos,0.0f);
	vout._ProjPos = mul(vout._WorldPos, _viewProjection);
	vout._TexCoord = _TexCoord;
	return vout;
}
