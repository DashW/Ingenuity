#include "ShaderDefines.sl"

BEGIN_PARAMS
DEFAULT_VTX_PARAMS
END_PARAMS(0)

BEGIN_PARAMS
DEFAULT_PIX_PARAMS
END_PARAMS(0)

DEFAULT_TEX_PARAMS

// The following structs are the fragment parameters

// A good point arises!
// This effect supports multiple vertex layouts and intermediate structs.
// DX9 is easy - unused structs will be ignored, either at compilation or at runtime
// WebGL is easy - vertex layouts will be chosen by the preprocessor
//                 BUT INTERMEDIATE PARAMETERS WILL BE DEFINED TWICE!!!
//                 Multiple files is not the answer, WebGL only uses one...
//                 Greatest common denominator??
// DX11 is hard - the one file needs to produce a different shader for each vertex layout :(
//                This means we need to run the compiler once for each vertex layout :(
//                And I don't want to create an additional metadata file :(

BEGIN_VARYING(PosCol)
DEFAULT_VARYING
END_VARYING

BEGIN_VARYING(PosNor)
	float3 WorldPos : TEXCOORD0; // AAAAARGH THIS IS NOT GOING TO WORK IT'S BEING DEFINED TWICE!!!
	float4 ProjPos : POSITION;
	float3 Normal : TEXCOORD1;
    float4 Color : COLOR;
END_VARYING

// varying vec3 vWorldPosition;
// varying vec3 vViewPosition;
// varying vec3 vNormal;
// varying vec3 vColor;

BEGIN_VARYING(PosNorTex)
	float3 WorldPos : TEXCOORD0;
	float4 ProjPos : POSITION;
	float3 Normal : TEXCOORD1;
    float4 Color : COLOR;
	float2 TexCoord : TEXCOORD2;
END_VARYING

// Greatest common denominator???

BEGIN_VARYING(PosNorTanTex)
	float3 WorldPos : TEXCOORD0;
	float4 ProjPos : POSITION;
	float3 Normal : TEXCOORD1;
	float4 Tangent : TEXCOORD2;
    float4 Color : COLOR;
	float2 TexCoord : TEXCOORD3;
END_VARYING

struct PixelOut 
{
  float4 color : COLOR;
};

// The following is the vertex code
// The following function parameters are the vertex parameters

VertexOutPosCol vtxPosCol( float3 pos : POSITION, float4 Color : COLOR )
{
	VertexOutPosCol vout;
	vout.ProjPos = mul(float4(pos, 1.0f), worldViewProjection);
    vout.Color = Color;
    
    return vout;
}

