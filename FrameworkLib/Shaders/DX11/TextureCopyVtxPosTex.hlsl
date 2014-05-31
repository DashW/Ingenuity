#pragma pack_matrix(row_major)

#define VERTEXSHADER
#include "TextureShader.inl"

VertexOut main( float3 inputPosition : POSITION, float2 inputTexCoord : TEXCOORD0 ) 
{
	VertexOut vout;
	vout.Position = float4(
		(inputPosition.x * 2.0f) - (1.0f), 
		(inputPosition.y * 2.0f) - (1.0f),
		inputPosition.z, 
		1.0f);
	vout.TexCoord = inputTexCoord;
	return vout;
}
