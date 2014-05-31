#line 1 "C:\\Users\\Richard\\SkyDrive\\Documents\\Gaming Projects\\FrameworkLib\\Darken.cgfx"
uniform extern texture2D tex;
uniform extern float texWidth;
uniform extern float texHeight;
uniform extern float param0; 

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

float4 pixPosTex(VertexOut vtx) : COLOR
{
	float4 color = tex2D(textureSampler, vtx.TexCoord); 
	color = clamp(color - param0, 0.0f, 1.0f);
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