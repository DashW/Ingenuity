#line 1 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\Sky.cgfx"
uniform extern float4x4 world;
uniform extern float4x4 worldInverseTranspose;
uniform extern float4x4 viewProjection;
uniform extern float4 materialColor;

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

sampler CubeSampler = sampler_state
{
    Texture   = <cubeMap>;
    MinFilter = LINEAR; 
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = WRAP;
    AddressV  = WRAP;
};


void SkyVS(float3 posL : POSITION0, 
           out float4 oPosH : POSITION0, 
           out float3 oCubeTex : TEXCOORD0)
{
	oCubeTex = mul(float4(posL, 1.0f), world);
	oPosH = mul(oCubeTex, viewProjection).xyww;
}

float4 SkyPS(float3 cubeTex : TEXCOORD0) : COLOR
{
    return texCUBE(CubeSampler, cubeTex - cameraPosition);
}

technique pos
{
    pass P
    {
        vertexShader = compile vs_2_0 SkyVS();
        pixelShader  = compile ps_2_0 SkyPS();
		CullMode = None;
    }
}

