#pragma once

#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

struct Path
{
	enum MonotoneDecompositionType
	{
		Above,
		Below,
		Neither
	};

	struct Point
	{
		friend struct Path;

		double x, y;
		double angle;
		MonotoneDecompositionType mdType;

		unsigned prev, next;

		Point(double x, double y) : x(x), y(y), angle(0.0f), prev(0), next(0) {}
		Point() : x(0.0f), y(0.0f), angle(0.0f), prev(0), next(0) {}
	};

	struct Edge
	{
		friend struct Path;

		//unsigned startIndex;
		//unsigned endIndex;

		unsigned helperIndex;
		unsigned index;

		double angle;
		double xPos;

		Edge() : /*startIndex(0), endIndex(0), */helperIndex(UINT_MAX), angle(0.0f), xPos(0.0f) {}
	};

	std::vector<Point> points;
	std::vector<Edge> edges;

	double pathAngle;

	bool IsClockwise()
	{
		return pathAngle > 0.0f;
	}

	bool PointIsConcave(unsigned index)
	{
		return PointIsConcave(&points[index]);
	}

	bool PointIsConcave(Path::Point * point)
	{
		return IsClockwise() ? point->angle < -FLT_EPSILON : point->angle > FLT_EPSILON;
	}

	unsigned Length()
	{
		return points.size();
	}

	void Clear()
	{
		points.clear();
		edges.clear();
		pathAngle = 0.0f;
	}

	void InsertPoint(unsigned index, float x, float y)
	{
		while(index >= points.size()) index -= points.size();
		points.insert(points.begin() + index, Point(x,y));
	}

	//void SplitSubpaths(unsigned a, unsigned b, Path & AToB, Path & BToA)
	//{
	//	AToB.Clear();
	//	BToA.Clear();

	//	if(a == b)
	//	{
	//		BToA.points = points;
	//		BToA.edges = edges;
	//	}
	//	else 
	//	{
	//		std::vector<Point>::iterator it_a = points.begin() + a;
	//		std::vector<Point>::iterator it_b = points.begin() + b;
	//		if(b > a)
	//		{
	//			AToB.points.assign(it_a, it_b);
	//			AToB.Finalize();

	//			BToA.points.assign(it_b, points.end());
	//			BToA.points.insert(BToA.points.end(), points.begin(), it_a);
	//			BToA.Finalize();
	//		}
	//		else // b < a
	//		{
	//			AToB.points.assign(it_a, points.end());
	//			AToB.points.insert(AToB.points.end(), points.begin(), it_b);
	//			AToB.Finalize();

	//			BToA.points.assign(it_b, it_a);
	//			BToA.Finalize();
	//		}
	//	}
	//}

	double Angle(Point & startPoint, Point & endPoint)
	{
		double dx = endPoint.x - startPoint.x;
		double dy = endPoint.y - startPoint.y;
		return atan2(dy, dx);
	}

	double Angle(unsigned start, unsigned end)
	{
		return Angle(points[start], points[end]);
	}

	double Angle(unsigned start, unsigned middle, unsigned end, bool fixWinding)
	{
		if(fixWinding)
		{
			int antiWinding = (int(middle)-int(start))*(int(end)-int(middle))*(int(start)-int(end));
			if(antiWinding > 0)
			{
				// positive antiWinding means backwards winding
				unsigned temp = end;
				end = start;
				start = temp;
			}
		}

		double prevAngle = Angle(start,middle);
		double nextAngle = Angle(middle,end);

		double pointAngle = nextAngle - prevAngle;

		if(pointAngle >  double(M_PI)) pointAngle -= double(2.0 * M_PI);
		if(pointAngle < -double(M_PI)) pointAngle += double(2.0 * M_PI);

		return pointAngle;
	}

