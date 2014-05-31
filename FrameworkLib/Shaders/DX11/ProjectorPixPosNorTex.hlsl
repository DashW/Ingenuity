#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNorTex {
    float3 _WorldPos1 : TEXCOORD0;
    float4 _ProjPos2 : SV_Position;
    float3 _Normal1 : TEXCOORD1;
    float4 _Color2 : COLOR0;
    float2 _TexCoord : TEXCOORD2;
	float4 _ProjCoord : TEXCOORD3;
};

struct PixelOut {
    float4 _color4 : SV_Target;
};

Texture2D<float4> projTex : register(t3);
SamplerState _projSampler : register(s3);

float4 projection(float4 projCoord, float4 color)
{
	// Project the texture coords and scale/offset to [0, 1].
	projCoord.xy /= projCoord.w;
	projCoord.x =  0.5f*projCoord.x + 0.5f;
	projCoord.y = -0.5f*projCoord.y + 0.5f;

	// Sample tex w/ projective texture coords.
	float4 texColor = projTex.Sample(_projSampler, projCoord.xy);

	// Only project/light in spotlight cone.
	return color * float4(texColor.rgb,1.0f);
}

PixelOut main(in VertexOutPosNorTex _vtx)
{
    PixelOut _output;

    //float3 _lightDirection3 = normalize(_lightPosition - _vtx._WorldPos1);
    //float3 _normalizedNormal3 = normalize(_vtx._Normal1);
	//float3 _cameraVector0022 = normalize(_cameraPosition - _vtx._WorldPos1);

	_output._color4 = _vtx._Color2 * tex.Sample(_textureSampler, _vtx._TexCoord);
	//_output._color4 = diffuse(_lightDirection3, _normalizedNormal3, _output._color4);
	////_output._color4 = cubeReflect(_normalizedNormal3, _cameraVector0022, _output._color4);
	//_output._color4 = specular(_normalizedNormal3, _cameraVector0022, _lightDirection3, _output._color4);
	//_output._color4 = projection(_vtx._ProjCoord, _output._color4);
	//_output._color4 = spot(_lightDirection3, _output._color4);

    return _output;
} 
