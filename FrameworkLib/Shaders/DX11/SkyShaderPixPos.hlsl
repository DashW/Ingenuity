#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPos
{
	float4 _WorldPos : TEXCOORD0;
    float4 _ProjPos : SV_Position;
};

struct PixelOut {
    float4 _color : SV_Target;
};

PixelOut main(in VertexOutPos vtx)
{
    PixelOut _output;
	_output._color = cubeMap.Sample(_cubeMapSampler, vtx._WorldPos.xyz - _cameraPosition);
    return _output;
} 
