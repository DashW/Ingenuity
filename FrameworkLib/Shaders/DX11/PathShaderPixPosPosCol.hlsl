#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPos
{
	float4 Pos : SV_Position;
	float2 Pos2D : TEXCOORD0;
	float4 Color : COLOR;
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

float4 main( VertexOutPos vtx ) : SV_TARGET
{
	float4 multipliedColor = float4(
		vtx.Color.r * vtx.Color.a,
		vtx.Color.g * vtx.Color.a,
		vtx.Color.b * vtx.Color.a,
		vtx.Color.a
	);
	
	static float    gradAngle    = atan2( y2-y1, x2-x1 );
	static float2x2 gradRotation = float2x2(cos(gradAngle), -sin(gradAngle), sin(gradAngle), cos(gradAngle));
	static float    gradScale    = length(float2(x2-x1,y2-y1));

	float2 translatedPoint = vtx.Pos2D - float2( x1, y1 );
	float2 rotatedPoint    = mul(translatedPoint, gradRotation);
	float2 scaledPoint     = rotatedPoint / gradScale;


	float4 gradColor       = gradientTex.Sample( gradientSampler, scaledPoint );

	return (multipliedColor * (1.0f-gradientAlpha)) + (gradColor * gradientAlpha);
}