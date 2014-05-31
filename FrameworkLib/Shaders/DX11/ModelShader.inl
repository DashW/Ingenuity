#ifdef VERTEXSHADER

cbuffer C0: register(b0)
{
	uniform float4x4 _world;
	uniform float4x4 _viewProjection;
    uniform float4x4 _worldInverseTranspose;
	uniform float4x4 _lightViewProjection;
    uniform float4 _materialColor;
};

struct VertexPos
{
	float3 Pos : POSITION;
};
struct VertexPosCol
{
	float3 Pos : POSITION; 
	float4 Color : COLOR0;
};
struct VertexPosNor
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL0;
};
struct VertexPosTex
{
	float3 Pos : POSITION;
	float2 TexCoord : TEXCOORD0;
};
struct VertexPosNorTex
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};
struct VertexPosNorTanTex
{
	float3 Pos : POSITION; 
	float3 Normal : NORMAL0; 
	float4 Tangent : TANGENT; 
	float2 TexCoord : TEXCOORD0;
};

struct InstancePos
{
	float3 Pos : COLOR0; 
};
struct InstancePosCol
{
	float3 Pos : COLOR0; 
	float4 Color : COLOR1;
};

#else

cbuffer C0: register(b0)
{
    uniform float3 _lightPosition;
    uniform float _specularPower;

    uniform float4 _lightColor;

    uniform float3 _cameraPosition;
    uniform float _ambient;

    uniform float3 _spotDirection;
    uniform float _spotPower;

    uniform float _cubeMapAlpha;
	uniform uint _numLights;
	uniform float _specularFactor;
	uniform float _diffuseFactor;
};

cbuffer C1 : register(b1)
{
	uniform float4 lightPositionSpecs[6];
	uniform float4 lightColorAttens[6];
	uniform float4 lightSpotDirPowers[6];
};

Texture2D<float4> tex : register(t0);
TextureCube<float4> cubeMap : register(t1);
Texture2D<float4> normalMap : register(t2);
SamplerState _textureSampler : register(s0);
SamplerState _cubeMapSampler : register(s1);
SamplerState _normalMapSampler : register(s2);

float4 diffuse(float3 lightDirection, float lightDistance, float4 lightColorAtten, float3 normalizedNormal, float4 color)
{
	float diffuseMagnitude = max(dot(lightDirection, normalizedNormal), 0.0f);
	//diffuseMagnitude *= exp(-lightColorAtten.a * lightDistance);

	float3 diffuseColor = color.rgb * lightColorAtten.rgb * diffuseMagnitude * _diffuseFactor;

	return float4(diffuseColor, color.a);
}

float4 cubeReflect(float3 normalizedNormal, float3 cameraVector, float4 color)
{
    float3 _cubeMapTheta0022 = reflect(-cameraVector, normalizedNormal);
    float4 _cubeMapColor0022 = cubeMap.Sample(_cubeMapSampler, _cubeMapTheta0022);

    float brightestComponent = max(max(_cubeMapColor0022.x, _cubeMapColor0022.y), _cubeMapColor0022.z);
    float _cubeMapIntensity0022 = ( 5.00000000000000000E-001f + (brightestComponent*5.00000000000000000E-001f) )*_cubeMapAlpha;

    float3 _TMP130022 = color.rgb*( 1.00000000000000000E000f - _cubeMapIntensity0022) + _cubeMapColor0022.xyz*_cubeMapIntensity0022;
    return float4(_TMP130022, color.a);
}

float4 specular(float3 lightDirection, float4 lightColorAtten, float specPower, float3 normalizedNormal, float3 cameraVector, float4 color, float lightDistance = 0.0f)
{
	float3 specularReflection = normalize(reflect(-lightDirection, normalizedNormal));
	float specularMagnitude = max(dot(cameraVector, specularReflection), 0.0f);

	//specularMagnitude *= exp(-lightColorAtten.a * lightDistance);
	//specularMagnitude /= (lightDistance * lightDistance);

	float specularPoweredMagnitude = pow(specularMagnitude, specPower);
	return float4(color.rgb + (specularPoweredMagnitude * lightColorAtten.rgb * _specularFactor), color.a);
}

float4 spot(float3 lightDirection, float3 spotDirection, float spotPower, float4 color)
{
    float spotTheta = dot(normalize(-spotDirection), lightDirection);
    spotTheta = max(spotTheta,  1.00000004749745130E-003f);
    float _spotFactor0036 = pow(spotTheta, spotPower);

    return float4((color.rgb*_spotFactor0036), color.a);
}

float3 mapNormal(float2 texCoord, float3 normalizedNormal, float4 chiralTangent)
{
	float3 tangent = chiralTangent.xyz;
	float chirality = chiralTangent.w;
	float3 normalMapSample = normalMap.Sample(_normalMapSampler, texCoord).xyz;

	float3 mappedNormal = (2.1f * normalMapSample) - 1.0f;

	float3 n = normalizedNormal;
	float3 t = normalize(tangent - (dot(tangent, normalizedNormal) * normalizedNormal));
	float3 b = cross(n,t) * chirality; // See DX9 Version for castle hack

	float3x3 tbn = float3x3(t,b,n);

	float3 bumpedNormal = mul(mappedNormal,tbn);

	return bumpedNormal;
}

#endif
