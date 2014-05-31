#pragma pack_matrix(row_major)

#include "TextureShader.inl"

cbuffer C10: register(b10)
{
	float deltaTime;
	float adaptionSpeed;
	float2 filler1;
};

Texture2D<float4> adaptedTex : register(t1);
SamplerState adaptedSampler : register(s1);

float4 main(VertexOut vtx) : SV_TARGET
{
	float currentLum = exp(tex.Sample(texSampler, float2(0.5f, 0.5f)).r);
	float adaptedLum = adaptedTex.Sample(adaptedSampler, float2(0.5f, 0.5f)).r;

	// The user's adapted luminance level is simulated by closing the gap between
	// adapted luminance and current luminance.
	float newAdaptation = adaptedLum + (currentLum - adaptedLum) * (1.0f - pow(abs(1.0f - adaptionSpeed), 100.0f * deltaTime));

	return float4(newAdaptation, newAdaptation, newAdaptation, 1.0f);
}
