#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNorTex {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos : SV_Position;
    float3 _Normal : TEXCOORD1;
    float4 _Color : COLOR0;
    float2 _TexCoord : TEXCOORD2;
	float4 _ProjCoord : TEXCOORD3;
};

struct PixelOut {
    float4 color : SV_Target;
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
	PixelOut output;
	output.color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 lightDirection = normalize(_lightPosition - _vtx._WorldPos);
	float lightDistance = length(_lightPosition - _vtx._WorldPos);
    float3 normalizedNormal = normalize(_vtx._Normal);
	float3 cameraVector = normalize(_cameraPosition - _vtx._WorldPos);
	float4 inputColor = _vtx._Color * tex.Sample(_textureSampler, _vtx._TexCoord);

	[loop]
	for(uint i = 0; i < _numLights; ++i)
	{
		float3 lightDirection = normalize(lightPositionSpecs[i].xyz - _vtx._WorldPos);
		float  lightDistance = distance(lightPositionSpecs[i].xyz, _vtx._WorldPos);
		float4 resultColor = float4(1.0f, 1.0f, 1.0f, 0.0f);

		resultColor = diffuse(lightDirection, lightDistance, lightColorAttens[i], normalizedNormal, resultColor);
		resultColor = specular(lightDirection, lightColorAttens[i], _specularPower, normalizedNormal, cameraVector, resultColor, lightDistance);
		if(i == 0)
		{
			resultColor = projection(_vtx._ProjCoord, resultColor);
		}
		resultColor = spot(lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor);

		output.color += (inputColor * float4(resultColor.rgb, 0.0f));
	}

    return output;
} 
