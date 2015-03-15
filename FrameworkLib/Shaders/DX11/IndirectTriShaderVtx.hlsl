
static const float3 vertices[] = { float3(0.0f, 0.05f, 0.5f), float3(0.5f, -0.5f, 0.5f), float3(-0.5f, -0.5f, 0.5f) };


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
float4 main( /*float4 Pos : POSITION,*/ uint id:SV_VERTEXID) : SV_POSITION
{
	uint triangleId = id / 3;
	uint vertexInTriangleId = id % 3;

	float4 Pos = float4(vertices[vertexInTriangleId], 1.0f) + triangleId * 0.2;

	return Pos;
}
