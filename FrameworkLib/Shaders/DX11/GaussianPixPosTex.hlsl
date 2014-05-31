#pragma pack_matrix(row_major)

#include "TextureShader.inl"

static const float PI = 3.14159265f;

cbuffer C10: register(b10)
{
	float rho;
	float xRange;
	float yRange;
	float mutiplier;
};

float4 main(VertexOut vtx) : SV_TARGET
{
	const float x = (vtx.TexCoord.x - 0.5f) * xRange;
	const float y = (vtx.TexCoord.y - 0.5f) * yRange;

	float g = 1.0f / sqrt(2.0f * PI * rho * rho);
	g *= exp(-(x * x + y * y) / (2 * rho * rho));

	return float4(g, g, g, 1.0f);
}
