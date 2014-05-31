#line 9 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\WaterShader.cgfx"

uniform extern float4x4 world;
uniform extern float4x4 worldInverseTranspose;
uniform extern float4x4 viewProjection;
uniform extern float4 materialColor;
 
#line 29 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\WaterShader.cgfx"

uniform extern float3 lightPosition;
uniform extern float specularPower;
uniform extern float4 lightColor;
uniform extern float3 cameraPosition;
uniform extern float ambient;
uniform extern float attenuation;
uniform extern float3 spotDirection;
uniform extern float spotPower;
uniform extern float cubeMapAlpha;

uniform extern texture2D tex;
uniform extern texture2D normalMap;
uniform extern texture cubeMap;

uniform extern texture2D param0; 
uniform extern float param1; 
uniform extern float param2; 
uniform extern float param3; 
uniform extern float param4; 



struct VertexOutPosNorTanTex
{
	float3 WorldPos : TEXCOORD0;
	float4 ProjPos  : POSITION;
	float3 Normal : TEXCOORD1;
	float4 Tangent : TEXCOORD2;
    float4 Color : COLOR;
	float2 NormalCoord1 : TEXCOORD3;
	float2 NormalCoord2 : TEXCOORD4;
};

struct PixelOut 
{
  float4 color : COLOR;
};

VertexOutPosNorTanTex vtxPosNorTanTex( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float4 inputTangent : TANGENT, float2 inputTexCoord : TEXCOORD0 )
{
	VertexOutPosNorTanTex vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz;
	vout.ProjPos = mul(float4(vout.WorldPos, 1.0f), viewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
	vout.Tangent = float4(mul(inputTangent.xyz, (float3x3) world), inputTangent.w);
    vout.Color = materialColor;
	vout.NormalCoord1 = inputTexCoord + float2(param1,param2);
	vout.NormalCoord2 = inputTexCoord + float2(param3,param4);

    return vout;
}

sampler2D textureSampler : register(s0) = sampler_state
{
	Texture = <tex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = Wrap;
	AddressV = Wrap;
};

samplerCUBE cubeMapSampler : register(s1) = sampler_state
{
	Texture = <cubeMap>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU  = WRAP;
    AddressV  = WRAP;
};

sampler2D normalSampler : register(s2) = sampler_state
{
	Texture = <normalMap>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = Wrap;
	AddressV = Wrap;
};

sampler2D normalSampler2 : register(s3) = sampler_state
{
	Texture = <param0>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 diffuse(float3 normalizedNormal, float3 worldPos, float3 lightDirection, float4 color)
{
	float4 diffuseMagnitude = max(dot(lightDirection, normalizedNormal),ambient);
	float4 diffuseColor = color * lightColor;
	return float4((diffuseMagnitude * diffuseColor).rgb, 1.0f);
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
	float cubeMapIntensity = 1.0f; 
	return float4((color.rgb * (1.0f - cubeMapIntensity)) + (cubeMapColor.rgb * cubeMapIntensity), color.a);
}

float3 bump(float2 normalCoord1, float2 normalCoord2, float3 normalizedNormal, float4 chiralTangent, float bumpMagnitude)
{
	float3 tangent = chiralTangent.xyz;
	float chirality = chiralTangent.w;
	float3 normalMapSample1 = tex2D(normalSampler, normalCoord1).rgb;
	float3 normalMapSample2 = tex2D(normalSampler2, normalCoord2).rgb;
	float3 normalMapSample = (normalMapSample1 + normalMapSample2) * 0.5f;

	
	float3 mappedNormal = (2.1f * normalMapSample) - 1.0f; 

	float3 n = normalizedNormal;
	float3 t = normalize(tangent - (dot(tangent, normalizedNormal) * normalizedNormal));
	float3 b = cross(n,t) * chirality;

	float3x3 tbn = float3x3(t,b,n);

	float3 bumpedNormal = mul(mappedNormal,tbn);
	
	bumpedNormal = normalize(bumpedNormal);
	bumpedNormal = normalizedNormal + ((bumpedNormal - normalizedNormal) * bumpMagnitude);

	return bumpedNormal;
}
 
PixelOut pixPosNorTanTex(VertexOutPosNorTanTex vtx)
{
	PixelOut output;
	float3 lightDirection = normalize(lightPosition - vtx.WorldPos);
	float3 normalizedNormal = normalize(vtx.Normal);
	float3 bumpedNormal = bump(vtx.NormalCoord1,vtx.NormalCoord2,normalizedNormal,vtx.Tangent, 0.1f);
	float3 semiBumpedNormal = bump(vtx.NormalCoord1,vtx.NormalCoord2,normalizedNormal,vtx.Tangent, 0.1f);
	output.color = float4(0.0f,0.0f,0.0f,1.0f);
	
	output.color = cubeReflect(semiBumpedNormal,vtx.WorldPos,output.color);
	output.color = specular(bumpedNormal,vtx.WorldPos,lightDirection,output.color);
	output.color = spot(lightDirection,output.color);
	output.color.a = 0.8f;
	return output;
}



technique posNorTanTex
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosNorTanTex();
		PixelShader = compile ps_2_0 pixPosNorTanTex();
	}
}


