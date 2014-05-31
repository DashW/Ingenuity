#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNorTanTex {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos3 : SV_Position;
    float3 _Normal : TEXCOORD1;
    float4 _Tangent : TEXCOORD2;
    float4 _Color : COLOR0;
    float2 _NormalCoord1 : TEXCOORD3;
	float2 _NormalCoord2 : TEXCOORD4;
};

Texture2D<float4> secondNormal : register(t3);
SamplerState secondSampler : TEXUNIT3;

float4 main(in VertexOutPosNorTanTex _vtx) : SV_TARGET
{/*
    float3 _lightDirection3 = normalize(_lightPosition - _vtx._WorldPos2);
    float3 _normalizedNormal3 = normalize(_vtx._Normal2);
	_normalizedNormal3 = normalize(mapNormal(_vtx._NormalCoord1, _normalizedNormal3, _vtx._Tangent));
	_normalizedNormal3 = normalize(mapNormal(_vtx._NormalCoord2, _normalizedNormal3, _vtx._Tangent));
	float3 _cameraVector0022 = normalize(_cameraPosition - _vtx._WorldPos2);
	
	float4 _output = _vtx._Color3;
	_output = cubeReflect(_normalizedNormal3, _cameraVector0022, _output);
	_output = specular(_normalizedNormal3, _cameraVector0022, _lightDirection3, _output);
	_output = spot(_lightDirection3, _output);

    return _output;
*/
	//PixelOut output;

	float3 normalizedNormal = normalize(_vtx._Normal);
	float3 normal1 = normalize(mapNormal(_vtx._NormalCoord1, normalizedNormal, _vtx._Tangent));
	float3 normal2 = normalize(mapNormal(_vtx._NormalCoord2, normalizedNormal, _vtx._Tangent));
	normalizedNormal = (normal1 + normal2) * 0.5;

	float3 cameraVector = normalize(_cameraPosition - _vtx._WorldPos);

	float4 outputColor = 0.0f;
	float4 inputColor = _vtx._Color;
	inputColor = cubeReflect(normalizedNormal, cameraVector, inputColor);

	[loop]
	for(uint i = 0; i < _numLights; ++i)
	{
		float3 lightDirection = normalize(lightPositionSpecs[i].xyz - _vtx._WorldPos);
		float4 resultColor = inputColor;

		//resultColor = diffuse(lightDirection, lightColors[i], normalizedNormal, resultColor);
		resultColor = specular(lightDirection, lightColorAttens[i], lightPositionSpecs[i].w, normalizedNormal, cameraVector, resultColor);
		resultColor = spot(lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor);

		outputColor += resultColor;
	}

	return outputColor;
}
