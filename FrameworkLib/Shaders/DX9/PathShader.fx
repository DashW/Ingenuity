#line 9 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\PathShader.cgfx"

uniform extern float4x4 world;
uniform extern float4x4 worldInverseTranspose;
uniform extern float4x4 viewProjection;
uniform extern float4 materialColor;


#line 29 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\PathShader.cgfx"

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

uniform extern float	 param0; 
uniform extern float     param1; 
uniform extern float	 param2; 
uniform extern float     param3; 
uniform extern float     param4; 
uniform extern texture2D param5; 


struct VertexOutPos
{
	float4 Pos : POSITION;
	float2 Pos2D : TEXCOORD0;
	float4 Color : COLOR;
};

struct VertexOutPosTex
{
	float4 Pos : POSITION;
	float2 Pos2D : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

struct PixelOut 
{
	float4 Color : COLOR;
};

VertexOutPos vtxPos( float3 pos : POSITION0 )
{
	VertexOutPos vtx;
	vtx.Pos = mul(mul(float4(pos.x,-pos.y,pos.z,1.0f),world),viewProjection);
	vtx.Pos2D = float2( pos.x, pos.y );
	vtx.Color = materialColor;
	return vtx;
}

VertexOutPosTex vtxPosTex( float3 pos : POSITION0, float2 tex : TEXCOORD0 )
{
	VertexOutPosTex vtx;
	vtx.Pos = mul(mul(float4(pos.x,-pos.y,pos.z,1.0f),world),viewProjection);
	vtx.Pos2D = float2( pos.x, pos.y );
	vtx.Tex = float2( tex.x, 1.0f - tex.y );
	return vtx;
}

VertexOutPos vtxPosPosCol( float3 pos : POSITION0, float3 instancePos : COLOR0, float4 instanceColor : COLOR1 )
{
	VertexOutPos vtx;
	vtx.Pos = mul(mul(float4(pos.x+instancePos.x,-pos.y-instancePos.y,pos.z+instancePos.z,1.0f),world),viewProjection);
	vtx.Pos2D = float2( pos.x, pos.y );
	vtx.Color = instanceColor;
	return vtx;
}

sampler2D gradientSampler : register(s1) = sampler_state
{
	Texture = <param5>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = Clamp;
	AddressV = Wrap;
};

PixelOut pixPos( VertexOutPos vtx )
{
	PixelOut pix;
	float4 multipliedColor = float4(
		vtx.Color.r * vtx.Color.a,
		vtx.Color.g * vtx.Color.a,
		vtx.Color.b * vtx.Color.a,
		vtx.Color.a
	);
	
	static float x1 = param1; static float y1 = param2; static float x2 = param3; static float y2 = param4;
	static float    gradAngle    = atan2( y2-y1, x2-x1 );
	static float2x2 gradRotation = float2x2(cos(gradAngle), -sin(gradAngle), sin(gradAngle), cos(gradAngle));
	static float    gradScale    = length(float2(x2-x1,y2-y1));

	float2 translatedPoint = vtx.Pos2D - float2( x1, y1 );
	float2 rotatedPoint    = mul(translatedPoint, gradRotation);
	float2 scaledPoint     = rotatedPoint / gradScale;


	float4 gradColor       = tex2D( gradientSampler, scaledPoint );

	pix.Color = (multipliedColor * (1.0f-param0)) + (gradColor * param0);
	return pix;
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

PixelOut pixPosTex( VertexOutPosTex vtx )
{
	PixelOut pix;
	pix.Color = tex2D( textureSampler, vtx.Tex );
	return pix;
}

technique pos
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPos();
		PixelShader = compile ps_2_0 pixPos();
	}
}

technique posTex
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosTex();
		PixelShader = compile ps_2_0 pixPosTex();
	}
}

technique posPosCol
{
	pass p0
	{
		VertexShader = compile vs_2_0 vtxPosPosCol();
		PixelShader = compile ps_2_0 pixPos();
	}
}