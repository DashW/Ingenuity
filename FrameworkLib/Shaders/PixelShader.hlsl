struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

float4 main(VertexOut vtx) : SV_TARGET
{
	return vtx.Color;
}