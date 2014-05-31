#pragma pack_matrix(row_major)

#include "ModelShader.inl"


struct VertexOutPosNorTex {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos : SV_Position;
    float3 _Normal : TEXCOORD1;
    float4 _Color : COLOR0;
    float2 _BlendCoord : TEXCOORD2;
    float2 _TexCoord : TEXCOORD3;
};

struct PixelOut {
    float4 color : SV_Target;
};

Texture2D<float4> _TMP20 : register(t3);
Texture2D<float4> _TMP21 : register(t4);
Texture2D<float4> _TMP22 : register(t5);
SamplerState _texSampler1 : TEXUNIT3;
SamplerState _texSampler2 : TEXUNIT4;
SamplerState _texSampler3 : TEXUNIT5;

PixelOut main(in VertexOutPosNorTex _vtx)
{
    PixelOut output;

	float3 normalizedNormal = normalize(_vtx._Normal);

	float4 _c0 = _TMP20.Sample(_texSampler1, _vtx._TexCoord);
	float4 _c1 = _TMP21.Sample(_texSampler2, _vtx._TexCoord);
	float4 _c2 = _TMP22.Sample(_texSampler3, _vtx._TexCoord);

	float4 blendColor = tex.Sample(_textureSampler, _vtx._BlendCoord);
	float _totalInverse =  1.00000000000000000E000f/(blendColor.x + blendColor.y + blendColor.z);
	_c0 = _c0*(blendColor.x*_totalInverse);
	_c1 = _c1*(blendColor.y*_totalInverse);
	_c2 = _c2*(blendColor.z*_totalInverse);

	float4 inputColor = _vtx._Color * (_c0 + _c1 + _c2);
	//inputColor = cubeReflect(normalizedNormal, cameraVector, inputColor);

	output.color = float4(inputColor.xyz * _ambient, inputColor.w);

	[loop]
	for(uint i = 0; i < _numLights; ++i)
	{
		float3 lightDirection = normalize(lightPositionSpecs[0].xyz - _vtx._WorldPos);
		float  lightDistance = distance(lightPositionSpecs[i].xyz, _vtx._WorldPos);
		float4 resultColor = inputColor;

		resultColor = diffuse(lightDirection, lightDistance, lightColorAttens[i], normalizedNormal, resultColor);
		//resultColor = specular(lightDirection, lightColors[i], lightPositionSpecs[i].w, normalizedNormal, cameraVector, resultColor);
		//resultColor = spot(lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor);

		output.color += resultColor;
	}

    return output;
}
