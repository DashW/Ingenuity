#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNorTanTex {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos3 : SV_Position;
    float3 _Normal : TEXCOORD1;
    float4 _Tangent : TEXCOORD2;
    float4 _Color : COLOR0;
    float2 _TexCoord : TEXCOORD3;
};

struct PixelOut {
    float4 color : SV_Target;
};

PixelOut main(in VertexOutPosNorTanTex _vtx)
{
 //   PixelOut _output;

 //   float3 _lightDirection3 = normalize(_lightPosition - _vtx._WorldPos2);
 //   float3 _normalizedNormal3 = normalize(_vtx._Normal2);
	//_normalizedNormal3 = normalize(mapNormal(_vtx._TexCoord1, _normalizedNormal3, _vtx._Tangent));
	//float3 _cameraVector0022 = normalize(_cameraPosition - _vtx._WorldPos2);

	//_output._color4 = _vtx._Color3 * tex.Sample(_textureSampler, _vtx._TexCoord1);
	//_output._color4 = diffuse(_lightDirection3, _normalizedNormal3, _output._color4);
	//_output._color4 = cubeReflect(_normalizedNormal3, _cameraVector0022, _output._color4);
	//_output._color4 = specular(_normalizedNormal3, _cameraVector0022, _lightDirection3, _output._color4);
	//_output._color4 = spot(_lightDirection3, _output._color4);

 //   return _output;

	PixelOut output;

	float3 normalizedNormal = normalize(_vtx._Normal);
	normalizedNormal = normalize(mapNormal(_vtx._TexCoord, normalizedNormal, _vtx._Tangent));
	float3 cameraVector = normalize(_cameraPosition - _vtx._WorldPos);

	float4 inputColor = _vtx._Color * tex.Sample(_textureSampler, _vtx._TexCoord);
	inputColor = cubeReflect(normalizedNormal, cameraVector, inputColor);

	output.color = inputColor * _ambient;

	[loop]
	for(uint i = 0; i < _numLights; ++i)
	{
		float3 lightDirection = normalize(lightPositionSpecs[i].xyz - _vtx._WorldPos);
		float  lightDistance = distance(lightPositionSpecs[i].xyz, _vtx._WorldPos);
		float4 resultColor = inputColor;

		resultColor = diffuse(lightDirection, lightDistance, lightColorAttens[i], normalizedNormal, resultColor);
		resultColor = specular(lightDirection, lightColorAttens[i], lightPositionSpecs[i].w, normalizedNormal, cameraVector, resultColor);
		resultColor = spot(lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor);

		output.color += resultColor;
	}

	return output;
}
