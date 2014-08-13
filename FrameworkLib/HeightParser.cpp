#include "stdafx.h"
#include "HeightParser.h"
#include "GeoBuilder.h"

#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

namespace Ingenuity {

float HeightParser::epsilon = 0.0001f;

bool HeightParser::inBounds(int i, int j)
{
	return
		i >= 0 &&
		i < int(sideLength) - 1 &&
		j >= 0 &&
		j < int(sideLength) - 1;
}

float HeightParser::sampleHeight3x3(int i, int j)
{
	float avg = 0.0f;
	float num = 0.0f;

	for(int m = i - 1; m <= i + 1; ++m)
	{
		for(int n = j - 1; n <= j + 1; ++n)
		{
			if(inBounds(m, n))
			{
				avg += heights[(int(sideLength) * m) + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

bool HeightParser::ParseHeightmap(const char * heightmap, unsigned sideLength)
{
	if(heightmap == 0)
		return false;

	/// DON'T FORGET... IS THIS PRODUCING LANDSCAPES THAT ARE FLIPPED?????

	heights.clear();
	this->sideLength = sideLength;

	unsigned numVertices = sideLength * sideLength;

	for(unsigned i = 0; i < numVertices; i++)
	{
		if(heightmap[i] == 0) return false;
		heights.emplace_back(float(unsigned char(heightmap[i])));
	}

	// Filtering

	std::vector<float> temp;
	temp.resize(heights.size());

	for(unsigned i = 0; i < sideLength; ++i)
		for(unsigned j = 0; j < sideLength; ++j)
			temp[(int(sideLength) * i) + j] = sampleHeight3x3(i, j);

	heights.assign(temp.begin(), temp.end());

	return true;
}

void HeightParser::SetScale(float width, float height, float depth)
{
	this->width = width;
	this->depth = depth;
	this->scale = height;
}

LocalMesh * HeightParser::GetMesh(Gpu::Rect * textureRect)
{
	IVertexBuffer * vertexBuffer = 0;
	unsigned numVertices = sideLength * sideLength;

	if(textureRect)
	{
		VertexBuffer<Vertex_PosNorTex> * v = new VertexBuffer<Vertex_PosNorTex>(numVertices);
		for(unsigned i = 0; i < sideLength; i++) {
			for(unsigned j = 0; j < sideLength; j++) {
				float x = (width * -.5f) + (j * (width / (sideLength - 1)));
				float y = heights[(sideLength * i) + j] * scale;
				float z = (depth * -.5f) + (i * (depth / (sideLength - 1)));
				const float texRectWidth = textureRect->right - textureRect->left;
				const float texRectHeight = textureRect->bottom - textureRect->top;
				float tx = textureRect->left + (j * (texRectWidth / (sideLength - 1)));
				float ty = textureRect->top + (i * (texRectHeight / (sideLength - 1)));

				v->Set((sideLength * i) + j, Vertex_PosNorTex(x, y, z, 0.0f, 1.0f, 0.0f, tx, ty));
			}
		}
		vertexBuffer = v;
	}
	else
	{
		VertexBuffer<Vertex_PosNor> * v = new VertexBuffer<Vertex_PosNor>(numVertices);
		for(unsigned i = 0; i < sideLength; i++) {
			for(unsigned j = 0; j < sideLength; j++) {
				float x = (width * -.5f) + (j * (width / (sideLength - 1)));
				float y = heights[(sideLength * i) + j] * scale;
				float z = (depth * -.5f) + (i * (depth / (sideLength - 1)));

				v->Set((sideLength * i) + j, Vertex_PosNor(x, y, z, 0.0f, 1.0f, 0.0f));
			}
		}
		vertexBuffer = v;
	}

	unsigned numTriangles = (sideLength - 1) * (sideLength - 1) * 2;

	unsigned * k = new unsigned[numTriangles * 3];

	int index = 0;

	for(unsigned i = 0; i < (sideLength - 1); i++) {
		for(unsigned j = 0; j < (sideLength - 1); j++) {
			k[index++] = j + (i * sideLength);
			k[index++] = j + ((i + 1) * sideLength);
			k[index++] = j + 1 + (i * sideLength);

			k[index++] = j + 1 + (i * sideLength);
			k[index++] = j + ((i + 1) * sideLength);
			k[index++] = j + 1 + ((i + 1) * sideLength);
		}
	}

	GeoBuilder().GenerateNormals(vertexBuffer, numTriangles, k);

	return new LocalMesh(vertexBuffer, k, numTriangles * 3);
}

float HeightParser::GetHeight(float x, float z)
{
	unsigned squareX = unsigned(floor(float((x / width) + 0.5f) * float(sideLength - 1)));
	unsigned squareZ = unsigned(floor(float((z / depth) + 0.5f) * float(sideLength - 1)));

	const float squareWidth = width / float(sideLength - 1);
	const float squareDepth = depth / float(sideLength - 1);

	//unsigned squareX = unsigned(floor(((x+0.5f)*width)/squareWidth));
	//unsigned squareZ = unsigned(floor(((z+0.5f)*depth)/squareDepth));

	float result = 0.0f;

	if(inBounds(squareX, squareZ))
	{
		// if (0.0f,0.0f), then height = 0
		// if (1.0f,1.0f), then height = (sidelength * sidelength) - 1
		// if (0.0f,0.001f), then squareDepth = depth / sideLength;
		//						nearness = (0.001f % squareDepth)/squareDepth;
		//						height = (1-nearness) * height[0] + nearness * height[1];
		// WRONG!! We do need to worry about triangles!!


		float h1 = heights[(sideLength * squareZ) + squareX] * scale;
		float h2 = heights[(sideLength * squareZ) + (squareX + 1)] * scale;
		float h3 = heights[(sideLength * (squareZ + 1)) + squareX] * scale;
		float h4 = heights[(sideLength * (squareZ + 1)) + (squareX + 1)] * scale;

		// so... which way round are they??

		// Assume triangle1 = h1,h2,h3, and triangle2 = h2,h4,h3

		// First, triangle 1

		float inSquareX = x;
		while(inSquareX >= squareWidth) inSquareX -= squareWidth;
		while(inSquareX < 0.0f) inSquareX += squareWidth;
		inSquareX /= squareWidth;

		float inSquareZ = z;
		while(inSquareZ >= squareDepth) inSquareZ -= squareDepth;
		while(inSquareZ < 0.0f) inSquareZ += squareDepth;
		inSquareZ /= squareDepth;

		if(inSquareX + inSquareZ < 1.0f)
		{
			// triangle1

			// h1 + ((h2-h1) * squareX)
			// xh + ((h3-h1) * squareZ)
			result = h1 + ((h2 - h1) * inSquareX) + ((h3 - h1) * inSquareZ);
		}
		else
		{
			//result = h4;
			result = h4 - ((h4 - h3) * (1.0f - inSquareX)) - ((h4 - h2) * (1.0f - inSquareZ));
		}

	}
	return result;
}

float HeightParser::GetHeight(unsigned x, unsigned z)
{
	if(x >= sideLength || z >= sideLength) return 0.0f;
	return heights[(sideLength * z) + x] * scale;
}

} // namespace Ingenuity
