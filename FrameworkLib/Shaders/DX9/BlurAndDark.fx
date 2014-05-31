#line 1 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\BlurAndDark.cgfx"
uniform extern texture2D tex;
uniform extern float texWidth;
uniform extern float texHeight;
uniform extern float darkAmount;

sampler2D textureSampler : register(s0) = sampler_state
{
	Texture = <tex>;
	MinFilter = NONE;
	MagFilter = NONE;
	MipFilter = NONE;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct VertexOut
{
	float4 Position : POSITION; 
	float2 TexCoord : TEXCOORD0;
};

VertexOut vtxPosTex(float3 inputPosition : POSITION, float2 inputTexCoord : TEXCOORD0)
{
	VertexOut vout = (VertexOut) 0;
	vout.Position = float4(
		(inputPosition.x * 2.0f) - (1.0f) - (1.0f / texWidth),  
		(inputPosition.y * 2.0f) - (1.0f) + (1.0f / texHeight), 
		inputPosition.z, 1.0f);
	vout.TexCoord = inputTexCoord;
	return vout;
}

float4 blur(float2 texCoord)
{
	float xStep = 2.0f / texWidth;
	float yStep = 2.0f / texHeight;
	
	float4 avg = 0.0f;
	
	for(float x = -1.0f; x < 2.0f; ++x)
	{
		for(float y = -1.0f; y < 2.0f; ++y)
		{
			avg += tex2D(textureSampler, texCoord + float2(x*xStep,y*yStep));
		}
	}
	
	return avg / 9.0f;

	
	
	
	

	
}

float4 darken(float4 color)
{
	float3 newColor = color.rgb - darkAmount;
	newColor = clamp(newColor, 0.0f, 1.0f);
	return float4(newColor,color.a);
}

float4 pixPosTex(VertexOut vtx) : COLOR
{
	float4 color = blur(vtx.TexCoord);
	color = darken(color);
	return float4(color.rgb,1.0f);
}
 
technique posTex
{
    pass P0
    {   
        vertexShader = compile vs_2_0 vtxPosTex();
        pixelShader  = compile ps_2_0 pixPosTex();
    }
}