	bool GetIntersection(unsigned edgeIndex1, unsigned edgeIndex2, double & x, double & y)
	{
		// http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

		Path::Point & edge1Start = points[edgeIndex1 == 0 ? points.size() - 1 : edgeIndex1 - 1];
		Path::Point & edge2Start = points[edgeIndex2 == 0 ? points.size() - 1 : edgeIndex2 - 1];
		Path::Point & edge1End = points[edgeIndex1];
		Path::Point & edge2End = points[edgeIndex2];

		double x1 = edge1Start.x;
		double y1 = edge1Start.y;
		double dx1 = edge1End.x - edge1Start.x;
		double dy1 = edge1End.y - edge1Start.y;
		double x2 = edge2Start.x;
		double y2 = edge2Start.y;
		double dx2 = edge2End.x - edge2Start.x;
		double dy2 = edge2End.y - edge2Start.y;

		double nx = x2 - x1;
		double ny = y2 - y1;

		double denominator = (dx1*dy2) - (dy1*dx2);
		if(denominator != 0)
		{
			double numerator1 = (nx*dy1) - (ny*dx1);
			double numerator2 = (nx*dy2) - (ny*dx2);
			double intersection1 = numerator1 / denominator;
			double intersection2 = numerator2 / denominator;

			if(intersection1 > 0.0f && intersection1 < 1.0f
				&& intersection2 > 0.0f && intersection2 < 1.0f)
			{
				double intersectionX = x2 + (dx2 * intersection1);
				double intersectionY = y2 + (dy2 * intersection1);
				x = intersectionX;
				y = intersectionY;
				return true;
			}
		}
		return false;
	}

	void ApplyJitter()
	{
		for(unsigned i = 0; i < points.size(); ++i)
		{
			// add 'jitter' // CRITICAL // two points should never have equal Y //
			points[i].y -= (((double)rand()/(double)RAND_MAX) - 0.5f) * 0.01f;
		}
	}

	void Finalize()
	{
		pathAngle = 0.0f;
		edges.clear();

		for(unsigned i = 0; i < points.size(); ++i)
		{
			edges.emplace_back();

			unsigned prev = (i == 0 ? points.size()-1 : i-1);
			
			double dx = points[i].x - points[prev].x;
			double dy = points[i].y - points[prev].y;

			//if(dy == 0.0f)
			//{
			//	points[i].y += FLT_EPSILON * 1000.0f;
			//	dy = points[i].y - points[prev].y;
			//}

			//float xJitter = (((float)rand()/(float)RAND_MAX) - 0.5f) * 0.01f;

			edges[i].angle = atan2(dy,dx);
			//edges[i].xPos = ((points[prev].x + points[i].x) / 2.0f);
			edges[i].index = i;
		}
		for(unsigned i = 0; i < points.size(); i++)
		{
			unsigned prev = (i == 0 ? points.size()-1 : i-1);
			unsigned next = (i+1 == points.size() ? 0 : i+1);

			double pointAngle = edges[next].angle - edges[i].angle;

			if(pointAngle >  double(M_PI)) pointAngle -= double(2.0 * M_PI);
			if(pointAngle < -double(M_PI)) pointAngle += double(2.0 * M_PI);

			points[i].angle = pointAngle;
			points[i].prev = prev;
			points[i].next = next;

			pathAngle += points[i].angle;
			
			if((points[prev].y < points[i].y && points[next].y < points[i].y)
				|| (edges[i].angle > 0.0f && edges[next].angle < 0.0f))
			{
				points[i].mdType = Below;
			}
			else if((points[prev].y > points[i].y && points[next].y > points[i].y)
				|| (edges[i].angle < 0.0f && edges[next].angle > 0.0f))
			{
				points[i].mdType = Above;
			}
			else
			{
				points[i].mdType = Neither;
			}			
		}
		for(unsigned i = 0; i < points.size(); i++)
		{
			unsigned prev = (i == 0 ? points.size()-1 : i-1);

			if(IsClockwise())
			{
				edges[i].xPos = (points[prev].x < points[i].x ? points[prev].x : points[i].x); 
			}
			else
			{
				edges[i].xPos = (points[prev].x > points[i].x ? points[prev].x : points[i].x); 
			}
		}
	}

	Path() : pathAngle(0.0f) {}
	Path(Point * newPoints, unsigned numPoints) : pathAngle(0.0f)
	{
		points.resize(numPoints);
		std::copy(newPoints, newPoints + numPoints, points.begin());
		ApplyJitter();
		Finalize();
	}
	~Path()
	{
	}

};
