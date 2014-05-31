#include "stdafx.h"
#include "DX11VertApi.h"

#ifdef USE_DX11_GPUAPI

ID3D11InputLayout* DX11Vertex_Pos::inputLayout = 0;
ID3D11InputLayout* DX11Vertex_PosCol::inputLayout = 0;
ID3D11InputLayout* DX11Vertex_PosNor::inputLayout = 0;
ID3D11InputLayout* DX11Vertex_PosNorTex::inputLayout = 0;

bool DX11_VertApi::InitInputLayout(VertexType type, ID3D11Device* device, 
		const void* shaderBytecode, SIZE_T bytecodeLength){
	switch(type) 
	{
	case VertexType_Pos:
		return DX11Vertex_Pos::initInputLayout(device,shaderBytecode,bytecodeLength);
	case VertexType_PosCol:
		return DX11Vertex_PosCol::initInputLayout(device,shaderBytecode,bytecodeLength);
	case VertexType_PosNor:
		return DX11Vertex_PosNor::initInputLayout(device,shaderBytecode,bytecodeLength);
	case VertexType_PosNorTex:
		return DX11Vertex_PosNorTex::initInputLayout(device,shaderBytecode,bytecodeLength);
	default:
		OutputDebugString(L"Could not initialise unrecognized vertex type\n");
		return false;
	}
}
void DX11_VertApi::ReleaseVertices(){
	if(DX11Vertex_Pos::inputLayout) DX11Vertex_Pos::inputLayout->Release();
	if(DX11Vertex_PosCol::inputLayout) DX11Vertex_PosCol::inputLayout->Release();
	if(DX11Vertex_PosNor::inputLayout) DX11Vertex_PosNor::inputLayout->Release();
	if(DX11Vertex_PosNorTex::inputLayout) DX11Vertex_PosNorTex::inputLayout->Release();
}
ID3D11InputLayout* DX11_VertApi::GetInputLayout(VertexType type)
{
	switch(type) 
	{
	case VertexType_Pos:
		return DX11Vertex_Pos::inputLayout;
	case VertexType_PosCol:
		return DX11Vertex_PosCol::inputLayout;
	case VertexType_PosNor:
		return DX11Vertex_PosNor::inputLayout;
	case VertexType_PosNorTex:
		return DX11Vertex_PosNorTex::inputLayout;
	default:
		//OutputDebugString(L"Could not get declaration of unrecognized vertex type\n");
		return 0;
	}
}
unsigned DX11_VertApi::GetSize(VertexType type)
{
	switch(type)
	{
	case VertexType_Pos:
		return sizeof(DX11Vertex_Pos);
	case VertexType_PosCol:
		return sizeof(DX11Vertex_PosCol);
	case VertexType_PosNor:
		return sizeof(DX11Vertex_PosNor);
	case VertexType_PosNorTex:
		return sizeof(DX11Vertex_PosNorTex);
	default:
		//OutputDebugString(L"Could not get size of unrecognized vertex type\n");
		return 0;
	}
}

#endif