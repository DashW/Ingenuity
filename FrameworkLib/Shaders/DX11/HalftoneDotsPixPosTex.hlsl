#pragma pack_matrix(row_major)

#include "TextureShader.inl"

cbuffer C1: register(b10)
{
    float dotsSize;
	float3 filler1;
};

Texture3D dotsTex : register(t1);
SamplerState dotsSampler : TEXUNIT1;

float4 main(VertexOut vtx) : SV_TARGET
{
	float4 sourceColor = tex.Sample(texSampler, vtx.TexCoord);
    float luminance = dot(float3(.2,.7,.1),sourceColor.xyz);
	float aspect = texWidth / texHeight;
    float3 lx = float3((vtx.TexCoord.x * aspect) / dotsSize, vtx.TexCoord.y / dotsSize, luminance * 0.7f);
    float4 dotColor = dotsTex.Sample(dotsSampler,lx);
    return float4(dotColor.xyz,1.0f);
}
