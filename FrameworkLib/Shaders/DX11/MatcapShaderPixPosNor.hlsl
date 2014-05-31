#pragma pack_matrix(row_major)

#include "ModelShader.inl"

struct VertexOutPosNor {
	float3 _WorldPos : TEXCOORD0;
	float4 _ProjPos1 : SV_Position;
	float3 _Normal : TEXCOORD1;
	float4 _Color1 : COLOR0;
	float4 _ViewNor : TEXCOORD2;
};

struct PixelOut {
	float4 _color4 : SV_Target;
};

Texture2D<float4> _matcapTex : register(t3);
SamplerState _matcapSampler : TEXUNIT3;

PixelOut main(in VertexOutPosNor _vtx)
{
	PixelOut _output;

	float2 matcapCoord = (normalize(_vtx._ViewNor).xy + 1.0f) / 2.0f;

	_output._color4 = _matcapTex.Sample(_matcapSampler, float2(matcapCoord.x, 1.0f-matcapCoord.y));
	//_output._color4 = _vtx._Color1 * _matcapTex.Sample(_matcapSampler, matcapCoord);
	//_output._color4 = float4(matcapCoord.x, 0.0f, matcapCoord.y, 1.0f);

	return _output;
}
