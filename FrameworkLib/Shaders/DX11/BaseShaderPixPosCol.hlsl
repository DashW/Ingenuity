#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosCol {
    float4 _ProjPos : SV_Position;
    float4 _Color : COLOR0;
};

struct PixelOut {
    float4 _color : SV_Target;
};

PixelOut main(in VertexOutPosCol vtx)
{
    PixelOut _output;
    _output._color = vtx._Color;
    return _output;
} 
