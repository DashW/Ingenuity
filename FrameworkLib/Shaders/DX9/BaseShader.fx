#line 9 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\BaseShader.cgfx"

uniform extern float4x4 world;
uniform extern float4x4 worldInverseTranspose;
uniform extern float4x4 viewProjection;
uniform extern float4 materialColor;

  
#line 29 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\BaseShader.cgfx"

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

struct PixelOut 
{
  float4 color : COLOR;
};

VertexOutPosCol vtxPosCol( float3 pos : POSITION, float4 Color : COLOR )
{
	VertexOutPosCol vout;
	vout.ProjPos = mul(mul(float4(pos, 1.0f), world), viewProjection);
    vout.Color = Color;
    
    return vout;
}

VertexOutPosNor vtxPosNor( float3 inputPos : POSITION, float3 inputNormal : NORMAL )
{
	VertexOutPosNor vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz;
	vout.ProjPos = mul(float4(vout.WorldPos, 1.0f), viewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
    vout.Color = materialColor;
    
    return vout;
}

VertexOutPosNorTex vtxPosNorTex( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float2 inputTexCoord : TEXCOORD0 )
{
	VertexOutPosNorTex vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz;
	vout.ProjPos = mul(float4(vout.WorldPos, 1.0f), viewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
    vout.Color = materialColor;
	vout.TexCoord = inputTexCoord;
    
    return vout;
}

VertexOutPosNorTanTex vtxPosNorTanTex( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float4 inputTangent : TANGENT, float2 inputTexCoord : TEXCOORD0 )
{
	VertexOutPosNorTanTex vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz;
	vout.ProjPos = mul(float4(vout.WorldPos, 1.0f), viewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
	vout.Tangent = float4(mul(inputTangent.xyz, (float3x3) world), inputTangent.w);
    vout.Color = materialColor;
	vout.TexCoord = inputTexCoord;

    return vout;
}

VertexOutPosNor vtxPosNorPos( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float3 worldPos : COLOR0 )
{
	VertexOutPosNor vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz + worldPos;
	vout.ProjPos = mul(float4(vout.WorldPos, 1.0f), viewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
    vout.Color = materialColor;
    
    return vout;
}

VertexOutPosNorTex vtxPosNorTexPos( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float2 inputTexCoord : TEXCOORD0, float3 worldPos : COLOR0 )
{
	VertexOutPosNorTex vout;
	vout.WorldPos = mul(float4(inputPos, 1.0f), world).xyz + worldPos;
	vout.ProjPos = mul(float4(vout.WorldPos, 1.0f), viewProjection);
	vout.Normal = mul(float4(inputNormal, 0.0f), worldInverseTranspose).xyz;
    vout.Color = materialColor;
	vout.TexCoord = inputTexCoord;
    
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

float3 bump(float2 texCoord, float3 normalizedNormal, float4 chiralTangent)
{
	float3 tangent = chiralTangent.xyz;
	float chirality = chiralTangent.w;
	float3 normalMapSample = tex2D(normalSampler, texCoord).rgb;

	
	float3 mappedNormal = (2.1f * normalMapSample) - 1.0f;

	float3 n = normalizedNormal;
	float3 t = normalize(tangent - (dot(tangent, normalizedNormal) * normalizedNormal));
	float3 b = cross(n,t) * -chirality; // THIS IS A HACK. The GeoBuilder does not set chirality correctly!!!

	float3x3 tbn = float3x3(t,b,n);

	float3 bumpedNormal = mul(mappedNormal,tbn);

	return bumpedNormal;
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
	float3 bumpedNormal = bump(vtx.TexCoord,normalizedNormal,vtx.Tangent);
	output.color = vtx.Color * tex2D( textureSampler, vtx.TexCoord );
	output.color = diffuse(bumpedNormal,vtx.WorldPos,lightDirection,lightDistance,output.color);
	output.color = cubeReflect(bumpedNormal,vtx.WorldPos,output.color);
	output.color = specular(bumpedNormal,vtx.WorldPos,lightDirection,output.color);
	output.color = spot(lightDirection,output.color);
	return output;
}



technique posCol
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosCol();
		PixelShader = compile ps_2_0 pixPosCol();
	}
}

technique posNor
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosNor();
		PixelShader = compile ps_2_0 pixPosNor();
	}
}

technique posNorTex
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosNorTex();
		PixelShader = compile ps_2_0 pixPosNorTex();
	}
}

technique posNorTanTex
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosNorTanTex();
		PixelShader = compile ps_2_0 pixPosNorTanTex();
	}
}

technique posNorPos
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosNorPos();
		PixelShader = compile ps_2_0 pixPosNorTex();
	}
}

technique posNorTexPos
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosNorTexPos();
		PixelShader = compile ps_2_0 pixPosNorTex();
	}
}
