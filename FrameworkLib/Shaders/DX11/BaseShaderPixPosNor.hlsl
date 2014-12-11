#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNor {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos1 : SV_Position;
    float3 _Normal : TEXCOORD1;
    float4 _Color1 : COLOR0;
};

struct PixelOut {
    float4 color : SV_Target;
};

PixelOut main(in VertexOutPosNor _vtx)
{
    PixelOut output;

	float3 normalizedNormal = normalize(_vtx._Normal);
	float3 cameraVector = normalize(_cameraPosition - _vtx._WorldPos);

	float4 inputColor = _vtx._Color1;
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

	output.color = float4(output.color.rgb, inputColor.a);

    return output;
}
