cbuffer cbPerObject : register(b0)
{
	matrix gWorldViewProj; 
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut main( float3 pos : POSITION, float4 Color : COLOR )
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(pos, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = Color;
    
    return vout;
}