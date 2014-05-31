
struct VertexOutPosCol
{
	float4 ProjPos  : POSITION;
    float4 Color : COLOR;
};

struct VertexOutPosNor
{
	float3 WorldPos : TEXCOORD0;
	float4 ProjPos  : POSITION;
	float3 Normal : TEXCOORD1;
    float4 Color : COLOR;
};

struct VertexOutPosNorTex
{
	float3 WorldPos : TEXCOORD0;
	float4 ProjPos  : POSITION;
	float3 Normal : TEXCOORD1;
    float4 Color : COLOR;
	float2 TexCoord : TEXCOORD2;
};

struct VertexOutPosNorTanTex
{
	float3 WorldPos : TEXCOORD0;
	float4 ProjPos  : POSITION;
	float3 Normal : TEXCOORD1;
	float4 Tangent : TEXCOORD2;
    float4 Color : COLOR;
	float2 TexCoord : TEXCOORD3;
};

float4 diffuse(float3 normalizedNormal, float3 lightDirection, float lightDistance, 
			   float lightAttenuation, float4 color, float ambient)
{
	float4 diffuseMagnitude = max(dot(lightDirection, normalizedNormal),ambient);
	diffuseMagnitude *= exp(-lightAttenuation*lightDistance);
	float4 diffuseColor = color * lightColor;
	return float4((diffuseMagnitude * diffuseColor).rgb, color.a);
}

float4 specular(float3 normalizedNormal, float3 cameraPos, float3 worldPos, float3 lightDirection, 
				float4 color, float specularMagnitude, float specularPower)
{
	float3 cameraVector = normalize(cameraPos - worldPos);
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

float3 bump(float2 texCoord, float3 normalizedNormal, float4 chiralTangent)
{
	float3 tangent = chiralTangent.xyz;
	float chirality = chiralTangent.w;
	float3 normalMapSample = tex2D(normalSampler, texCoord).rgb;

	
	float3 mappedNormal = (2.1f * normalMapSample) - 1.0f;

	float3 n = normalizedNormal;
	float3 t = normalize(tangent - (dot(tangent, normalizedNormal) * normalizedNormal));
	float3 b = cross(n,t) * chirality;

	float3x3 tbn = float3x3(t,b,n);

	float3 bumpedNormal = mul(mappedNormal,tbn);

	return bumpedNormal;
}