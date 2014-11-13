#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNorTex {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos2 : SV_Position;
    float3 _Normal : TEXCOORD1;
    float4 _Color2 : COLOR0;
    float2 _TexCoord : TEXCOORD2;
};

struct PixelOut {
    float4 color : SV_Target;
};

PixelOut main(in VertexOutPosNorTex _vtx)
{
    PixelOut output;

	float3 normalizedNormal = normalize(_vtx._Normal);
	float3 cameraVector = normalize(_cameraPosition - _vtx._WorldPos);
	float4 inputColor = _vtx._Color2 * tex.Sample(_textureSampler, _vtx._TexCoord);

	output.color = float4(inputColor.rgb * _ambient, inputColor.a);
	  
	[loop]
	for(uint i = 0; i < _numLights; ++i)
	{
		float3 lightDirection = normalize(lightPositionSpecs[i].xyz - _vtx._WorldPos);
		float  lightDistance = distance(lightPositionSpecs[i].xyz, _vtx._WorldPos);
		float4 resultColor = float4(1.0f,1.0f,1.0f,0.0f);

		resultColor = diffuse(lightDirection, lightDistance, lightColorAttens[i], normalizedNormal, resultColor);
		resultColor = specular(lightDirection, lightColorAttens[i], _specularPower, normalizedNormal, cameraVector, resultColor, lightDistance);
		resultColor = spot(lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor);

		output.color += (inputColor * float4(resultColor.rgb, 0.0f));
	}

	// This needs to be (1-sourcebrightness) * (cubemapbrightness)
	// so the lightest parts of the cubemap appear in the darkest parts of the model

	//output.color = cubeReflect(normalizedNormal, cameraVector, output.color);

    return output;
} 


// Pre-multiple-lights lighting code:

//float3 lightDirection = normalize(lightPositionSpecs[i].xyz - _vtx._WorldPos);
//float4 resultColor = inputColor;

//resultColor = diffuse(lightDirection, lightColors[i], normalizedNormal, resultColor);
//resultColor = specular(lightDirection, lightColors[i], lightPositionSpecs[i].w, normalizedNormal, cameraVector, resultColor);
//resultColor = spot(lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor);

//output.color += resultColor; 

// Calculate illumination variables
//float3 vLightToPoint = normalize(vViewPosition - g_avLightPositionView[iLight]);
//	float3 vReflection = reflect(vLightToPoint, vNormal);
//	float  fPhongValue = saturate(dot(vReflection, vPointToCamera));

// Calculate diffuse term
//float  fDiffuse = g_fDiffuseCoefficient * saturate(dot(vNormal, -vLightToPoint));

// Calculate specular term
//float  fSpecular = g_fPhongCoefficient * pow(fPhongValue, g_fPhongExponent);

// Scale according to distance from the light
//float fDistance = distance(g_avLightPositionView[iLight], vViewPosition);
//vIntensity += (fDiffuse + fSpecular) * g_avLightIntensity[iLight] / (fDistance*fDistance);