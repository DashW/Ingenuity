#line 12 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\MultiTextureAnimY.cgfx"

uniform extern float4x4 world;
uniform extern float4x4 worldInverseTranspose;
uniform extern float4x4 viewProjection;
uniform extern float4 materialColor;

uniform extern float param3; 
uniform extern float param4; 

#line 34 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\MultiTextureAnimY.cgfx"

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
uniform extern texture cubeMap;

uniform extern texture2D param0; 
uniform extern texture2D param1; 
uniform extern texture2D param2; 


sampler2D blendSampler : register(s0) = sampler_state
{
	Texture = <tex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = Wrap;
	AddressV = Wrap;
};

sampler2D texSampler1 : register(s1) = sampler_state
{
	Texture = <param0>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = Wrap;
	AddressV = Wrap;
};

sampler2D texSampler2 : register(s2) = sampler_state
{
	Texture = <param1>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = Wrap;
	AddressV = Wrap;
};

sampler2D texSampler3 : register(s3) = sampler_state
{
	Texture = <param2>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexOutPosNorTex
{
	float3 WorldPos : TEXCOORD0;
	float4 ProjPos  : POSITION;
	float3 Normal : TEXCOORD1;
    float4 Color : COLOR;
	float2 BlendCoord : TEXCOORD2;
	float2 TexCoord : TEXCOORD3;
};

struct PixelOut 
{
  float4 color : COLOR;
};

VertexOutPosNorTex vtxPosNorTex( float3 inputPos : POSITION, float3 inputNormal : NORMAL, float2 inputTexCoord : TEXCOORD0 )
{
	VertexOutPosNorTex vout;
	inputPos.y = ( inputPos.y * param4 ) + (param3 * (1.0f - param4));
	inputNormal = ( inputNormal * param4 ) + (float3(0.0f,1.0f,0.0f) * (1.0f - param4));
	vout.WorldPos = mul( float4(inputPos, 1.0f), world ).xyz;
	vout.ProjPos = mul( float4(vout.WorldPos, 1.0f), viewProjection );
	vout.Normal = mul( float4(inputNormal, 0.0f), worldInverseTranspose ).xyz;
    vout.Color = materialColor  ;
	vout.BlendCoord = inputTexCoord;
	vout.TexCoord = inputTexCoord * 16.0f;
    return vout;
}

float4 diffuse(float3 normalizedNormal, float3 worldPos, float3 lightDirection, float4 color)
{
	float4 diffuseMagnitude = max(dot(lightDirection, normalizedNormal),ambient  );
	float4 diffuseColor = color * lightColor  ;
	return float4((diffuseMagnitude * diffuseColor).rgb, color.a);
}

float4 spot(float3 lightDirection, float4 color)
{
	float spotFactor = pow(max(dot(normalize(-spotDirection  ), lightDirection), 0.001f), spotPower  );
	return float4(color.rgb * spotFactor,color.a);
}

PixelOut pixPosNorTex(VertexOutPosNorTex vtx)
{
	PixelOut output;
	float3 lightDirection = normalize(lightPosition   - vtx.WorldPos);
	float3 normalizedNormal = normalize(vtx.Normal);

	
    float4 c0 = tex2D(texSampler1, vtx.TexCoord);
    float4 c1 = tex2D(texSampler2, vtx.TexCoord);
    float4 c2 = tex2D(texSampler3, vtx.TexCoord);
    
    
    float3 B = tex2D(blendSampler, vtx.BlendCoord).rgb;

	
	
    float totalInverse = 1.0f / (B.r + B.g + B.b);
    
    
    
    c0 *= B.r * totalInverse;
    c1 *= B.g * totalInverse;
    c2 *= B.b * totalInverse;

	output.color = (c0 + c1 + c2);

	output.color = diffuse(normalizedNormal,vtx.WorldPos,lightDirection,output.color);
	output.color = spot(lightDirection,output.color);
	return output;
}




technique posNorTex
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosNorTex();
		PixelShader = compile ps_2_0 pixPosNorTex();
	}
}


