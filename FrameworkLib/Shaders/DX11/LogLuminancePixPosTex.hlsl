#pragma pack_matrix(row_major)

#include "TextureShader.inl"

static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

float4 main(VertexOut vtx) : SV_TARGET
{
	float3 samp = tex.SampleLevel(texSampler, vtx.TexCoord, 0).rgb;

	float logLum = log(dot(samp, LUMINANCE_VECTOR) + 0.0001f);

	return float4(logLum, logLum, logLum, 1.0f);
}
