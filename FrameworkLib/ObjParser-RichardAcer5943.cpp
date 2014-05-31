#include "stdafx.h"

#include "ObjParser.h"

#include "FileMgr.h"
#include "GpuApi.h"
#include <sstream>
#include <vector>

void ObjParser::ParseMesh(GpuApi *gpu, const char *text)
{

}

void ObjParser::ParseIndexedMesh(GpuApi *gpu, const char *text) // SYNCHRONOUS
{
	struct Vector3 {
		float x, y, z;
		Vector3(float x, float y, float z) 
			: x(x), y(y), z(z) {}
	};

	struct Vector2 {
		float u, v;
		Vector2(float u, float v)
			: u(u), v(v) {}
	};

	struct VertexRef {
		int vertex, texture, normal;
		VertexRef(int v,int t,int n) 
			: vertex(v), texture(t), normal(n) {}
	};

	std::stringstream stream(text);
	std::string line;

	std::vector<Vector3> vertexBank;
	std::vector<Vector3> normalBank;
	std::vector<Vector2> texUVBank;
	std::vector<VertexRef> refBank;

	bool hasTexCoords = false;
	
	while (std::getline(stream,line,'\n'))									// Start reading file data
	{		
		if (line.find("mtllib ") == 0)
		{
			line = line.substr(strlen("mtllib "));
			line.erase(line.find_last_not_of(" \n\r\t")+1);

			materialLib = line;
		}
		if (line.find("v ") == 0)										// The first character is a v: on this line is a vertex stored.
		{
			line = line.substr(strlen("v "));

			float x = 0.0f, y = 0.0f, z = 0.0f;
 
			if(sscanf_s(line.c_str(),"%f %f %f", &x, &y, &z) != 3)
			{
				return;
			}

			vertexBank.emplace_back(Vector3(x,y,z)); 
		}
		if (line.find("vt ") == 0)
		{
			hasTexCoords = true;

			float u = 0.0f, v = 0.0f;

			if(sscanf_s(line.c_str(),"%f %f", &u, &v) != 0)
			{
				return;
			}

			texUVBank.emplace_back(Vector2(u,v));
		}
		if (line.find("vn ") == 0)
		{
			line = line.substr(strlen("vn "));

			float x = 0.0f, y = 0.0f, z = 0.0f;

			sscanf_s(line.c_str(), "%f %f %f", &x, &y, &z);

			normalBank.emplace_back(Vector3(x,y,z));
		}
		if (line.find("usemtl ") == 0)
		{
			line = line.substr(strlen("usemtl "));
			line.erase(line.find_last_not_of(" \n\r\t")+1);

			materials.emplace_back(line);

			// Create a new model here! 
			
			// At some point, you're also going to have to group models heirarchically!
		}
		if (line.find("f ") == 0)
		{
			line = line.substr(strlen("f "));
 
			int a = 0, ta = 0, na = 0, b = 0, tb = 0, nb = 0, c = 0, tc = 0, nc = 0;

			if(hasTexCoords)
			{
				if(sscanf_s(line.c_str(),"%d/%d/%d %d/%d/%d %d/%d/%d", &a, &ta, &na, &b, &tb, &nb, &c, &tc, &nc) != 9)
				{
					return;
				}
				ta -= 1; tb -= 1; tc -= 1;
			}
			else // ! hasTexCoords
			{
				if(sscanf_s(line.c_str(), "%d//%d %d//%d %d//%d", &a, &na, &b, &nb, &c, &nc) != 6)
				{
					return;
				}
			}

			 a -= 1;  b -= 1;  c -= 1; // OBJ file starts counting from 1
			na -= 1; nb -= 1; nc -= 1;

			refBank.emplace_back(VertexRef(a,ta,na));
			refBank.emplace_back(VertexRef(b,tb,nb));
			refBank.emplace_back(VertexRef(c,tc,nc)); 
 
			///********************************************************************
			//	* Create triangles (f 1 2 3) from points: (v X Y Z) (v X Y Z) (v X Y Z). 
			//	* The vertexBuffer contains all verteces
			//	* The triangles will be created using the verteces we read previously
			//	*/
 
			//int tCounter = 0;
			//for (int i = 0; i < POINTS_PER_VERTEX; i++)					
			//{
			//	Faces_Triangles[triangle_index + tCounter   ] = vertexBuffer[3*vertexNumber[i] ];
			//	Faces_Triangles[triangle_index + tCounter +1 ] = vertexBuffer[3*vertexNumber[i]+1 ];
			//	Faces_Triangles[triangle_index + tCounter +2 ] = vertexBuffer[3*vertexNumber[i]+2 ];
			//	tCounter += POINTS_PER_VERTEX;
			//}
 
			///*********************************************************************
			//	* Calculate all normals, used for lighting
			//	*/ 
			//float coord1[3] = { Faces_Triangles[triangle_index], Faces_Triangles[triangle_index+1],Faces_Triangles[triangle_index+2]};
			//float coord2[3] = {Faces_Triangles[triangle_index+3],Faces_Triangles[triangle_index+4],Faces_Triangles[triangle_index+5]};
			//float coord3[3] = {Faces_Triangles[triangle_index+6],Faces_Triangles[triangle_index+7],Faces_Triangles[triangle_index+8]};
			//float *norm = this->calculateNormal( coord1, coord2, coord3 );
 
			//tCounter = 0;
			//for (int i = 0; i < POINTS_PER_VERTEX; i++)
			//{
			//	normals[normal_index + tCounter ] = norm[0];
			//	normals[normal_index + tCounter +1] = norm[1];
			//	normals[normal_index + tCounter +2] = norm[2];
			//	tCounter += POINTS_PER_VERTEX;
			//}
 
			//triangle_index += TOTAL_FLOATS_IN_TRIANGLE;
			//normal_index += TOTAL_FLOATS_IN_TRIANGLE;
			//TotalConnectedTriangles += TOTAL_FLOATS_IN_TRIANGLE;	
		}	
	}

	unsigned vertices = 0, triangles = 0, indices = 0, normals = 0;

	vertices = 0;

	triangles = refBank.size() / 3;

	VertApi *vtx = gpu->GetVertApi();

	if(hasTexCoords)
	{
		VertexBuffer_PosNorTex *v = vtx->CreateVertexBuffer_PosNorTex(triangles * 3);
		unsigned *k = new unsigned[triangles * 3];

		std::vector<VertexRef>::iterator it;
		for(it = refBank.begin(); it != refBank.end(); it++)
		{
			VertexRef& i = *it;

			k[indices++] = vertices;

			v->Insert(vertices++,
				vertexBank[i.vertex].x, vertexBank[i.vertex].y, vertexBank[i.vertex].z, 
				normalBank[i.normal].x, normalBank[i.normal].y, normalBank[i.normal].z,
				texUVBank[i.texture].u, texUVBank[i.texture].v);
		}
	
		indexedMesh = gpu->CreateGpuIndexedMesh(v, triangles, sizeof(unsigned) * triangles * 3, k);
	}
	else // ! hasTexCoords
	{
		VertexBuffer_PosNor *v = vtx->CreateVertexBuffer_PosNor(triangles * 3);
		unsigned *k = new unsigned[triangles * 3];

		std::vector<VertexRef>::iterator it;
		for(it = refBank.begin(); it != refBank.end(); it++)
		{
			VertexRef& i = *it;

			k[indices++] = vertices;

			v->Insert(vertices++,
				vertexBank[i.vertex].x, vertexBank[i.vertex].y, vertexBank[i.vertex].z, 
				normalBank[i.normal].x, normalBank[i.normal].y, normalBank[i.normal].z);
		}
	
		indexedMesh = gpu->CreateGpuIndexedMesh(v, triangles, sizeof(unsigned) * triangles * 3, k);
	}

	ready = true;
}

void ObjParser::ApplyMaterials(MtlParser *mtl)
{
	for(int i = 0; i < mtl->GetNumMtls(); i++)
	{
		if(mtl->GetMtl(i).name.compare(materials[0]) == 0)
		{
			if(indexedMesh == 0 && mesh != 0)
			{
				mesh->texture = mtl->GetMtl(i).tex;
			}
			if(mesh == 0 && indexedMesh != 0)
			{
				indexedMesh->texture = mtl->GetMtl(i).tex;
			}
		}
	}
}