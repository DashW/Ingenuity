//--------------------------------------------------------------------------------
// PhongShading.hlsl
//
// This set of shaders implements the most basic phong shading.
//
// Copyright (C) 2010 Jason Zink.  All rights reserved.
//--------------------------------------------------------------------------------

#define VERTEXSHADER
#include "ModelShader.inl"

//--------------------------------------------------------------------------------
// Resources
//--------------------------------------------------------------------------------
struct Particle
{
    float3 position;
	float3 direction;
	float  time;
};

StructuredBuffer<Particle> SimulationState;

cbuffer Constants: register(b10)
{
	float lifetime;
	float3 filler;
};

//--------------------------------------------------------------------------------
// Inter-stage structures
//--------------------------------------------------------------------------------
struct VS_INPUT
{
	uint vertexid			: SV_VertexID;
};
//--------------------------------------------------------------------------------
struct GS_INPUT
{
    float3 position			: Position;
	float  opacity			: Color;
};
//--------------------------------------------------------------------------------
//struct PS_INPUT
//{
//	float4 position			: SV_Position;
//	float2 texcoords		: TEXCOORD0;
//	float4 color			: Color;
//};
//--------------------------------------------------------------------------------
GS_INPUT main(in VS_INPUT input)
{
	GS_INPUT output;
	
	output.position = SimulationState[input.vertexid].position;
	output.opacity = saturate((lifetime - SimulationState[input.vertexid].time) / lifetime);
	output.opacity = pow(output.opacity, 10.0f);

	return output;
}

//static const float3 vertices[] = { float3(0.0f, 0.05f, 0.5f), float3(0.5f, -0.5f, 0.5f), float3(-0.5f, -0.5f, 0.5f) };
//
////--------------------------------------------------------------------------------------
//// Vertex Shader
////--------------------------------------------------------------------------------------
//PS_INPUT main( /*float4 Pos : POSITION,*/ uint id:SV_VERTEXID)
//{
//	uint triangleId = id / 3;
//	uint vertexInTriangleId = id % 3;
//
//	PS_INPUT output;
//
//	output.position = float4(vertices[vertexInTriangleId], 1.0f) + triangleId * 0.2;
//	output.texcoords = float2(0.0f, 0.0f);
//	output.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
//
//	return output;
//}


//--------------------------------------------------------------------------------
