// Cube Demo - 3d rendering - Richard Copperwaite

#include "pch.h"
#define _X86_
#define _USE_MATH_DEFINES

#include <RealtimeApp.h>
#include <GpuApi.h>
#include <math.h>

using namespace Ingenuity;

class ColoredCubeDemo : public RealtimeApp
{
	Gpu::Model* cube;
	Gpu::Font* font;
	Gpu::Camera camera;
	float cameraY, cameraRadius, cameraAngle, fullCircle;
	Gpu::Rect textPos;

	void InitCube()
	{
		VertexBuffer<Vertex_PosCol> b(8);
		b.Set(0, Vertex_PosCol(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f)); // WHITE
		b.Set(1, Vertex_PosCol(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f)); // BLACK
		b.Set(2, Vertex_PosCol(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f)); // BLUE
		b.Set(3, Vertex_PosCol(1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f)); // GREEN
		b.Set(4, Vertex_PosCol(-1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f)); // CYAN
		b.Set(5, Vertex_PosCol(-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f)); // RED
		b.Set(6, Vertex_PosCol(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f)); // YELLOW
		b.Set(7, Vertex_PosCol(1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f)); // PURPLE

		unsigned k[36];
		// Front face.
		k[0] = 0;  k[1]  = 1; k[2]  = 2;	k[3]  = 0; k[4]  = 2; k[5]  = 3;
		// Back face.
		k[6] = 4;  k[7]  = 6; k[8]  = 5;	k[9]  = 4; k[10] = 7; k[11] = 6;
		// Left face.
		k[12] = 4; k[13] = 5; k[14] = 1;	k[15] = 4; k[16] = 1; k[17] = 0;
		// Right face.
		k[18] = 3; k[19] = 2; k[20] = 6;	k[21] = 3; k[22] = 6; k[23] = 7;
		// Top face.
		k[24] = 1; k[25] = 5; k[26] = 6;	k[27] = 1; k[28] = 6; k[29] = 2;
		// Bottom face.
		k[30] = 4; k[31] = 0; k[32] = 3;	k[33] = 4; k[34] = 3; k[35] = 7;
		
		cube = new Gpu::Model();
		cube->mesh = gpu->CreateGpuMesh(&b, 12, k);
		cube->wireframe = true;
		cube->backFaceCull = false;
	}

public:
	virtual void Begin() override
	{
		gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		font = gpu->CreateGpuFont(40,L"Arial");
		InitCube();

		cameraRadius = 8.0f;
		camera.position.y = 5.0f;
		cameraAngle = 0.0f;

		textPos.top = textPos.left = 0.0f;
		textPos.right = 200.0f;
		textPos.bottom = 40.0f;

		fullCircle = (float)(M_PI * 2);
	}

	virtual void End() override
	{
		if(cube)
		{
			if(cube->mesh) delete cube->mesh;
			delete cube;
		}
		if(font) delete font;
	}

	virtual void Update(float secs) override
	{
		cameraAngle = cameraAngle + secs;
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera.position.x = sin(cameraAngle)*cameraRadius;
		camera.position.z = cos(cameraAngle)*cameraRadius;
	}

	virtual void Draw() override
	{		
		gpu->DrawGpuModel(cube, &camera, 0, 0);

		gpu->DrawGpuText(font, L"Ingenuity", 0.0f, 0.0f, false);
	}
};

MAIN_WITH(ColoredCubeDemo)