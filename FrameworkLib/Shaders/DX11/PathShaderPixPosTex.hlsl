#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOut
{
	float4 Pos : SV_Position;
	float2 Pos2D : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

cbuffer C1: register(b10)
{
	float gradientAlpha;
    float x1;
	float y1;
	float x2;
	float y2;
	float3 filler;
};

Texture2D gradientTex : register(t3);
SamplerState gradientSampler : TEXUNIT3;

float4 main( VertexOut vtx ) : SV_TARGET
{
	return gradientTex.Sample(gradientSampler, vtx.Tex);
}