VertexOutPosNor vtxPosNor( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float2 inputTexCoord : TEXCOORD0 )
{
	VertexOutPosNor vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz;
	vout.ProjPos = mul(float4(inputPos, 1.0f), worldViewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
    vout.Color = materialColor;
    
    return vout;
}

VertexOutPosNorTex vtxPosNorTex( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float2 inputTexCoord : TEXCOORD0 )
{
	VertexOutPosNorTex vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz;
	vout.ProjPos = mul(float4(inputPos, 1.0f), worldViewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
    vout.Color = materialColor;
	vout.TexCoord = inputTexCoord;
    
    return vout;
}

VertexOutPosNorTanTex vtxPosNorTanTex( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float4 inputTangent : TANGENT, float2 inputTexCoord : TEXCOORD0 )
{
	VertexOutPosNorTanTex vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz;
	vout.ProjPos = mul(float4(inputPos, 1.0f), worldViewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
	vout.Tangent = float4(mul(inputTangent.xyz, (float3x3) world), inputTangent.w);
    vout.Color = materialColor;
	vout.TexCoord = inputTexCoord;

    return vout;
}

// The following is the fragment code

float4 diffuse(float3 normalizedNormal, float3 worldPos, float3 lightDirection, float lightDistance, float4 color)
{
	float4 diffuseMagnitude = max(dot(lightDirection, normalizedNormal),ambient);
	diffuseMagnitude *= exp(-attenuation*lightDistance);
	float4 diffuseColor = color * lightColor;
	return float4((diffuseMagnitude * diffuseColor).rgb, color.a);
}

float4 specular(float3 normalizedNormal, float3 worldPos, float3 lightDirection, float4 color)
{
	float3 cameraVector = normalize(cameraPosition - worldPos);
	float3 specularReflection = normalize(reflect(-lightDirection,normalizedNormal));
	float specularMagnitude = max(dot(cameraVector,specularReflection),0.0f);
	float specularPoweredMagnitude = pow(specularMagnitude,specularPower);
	return color + (specularPoweredMagnitude * lightColor);
}

float4 spot(float3 lightDirection, float4 color)
{
	float spotFactor = pow(max(dot(normalize(-spotDirection), lightDirection), 0.001f), spotPower);
	return float4(color.rgb * spotFactor,color.a);
}

float4 cubeReflect(float3 normalizedNormal, float3 worldPos, float4 color)
{
	float3 cameraVector = normalize(cameraPosition - worldPos);
	float3 cubeMapTheta = reflect(-cameraVector, normalizedNormal);
	float4 cubeMapColor = texCUBE(cubeMapSampler, cubeMapTheta);
	float cubeMapIntensity = (0.5f + (max(max(cubeMapColor.r, cubeMapColor.g), cubeMapColor.b) * 0.5f)) * cubeMapAlpha;
	return float4((color.rgb * (1.0f - cubeMapIntensity)) + (cubeMapColor.rgb * cubeMapIntensity), color.a);
}

float3 applyNormal(float2 texCoord, float3 normalizedNormal, float4 chiralTangent)
{
	float3 tangent = chiralTangent.xyz;
	float chirality = chiralTangent.w;
	float3 normalMapSample = tex2D(normalSampler, texCoord).rgb;

	// Uncompress each component from [0,1] to [-1,1].
	float3 mappedNormal = (2.1f * normalMapSample) - 1.0f;

	float3 n = normalizedNormal;
	float3 t = normalize(tangent - (dot(tangent, normalizedNormal) * normalizedNormal));
	float3 b = cross(n,t) * chirality;

	float3x3 tbn = float3x3(t,b,n);

	return mul(mappedNormal,tbn);
}

PixelOut pixPosCol(VertexOutPosCol vtx)
{
	PixelOut output;
	output.color = vtx.Color;
	return output;
}

PixelOut pixPosNor(VertexOutPosNor vtx)
{
	PixelOut output;
	float3 lightDirection = normalize(lightPosition - vtx.WorldPos);
	float lightDistance = distance(lightPosition,vtx.WorldPos);
	float3 normalizedNormal = normalize(vtx.Normal);
	output.color = vtx.Color;
	output.color = diffuse(normalizedNormal,vtx.WorldPos,lightDirection,lightDistance,output.color);
	output.color = cubeReflect(normalizedNormal,vtx.WorldPos,output.color);
	output.color = specular(normalizedNormal,vtx.WorldPos,lightDirection,output.color);
	output.color = spot(lightDirection,output.color);
	return output;
}

PixelOut pixPosNorTex(VertexOutPosNorTex vtx)
{
	PixelOut output;
	float3 lightDirection = normalize(lightPosition - vtx.WorldPos);
	float lightDistance = distance(lightPosition,vtx.WorldPos);
	float3 normalizedNormal = normalize(vtx.Normal);
	output.color = vtx.Color * tex2D( textureSampler, vtx.TexCoord );
	output.color = diffuse(normalizedNormal,vtx.WorldPos,lightDirection,lightDistance,output.color);
	output.color = cubeReflect(normalizedNormal,vtx.WorldPos,output.color);
	output.color = specular(normalizedNormal,vtx.WorldPos,lightDirection,output.color);
	output.color = spot(lightDirection,output.color);
	return output;
}

PixelOut pixPosNorTanTex(VertexOutPosNorTanTex vtx)
{
	PixelOut output;
	float3 lightDirection = normalize(lightPosition - vtx.WorldPos);
	float lightDistance = distance(lightPosition,vtx.WorldPos);
	float3 normalizedNormal = normalize(vtx.Normal);
	float3 bumpedNormal = applyNormal(vtx.TexCoord,normalizedNormal,vtx.Tangent);
	output.color = vtx.Color * tex2D( textureSampler, vtx.TexCoord );
	output.color = diffuse(bumpedNormal,vtx.WorldPos,lightDirection,lightDistance,output.color);
	output.color = cubeReflect(bumpedNormal,vtx.WorldPos,output.color);
	output.color = specular(bumpedNormal,vtx.WorldPos,lightDirection,output.color);
	output.color = spot(lightDirection,output.color);
	return output;
}

TECHNIQUE(PosCol)
TECHNIQUE(PosNor)
TECHNIQUE(PosNorTex)
TECHNIQUE(PosNorTanTex)
