//--------------------------------------------------------------------------------
// PhongShading.hlsl
//
// This set of shaders implements the most basic phong shading.
//
// Copyright (C) 2010 Jason Zink.  All rights reserved.
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Resources
//--------------------------------------------------------------------------------

Texture2D       ParticleTexture : register( t0 );           
SamplerState    LinearSampler : register( s0 );

//--------------------------------------------------------------------------------
// Inter-stage structures
//--------------------------------------------------------------------------------
struct PS_INPUT
{
    float4 position			: SV_Position;
	float2 texcoords		: TEXCOORD0;
	float4 color			: Color;
};
//--------------------------------------------------------------------------------
float4 main( in PS_INPUT input ) : SV_Target
{
	float4 color = ParticleTexture.Sample( LinearSampler, input.texcoords );
	color = color * input.color;

	return( color );
}
//--------------------------------------------------------------------------------
