#pragma once

#ifdef USE_DX11_GPUAPI

#include "GpuVertices.h"

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
#include <d3d11.h>
#else
#include <d3d11_1.h>
#endif

#include <DirectXMath.h>
//#include <DirectXPackedVector.h>

using namespace DirectX;

struct DX11Vertex_Pos
{
	XMFLOAT3 position;
	static ID3D11InputLayout* inputLayout;

	void fill(float x, float y, float z)
	{
		position.x = x; position.y = y; position.z = z;
	}
	static bool initInputLayout(ID3D11Device* device, 
		const void* shaderBytecode, SIZE_T bytecodeLength)
	{
		const D3D11_INPUT_ELEMENT_DESC vertexElements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		return SUCCEEDED(device->CreateInputLayout(vertexElements,1,shaderBytecode,bytecodeLength,&inputLayout));
	}
};
struct DX11VertexBuffer_Pos : public VertexBuffer_Pos
{
	DX11Vertex_Pos* buffer;
	DX11VertexBuffer_Pos(unsigned length) : VertexBuffer_Pos(length)
	{
		buffer = new DX11Vertex_Pos[length];
	}
	virtual ~DX11VertexBuffer_Pos()
	{
		delete[] buffer;
	}
	virtual void Insert(unsigned index, float x, float y, float z) override
	{
		buffer[index].fill(x,y,z);
	}
	virtual void* GetData() override
	{
		return static_cast<void*>(buffer);
	}
	virtual unsigned GetElementSize() override
	{
		return sizeof(DX11Vertex_Pos);
	}
};

struct DX11Vertex_PosCol : public DX11Vertex_Pos
{
	XMFLOAT3 color;
	static ID3D11InputLayout* inputLayout;

	void fill(float x, float y, float z,
		float r, float g, float b)
	{
		position.x = x; position.y = y; position.z = z;
		color.x = r; color.y = g; color.z = b;
	}
	
	static bool initInputLayout(ID3D11Device* device,
		const void* shaderBytecode, SIZE_T bytecodeLength)
	{
		const D3D11_INPUT_ELEMENT_DESC vertexElements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};	
		return SUCCEEDED(device->CreateInputLayout(vertexElements,2,shaderBytecode,bytecodeLength,&inputLayout));
	}
};
struct DX11VertexBuffer_PosCol : public VertexBuffer_PosCol
{
	DX11Vertex_PosCol* buffer;
	DX11VertexBuffer_PosCol(unsigned length) : VertexBuffer_PosCol(length)
	{
		buffer = new DX11Vertex_PosCol[length];
	}
	virtual ~DX11VertexBuffer_PosCol()
	{
		delete[] buffer;
	}
	virtual void Insert(unsigned index, float x, float y, float z, float r, float g, float b) override
	{
		buffer[index].fill(x,y,z,r,g,b);
	}
	virtual void* GetData() override
	{
		return static_cast<void*>(buffer);
	}
	virtual unsigned GetElementSize() override
	{
		return sizeof(DX11Vertex_PosCol);
	}
};

struct DX11Vertex_PosNor
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	static ID3D11InputLayout* inputLayout;

	void fill(float x, float y, float z,
		float u, float v, float w)
	{
		position.x = x; position.y = y; position.z = z;
		normal.x = u; normal.y = v; normal.z = w;
	}
	
	static bool initInputLayout(ID3D11Device* device,
		const void* shaderBytecode, SIZE_T bytecodeLength)
	{
		const D3D11_INPUT_ELEMENT_DESC vertexElements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};	
		return SUCCEEDED(device->CreateInputLayout(vertexElements,2,shaderBytecode,bytecodeLength,&inputLayout));
	}
};
struct DX11VertexBuffer_PosNor : public VertexBuffer_PosNor
{
	DX11Vertex_PosNor* buffer;
	DX11VertexBuffer_PosNor(unsigned length) : VertexBuffer_PosNor(length)
	{
		buffer = new DX11Vertex_PosNor[length];
	}
	virtual ~DX11VertexBuffer_PosNor()
	{
		delete[] buffer;
	}
	virtual void Insert(unsigned index, float x, float y, float z, float u, float v, float w) override
	{
		buffer[index].fill(x,y,z,u,v,w);
	}
	virtual void* GetData() override
	{
		return static_cast<void*>(buffer);
	}
	virtual unsigned GetElementSize() override
	{
		return sizeof(DX11Vertex_PosCol);
	}
};


struct DX11Vertex_PosNorTex 
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texCoord;
	static ID3D11InputLayout* inputLayout;

	void fill(float x, float y, float z,
		float u, float v, float w,
		float tx, float ty)
	{
		position.x = x; position.y = y; position.z = z;
		normal.x = u; normal.y = v; normal.z = w;
		texCoord.x = tx; texCoord.y = ty;
	}

	static bool initInputLayout(ID3D11Device* device,
		const void* shaderBytecode, SIZE_T bytecodeLength)
	{
		const D3D11_INPUT_ELEMENT_DESC vertexElements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};	
		return SUCCEEDED(device->CreateInputLayout(vertexElements,3,shaderBytecode,bytecodeLength,&inputLayout));
	}
};
struct DX11VertexBuffer_PosNorTex : public VertexBuffer_PosNorTex
{
	DX11Vertex_PosNorTex* buffer;
	DX11VertexBuffer_PosNorTex(unsigned length) : VertexBuffer_PosNorTex(length)
	{
		buffer = new DX11Vertex_PosNorTex[length];
	}
	virtual ~DX11VertexBuffer_PosNorTex()
	{
		delete[] buffer;
	}
	virtual void Insert(unsigned index, 
		float x, float y, float z, 
		float r, float g, float b, 
		float tx, float ty) override
	{
		buffer[index].fill(x,y,z,r,g,b,tx,ty);
	}
	virtual void* GetData() override
	{
		return static_cast<void*>(buffer);
	}
	virtual unsigned GetElementSize() override
	{
		return sizeof(DX11Vertex_PosNorTex);
	}
};

class DX11_VertApi : public VertApi {
	virtual VertexBuffer_Pos* CreateVertexBuffer_Pos(unsigned length) override
	{
		return new DX11VertexBuffer_Pos(length);
	}
	virtual VertexBuffer_PosCol* CreateVertexBuffer_PosCol(unsigned length) override
	{
		return new DX11VertexBuffer_PosCol(length);
	}
	virtual VertexBuffer_PosNor* CreateVertexBuffer_PosNor(unsigned length) override
	{
		return new DX11VertexBuffer_PosNor(length);
	}
	virtual VertexBuffer_PosNorTex* CreateVertexBuffer_PosNorTex(unsigned length) override
	{
		return new DX11VertexBuffer_PosNorTex(length);
	}
public:
	static bool InitInputLayout(VertexType type, ID3D11Device* device,
		const void* shaderBytecode, SIZE_T bytecodeLength);
	static void ReleaseVertices();
	static ID3D11InputLayout* GetInputLayout(VertexType type);
	static unsigned GetSize(VertexType type);
};

#endif