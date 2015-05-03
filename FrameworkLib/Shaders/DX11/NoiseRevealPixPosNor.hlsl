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

cbuffer C10 : register(b10)
{
	float3 gradientClear;
	float colorGradScale;
	float3 gradientFill;
	float noiseScale;
}

Texture2D<float4> randomTex : register(t10);
SamplerState randomSampler : register(s10);

// IQ'S SIMPLEX NOISE:

float snoise(float3 x)
{
	float3 p = floor(x);
	float3 f = frac(x);    
//#ifdef HIGH_QUALITY
	f = f*f*f*(f*(f*6.f - 15.f) + 10.f); //Quintic smoothing
//#else
	//f = f*f*(3.0 - 2.0*f); //Cubic smoothing
//#endif
	float2 uv = (p.xy + float2(37.0f, 17.0f)*p.z) + f.xy;
	float2 rg = randomTex.Sample(randomSampler, (uv + 0.5f) / 256.0f).yx;
	return /*-1.0f + 2.0f**/lerp(rg.x, rg.y, f.z);
}

// END

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

	float3 gradientDir = gradientFill - gradientClear;
	float gradientDistance = length(gradientDir);
	float gradientD = dot(normalize(-gradientDir), gradientClear);
	float signedDistance = dot(normalize(gradientDir), _vtx._WorldPos) + gradientD;
	signedDistance = clamp(-1.0f + signedDistance + snoise(_vtx._WorldPos * noiseScale), 0.0f, 1.0f);

	clip(signedDistance - 0.01f);

	output.color = float4(output.color.rgb + ((float3(0.5f, 0.5f, 1.0f) - output.color.rgb) * (1.0f - clamp(signedDistance * colorGradScale, 0.0f, 1.0f))), 1.0f);

    return output;
}
