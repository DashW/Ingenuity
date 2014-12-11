#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNor {
    float3 _WorldPos : TEXCOORD0;
    float4 _ProjPos1 : SV_Position;
};

struct PixelOut {
    float4 color : SV_Target;
};

PixelOut main(in VertexOutPosNor _vtx)
{
    PixelOut output;
	output.color = float4(0.0f,0.0f,0.0f,1.0f);
    return output;
}
