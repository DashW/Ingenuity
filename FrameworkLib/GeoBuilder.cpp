#include "GeoBuilder.h"

#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/ext.hpp>

#include <algorithm>
#include <list>
#include <map>

namespace Ingenuity {

Path * GeoBuilder::staticPath = 0;
std::vector<unsigned> * GeoBuilder::staticMonotone = 0;
const double GeoBuilder::crossoverFixingOffset = 0.01f;

LocalMesh * LocalMesh::CombineWith(LocalMesh * other)
{
	if(other->vertexBuffer->GetVertexType() != vertexBuffer->GetVertexType()) return 0;

	// Create the new vertex and index buffers

	const unsigned newTriangleCount = numTriangles + other->numTriangles;
	unsigned * newIndexBuffer = new unsigned[newTriangleCount * 3];
	IVertexBuffer * newVertexBuffer;
	switch(vertexBuffer->GetVertexType())
	{
	case VertexType_Pos:
		newVertexBuffer = new VertexBuffer<Vertex_Pos>(vertexBuffer->GetLength() + other->vertexBuffer->GetLength());
		break;
	case VertexType_PosCol:
		newVertexBuffer = new VertexBuffer<Vertex_PosCol>(vertexBuffer->GetLength() + other->vertexBuffer->GetLength());
		break;
	case VertexType_PosNor:
		newVertexBuffer = new VertexBuffer<Vertex_PosNor>(vertexBuffer->GetLength() + other->vertexBuffer->GetLength());
		break;
	case VertexType_PosTex:
		newVertexBuffer = new VertexBuffer<Vertex_PosTex>(vertexBuffer->GetLength() + other->vertexBuffer->GetLength());
		break;
	case VertexType_PosNorTex:
		newVertexBuffer = new VertexBuffer<Vertex_PosNorTex>(vertexBuffer->GetLength() + other->vertexBuffer->GetLength());
		break;
	case VertexType_PosNorTanTex:
		newVertexBuffer = new VertexBuffer<Vertex_PosNorTanTex>(vertexBuffer->GetLength() + other->vertexBuffer->GetLength());
		break;
	}

	// Copy the vertex buffers

	char * newData = (char*) newVertexBuffer->GetData();
	const unsigned myDataLength = vertexBuffer->GetElementSize() * vertexBuffer->GetLength();
	const unsigned otherDataLength = other->vertexBuffer->GetElementSize() * other->vertexBuffer->GetLength();

	memcpy(newData, vertexBuffer->GetData(), myDataLength);
	memcpy(newData + myDataLength, other->vertexBuffer->GetData(), otherDataLength);

	// Copy the index buffers

	const unsigned indexCount = (numTriangles * 3);
	const unsigned otherIndexCount = (other->numTriangles * 3);
	memcpy(newIndexBuffer, indexBuffer, indexCount * sizeof(unsigned));
	memcpy(newIndexBuffer + indexCount, other->indexBuffer, otherIndexCount * sizeof(unsigned));

	// Correct the second set of indices

	for(unsigned i = 0; i < otherIndexCount; ++i)
	{
		newIndexBuffer[indexCount + i] += vertexBuffer->GetLength();
	}

	return new LocalMesh(newVertexBuffer, newIndexBuffer, newTriangleCount * 3);
}

IVertexBuffer * GeoBuilder::BuildVertexBufferHemisphere(
	const float radius, const unsigned sectors, const unsigned stacks)
{
	unsigned numVertices = ((sectors + 1) * (stacks + 1)) + 1;
	VertexBuffer<Vertex_PosNor> * b = new VertexBuffer<Vertex_PosNor>(numVertices);
	b->Set(0, Vertex_PosNor(-0.0f, -0.0f, -radius, -0.0f, -0.0f, -1.0f));

	for(unsigned i = 0; i < (stacks + 1); i++)
	{
		float stackAngle = (float(M_PI_2) / float(stacks + 1));
		const float depth = -cosf(stackAngle * float(i + 1)) * radius;
		float stackRadius = sqrtf((radius*radius) - (depth*depth));
		float sectorAngle = float(M_PI * 2.0) / float(sectors);

		for(unsigned j = 0; j < (sectors + 1); j++)
		{
			float x = sinf(sectorAngle * float(j)) * stackRadius;
			float y = cosf(sectorAngle * float(j)) * stackRadius;
			float normalx = x / radius;
			float normaly = y / radius;
			float normalz = depth / radius;
			unsigned index = ((sectors + 1) * i) + j + 1;

			b->Set(index, Vertex_PosNor(x, y, depth, normalx, normaly, normalz));
		}
	}
	return b;
}

unsigned * GeoBuilder::BuildIndexBufferHemisphere(
	const unsigned sectors, const unsigned stacks, unsigned& numTriangles)
{
	numTriangles = sectors * ((stacks * 2) + 1);
	unsigned numVertices = ((sectors + 1) * (stacks + 1)) + 1;

	unsigned* k = new unsigned[numTriangles * 3];
	int index = 0;

	for(unsigned i = 0; i < sectors; i++)
	{
		// Bottom
		k[index++] = 0;
		k[index++] = i + 1;
		k[index++] = ((i + 1)) + 1;

		// Quads
		for(unsigned j = 0; j < stacks; j++)
		{
			k[index++] = (i + 1) + j * (sectors + 1);               //TOPLEFT
			k[index++] = (i + 1) + (j + 1) * (sectors + 1);           //BOTTOMLEFT
			k[index++] = ((i + 1)) + 1 + (j * (sectors + 1));		//TOPRIGHT

			k[index++] = ((i + 1)) + 1 + (j * (sectors + 1));		//TOPRIGHT
			k[index++] = (i + 1) + ((j + 1) * (sectors + 1));         //BOTTOMLEFT
			k[index++] = ((i + 1)) + 1 + ((j + 1) * (sectors + 1));	//BOTTOMRIGHT
		}
	}

	return k;
}

IVertexBuffer * GeoBuilder::BuildVertexBufferDisc(
	const float radius, const unsigned sectors)
{
	unsigned numVertices = sectors + 1;
	VertexBuffer<Vertex_PosNor> * b = new VertexBuffer<Vertex_PosNor>(numVertices);
	b->Set(0, Vertex_PosNor(-0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -1.0f));

	const float depth = 0.0f;
	const float sectorAngle = float(M_PI * 2.0) / float(sectors);

	for(unsigned j = 0; j < sectors; j++)
	{
		float x = sinf(sectorAngle * float(j)) * radius;
		float y = cosf(sectorAngle * float(j)) * radius;
		float normalx = 0.0f;
		float normaly = 0.0f;
		float normalz = -1.0f;
		unsigned index = j + 1;
		b->Set(index, Vertex_PosNor(x, y, depth, normalx, normaly, normalz));
	}
	return b;
}

unsigned * GeoBuilder::BuildIndexBufferDisc(
	const unsigned sectors, unsigned& numTriangles)
{
	numTriangles = sectors;

	unsigned* k = new unsigned[numTriangles * 3];
	int index = 0;

	for(unsigned i = 0; i < sectors; i++)
	{
		k[index++] = 0;
		k[index++] = i + 1;
		k[index++] = ((i + 1) % sectors) + 1;
	}

	return k;
}

IVertexBuffer * GeoBuilder::BuildVertexBufferTube(
	const float height, const float radius, const unsigned sectors, const unsigned stacks)
{
	unsigned numVertices = (sectors + 1) * (stacks + 1);
	VertexBuffer<Vertex_PosNor> * b = 0;
	VertexBuffer<Vertex_PosNorTex> * bTex = 0;
	b = new VertexBuffer<Vertex_PosNor>(numVertices);

	for(unsigned i = 0; i < (stacks + 1); i++)
	{
		const float depth = ((-1.0f + ((2.0f / (stacks)) * i)) * (height / 2.0f));

		for(unsigned j = 0; j < (sectors + 1); j++)
		{
			float x = sin((2 * float(M_PI) / (sectors)) * j) * radius;
			float y = cos((2 * float(M_PI) / (sectors)) * j) * radius;
			float normalx = x / radius;
			float normaly = y / radius;
			float normalz = 0.0f;
			unsigned index = ((sectors + 1) * i) + j;
			b->Set(index, Vertex_PosNor(x, y, depth, normalx, normaly, normalz));
		}
	}
	return b;
}

unsigned * GeoBuilder::BuildIndexBufferTube(
	const unsigned sectors, const unsigned stacks, unsigned& numTriangles)
{
	numTriangles = sectors * stacks * 2;
	unsigned numVertices = (sectors + 1) * (stacks + 1);

	unsigned* k = new unsigned[numTriangles * 3];
	int index = 0;

	for(unsigned i = 0; i < sectors; i++)
	{
		// Quads
		for(unsigned j = 0; j < stacks; j++)
		{
			k[index++] = i + (j * (sectors + 1));               //TOPLEFT
			k[index++] = i + ((j + 1) * (sectors + 1));         //BOTTOMLEFT
			k[index++] = i + 1 + (j * (sectors + 1));		    //TOPRIGHT

			k[index++] = i + 1 + (j * (sectors + 1));		    //TOPRIGHT
			k[index++] = i + ((j + 1) * (sectors + 1));         //BOTTOMLEFT
			k[index++] = i + 1 + ((j + 1) * (sectors + 1));	    //BOTTOMRIGHT
		}
	}

	return k;
}

IVertexBuffer * GeoBuilder::BuildVertexBufferCylinder(
		const float height, const float radius, const unsigned sectors, 
		const unsigned stacks, bool texCoords, const bool sphere)
{
	unsigned numVertices = ((sectors+1) * (stacks + 1)) + 2;
	VertexBuffer<Vertex_PosNor> * b = 0;
	VertexBuffer<Vertex_PosNorTex> * bTex = 0;
	if(texCoords)
	{	
		bTex = new VertexBuffer<Vertex_PosNorTex>(numVertices);
		bTex->Set(0, Vertex_PosNorTex(-0.0f, -0.0f, -height/2.0f, -0.0f, -0.0f, -1.0f, 0.0f, 0.0f));
		bTex->Set(numVertices - 1, Vertex_PosNorTex(-0.0f, -0.0f, height/2.0f, -0.0f, -0.0f, 1.0f, 1.0f, 1.0f));
	}
	else
	{
		b = new VertexBuffer<Vertex_PosNor>(numVertices);
		b->Set(0, Vertex_PosNor(-0.0f, -0.0f, -height/2.0f, -0.0f, -0.0f, -1.0f));
		b->Set(numVertices - 1, Vertex_PosNor(-0.0f, -0.0f, height/2.0f, -0.0f, -0.0f, 1.0f));
	}
	
	for(unsigned i = 0; i < (stacks + 1); i++)
	{
		const float depth = ((-1.0f + 
			(sphere ? (2.0f / (stacks + 2)) * (i + 1) : (2.0f / (stacks)) * i)) 
			* (height/2.0f));

		// Depth needs to be special for capsules! Stacks no longer mean the same as they mean for cylinders

		float stackRadius;
		if(sphere)
			stackRadius = sqrtf( (radius*radius) - (depth*depth) );
		else
			stackRadius = radius;

		for(unsigned j = 0; j < (sectors+1); j++)
		{
			float x = sin((2 * float(M_PI) / (sectors)) * j) * stackRadius;
			float y = cos((2 * float(M_PI) / (sectors)) * j) * stackRadius;
			float normalx = x/radius;
			float normaly = y/radius;
			float normalz = sphere ? depth/radius : 0.0f;
			unsigned index = ((sectors + 1) * i) + j + 1;
			if(texCoords)
			{
				float texU = float(j) / float(sectors);
				float texV = float(i) / float(stacks);
				bTex->Set(index, Vertex_PosNorTex(x, y, depth, normalx, normaly, normalz, texU, texV));
			}
			else
			{
				b->Set(index, Vertex_PosNor(x, y, depth, normalx, normaly, normalz));
			}
		}
	}
	if(texCoords)
		return bTex;
	else
		return b;
}

unsigned * GeoBuilder::BuildIndexBufferCylinder(const unsigned sectors, const unsigned stacks, unsigned& numTriangles)
{
	numTriangles = (sectors+1) * 2 + (stacks * (sectors+1) * 2);
	unsigned numVertices = ((sectors+1) * (stacks + 1)) + 2;

	unsigned* k = new unsigned[numTriangles * 3];
	int index = 0;

	for(unsigned i = 0; i < sectors; i++)
	{
		// Bottom
		k[index++] = 0; 
		k[index++] = i + 1; 
		k[index++] = ((i+1)) + 1;

		// Quads
		for(unsigned j = 0; j < stacks; j++)
		{
			k[index++] = (i+1) + j * (sectors+1);               //TOPLEFT
			k[index++] = (i+1) + (j+1) * (sectors+1);           //BOTTOMLEFT
			k[index++] = ((i+1)) + 1 + (j * (sectors+1));		//TOPRIGHT

			k[index++] = ((i+1)) + 1 + (j * (sectors+1));		//TOPRIGHT
			k[index++] = (i+1) + ((j+1) * (sectors+1));         //BOTTOMLEFT
			k[index++] = ((i+1)) + 1 + ((j+1) * (sectors+1));	//BOTTOMRIGHT
		}

		//Top
		k[index++] = numVertices - 1;
		k[index++] = (numVertices - sectors) + i - 1;
		k[index++] = (numVertices - sectors) + i - 2;
	}

	return k;
}

void GeoBuilder::GenerateSortedIndices(Path & path, std::vector<unsigned> & sortedIndices)
{
	sortedIndices.clear();
	for(unsigned i = 0; i < path.Length(); ++i)
	{
		sortedIndices.push_back(i);
	}
	staticPath = &path;
	std::sort(sortedIndices.begin(), sortedIndices.end(), ComparePathPoints);
}

void GeoBuilder::FixPathCrossover(double crossX, double crossY, unsigned edge1, unsigned edge2, Path & path)
{
	if(edge1 > edge2)
	{
		unsigned temp = edge1;
		edge1 = edge2;
		edge2 = temp;
	}

	double edge1angle = path.edges[edge1].angle;
	double edge2angle = path.edges[edge2].angle;

	double averageAngle = (edge1angle + edge2angle) / 2.0f;

	double deltaAngle = edge2angle - edge1angle;
	if(deltaAngle < M_PI && deltaAngle > -M_PI) averageAngle += M_PI;

	double offsetX = cos(averageAngle) * crossoverFixingOffset;
	double offsetY = sin(averageAngle) * crossoverFixingOffset;

	if(offsetY == 0.0f) offsetY = crossoverFixingOffset;

	Path::Point edge1Point(crossX+offsetX,crossY+offsetY);
	Path::Point edge2Point(crossX-offsetX,crossY-offsetY);

	path.points.insert(path.points.begin() + edge2, edge2Point);
	path.points.insert(path.points.begin() + edge1, edge1Point);

	unsigned startFlip = edge1 + 1;
	unsigned endFlip = edge2;

	while(startFlip < endFlip)
	{
		Path::Point temp = path.points[endFlip];
		path.points[endFlip] = path.points[startFlip];
		path.points[startFlip] = temp;
		endFlip--;
		startFlip++;
	}

	path.Finalize();

	//if(IsDebuggerPresent()) DebugBreak();
}

void GeoBuilder::FixPathCrossovers(Path & path)
{
	std::vector<unsigned> sortedIndices;
	EdgeTree crossoverTree;
	bool possibleCrossovers = true;

	while(possibleCrossovers)
	{
		possibleCrossovers = false;
		crossoverTree.clear();

		GenerateSortedIndices(path, sortedIndices);

		for(unsigned i = 0; i < sortedIndices.size(); ++i)
		{
			unsigned index = sortedIndices[i];
			Path::Point & point = path.points[index];
			unsigned prevIndex = (index == 0 ? path.Length() - 1 : index - 1);
			unsigned nextIndex = (index == path.Length() - 1 ? 0 : index + 1);
			EdgeTree::iterator leftEdge = crossoverTree.end();
			EdgeTree::iterator rightEdge = crossoverTree.end();

			if(point.mdType == Path::Above)
			{
				if(path.points[nextIndex].x > point.x)
				{
					rightEdge = crossoverTree.emplace(point.x,&path.edges[nextIndex]);
					leftEdge  = crossoverTree.emplace(point.x,&path.edges[index]);
				}
				else
				{
					rightEdge = crossoverTree.emplace(point.x,&path.edges[index]);
					leftEdge  = crossoverTree.emplace(point.x,&path.edges[nextIndex]);
				}
			}
			else if(point.mdType == Path::Below)
			{
				EdgeTree::iterator prevEdgeIt = crossoverTree.lower_bound(path.points[prevIndex].x);
				while(prevEdgeIt != crossoverTree.end() 
					&& prevEdgeIt->first == path.points[prevIndex].x 
					&& prevEdgeIt->second->index != index)
				{
					prevEdgeIt++;
				}
				EdgeTree::iterator nextEdgeIt = crossoverTree.lower_bound(path.points[nextIndex].x);
				while(nextEdgeIt != crossoverTree.end() 
					&& nextEdgeIt->first == path.points[nextIndex].x 
					&& nextEdgeIt->second->index != nextIndex)
				{
					nextEdgeIt++;
				}

				if(prevEdgeIt != crossoverTree.end() && prevEdgeIt->second->index == index) 
					crossoverTree.erase(prevEdgeIt); 
				else 
					if(IsDebuggerPresent()) __debugbreak();

				if(nextEdgeIt != crossoverTree.end() && nextEdgeIt->second->index == nextIndex) 
					crossoverTree.erase(nextEdgeIt); 
				else 
					if(IsDebuggerPresent()) __debugbreak();
			}
			else if(point.mdType == Path::Neither)
			{
				if(path.edges[index].angle < 0.0f) // NEGATIVE UPWARD (LEFT)
				{
					EdgeTree::iterator nextEdgeIt = crossoverTree.lower_bound(path.points[nextIndex].x);
					while(nextEdgeIt != crossoverTree.end() 
						&& nextEdgeIt->first == path.points[nextIndex].x 
						&& nextEdgeIt->second->index != nextIndex)
					{
						nextEdgeIt++;
					}
					if(nextEdgeIt != crossoverTree.end() && nextEdgeIt->second->index == nextIndex) 
						crossoverTree.erase(nextEdgeIt); 
					else 
						if(IsDebuggerPresent()) __debugbreak();

					if(path.points[prevIndex].x > point.x)
					{
						rightEdge = crossoverTree.emplace(point.x,&path.edges[index]);
					}
					else
					{
						leftEdge = crossoverTree.emplace(point.x,&path.edges[index]);
					}
				}
				else // POSITIVE DOWNWARD (RIGHT)
				{
					EdgeTree::iterator prevEdgeIt = crossoverTree.lower_bound(path.points[prevIndex].x);
					while(prevEdgeIt != crossoverTree.end() 
						&& prevEdgeIt->first == path.points[prevIndex].x 
						&& prevEdgeIt->second->index != index)
					{
						prevEdgeIt++;
					}
					if(prevEdgeIt != crossoverTree.end() && prevEdgeIt->second->index == index) 
						crossoverTree.erase(prevEdgeIt); 
					else 
						if(IsDebuggerPresent()) __debugbreak();

					if(path.points[nextIndex].x > point.x)
					{
						rightEdge = crossoverTree.emplace(point.x,&path.edges[nextIndex]);
					}
					else
					{
						leftEdge = crossoverTree.emplace(point.x,&path.edges[nextIndex]);
					}
				}
			}

			//EdgeTree::iterator edge1 = crossoverTree.find(point.x);
			//EdgeTree::iterator edge2 = edge1;

			//if(edge1 != crossoverTree.end()) edge2 ++;

			//unsigned belowIndex = 0;
			////unsigned aboveIndex = 0;
			//if(leftEdge != crossoverTree.end()) // upward
			//{
			//	unsigned edgeIndex = leftEdge->second->index;
			//	if(leftEdge->second->angle < 0.0f)
			//	{
			//		//aboveIndex = edgeIndex;
			//		belowIndex = edgeIndex == 0 ? path.points.size() - 1 : edgeIndex - 1;
			//	}
			//	else
			//	{
			//		//aboveIndex = edgeIndex == 0 ? path.points.size() - 1 : edgeIndex - 1;
			//		belowIndex = edgeIndex;
			//	}
			//}

			if(leftEdge != crossoverTree.end())// && aboveIndex == index)
			{
				//EdgeTree::iterator leftEdgeNeighbour = leftEdge;
				//if(path.points[belowIndex].x < point.x)
				//{
				//	leftEdgeNeighbour++;
				//}
				//else
				//{
				//	nextEdgeNeighbour++;
				//}

				EdgeTree::iterator it = crossoverTree.begin();

				for(; it != crossoverTree.end(); it++)
				{
					if(it != leftEdge)
					{
						double intersectX = 0.0f;
						double intersectY = 0.0f;
						if(path.GetIntersection(leftEdge->second->index,it->second->index,intersectX,intersectY))
						{
							FixPathCrossover(intersectX,intersectY,leftEdge->second->index,it->second->index,path);
							possibleCrossovers = true;
							break;
						}
					}
				}
				if(possibleCrossovers) break;
			}

			//if(rightEdge != crossoverTree.end()) // upward
			//{
			//	unsigned edgeIndex = rightEdge->second->index;
			//	if(rightEdge->second->angle < 0.0f)
			//	{
			//		//aboveIndex = edgeIndex;
			//		belowIndex = edgeIndex == 0 ? path.points.size() - 1 : edgeIndex - 1;
			//	}
			//	else
			//	{
			//		//aboveIndex = edgeIndex == 0 ? path.points.size() - 1 : edgeIndex - 1;
			//		belowIndex = edgeIndex;
			//	}
			//}

			if(rightEdge != crossoverTree.end())// && aboveIndex == index)
			{
				//EdgeTree::iterator rightEdgeNeighbour = rightEdge;
				//if(path.points[belowIndex].x > point.x)
				//{
				//	prevEdgeNeighbour--;
				//}
				//else
				//{
				//	rightEdgeNeighbour--;
				//}

				EdgeTree::iterator it = crossoverTree.begin();

				for(; it != crossoverTree.end(); it++)
				{
					if(it != rightEdge)
					{
						double intersectX = 0.0f;
						double intersectY = 0.0f;
						if(path.GetIntersection(rightEdge->second->index,it->second->index,intersectX,intersectY))
						{
							FixPathCrossover(intersectX,intersectY,it->second->index,rightEdge->second->index,path);
							possibleCrossovers = true;
							break;
						}
					}
				}
				if(possibleCrossovers) break;
			}
			if(i == path.Length() - 1)
			{
				if(crossoverTree.size() > 0) 
				{
					if(IsDebuggerPresent()) __debugbreak();
					//return 0; // An edge has been missed! All edges should be cleared when finished
				}
			}
		}
	}
}

LocalMesh * GeoBuilder::CombineLocalMeshes(LocalMesh ** meshes, unsigned numMeshes)
{
	unsigned numVertices = 0;
	unsigned numIndices = 0;

	for(unsigned i = 0; i < numMeshes; ++i)
	{
		numVertices += meshes[i]->vertexBuffer->GetLength();
		numIndices += meshes[i]->GetNumIndices();
	}

	VertexBuffer<Vertex_Pos> * vb = new VertexBuffer<Vertex_Pos>(numVertices);
	unsigned * ib = new unsigned[numIndices];

	unsigned vertexOffset = 0;
	unsigned indexOffset = 0;

	for(unsigned i = 0; i < numMeshes; i++)
	{
		if(meshes[i] != 0)
		{
			if(meshes[i]->vertexBuffer->GetVertexType() == VertexType_Pos)
			{
				VertexBuffer<Vertex_Pos> * miv = static_cast<VertexBuffer<Vertex_Pos>*>(meshes[i]->vertexBuffer);

				for(unsigned j = 0; j < miv->GetLength(); j++)
				{
					Vertex_Pos & vtx = miv->Get(j);
					vb->Set(vertexOffset+j,vtx);
				}

				for(unsigned k = 0; k < meshes[i]->GetNumIndices(); k++)
				{
					ib[indexOffset+k] = meshes[i]->indexBuffer[k] + vertexOffset;
				}

				vertexOffset += miv->GetLength();
				indexOffset += meshes[i]->GetNumIndices();
			}
			delete meshes[i];
		}
	}

	delete[] meshes;

	return new LocalMesh(vb,ib,numIndices);
}

LocalMesh * GeoBuilder::BuildDiagonalDebugMesh(Path & path, std::vector<Diagonal> & diagonals)
{
	LocalMesh ** meshes = new LocalMesh*[diagonals.size() + 1];
	for(unsigned i = 0; i < diagonals.size(); ++i)
	{
		Path::Point diagonalPoints[2];
		diagonalPoints[0] = path.points[diagonals[i].a];
		diagonalPoints[1] = path.points[diagonals[i].b];
		meshes[i] = BuildStroke(diagonalPoints,2,2.0f);
	}
		
	meshes[diagonals.size()] = BuildStroke(path.points.data(),path.points.size(),2.0f); // outline

	return CombineLocalMeshes(meshes,diagonals.size() + 1);
}

LocalMesh * GeoBuilder::BuildMonotoneDebugMesh(Path & path, std::vector<unsigned> * monotones, unsigned numMonotones)
{
	LocalMesh ** meshes = new LocalMesh*[numMonotones];
	for(unsigned i = 0; i < numMonotones; i++)
	{
		if(monotones[i].size() > 2)
		{
			Path::Point * monotonePoints = new Path::Point[monotones[i].size()];
			for(unsigned j = 0; j < monotones[i].size(); j++)
			{
				monotonePoints[j] = path.points[monotones[i][j]];
			}
			meshes[i] = BuildStroke(monotonePoints,monotones[i].size(),2.0f);
			delete[] monotonePoints;
		}
		else
		{
			meshes[i] = 0;
		}
	}

	delete[] monotones;

	return CombineLocalMeshes(meshes,numMonotones);
}

inline float winding(Path & path)
{
	return path.IsClockwise() ? 1.0f : -1.0f;
}

void GeoBuilder::SweepDiagonals(Path & path, std::vector<Diagonal> & diagonals)
{
	std::vector<unsigned> sortedIndices;
	GenerateSortedIndices(path, sortedIndices);

	EdgeTree edgeTree;

	for(unsigned i = 0; i < path.Length(); ++i)
	{
		unsigned index = sortedIndices[i];
		Path::Point & point = path.points[index];
		unsigned prevIndex = (index == 0 ? path.Length() - 1 : index - 1);
		unsigned nextIndex = (index == path.Length() - 1 ? 0 : index + 1);
		
		if(point.mdType == Path::Above)
		{
			if(path.PointIsConcave(&point))
			{
				// Point is a SPLIT point, need to eliminate!
				EdgeTree::iterator it = edgeTree.lower_bound(point.x * winding(path));
				if(it != edgeTree.end())
				{
					diagonals.emplace_back(index, it->second->helperIndex);
					it->second->helperIndex = index;
				}
				
				path.edges[index].helperIndex = index;
				if(edgeTree.find(path.edges[index].xPos * winding(path)) != edgeTree.end())
					path.edges[index].xPos -= FLT_EPSILON * 1000.0f;
				edgeTree.emplace(path.edges[index].xPos * winding(path), &path.edges[index]);
			}
			else
			{
				// Point is a START point, add edge
				path.edges[index].helperIndex = index;
				if(edgeTree.find(path.edges[index].xPos * winding(path)) != edgeTree.end())
					path.edges[index].xPos -= FLT_EPSILON * 1000.0f;
				edgeTree.emplace(path.edges[index].xPos * winding(path), &path.edges[index]);
			}
		}
		else if(point.mdType == Path::Below)
		{
			if(path.PointIsConcave(&point))
			{
				// Point is a MERGE point, need to eliminate!
				unsigned helperIndex = path.edges[nextIndex].helperIndex;
				if(helperIndex != UINT_MAX)
				{
					Path::Point * helper = &path.points[helperIndex];
					if(helper->mdType == Path::Below && path.PointIsConcave(helper))
					{
						diagonals.emplace_back(index, helperIndex);
					}
				}
				edgeTree.erase(path.edges[nextIndex].xPos * winding(path));
				EdgeTree::iterator it = edgeTree.lower_bound(point.x * winding(path));
				if(it != edgeTree.end())
				{
					unsigned helperIndex = it->second->helperIndex;
					if(helperIndex != UINT_MAX)
					{
						Path::Point * helper = &path.points[helperIndex];
						if(helper->mdType == Path::Below && path.PointIsConcave(helper))
						{
							diagonals.emplace_back(index,helperIndex);
						}
					}
					it->second->helperIndex = index;
				}
			}
			else
			{
				// Point is an END point, remove edge
				unsigned helperIndex = path.edges[nextIndex].helperIndex;
				if(helperIndex != UINT_MAX)
				{
					Path::Point * helper = &path.points[helperIndex];
					if(helper->mdType == Path::Below && path.PointIsConcave(helper))
					{
						diagonals.emplace_back(index,path.edges[nextIndex].helperIndex);
					}
				}
				edgeTree.erase(path.edges[nextIndex].xPos * winding(path));
			}
		}
		else
		{
			if(path.edges[index].angle < 0.0f) // NEGATIVE UPWARD (LEFT)
			{
				unsigned helperIndex = path.edges[nextIndex].helperIndex;
				if(helperIndex != UINT_MAX)
				{
					Path::Point * helper = &path.points[helperIndex];
					if(helper->mdType == Path::Below && path.PointIsConcave(helper))
					{
						diagonals.emplace_back(index,helperIndex);
					}
				}
				edgeTree.erase(path.edges[nextIndex].xPos * winding(path));
				path.edges[index].helperIndex = index;
				if(edgeTree.find(path.edges[index].xPos * winding(path)) != edgeTree.end())
					path.edges[index].xPos -= FLT_EPSILON * 1000.0f;
				edgeTree.emplace(path.edges[index].xPos * winding(path), &path.edges[index]);
			}
			else // POSITIVE DOWNWARD (RIGHT)
			{
				EdgeTree::iterator it = edgeTree.lower_bound(point.x * winding(path));
				if(it != edgeTree.end())
				{
					unsigned helperIndex = it->second->helperIndex;
					if(helperIndex != UINT_MAX)
					{
						Path::Point * helper = &path.points[helperIndex];
						if(helper->mdType == Path::Below && path.PointIsConcave(helper))
						{
							diagonals.emplace_back(index,helperIndex);
						}
					}
					it->second->helperIndex = index;
				}
			}
		}
	}

	if(edgeTree.size() > 0) 
	{
		if(IsDebuggerPresent()) __debugbreak(); // An edge has been missed! All edges should be cleared when finished
	}
}

unsigned GeoBuilder::BuildMonotones(Path & path, std::vector<Diagonal> & diagonals, std::vector<unsigned> * monotones)
{
	unsigned numMonotones = diagonals.size() + 1;

	for(unsigned i = 0; i < diagonals.size(); ++i)
	{
		if(diagonals[i].a > diagonals[i].b) 
		{
			unsigned temp = diagonals[i].a;
			diagonals[i].a = diagonals[i].b;
			diagonals[i].b = temp;
		}
	}

	std::sort(diagonals.begin(), diagonals.end());

	if(diagonals.size() > 0)
	{
		unsigned finalA = diagonals[diagonals.size()-1].b;
		unsigned finalB = diagonals[diagonals.size()-1].a;
		diagonals.emplace_back(finalA, finalB);
	}
	else
	{
		diagonals.emplace_back(0,path.Length()-1);
	}

	for(unsigned i = 0; i < numMonotones; ++i)
	{
		unsigned currentIndex = diagonals[i].a;
		monotones[i].push_back(currentIndex);
		while(currentIndex != diagonals[i].b)
		{
			currentIndex = path.points[currentIndex].next;
			monotones[i].push_back(currentIndex);

			if(monotones[i].size() > 10000)
			{
				if(IsDebuggerPresent()) __debugbreak();
				numMonotones = i;
				break;
			}
			if(currentIndex == diagonals[i].a) break;
		}
		path.points[diagonals[i].a].next = diagonals[i].b;
	}

	return numMonotones;
}

void GeoBuilder::TriangulateMonotone(Path & path, std::vector<unsigned> & monotone, std::vector<Triangle> & triangles)
{
	// http://www.personal.kent.edu/~rmuhamma/Compgeometry/MyCG/PolyPart/polyPartition.htm 

	if(monotone.size() < 3) return;
	if(monotone.size() == 3)
	{
		triangles.emplace_back();
		triangles.back().a = monotone[0];
		triangles.back().b = monotone[1];
		triangles.back().c = monotone[2];
		return;
	}

	std::vector<unsigned> sortedMonotoneIndices;
	for(unsigned j = 0; j < monotone.size(); ++j)
	{
		sortedMonotoneIndices.push_back(j);
	}
	staticPath = &path;
	staticMonotone = &monotone;
	std::sort(sortedMonotoneIndices.begin(), sortedMonotoneIndices.end(), CompareMonotonePathPoints);

	std::list<unsigned> monotoneIndexList;
		
	monotoneIndexList.push_back(sortedMonotoneIndices[0]);
	monotoneIndexList.push_back(sortedMonotoneIndices[1]);

	for(unsigned sortedMonotoneIndex = 2; sortedMonotoneIndex < sortedMonotoneIndices.size() - 1; ++sortedMonotoneIndex)
	{
		unsigned monotoneIndex = sortedMonotoneIndices[sortedMonotoneIndex];
		unsigned comparatorIndex = monotoneIndexList.back();

		unsigned nextMonotoneIndex = monotoneIndex == 0 ? monotone.size()-1 : monotoneIndex-1;
		unsigned nextComparatorIndex = comparatorIndex == 0 ? monotone.size()-1 : comparatorIndex-1;

		Path::Point & point = path.points[monotone[monotoneIndex]];
		Path::Point & prevPoint = path.points[monotone[nextMonotoneIndex]];

		Path::Point & comparator = path.points[monotone[comparatorIndex]];
		Path::Point & prevComparator = path.points[monotone[nextComparatorIndex]];

		double pointOffset = (point.y - prevPoint.y);
		double comparatorOffset = (comparator.y - prevComparator.y);
			
		while(pointOffset == 0.0f)
		{
			point.y += FLT_EPSILON * 1000.0f;
			pointOffset = (point.y - prevPoint.y);
		}
		while(comparatorOffset == 0.0f)
		{
			comparator.y += FLT_EPSILON * 1000.0f;
			comparatorOffset = (comparator.y - prevComparator.y);
		}

		bool pointVerticality = pointOffset > 0.0f;
		bool comparatorVerticality = comparatorOffset > 0.0f;

		if(pointVerticality != comparatorVerticality)
		{
			// Different chains
			while(monotoneIndexList.size() > 1)
			{
				triangles.emplace_back();
				triangles.back().a = monotone[monotoneIndex];
				triangles.back().b = monotone[monotoneIndexList.front()];
				monotoneIndexList.pop_front();
				triangles.back().c = monotone[monotoneIndexList.front()];
			}
		}
		else
		{
			// Same chain
			unsigned temp = monotoneIndexList.back();
			unsigned current = monotone[monotoneIndex];
			unsigned prev = monotone[monotoneIndexList.back()];
			monotoneIndexList.pop_back();
			unsigned prevPrev = monotone[monotoneIndexList.back()];
			monotoneIndexList.push_back(temp);
			double angle = path.Angle(prevPrev,prev,current,true);

			while((path.IsClockwise() ? angle > 0.0f : angle < 0.0f) && monotoneIndexList.size() > 1)
			{
				triangles.emplace_back();
				triangles.back().a = current;
				triangles.back().c = prev;
				triangles.back().b = prevPrev;
				monotoneIndexList.pop_back();

				if(monotoneIndexList.size() > 1)
				{
					temp = monotoneIndexList.back();
					prev = monotone[monotoneIndexList.back()];
					monotoneIndexList.pop_back();
					prevPrev = monotone[monotoneIndexList.back()];
					monotoneIndexList.push_back(temp);
					angle = path.Angle(prevPrev,prev,current,true);
				}
			}
		}

		monotoneIndexList.push_back(monotoneIndex);
	}

	while(monotoneIndexList.size() > 1)
	{
		triangles.emplace_back();
		triangles.back().a = monotone[sortedMonotoneIndices.back()];
		triangles.back().b = monotone[monotoneIndexList.front()];
		monotoneIndexList.pop_front();
		triangles.back().c = monotone[monotoneIndexList.front()];
	}
}

LocalMesh * GeoBuilder::BuildCube(bool texCoords)
{
	IVertexBuffer * ivb = 0;
	if(texCoords)
	{
		VertexBuffer<Vertex_PosNorTex> * vb = new VertexBuffer<Vertex_PosNorTex>(24);

		// Fill in the front face vertex data.
		vb->Set(0, Vertex_PosNorTex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));
		vb->Set(1, Vertex_PosNorTex(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));
		vb->Set(2, Vertex_PosNorTex(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));
		vb->Set(3, Vertex_PosNorTex(1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));

		// Fill in the back face vertex data.
		vb->Set(4, Vertex_PosNorTex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));
		vb->Set(5, Vertex_PosNorTex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));
		vb->Set(6, Vertex_PosNorTex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
		vb->Set(7, Vertex_PosNorTex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));

		// Fill in the top face vertex data.
		vb->Set(8, Vertex_PosNorTex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f));
		vb->Set(9, Vertex_PosNorTex(-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f));
		vb->Set(10, Vertex_PosNorTex(1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f));
		vb->Set(11, Vertex_PosNorTex(1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f));

		// Fill in the bottom face vertex data.
		vb->Set(12, Vertex_PosNorTex(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f));
		vb->Set(13, Vertex_PosNorTex(1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f));
		vb->Set(14, Vertex_PosNorTex(1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f));
		vb->Set(15, Vertex_PosNorTex(-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f));

		// Fill in the left face vertex data.
		vb->Set(16, Vertex_PosNorTex(-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
		vb->Set(17, Vertex_PosNorTex(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
		vb->Set(18, Vertex_PosNorTex(-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
		vb->Set(19, Vertex_PosNorTex(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));

		// Fill in the right face vertex data.
		vb->Set(20, Vertex_PosNorTex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
		vb->Set(21, Vertex_PosNorTex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
		vb->Set(22, Vertex_PosNorTex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
		vb->Set(23, Vertex_PosNorTex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));

		ivb = vb;
	}
	else
	{
		VertexBuffer<Vertex_PosNor> * vb = new VertexBuffer<Vertex_PosNor>(24);

		// Fill in the front face vertex data.
		vb->Set(0, Vertex_PosNor(-1.0f,-1.0f, -1.0f, 0.0f, 0.0f, -1.0f));
		vb->Set(1, Vertex_PosNor(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f));
		vb->Set(2, Vertex_PosNor(1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f));
		vb->Set(3, Vertex_PosNor(1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f));

		// Fill in the back face vertex data.
		vb->Set(4, Vertex_PosNor(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f));
		vb->Set(5, Vertex_PosNor( 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f));
		vb->Set(6, Vertex_PosNor( 1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f));
		vb->Set(7, Vertex_PosNor(-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f));

		// Fill in the top face vertex data.
		vb->Set(8,  Vertex_PosNor(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f));
		vb->Set(9,  Vertex_PosNor(-1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f));
		vb->Set(10, Vertex_PosNor( 1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f));
		vb->Set(11, Vertex_PosNor( 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f));

		// Fill in the bottom face vertex data.
		vb->Set(12, Vertex_PosNor(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f));
		vb->Set(13, Vertex_PosNor( 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f));
		vb->Set(14, Vertex_PosNor( 1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f));
		vb->Set(15, Vertex_PosNor(-1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f));

		// Fill in the left face vertex data.
		vb->Set(16, Vertex_PosNor(-1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f));
		vb->Set(17, Vertex_PosNor(-1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f));
		vb->Set(18, Vertex_PosNor(-1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f));
		vb->Set(19, Vertex_PosNor(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f));

		// Fill in the right face vertex data.
		vb->Set(20, Vertex_PosNor(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f));
		vb->Set(21, Vertex_PosNor(1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f));
		vb->Set(22, Vertex_PosNor(1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f));
		vb->Set(23, Vertex_PosNor(1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f));

		ivb = vb;
	}

	// Write box indices to the index buffer.
	unsigned * i = new unsigned[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7]  = 5; i[8]  = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] =  9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	return new LocalMesh(ivb, i, 36);
}

LocalMesh * GeoBuilder::BuildSkyCube()
{
	VertexBuffer<Vertex_Pos> * vb = new VertexBuffer<Vertex_Pos>(24);

	// Fill in the front face vertex data.
	vb->Set(0, Vertex_Pos(-1.0f, -1.0f, -1.0f));
	vb->Set(1, Vertex_Pos(-1.0f, 1.0f, -1.0f));
	vb->Set(2, Vertex_Pos(1.0f, 1.0f, -1.0f));
	vb->Set(3, Vertex_Pos(1.0f, -1.0f, -1.0f));

	// Fill in the back face vertex data.
	vb->Set(4, Vertex_Pos(-1.0f, -1.0f, 1.0f));
	vb->Set(5, Vertex_Pos(1.0f, -1.0f, 1.0f));
	vb->Set(6, Vertex_Pos(1.0f, 1.0f, 1.0f));
	vb->Set(7, Vertex_Pos(-1.0f, 1.0f, 1.0f));

	// Fill in the top face vertex data.
	vb->Set(8, Vertex_Pos(-1.0f, 1.0f, -1.0f));
	vb->Set(9, Vertex_Pos(-1.0f, 1.0f, 1.0f));
	vb->Set(10, Vertex_Pos(1.0f, 1.0f, 1.0f));
	vb->Set(11, Vertex_Pos(1.0f, 1.0f, -1.0f));

	// Fill in the bottom face vertex data.
	vb->Set(12, Vertex_Pos(-1.0f, -1.0f, -1.0f));
	vb->Set(13, Vertex_Pos(1.0f, -1.0f, -1.0f));
	vb->Set(14, Vertex_Pos(1.0f, -1.0f, 1.0f));
	vb->Set(15, Vertex_Pos(-1.0f, -1.0f, 1.0f));

	// Fill in the left face vertex data.
	vb->Set(16, Vertex_Pos(-1.0f, -1.0f, 1.0f));
	vb->Set(17, Vertex_Pos(-1.0f, 1.0f, 1.0f));
	vb->Set(18, Vertex_Pos(-1.0f, 1.0f, -1.0f));
	vb->Set(19, Vertex_Pos(-1.0f, -1.0f, -1.0f));

	// Fill in the right face vertex data.
	vb->Set(20, Vertex_Pos(1.0f, -1.0f, -1.0f));
	vb->Set(21, Vertex_Pos(1.0f, 1.0f, -1.0f));
	vb->Set(22, Vertex_Pos(1.0f, 1.0f, 1.0f));
	vb->Set(23, Vertex_Pos(1.0f, -1.0f, 1.0f));

	// Write box indices to the index buffer.
	unsigned * i = new unsigned[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	return new LocalMesh(vb, i, 36);
}

LocalMesh * GeoBuilder::BuildCylinder(float radius, float length, unsigned sectors, unsigned stacks, bool texCoords)
{
	LocalMesh * tube = new LocalMesh();
	tube->vertexBuffer = BuildVertexBufferTube(length, radius, sectors, stacks);
	tube->indexBuffer = BuildIndexBufferTube(sectors, stacks, tube->numTriangles);

	LocalMesh * cap = new LocalMesh();
	cap->vertexBuffer = BuildVertexBufferDisc(radius, sectors);
	cap->indexBuffer = BuildIndexBufferDisc(sectors, cap->numTriangles);

	LocalMesh * cap2 = new LocalMesh();
	cap2->vertexBuffer = BuildVertexBufferDisc(radius, sectors);
	cap2->indexBuffer = BuildIndexBufferDisc(sectors, cap2->numTriangles);

	glm::mat4 moveMatrix = glm::translate(glm::vec3(0.0f, 0.0f, length * -0.5f));
	cap->vertexBuffer->Transform(moveMatrix);

	moveMatrix = glm::translate(glm::vec3(0.0f, 0.0f, length * 0.5f)) * glm::eulerAngleY(float(M_PI));
	cap2->vertexBuffer->Transform(moveMatrix);

	LocalMesh * openTube = tube->CombineWith(cap);
	LocalMesh * cylinder = openTube->CombineWith(cap2);
	
	delete tube;
	delete cap;
	delete cap2;
	delete openTube;

	return cylinder;
}

LocalMesh * GeoBuilder::BuildSphere(float radius, unsigned sectors, unsigned stacks, bool texCoords)
{
	LocalMesh * hemi1 = new LocalMesh();
	hemi1->vertexBuffer = BuildVertexBufferHemisphere(radius, sectors, stacks);
	hemi1->indexBuffer = BuildIndexBufferHemisphere(sectors, stacks, hemi1->numTriangles);

	LocalMesh * hemi2 = new LocalMesh();
	hemi2->vertexBuffer = BuildVertexBufferHemisphere(radius, sectors, stacks);
	hemi2->indexBuffer = BuildIndexBufferHemisphere(sectors, stacks, hemi2->numTriangles);

	glm::mat4 moveMatrix = glm::eulerAngleY(float(M_PI));
	hemi2->vertexBuffer->Transform(moveMatrix);

	LocalMesh * sphere = hemi1->CombineWith(hemi2);

	delete hemi1;
	delete hemi2;

	return sphere;
}

LocalMesh * GeoBuilder::BuildCapsule(float radius, float length, unsigned sectors, unsigned stacks)
{
	LocalMesh * tube = new LocalMesh();
	tube->vertexBuffer = BuildVertexBufferTube(length, radius, sectors, stacks);
	tube->indexBuffer = BuildIndexBufferTube(sectors, stacks, tube->numTriangles);

	LocalMesh * cap = new LocalMesh();
	cap->vertexBuffer = BuildVertexBufferHemisphere(radius, sectors, stacks);
	cap->indexBuffer = BuildIndexBufferHemisphere(sectors, stacks, cap->numTriangles);

	LocalMesh * cap2 = new LocalMesh();
	cap2->vertexBuffer = BuildVertexBufferHemisphere(radius, sectors, stacks);
	cap2->indexBuffer = BuildIndexBufferHemisphere(sectors, stacks, cap2->numTriangles);

	glm::mat4 moveMatrix = glm::translate(glm::vec3(0.0f, 0.0f, length * -0.5f));
	cap->vertexBuffer->Transform(moveMatrix);

	moveMatrix = glm::translate(glm::vec3(0.0f, 0.0f, length * 0.5f)) * glm::eulerAngleY(float(M_PI));
	cap2->vertexBuffer->Transform(moveMatrix);

	LocalMesh * testTube = tube->CombineWith(cap);
	LocalMesh * capsule = testTube->CombineWith(cap2);

	delete tube;
	delete cap;
	delete cap2;
	delete testTube;

	moveMatrix = glm::eulerAngleY(float(M_PI / 2.0));
	capsule->vertexBuffer->Transform(moveMatrix);

	return capsule;
}

LocalMesh * GeoBuilder::BuildGrid(float width, float depth, unsigned columns, unsigned rows, Gpu::Rect* textureRect, bool tangents)
{
	IVertexBuffer * v; 

	if(tangents) v = new VertexBuffer<Vertex_PosNorTanTex>(columns * rows);
	else if(textureRect) v = new VertexBuffer<Vertex_PosNorTex>(columns * rows);
	else v = new VertexBuffer<Vertex_PosNor>(columns * rows);

	for(unsigned i = 0; i < rows; i++) {
		for(unsigned j = 0; j < columns; j++) {
			float x = (width * -.5f) + (j * (width/(columns - 1)));
			float z = (depth * -.5f) + (i * (depth/(rows - 1)));

			if(textureRect)
			{
				const float texRectWidth = textureRect->right - textureRect->left;
				const float texRectHeight = textureRect->bottom - textureRect->top;
				float tx = textureRect->left + (j * (texRectWidth/(columns - 1)));
				float ty = textureRect->top + (i * (texRectHeight/(rows - 1)));
				if(tangents)
				{
					VertexBuffer<Vertex_PosNorTanTex> * vv = (VertexBuffer<Vertex_PosNorTanTex> *) v;
					vv->Set((columns * i) + j, Vertex_PosNorTanTex(x, 0.0f, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, tx, ty));
				}
				else
				{
					VertexBuffer<Vertex_PosNorTex> * vv = (VertexBuffer<Vertex_PosNorTex> *) v;
					vv->Set((columns * i) + j, Vertex_PosNorTex(x, 0.0f, z, 0.0f, 1.0f, 0.0f, tx, ty));
				}
			}
			else
			{
				VertexBuffer<Vertex_PosNor> * vv = (VertexBuffer<Vertex_PosNor> *) v;
				vv->Set((columns * i) + j, Vertex_PosNor(x, 0.0f, z, 0.0f, 1.0f, 0.0f));
			}
		}
	}

	unsigned numTriangles = (columns - 1) * (rows - 1) * 2;

	unsigned * k = new unsigned[numTriangles * 3];

	int index = 0;

	for(unsigned i = 0; i < (rows - 1); i++) {
		for(unsigned j = 0; j < (columns - 1); j++) {
			k[index++] = j + (i * columns);
			k[index++] = j + ((i+1) * columns);
			k[index++] = j + 1 + (i * columns);

			k[index++] = j + 1 + (i * columns);
			k[index++] = j + ((i+1) * columns);
			k[index++] = j + 1 + ((i+1) * columns);
		}
	}

	return new LocalMesh(v, k, numTriangles * 3);
}

void GeoBuilder::GenerateNormals(IVertexBuffer *buffer, unsigned numTriangles, unsigned *indexData)
{
	if(!buffer || buffer->GetVertexType() == VertexType_PosCol) return;

	struct VertexNode
	{
		glm::vec3 accumulator;
		VertexNode() : accumulator(0.0f,0.0f,0.0f) {}
	};
	VertexNode *nodes = new VertexNode[buffer->GetLength()];
	glm::vec3 positions[3];

	// For every triangle, generate a normal...
	for(unsigned i = 0; i < numTriangles; ++i)
	{
		// For every index in the triangle
		unsigned index;
		char* datum;
		for(unsigned j = 0; j<3; ++j)
		{
			index = indexData[(i*3) + j];

			// Find its vertex and its world position
			datum = ((char*)buffer->GetData()) + (index * buffer->GetElementSize());
			switch(buffer->GetVertexType())
			{
			case VertexType_PosNor:
				positions[j] = ((Vertex_PosNor*) datum)->position;
				break;
			case VertexType_PosNorTex:
				positions[j] = ((Vertex_PosNorTex*) datum)->position;
				break;
			}
		}

		// Subtract the world positions to find two displacement vectors
		glm::vec3 oneZero = positions[1] - positions[0];
		glm::vec3 twoZero = positions[2] - positions[0];

		// Cross the difference vectors and submit the normal
		glm::vec3 normal = glm::cross(oneZero, twoZero);
		normal = glm::normalize(normal);
		for(unsigned j = 0; j < 3; j++)
		{
			nodes[indexData[(i*3)+j]].accumulator += normal;
		}
	}

	// For every vertex in the vertex buffer
	for(unsigned i = 0; i < buffer->GetLength(); ++i)
	{
		// Normalize
		nodes[i].accumulator = glm::normalize(nodes[i].accumulator);

		// Write the new normal to that vertex
		char* datum = ((char*)buffer->GetData()) + (i * buffer->GetElementSize());
		switch(buffer->GetVertexType())
		{
		case VertexType_PosNor:
			((Vertex_PosNor*) datum)->normal = nodes[i].accumulator;
			break;
		case VertexType_PosNorTex:
			((Vertex_PosNorTex*) datum)->normal = nodes[i].accumulator;
			break;
		}
	}

	delete[] nodes;
}

void GeoBuilder::GenerateTangents(IVertexBuffer *& buffer, unsigned numTriangles, unsigned * indexData)
{
	if(!buffer) return;
	if(buffer->GetVertexType() != VertexType_PosNorTex 
		&& buffer->GetVertexType() != VertexType_PosNorTanTex) return;

	VertexBuffer<Vertex_PosNorTanTex> * newBuffer = new VertexBuffer<Vertex_PosNorTanTex>(buffer->GetLength());

	struct VertexNode
	{
		glm::vec3 tangent;
		glm::vec3 bitangent;
		VertexNode() : tangent(0.0f,0.0f,0.0f), bitangent(0.0f,0.0f,0.0f) {}
	};
	VertexNode *nodes = new VertexNode[buffer->GetLength()];
	glm::vec3 positions[3];
	glm::vec2 texCoords[3];
	glm::vec3 normals[3];

	for(unsigned i = 0; i < numTriangles; i++)
	{
		// For every index in the triangle
		unsigned index;
		char* datum;
		for(unsigned j = 0; j<3; ++j)
		{
			index = indexData[(i*3) + j];

			// Find its vertex and its world position
			datum = ((char*)buffer->GetData()) + (index * buffer->GetElementSize());
			switch(buffer->GetVertexType())
			{
			case VertexType_PosNorTex:
				{
					Vertex_PosNorTex * item = (Vertex_PosNorTex*) datum;
					positions[j] = item->position;
					texCoords[j] = item->texCoord;
					normals[j] =   item->normal;
				}
				break;
			case VertexType_PosNorTanTex:
				{
					Vertex_PosNorTanTex * item = (Vertex_PosNorTanTex*) datum;
					positions[j] = item->position;
					texCoords[j] = item->texCoord;
					normals[j] =   item->normal;
				}
				break;
			}
		}

		float x1 = positions[1].x - positions[0].x;
		float x2 = positions[2].x - positions[0].x;
		float y1 = positions[1].y - positions[0].y;
		float y2 = positions[2].y - positions[0].y;
		float z1 = positions[1].z - positions[0].z;
		float z2 = positions[2].z - positions[0].z;

		glm::vec3 one = positions[1] - positions[0];
		glm::vec3 two = positions[2] - positions[0];

		glm::vec3 triNormal = glm::cross(one, two);
		triNormal = glm::normalize(triNormal);

		float s1 = texCoords[1].x - texCoords[0].x;
		float s2 = texCoords[2].x - texCoords[0].x;
		float t1 = texCoords[1].y - texCoords[0].y;
		float t2 = texCoords[2].y - texCoords[0].y;

		glm::vec2 tex1 = texCoords[1] - texCoords[0];
		glm::vec2 tex2 = texCoords[2] - texCoords[0];

		float rad = 1.0f/((tex1.x * tex2.y) - (tex2.x * tex1.y));

		float r = 1.0f / ((s1 * t2) - (s2 * t1));

		glm::vec3 sdir(
			((t2 * x1) - (t1 * x2)) * r,
			((t2 * y1) - (t1 * y2)) * r,
			((t2 * z1) - (t1 * z2)) * r);

		glm::vec3 tdir(
			((s1 * x2) - (s2 * x1)) * r,
			((s1 * y2) - (s2 * y1)) * r,
			((s1 * z2) - (s2 * z1)) * r);

		glm::vec3 tangent(
			rad * ((tex2.y * one.x) - (tex1.y * two.x)),
			rad * ((tex2.y * one.y) - (tex1.y * two.y)),
			rad * ((tex2.y * one.z) - (tex1.y * two.z)));

		glm::vec3 bitangent(
			rad * ((tex1.x * two.x) - (tex2.x * one.x)),
			rad * ((tex1.x * two.y) - (tex2.x * one.y)),
			rad * ((tex1.x * two.z) - (tex2.x * one.z)));

		for(unsigned j = 0; j < 3; j++)
		{
			nodes[indexData[(i*3)+j]].tangent += sdir;

			nodes[indexData[(i*3)+j]].bitangent += tdir;
		}
	}

	// For every vertex in the vertex buffer
	for(unsigned i = 0; i < buffer->GetLength(); ++i)
	{
		float tanChirality; // 1.0f or -1.0f
		glm::vec3 inputBitangent = nodes[indexData[i]].bitangent;
		char* datum = ((char*)buffer->GetData()) + (i * buffer->GetElementSize());
		Vertex_PosNorTanTex* output = ((Vertex_PosNorTanTex*)newBuffer->GetData()) + (i);
		Vertex_PosNorTex input;

		switch(buffer->GetVertexType())
		{
		case VertexType_PosNorTex:
			{
				Vertex_PosNorTex * item = (Vertex_PosNorTex*) datum;
				input.normal = item->normal;
				input.position = item->position;
				input.texCoord = item->texCoord;
			}
			break;
		case VertexType_PosNorTanTex:
			{
				Vertex_PosNorTanTex * item = (Vertex_PosNorTanTex*) datum;
				input.normal = item->normal;
				input.position = item->position;
				input.texCoord = item->texCoord;
			}
			break;
		}

		glm::vec3 n = input.normal;
		glm::vec3 t = nodes[i].tangent;
		glm::vec3 outputTangent = t - (n * glm::dot(n, t));
		outputTangent = glm::normalize(outputTangent);

		glm::vec3 bitangent = glm::cross(n,t);

		tanChirality = ( glm::dot(bitangent, inputBitangent) < 0.0f ? -1.0f : 1.0f );

		output->position = input.position;
		output->normal = input.normal;
		output->tangent = outputTangent;
		output->tanChirality = tanChirality;
		output->texCoord = input.texCoord;
	}

	delete[] nodes;
	delete buffer;
	buffer = newBuffer;
}

void GeoBuilder::GenerateRotationalTangents(IVertexBuffer *& buffer, unsigned numTriangles, unsigned * indexData)
{
	// My own algorithm for generating tangents for an arbitrary mesh.
	// Not as foolproof as the official example above, but works reasonably well

	if(!buffer) return;
	if(buffer->GetVertexType() != VertexType_PosNorTex 
		&& buffer->GetVertexType() != VertexType_PosNorTanTex) return;

	VertexBuffer<Vertex_PosNorTanTex> * newBuffer = new VertexBuffer<Vertex_PosNorTanTex>(buffer->GetLength());

	struct VertexNode
	{
		glm::vec3 tangent;
		glm::vec3 bitangent;
		VertexNode() : tangent(0.0f,0.0f,0.0f), bitangent(0.0f,0.0f,0.0f) {}
	};
	VertexNode *nodes = new VertexNode[buffer->GetLength()];
	glm::vec3 positions[3];
	glm::vec2 texCoords[3];
	glm::vec3 normals[3];

	for(unsigned i = 0; i < numTriangles; i++)
	{
		// For every index in the triangle
		unsigned index;
		char* datum;
		for(unsigned j = 0; j<3; ++j)
		{
			index = indexData[(i*3) + j];

			// Find its vertex and its world position
			datum = ((char*)buffer->GetData()) + (index * buffer->GetElementSize());
			switch(buffer->GetVertexType())
			{
			case VertexType_PosNorTex:
				{
					Vertex_PosNorTex * item = (Vertex_PosNorTex*) datum;
					positions[j] = item->position;
					texCoords[j] = item->texCoord;
					normals[j] =   item->normal;
				}
				break;
			case VertexType_PosNorTanTex:
				{
					Vertex_PosNorTanTex * item = (Vertex_PosNorTanTex*) datum;
					positions[j] = item->position;
					texCoords[j] = item->texCoord;
					normals[j] =   item->normal;
				}
				break;
			}
		}

		glm::vec3 one = positions[1] - positions[0];
		glm::vec3 two = positions[2] - positions[0];

		glm::vec2 tex1 = texCoords[1] - texCoords[0];
		glm::vec2 tex2 = texCoords[2] - texCoords[0];
		glm::vec2 xAxis(1.0f,0.0f);

		float angle = atan2f(tex1.y,tex1.x);
		float angle2 = atan2f(tex2.y,tex2.x);

		glm::vec3 onePerp = glm::cross(normals[0],one);
		onePerp = glm::cross(onePerp,normals[0]);

		glm::vec3 twoPerp = glm::cross(normals[0],two);
		twoPerp = glm::cross(twoPerp,normals[0]);

		//float dotProd = glm::dot(onePerp,normals[0]);// should be zero!

		glm::vec3 tangent = glm::rotate(onePerp,angle,normals[0]);
		tangent = glm::normalize(tangent);

		glm::vec3 tangent2 = glm::rotate(twoPerp,angle2,normals[0]);
		tangent2 = glm::normalize(tangent2);

		for(unsigned j = 0; j < 3; j++)
		{
			nodes[indexData[(i*3)+j]].tangent += (tangent + tangent2);

			//nodes[indexData[(i*3)+j]].bitangent += tdir;
		}

		//D3DXVec3Normalize(&t,&onePerp);
		//nodes[indexData[(i*3)]].tangent = tangent;
	}

	// For every vertex in the vertex buffer
	for(unsigned i = 0; i < buffer->GetLength(); ++i)
	{
		float tanChirality; // 1.0f or -1.0f
		glm::vec3 inputBitangent = nodes[indexData[i]].bitangent;
		char* datum = ((char*)buffer->GetData()) + (i * buffer->GetElementSize());
		Vertex_PosNorTanTex* output = ((Vertex_PosNorTanTex*)newBuffer->GetData()) + (i);
		Vertex_PosNorTex input;

		switch(buffer->GetVertexType())
		{
		case VertexType_PosNorTex:
			{
				Vertex_PosNorTex * item = (Vertex_PosNorTex*) datum;
				input.normal = item->normal;
				input.position = item->position;
				input.texCoord = item->texCoord;
			}
			break;
		case VertexType_PosNorTanTex:
			{
				Vertex_PosNorTanTex * item = (Vertex_PosNorTanTex*) datum;
				input.normal = item->normal;
				input.position = item->position;
				input.texCoord = item->texCoord;
			}
			break;
		}

		glm::vec3 n = input.normal;
		glm::vec3 t = nodes[i].tangent;
		glm::vec3 outputTangent = t - (n * glm::dot(n, t));
		outputTangent = glm::normalize(outputTangent);

		//D3DXVECTOR3 bitangent;
		//D3DXVec3Cross(&bitangent, &n, &outputTangent);

		tanChirality = 1.0f; //( D3DXVec3Dot(&bitangent, &inputBitangent) < 0.0f ? -1.0f : 1.0f );

		output->position = input.position;
		output->normal = input.normal;
		output->tangent = outputTangent;
		output->tanChirality = tanChirality;
		output->texCoord = input.texCoord;
	}

	delete[] nodes;
	delete buffer;
	buffer = newBuffer;
}

Gpu::BoundingBox GeoBuilder::GenerateBoundingBox(IVertexBuffer * buffer)
{
	Gpu::BoundingBox boundingBox;

	unsigned vertexSize = buffer->GetElementSize();
	char * data = (char*)buffer->GetData();

	for(unsigned i = 0; i < buffer->GetLength(); ++i)
	{
		glm::vec3 * pos = (glm::vec3*) &data[i * vertexSize];

		if(pos->x < boundingBox.origin.x) boundingBox.origin.x = pos->x;
		if(pos->y < boundingBox.origin.y) boundingBox.origin.y = pos->y;
		if(pos->z < boundingBox.origin.z) boundingBox.origin.z = pos->z;
		if(pos->x > boundingBox.dimensions.x) boundingBox.dimensions.x = pos->x;
		if(pos->y > boundingBox.dimensions.y) boundingBox.dimensions.y = pos->y;
		if(pos->z > boundingBox.dimensions.z) boundingBox.dimensions.z = pos->z;
	}

	boundingBox.dimensions = boundingBox.dimensions - boundingBox.origin;

	return boundingBox;
}

Gpu::BoundingSphere GeoBuilder::GenerateBoundingSphere(IVertexBuffer * buffer)
{
	Gpu::BoundingSphere boundingSphere;

	Gpu::BoundingBox boundingBox = GenerateBoundingBox(buffer);

	boundingSphere.origin = boundingBox.origin + (boundingBox.dimensions / 2.0f);

	unsigned vertexSize = buffer->GetElementSize();
	char * data = (char*) buffer->GetData();

	for(unsigned i = 0; i < buffer->GetLength(); ++i)
	{
		glm::vec3 * pos = (glm::vec3*) &data[i * vertexSize];

		float distance = glm::distance(boundingSphere.origin, *pos);
		if(distance > boundingSphere.radius) boundingSphere.radius = distance;
	}

	return boundingSphere;
}

LocalMesh * GeoBuilder::BuildRect(float x, float y, float width, float height, bool texCoords)
{
	IVertexBuffer * v = 0;
	if(texCoords)
	{
		VertexBuffer<Vertex_PosTex> * tmp = new VertexBuffer<Vertex_PosTex>(4);
		tmp->Set(0, Vertex_PosTex(x,y,0.0f,0.0f,1.0f));
		tmp->Set(1, Vertex_PosTex(x,y+height,0.0f,0.0f,0.0f));
		tmp->Set(2, Vertex_PosTex(x+width,y,0.0f,1.0f,1.0f));
		tmp->Set(3, Vertex_PosTex(x+width,y+height,0.0f,1.0f,0.0f));
		v = tmp;
	}
	else
	{
		VertexBuffer<Vertex_Pos> * tmp = new VertexBuffer<Vertex_Pos>(4);
		tmp->Set(0, Vertex_Pos(x,y,0.0f));
		tmp->Set(1, Vertex_Pos(x,y+height,0.0f));
		tmp->Set(2, Vertex_Pos(x+width,y,0.0f));
		tmp->Set(3, Vertex_Pos(x+width,y+height,0.0f));
		v = tmp;
	}

	unsigned * k = new unsigned[6];
	k[0] = 0; k[1] = 1; k[2] = 2;
	k[3] = 1; k[4] = 3; k[5] = 2;

	return new LocalMesh(v,k,6);
}

LocalMesh * GeoBuilder::BuildRectStroke(float x, float y, float width, float height, float strokeWidth)
{
	VertexBuffer<Vertex_Pos> * v = new VertexBuffer<Vertex_Pos>(8);

	float halfStrokeWidth = strokeWidth/2.0f;

	v->Set(0,Vertex_Pos(x-halfStrokeWidth,y-halfStrokeWidth,0.0f));
	v->Set(1,Vertex_Pos(x+halfStrokeWidth,y+halfStrokeWidth,0.0f));
	v->Set(2,Vertex_Pos(x+halfStrokeWidth+width,y-halfStrokeWidth,0.0f));
	v->Set(3,Vertex_Pos(x-halfStrokeWidth+width,y+halfStrokeWidth,0.0f));
	v->Set(4,Vertex_Pos(x+width+halfStrokeWidth,y+height+halfStrokeWidth,0.0f));
	v->Set(5,Vertex_Pos(x+width-halfStrokeWidth,y+height-halfStrokeWidth,0.0f));
	v->Set(6,Vertex_Pos(x-halfStrokeWidth,y+height+halfStrokeWidth,0.0f));
	v->Set(7,Vertex_Pos(x+halfStrokeWidth,y+height-halfStrokeWidth,0.0f));

	unsigned * k = new unsigned[24];

	for(unsigned i = 0; i < 4; i++)
	{
		k[(i*6)+0] = i*2;
		k[(i*6)+1] = (i==3 ? 0 : (i+1)*2);
		k[(i*6)+2] = (i*2)+1;

		k[(i*6)+3] = (i==3 ? 0 : (i+1)*2);
		k[(i*6)+4] = (i*2)+1;
		k[(i*6)+5] = (i==3 ? 1 : ((i+1)*2)+1);
	}

	return new LocalMesh(v,k,24);
}

LocalMesh * GeoBuilder::BuildEllipse(float cx, float cy, float rx, float ry)
{
	std::vector<Path::Point> points;
	const float ELLIPSE_POINTS = 100.0f;
	for(float i = 0.0f; i < ELLIPSE_POINTS; ++i)
	{
		float angle = (i / ELLIPSE_POINTS) * 2.0f * float(M_PI);
		float x = cx + (cosf(angle) * rx);
		float y = cy + (sinf(angle) * ry);
		points.emplace_back(x,y);
	}

	return BuildPath(points.data(),points.size());
}

LocalMesh * GeoBuilder::BuildEllipseStroke(float cx, float cy, float rx, float ry, float strokeWidth)
{
	std::vector<Path::Point> points;
	const float ELLIPSE_POINTS = 100.0f;
	for(float i = 0.0f; i <= ELLIPSE_POINTS; ++i)
	{
		float angle = (i / ELLIPSE_POINTS) * 2.0f * float(M_PI);
		float x = cx + (cosf(angle) * rx);
		float y = cy + (sinf(angle) * ry);
		points.emplace_back(x,y);
	}

	return BuildStroke(points.data(),points.size(),strokeWidth);
}

LocalMesh * GeoBuilder::BuildPath(Path::Point * points, unsigned numPoints, GeoBuilder::PathDebugFlag debugFlag)
{
	if(numPoints == 0) return 0;

	Path path(points, numPoints);

	FixPathCrossovers(path);

	std::vector<Diagonal> diagonals;
	SweepDiagonals(path, diagonals);

	if(debugFlag == DebugDiagonals) return BuildDiagonalDebugMesh(path, diagonals);

	std::vector<unsigned> * monotones = new std::vector<unsigned>[diagonals.size() + 1];
	unsigned numMonotones = BuildMonotones(path, diagonals, monotones);

	if(debugFlag == DebugMonotones) return BuildMonotoneDebugMesh(path, monotones, numMonotones);

	std::vector<Triangle> triangles;
	for(unsigned i = 0; i < numMonotones; ++i)
	{
		TriangulateMonotone(path, monotones[i], triangles);
	}
	delete[] monotones;

	if(triangles.size() < 1) return 0;

	VertexBuffer<Vertex_Pos> * v = new VertexBuffer<Vertex_Pos>(path.points.size());
	for(unsigned i = 0; i < path.points.size(); ++i)
	{
		v->Set(i, Vertex_Pos(float(path.points[i].x), float(path.points[i].y), 0.0f));
	}

	unsigned * k = new unsigned[triangles.size() * 3];
	for(unsigned i = 0; i < triangles.size(); ++i)
	{
		k[(i*3)+0] = triangles[i].a;
		k[(i*3)+1] = triangles[i].b;
		k[(i*3)+2] = triangles[i].c;
	}

	return new LocalMesh(v,k,triangles.size()*3);
}

LocalMesh * GeoBuilder::BuildStroke(Path::Point * points, unsigned numPoints, float width,
	GeoBuilder::StrokeCornerType cornerType, GeoBuilder::StrokeCapType capType, float miterLimit, float animProgress)
{
	if(numPoints == 0) return 0;

	Path path(points, numPoints);

	unsigned numVertices = 4;
	unsigned numIndices = 6;

	double stepTheta = M_PI / 6.0f;

	if(capType == CapRound)
	{
		unsigned numRoundPoints = unsigned(ceil(M_PI/stepTheta));
		numVertices += (numRoundPoints) * 2;
		numIndices  += (numRoundPoints) * 6;
	}

	for(unsigned i = 1; i < numPoints - 1; ++i)
	{
		if((float(i+1) / float(numPoints)) > animProgress) break;

		double angle = path.Angle(i-1,i,i+1,false);
		double miterRatio = 1.0f / sin((M_PI - angle) / 2.0f);
		bool miterToBevel = miterRatio > miterLimit;

		if(cornerType == CornerMiter && !miterToBevel)
		{
			numVertices += 2;
			numIndices  += 6;
		}
		else if(cornerType == CornerBevel || miterToBevel)
		{
			numVertices += 3;
			numIndices  += 9;
		}
		else if(cornerType == CornerRound)
		{
			unsigned numSegments = unsigned(ceil(fabs(angle)/stepTheta));
			numVertices += numSegments + 2;
			numIndices  += ((numSegments-1) * 3) + 9;
		}
	}

	VertexBuffer<Vertex_Pos> * v = new VertexBuffer<Vertex_Pos>(numVertices);
	unsigned * k = new unsigned[numIndices];

	unsigned pointAccumulator = 0;
	unsigned indexAccumulator = 0;

	float perpendicularShift = width / 2.0f;

	for(unsigned i = 0; i < numPoints; ++i)
	{
		if((float(i) / float(numPoints)) > animProgress) break;

		Path::Point & point = path.points[i];

		double lineAngle = path.Angle(i, (i+1 == numPoints ? i : i+1));
		double prevLineAngle = path.Angle((i == 0 ? numPoints-1 : i-1), i);
		
		if(i == 0 || i == numPoints - 1 || (float(i+1) / float(numPoints)) > animProgress)
		{
			double shiftAngle = M_PI * 0.5f;
			if(capType == CapSquare) shiftAngle *= (i==0 ? 1.5f : 0.5f);
			double hypotenuse = sqrtf(powf(perpendicularShift,2.0f) * (capType == CapSquare ? 2.0f : 1.0f));

			double angle1 = (i==0 ? lineAngle : prevLineAngle) + shiftAngle;
			double angle2 = (i==0 ? lineAngle : prevLineAngle) - shiftAngle;

			double xShift = cos(angle1) * hypotenuse;
			double yShift = sin(angle1) * hypotenuse;

			v->Set(pointAccumulator+0, Vertex_Pos(float(point.x + xShift), float(point.y + yShift), 0.0f));

			xShift = cos(angle2) * hypotenuse;
			yShift = sin(angle2) * hypotenuse;

			v->Set(pointAccumulator+1, Vertex_Pos(float(point.x + xShift), float(point.y + yShift), 0.0f));

			if(i == 0)
			{
				k[indexAccumulator++] = pointAccumulator;
				k[indexAccumulator++] = pointAccumulator + 1;
				k[indexAccumulator++] = pointAccumulator + 2;
				k[indexAccumulator++] = pointAccumulator + 1;
				k[indexAccumulator++] = pointAccumulator + 3;
				k[indexAccumulator++] = pointAccumulator + 2;
			}

			pointAccumulator += 2;
		}
		else
		{
			double miterRatio = 1.0f / sin((M_PI - point.angle) / 2.0f);
			bool miterToBevel = miterRatio > miterLimit;

			if(cornerType == CornerMiter && !miterToBevel)
			{
				double parallelShift = tan(point.angle / 2.0f) * perpendicularShift;

				double shiftAngle = atan2(perpendicularShift,parallelShift);
				double hypotenuse = sqrt(pow(perpendicularShift,2.0f) + pow(parallelShift,2.0f));

				double angle = lineAngle + shiftAngle;
				if(angle > M_PI * 2.0f) angle -= M_PI * 2.0f;
				if(angle < 0.0f) angle += M_PI * 2.0f;

				double xShift = cos(angle) * hypotenuse;
				double yShift = sin(angle) * hypotenuse;

				v->Set(pointAccumulator+0, Vertex_Pos(float(point.x + xShift), float(point.y + yShift), 0.0f));
				v->Set(pointAccumulator+1, Vertex_Pos(float(point.x - xShift), float(point.y - yShift), 0.0f));

				k[indexAccumulator++] = pointAccumulator + 0;
				k[indexAccumulator++] = pointAccumulator + 1;
				k[indexAccumulator++] = pointAccumulator + 2;
				k[indexAccumulator++] = pointAccumulator + 1;
				k[indexAccumulator++] = pointAccumulator + 3;
				k[indexAccumulator++] = pointAccumulator + 2;

				pointAccumulator += 2;
			}
			else //if (cornerType == CornerRound || cornerType == CornerBevel)
			{
				double pointAngle = path.Angle(i-1,i,i+1,false);

				double parallelShift = tan(point.angle / 2.0f) * perpendicularShift;
				double prevLineAngle = path.Angle(i-1,i);
				
				double shiftAngle = atan2(perpendicularShift,parallelShift);
				double hypotenuse = sqrt(pow(perpendicularShift,2.0f) + pow(parallelShift,2.0f));

				double clockwiseMultiplier = (pointAngle > 0.0f ? 1.0f : -1.0f);

				double startAngle = prevLineAngle + (M_PI_2 * -clockwiseMultiplier);
				double internalAngle = lineAngle + shiftAngle;
				double cornerAngle = pointAngle;
				double cornerTheta = fabs(cornerAngle);
				double xShiftInternal = cos(internalAngle) * hypotenuse * clockwiseMultiplier;
				double yShiftInternal = sin(internalAngle) * hypotenuse * clockwiseMultiplier;

				if(cornerType == CornerBevel || miterToBevel)
				{
					stepTheta = cornerTheta;
				}

				if(pointAngle > 0.0f)
				{
					unsigned internalIndex = pointAccumulator;

					v->Set(internalIndex, Vertex_Pos(float(point.x + xShiftInternal), float(point.y + yShiftInternal), 0.0f));

					for(double currentTheta = 0.0f; currentTheta < cornerTheta; currentTheta += stepTheta)
					{
						double currentAngle = (cornerAngle < 0.0f ? currentTheta * -1.0f : currentTheta);
						double xShift = cos(startAngle + currentAngle) * perpendicularShift;
						double yShift = sin(startAngle + currentAngle) * perpendicularShift;

						v->Set(pointAccumulator+1, Vertex_Pos(float(point.x + xShift), float(point.y + yShift), 0.0f));

						k[indexAccumulator++] = internalIndex;
						k[indexAccumulator++] = pointAccumulator + 1;
						k[indexAccumulator++] = pointAccumulator + 2;

						pointAccumulator ++;
					}

					double xShiftEnd = cos(startAngle + cornerAngle) * perpendicularShift;
					double yShiftEnd = sin(startAngle + cornerAngle) * perpendicularShift;

					v->Set(pointAccumulator+1, Vertex_Pos(float(point.x + xShiftEnd), float(point.y + yShiftEnd), 0.0f));

					k[indexAccumulator++] = internalIndex;
					k[indexAccumulator++] = pointAccumulator + 1;
					k[indexAccumulator++] = pointAccumulator + 2;
					k[indexAccumulator++] = pointAccumulator + 1;
					k[indexAccumulator++] = pointAccumulator + 3;
					k[indexAccumulator++] = pointAccumulator + 2;

					pointAccumulator += 2;
				}
				else
				{
					double xShiftStart = cos(startAngle) * perpendicularShift;
					double yShiftStart = sin(startAngle) * perpendicularShift;
					v->Set(pointAccumulator, Vertex_Pos(float(point.x + xShiftStart), float(point.y + yShiftStart), 0.0f));

					unsigned internalIndex = pointAccumulator + 1;
					v->Set(internalIndex, Vertex_Pos(float(point.x + xShiftInternal), float(point.y + yShiftInternal), 0.0f));

					k[indexAccumulator++] = pointAccumulator;
					k[indexAccumulator++] = internalIndex;
					k[indexAccumulator++] = pointAccumulator + 2;

					pointAccumulator += 2;

					for(double currentTheta = stepTheta; currentTheta < cornerTheta; currentTheta += stepTheta)
					{
						double currentAngle = (cornerAngle < 0.0f ? currentTheta * -1.0f : currentTheta);
						double xShift = cos(startAngle + currentAngle) * perpendicularShift;
						double yShift = sin(startAngle + currentAngle) * perpendicularShift;
						v->Set(pointAccumulator, Vertex_Pos(float(point.x + xShift), float(point.y + yShift), 0.0f));

						k[indexAccumulator++] = pointAccumulator;
						k[indexAccumulator++] = internalIndex;
						k[indexAccumulator++] = pointAccumulator + 1;

						pointAccumulator ++;
					}

					double xShiftEnd = cos(startAngle + cornerAngle) * perpendicularShift;
					double yShiftEnd = sin(startAngle + cornerAngle) * perpendicularShift;
					v->Set(pointAccumulator, Vertex_Pos(float(point.x + xShiftEnd), float(point.y + yShiftEnd), 0.0f));

					k[indexAccumulator++] = pointAccumulator;
					k[indexAccumulator++] = internalIndex;
					k[indexAccumulator++] = pointAccumulator + 1;
					k[indexAccumulator++] = internalIndex;
					k[indexAccumulator++] = pointAccumulator + 2;
					k[indexAccumulator++] = pointAccumulator + 1;

					pointAccumulator += 1;
				}
			}
		}
	}

	if(capType == CapRound)
	{
		unsigned endPoint1 = pointAccumulator - 2;
		unsigned endPoint2 = pointAccumulator - 1;

		unsigned lastPoint = unsigned(float(numPoints - 2) * animProgress) + 1;

		double firstLineAngle = path.Angle(0,1);
		double lastLineAngle = path.Angle(lastPoint - 1, lastPoint);

		Path::Point & start = path.points[0];
		Path::Point & end = path.points[lastPoint];

		double startAngle = firstLineAngle + M_PI_2;
		for(double currentTheta = stepTheta; currentTheta < M_PI; currentTheta += stepTheta)
		{
			double xShift = cos(startAngle + currentTheta) * perpendicularShift;
			double yShift = sin(startAngle + currentTheta) * perpendicularShift;

			v->Set(pointAccumulator, Vertex_Pos(float(start.x + xShift), float(start.y + yShift), 0.0f));

			k[indexAccumulator++] = pointAccumulator;
			k[indexAccumulator++] = (currentTheta + stepTheta < M_PI ? pointAccumulator + 1 : 1);
			k[indexAccumulator++] = 0;

			pointAccumulator ++;
		}

		startAngle = lastLineAngle - M_PI_2;
		for(double currentTheta = stepTheta; currentTheta < M_PI; currentTheta += stepTheta)
		{
			double xShift = cos(startAngle + currentTheta) * perpendicularShift;
			double yShift = sin(startAngle + currentTheta) * perpendicularShift;

			v->Set(pointAccumulator, Vertex_Pos(float(end.x + xShift), float(end.y + yShift), 0.0f));

			k[indexAccumulator++] = pointAccumulator;
			k[indexAccumulator++] = (currentTheta + stepTheta < M_PI ? pointAccumulator + 1 : endPoint1);
			k[indexAccumulator++] = endPoint2;

			pointAccumulator ++;
		}
	}

	return new LocalMesh(v,k,numIndices);
}

void TangentGenerator::Step()
{
	IVertexBuffer *& buffer = mesh->vertexBuffer;
	unsigned numTriangles = mesh->numTriangles;
	unsigned * indexData = mesh->indexBuffer;

	glm::vec3 positions[3];
	glm::vec2 texCoords[3];
	glm::vec3 normals[3];

	if(!nodes)
	{
		nodes = new VertexNode[buffer->GetLength()];
		newBuffer = new VertexBuffer<Vertex_PosNorTanTex>(mesh->vertexBuffer->GetLength());
	}

	if(triIndex < numTriangles)
	{
		// For every index in the triangle
		unsigned index;
		char* datum;
		for(unsigned j = 0; j<3; ++j)
		{
			index = indexData[(triIndex * 3) + j];

			// Find its vertex and its world position
			datum = ((char*)buffer->GetData()) + (index * buffer->GetElementSize());
			switch(buffer->GetVertexType())
			{
			case VertexType_PosNorTex:
			{
				Vertex_PosNorTex * item = (Vertex_PosNorTex*)datum;
				positions[j] = item->position;
				texCoords[j] = item->texCoord;
				normals[j] = item->normal;
			}
				break;
			case VertexType_PosNorTanTex:
			{
				Vertex_PosNorTanTex * item = (Vertex_PosNorTanTex*)datum;
				positions[j] = item->position;
				texCoords[j] = item->texCoord;
				normals[j] = item->normal;
			}
				break;
			}
		}

		float x1 = positions[1].x - positions[0].x;
		float x2 = positions[2].x - positions[0].x;
		float y1 = positions[1].y - positions[0].y;
		float y2 = positions[2].y - positions[0].y;
		float z1 = positions[1].z - positions[0].z;
		float z2 = positions[2].z - positions[0].z;

		glm::vec3 one = positions[1] - positions[0];
		glm::vec3 two = positions[2] - positions[0];

		glm::vec3 triNormal = glm::cross(one, two);
		triNormal = glm::normalize(triNormal);

		float s1 = texCoords[1].x - texCoords[0].x;
		float s2 = texCoords[2].x - texCoords[0].x;
		float t1 = texCoords[1].y - texCoords[0].y;
		float t2 = texCoords[2].y - texCoords[0].y;

		glm::vec2 tex1 = texCoords[1] - texCoords[0];
		glm::vec2 tex2 = texCoords[2] - texCoords[0];

		float rad = 1.0f / ((tex1.x * tex2.y) - (tex2.x * tex1.y));

		float r = 1.0f / ((s1 * t2) - (s2 * t1));

		glm::vec3 sdir(
			((t2 * x1) - (t1 * x2)) * r,
			((t2 * y1) - (t1 * y2)) * r,
			((t2 * z1) - (t1 * z2)) * r);

		glm::vec3 tdir(
			((s1 * x2) - (s2 * x1)) * r,
			((s1 * y2) - (s2 * y1)) * r,
			((s1 * z2) - (s2 * z1)) * r);

		glm::vec3 tangent(
			rad * ((tex2.y * one.x) - (tex1.y * two.x)),
			rad * ((tex2.y * one.y) - (tex1.y * two.y)),
			rad * ((tex2.y * one.z) - (tex1.y * two.z)));

		glm::vec3 bitangent(
			rad * ((tex1.x * two.x) - (tex2.x * one.x)),
			rad * ((tex1.x * two.y) - (tex2.x * one.y)),
			rad * ((tex1.x * two.z) - (tex2.x * one.z)));

		for(unsigned j = 0; j < 3; j++)
		{
			nodes[indexData[(triIndex * 3) + j]].tangent += sdir;

			nodes[indexData[(triIndex * 3) + j]].bitangent += tdir;
		}

		triIndex++;
	}
	else if(vbIndex < buffer->GetLength())
	{
		// For every vertex in the vertex buffer
		float tanChirality; // 1.0f or -1.0f
		glm::vec3 inputBitangent = nodes[indexData[vbIndex]].bitangent;
		char* datum = ((char*)buffer->GetData()) + (vbIndex * buffer->GetElementSize());
		Vertex_PosNorTanTex* output = ((Vertex_PosNorTanTex*)newBuffer->GetData()) + (vbIndex);
		Vertex_PosNorTex input;

		switch(buffer->GetVertexType())
		{
		case VertexType_PosNorTex:
		{
			Vertex_PosNorTex * item = (Vertex_PosNorTex*)datum;
			input.normal = item->normal;
			input.position = item->position;
			input.texCoord = item->texCoord;
		}
			break;
		case VertexType_PosNorTanTex:
		{
			Vertex_PosNorTanTex * item = (Vertex_PosNorTanTex*)datum;
			input.normal = item->normal;
			input.position = item->position;
			input.texCoord = item->texCoord;
		}
			break;
		}

		glm::vec3 n = input.normal;
		glm::vec3 t = nodes[vbIndex].tangent;
		glm::vec3 outputTangent = t - (n * glm::dot(n, t));
		outputTangent = glm::normalize(outputTangent);

		glm::vec3 bitangent = glm::cross(n, t);

		tanChirality = (glm::dot(bitangent, inputBitangent) < 0.0f ? -1.0f : 1.0f);

		output->position = input.position;
		output->normal = input.normal;
		output->tangent = outputTangent;
		output->tanChirality = tanChirality;
		output->texCoord = input.texCoord;

		vbIndex++;
	}

	if(vbIndex >= buffer->GetLength())
	{
		delete[] nodes;
		delete mesh->vertexBuffer;
		mesh->vertexBuffer = newBuffer;
	}
}

} // namespace Ingenuity
