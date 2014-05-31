#pragma pack_matrix(row_major)

#include "TextureShader.inl"

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 samp = 0.0f;
	float xStep = (1.0f / texWidth);
	float yStep = (1.0f / texHeight);
	vtx.TexCoord -= float2(2.0f * xStep, 2.0f * yStep);

	for(uint i = 0; i < 16; i++)
	{
		samp += tex.Sample(texSampler, float2(
			vtx.TexCoord.x + (float(i/4) * xStep),
			vtx.TexCoord.y + (float(i%4) * yStep)));
	}

	return samp / 16;
}
