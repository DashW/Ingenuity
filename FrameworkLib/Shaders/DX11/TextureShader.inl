
struct VertexOut
{
	float4 Position : SV_Position; 
	float2 TexCoord : TEXCOORD0;
}; 

#ifndef VERTEXSHADER
cbuffer C0: register(b0)
{
    float texWidth;
	float texHeight;
	float2 filler;
};

Texture2D<float4> tex : register(t0);
SamplerState texSampler : register(s0);
#endif