#include "SvgParser.h"
#include "tinyxml2.h"
#include "GeoStructs.h"
#include "AssetMgr.h"
#include <sstream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Ingenuity {

SvgParser::~SvgParser()
{
	SvgDefs::iterator it = definitions.begin();
	for(; it != definitions.end(); it++)
	{
		delete it->second;
	}
	for(unsigned i = 0; i < graphics.size(); ++i)
	{
		if(graphics[i].fill) delete graphics[i].fill;
		if(graphics[i].stroke) delete graphics[i].stroke;
	}
}

void SvgParser::ParseRect(tinyxml2::XMLElement * element, Graphic & graphic, bool image)
{
	const char * xText = element->Attribute("x");
	const char * yText = element->Attribute("y");
	const char * widthText = element->Attribute("width");
	const char * heightText = element->Attribute("height");

	if(widthText && heightText && xText && yText)
	{
		float x = (float) atof(xText);
		float y = (float) atof(yText);
		float width = (float) atof(widthText);
		float height = (float) atof(heightText);

		graphic.shape.type = ShapeRect;
		graphic.shape.x = x;
		graphic.shape.y = y;
		graphic.shape.w = width;
		graphic.shape.h = height;

		graphic.fill = builder.BuildRect(x,-y-height,width,height,image);
		if(!image) graphic.stroke = builder.BuildRectStroke(x,-y-height,width,height,graphic.stylesheet.strokeWidth);
	}
}

void SvgParser::ParseCircle(tinyxml2::XMLElement * element, Graphic & graphic)
{
	const char * cxText = element->Attribute("cx");
	const char * cyText = element->Attribute("cy");
	const char * rText = element->Attribute("r");

	if(cxText && cyText && rText)
	{
		float cx = float(atof(cxText));
		float cy = float(atof(cyText));
		float radius = float(atof(rText));

		graphic.shape.type = ShapeCircle;
		graphic.shape.x = cx;
		graphic.shape.y = cy;
		graphic.shape.w = radius;
		graphic.shape.w = radius;

		graphic.fill = builder.BuildEllipse(cx,-cy,radius,radius);
		graphic.stroke = builder.BuildEllipseStroke(cx,-cy,radius,radius,graphic.stylesheet.strokeWidth);
	}
}

void SvgParser::ParseStyle(const char * styleString, SvgParser::Stylesheet & stylesheet)
{
	if(styleString)
	{
		std::stringstream style(styleString);
		std::string key, value;

		while(GetNextCssKeyValue(style,key,value))
		{
			ParseStyleProperty(key.c_str(),value.c_str(),stylesheet);
		}
	}
};

void SvgParser::ParseDescription(const char * description, Graphic & graphic)
{
	std::string desc(description);
	std::queue<DescToken> tokenQueue;
	
	if(!TokenizeDescription(desc,tokenQueue))
	{
		return; // Error
	}

	// M/m require 2 parameters, then default to L/l
	// L/l require 2 parameters, then default to L/l
	// C/c Q/q T/t S/s accept any number of parameters
	// A/a require 7 parameters, then default to A/a

	std::vector<Path::Point> pathPoints;

	float penPosX = 0.0f;
	float penPosY = 0.0f;
	float pathStartX = 0.0f;
	float pathStartY = 0.0f;
	unsigned paramNum = 0;
	char opChar = 0;
	bool pathStarted = false;

	float arcRadiusX = 0.0f;
	float arcRadiusY = 0.0f;
	float arcRotation = 0.0f;
	float arcTargetX = 0.0f;
	float arcTargetY = 0.0f;
	bool arcLargeFlag = false;
	bool arcSweepFlag = false;

	float cubicParams[6]; // control points 1 & 2, target point
	float quadParams[4];  // control point, target point
	bool previousCubic = false;
	bool previousQuad = false;

	//bool firstMove = false; // HACK

	for(; tokenQueue.size() > 0; tokenQueue.pop())
	{
		DescToken & token = tokenQueue.front();
		if(token.op)
		{
			previousCubic = (opChar == 'c' || opChar == 'C' || opChar == 's' || opChar == 'S');
			previousQuad  = (opChar == 'q' || opChar == 'Q' || opChar == 't' || opChar == 'T');

			opChar = token.opChar;
			paramNum = 0;

			if(opChar == 'z' || opChar == 'Z')
			{
				if(pathStarted)
				{
					penPosX = pathStartX;
					penPosY = pathStartY + (penPosY > pathStartY ? 0.1f : -0.1f);
					pathPoints.emplace_back(penPosX,penPosY);
					pathStarted = false;
				}
			}
		}
		else
		{
			if(opChar != 'M' && opChar != 'm')
			{
				if(!pathStarted)
				{
					pathStartX = penPosX; pathStartY = penPosY;
					pathPoints.emplace_back(pathStartX,pathStartY);
					pathStarted = true;
				}
			}
			switch (opChar)
			{
			// moveto
			case 'M':
				//if(firstMove) break; // HACK
				if(paramNum == 0)
				{
					if(pathStarted)
					{
						penPosX = pathStartX;
						penPosY = pathStartY;
						pathPoints.emplace_back(penPosX,penPosY);
						pathStarted = false;
					}
					penPosX = token.value;
					++ paramNum;
				}
				else if(paramNum == 1)
				{
					penPosY = token.value;
					//firstMove = true; // HACK
					opChar = 'L';
					paramNum = 0;
				}
				break;
			case 'm':
				//if(firstMove) break; // HACK
				if(paramNum == 0)
				{
					if(pathStarted)
					{
						penPosX = pathStartX;
						penPosY = pathStartY;
						pathPoints.emplace_back(penPosX,penPosY);
						pathStarted = false;
					}
					penPosX += token.value;
					++ paramNum;
				}
				else if(paramNum == 1)
				{
					penPosY += token.value;
					//firstMove = true; // HACK
					opChar = 'l';
					paramNum = 0;
				}
				break;
			// horizontal
			case 'h':
				penPosX += token.value;
				pathPoints.emplace_back(penPosX,penPosY);
				opChar = 'l';
				paramNum = 0;
				break;
			// vertical
			case 'v':
				penPosY += token.value;
				pathPoints.emplace_back(penPosX,penPosY);
				opChar = 'l';
				paramNum = 0;
				break;
			// lineto
			case 'L':
				if(paramNum == 0)
				{
					penPosX = token.value;
					++ paramNum;
				}
				else if(paramNum == 1)
				{
					penPosY = token.value;
					pathPoints.emplace_back(penPosX,penPosY);
					paramNum = 0;
				}
				break;
			case 'l':
				if(paramNum == 0)
				{
					penPosX += token.value;
					++ paramNum;
				}
				else if(paramNum == 1)
				{
					penPosY += token.value;
					pathPoints.emplace_back(penPosX,penPosY);
					paramNum = 0;
				}
				break;
			// quadratic curveto
			case 'Q':
				quadParams[paramNum] = token.value;

				if(paramNum > 2)
				{
					BuildQuadBezier(penPosX,penPosY,
						quadParams[2],quadParams[3],
						quadParams[0],quadParams[1],
						pathPoints);

					penPosX = quadParams[2];
					penPosY = quadParams[3];
					paramNum = 0;
				}
				else
				{
					paramNum ++;
				}
				break;
			case 'q':
				quadParams[paramNum] = (paramNum % 2 == 0 ? penPosX : penPosY) + token.value;

				if(paramNum > 2)
				{
					BuildQuadBezier(penPosX,penPosY,
						quadParams[2],quadParams[3],
						quadParams[0],quadParams[1],
						pathPoints);

					penPosX = quadParams[2];
					penPosY = quadParams[3];
					paramNum = 0;
				}
				else
				{
					paramNum ++;
				}
				break;
			// cubic curveto
			case 'C':
				cubicParams[paramNum] = token.value;

				if(paramNum > 4)
				{
					BuildCubicBezier(penPosX,penPosY,
						cubicParams[4],cubicParams[5],
						cubicParams[0],cubicParams[1],
						cubicParams[2],cubicParams[3],
						pathPoints);

					penPosX = cubicParams[4];
					penPosY = cubicParams[5];
					paramNum = 0;
				}
				else
				{
					paramNum ++;
				}
				break;
			case 'c':
				cubicParams[paramNum] = (paramNum % 2 == 0 ? penPosX : penPosY) + token.value;

				if(paramNum > 4)
				{
					BuildCubicBezier(penPosX,penPosY,
						cubicParams[4],cubicParams[5],
						cubicParams[0],cubicParams[1],
						cubicParams[2],cubicParams[3],
						pathPoints);

					penPosX = cubicParams[4];
					penPosY = cubicParams[5];
					paramNum = 0;
				}
				else
				{
					paramNum ++;
				}
				break;
			// quadratic polycurve
			case 'T':
				quadParams[paramNum+2] = token.value;

				if(paramNum > 0)
				{
					if(previousQuad)
					{
						quadParams[0] = ((2*penPosX) - quadParams[0]);
						quadParams[1] = ((2*penPosY) - quadParams[1]);
					}
					else
					{
						quadParams[0] = penPosX;
						quadParams[1] = penPosY;
					}
					
					BuildQuadBezier(penPosX,penPosY,
						quadParams[2],quadParams[3],
						quadParams[0],quadParams[1],
						pathPoints);

					penPosX = quadParams[2];
					penPosY = quadParams[3];
					paramNum = 0;
				}
				else
				{
					paramNum ++;
				}
				break;
			case 't':
				quadParams[paramNum+2] = (paramNum % 2 == 0 ? penPosX : penPosY) + token.value;

				if(paramNum > 0)
				{
					if(previousQuad)
					{
						quadParams[0] = ((2*penPosX) - quadParams[0]);
						quadParams[1] = ((2*penPosY) - quadParams[1]);
					}
					else
					{
						quadParams[0] = penPosX;
						quadParams[1] = penPosY;
					}

					BuildQuadBezier(penPosX,penPosY,
						quadParams[2],quadParams[3],
						quadParams[0],quadParams[1],
						pathPoints);

					penPosX = quadParams[2];
					penPosY = quadParams[3];
					paramNum = 0;
				}
				else
				{
					paramNum ++;
				}
				break;
			// cubic polycurve
			case 'S':
				cubicParams[paramNum+2] = token.value;

				if(paramNum > 2)
				{
					if(previousCubic)
					{
						cubicParams[0] = ((2*penPosX) - cubicParams[2]);
						cubicParams[1] = ((2*penPosY) - cubicParams[3]);
					}
					else
					{
						cubicParams[0] = penPosX;
						cubicParams[1] = penPosY;
					}

					BuildCubicBezier(penPosX,penPosY,
						cubicParams[4],cubicParams[5],
						cubicParams[0],cubicParams[1],
						cubicParams[2],cubicParams[3],
						pathPoints);

					penPosX = cubicParams[4];
					penPosY = cubicParams[5];
					paramNum = 0;
				}
				else
				{
					paramNum ++;
				}
				break;
			case 's':
				cubicParams[paramNum+2] = (paramNum % 2 == 0 ? penPosX : penPosY) + token.value;

				if(paramNum > 2)
				{
					if(previousCubic)
					{
						cubicParams[0] = ((2*penPosX) - cubicParams[2]);
						cubicParams[1] = ((2*penPosY) - cubicParams[3]);
					}
					else
					{
						cubicParams[0] = penPosX;
						cubicParams[1] = penPosY;
					}

					BuildCubicBezier(penPosX,penPosY,
						cubicParams[4],cubicParams[5],
						cubicParams[0],cubicParams[1],
						cubicParams[2],cubicParams[3],
						pathPoints);

					penPosX = cubicParams[4];
					penPosY = cubicParams[5];
					paramNum = 0;
				}
				else
				{
					paramNum ++;
				}
				break;
			// arcto
			case 'A':
				switch(paramNum++)
				{
				case 0: // radius x
					arcRadiusX = token.value;
					break;
				case 1: // radius y
					arcRadiusY = token.value;
					break;
				case 2: // ellipse rotation (degrees)
					arcRotation = token.value * ((float)M_PI) / 180.0f;
					break;
				case 3: // large arc?
					arcLargeFlag = token.value > 0.0f;
					break;
				case 4: // positive direction?
					arcSweepFlag = token.value > 0.0f;
					break;
				case 5: // target x
					arcTargetX = token.value;
					break;
				case 6: // target y
					arcTargetY = token.value;
						
					BuildArc(arcRadiusX,arcRadiusY,arcRotation,arcLargeFlag,arcSweepFlag,
						penPosX,penPosY,arcTargetX,arcTargetY,pathPoints);

					penPosX = arcTargetX;
					penPosY = arcTargetY;
					opChar = 'A';
					paramNum = 0;
				}
				break;
			case 'a':
				switch(paramNum++)
				{
				case 0: // radius x
					arcRadiusX = token.value;
					break;
				case 1: // radius y
					arcRadiusY = token.value;
					break;
				case 2: // ellipse rotation (degrees)
					arcRotation = token.value * ((float)M_PI) / 180.0f;
					break;
				case 3: // large arc?
					arcLargeFlag = token.value > 0.0f;
					break;
				case 4: // positive direction?
					arcSweepFlag = token.value > 0.0f;
					break;
				case 5: // target x
					arcTargetX = penPosX + token.value;
					break;
				case 6: // target y
					arcTargetY = penPosY + token.value;		

					BuildArc(arcRadiusX,arcRadiusY,arcRotation,arcLargeFlag,arcSweepFlag,
						penPosX,penPosY,arcTargetX,arcTargetY,pathPoints);

					penPosX = arcTargetX;
					penPosY = arcTargetY;
					opChar = 'a';
					paramNum = 0;
				}
				break;
			default: // case 'l':
				break;
			}
		}
	}

	for(unsigned i = 0; i < pathPoints.size(); ++i)
	{
		pathPoints[i].y *= -1;
	}

	graphic.shape.type = ShapePath;
	graphic.shape.pathPoints = pathPoints;

	//graphic.fill = builder.BuildPath(pathPoints.data(),pathPoints.size());
	graphic.stroke = builder.BuildStroke(pathPoints.data(),pathPoints.size(),
		graphic.stylesheet.strokeWidth,
		graphic.stylesheet.cornerType,
		graphic.stylesheet.capType,
		graphic.stylesheet.miterLimit);
}

SvgParser::Transform SvgParser::ParseTransform(const char * transformChars)
{
	// matrix(a,b,c,d,e,f)
	// translate(x,y)
	// scale(x[,y])
	// rotate(a[,x,y])
	// skewX(a)
	// skewY(a)

	//if(transformChars)
	//{

	//	std::string transformString(transformChars);
	//	Transform result;

	//	unsigned leftBracketPos = std::string::npos;
	//	while((leftBracketPos = transformString.find("(")) != std::string::npos)
	//	{
	//		std::string thisTransform = transformString.substr(0,leftBracketPos);

	//		unsigned firstNonSpace = 0;
	//		while(thisTransform[firstNonSpace] == ' ') firstNonSpace++;
	//		thisTransform = thisTransform.substr(firstNonSpace);

	//		unsigned rightBracketPos = transformString.find(")");
	//		if(rightBracketPos == std::string::npos || rightBracketPos <= leftBracketPos)
	//			return Transform(); // Invalid transform!!

	//		std::string parameterString = transformString.substr(leftBracketPos + 1, rightBracketPos - leftBracketPos - 1);
	//		std::vector<float> parameters;
	//		unsigned commaPos = std::string::npos;
	//		while((commaPos = parameterString.find(",")) != std::string::npos)
	//		{
	//			float value = (float) atof(parameterString.substr(0,commaPos).c_str());
	//			parameters.push_back(value);
	//			parameterString = parameterString.substr(commaPos+1);
	//		}
	//		parameters.push_back(float(atof(parameterString.c_str())));

	//		if(_stricmp(thisTransform.c_str(),"matrix") == 0)
	//		{
	//			if(parameters.size() == 6)
	//			{
	//				glm::mat3 matrix( parameters[0], parameters[2], parameters[4],
	//								  parameters[1], parameters[3], parameters[5],
	//								  0.0f,          0.0f,          1.0f           );

	//				// a cos(delta), -sin(delta),
	//				// sin(delta), b cos(delta);
	//			
	//				float rotation = asinf(parameters[1]);
	//				float rotation2 = asinf(-parameters[2]);
	//				float avgRotation = (rotation + rotation2) / 2.0f; 

	//				result.translateX += parameters[4];
	//				result.translateY += parameters[5];
	//			}
	//		}
	//		if(_stricmp(thisTransform.c_str(),"translate") == 0)
	//		{
	//			if(parameters.size() == 2)
	//			{
	//				result.translateX += parameters[0];
	//				result.translateY += parameters[1];
	//			}
	//		}
	//		if(_stricmp(thisTransform.c_str(),"scale") == 0)
	//		{
	//			if(parameters.size() == 1)
	//			{
	//				parameters.push_back(parameters[0]);
	//			}
	//			if(parameters.size() == 2)
	//			{
	//				result.scaleX *= parameters[0];
	//				result.scaleY *= parameters[1];
	//			}
	//		}
	//		if(_stricmp(thisTransform.c_str(),"rotate") == 0)
	//		{
	//			if(parameters.size() == 1)
	//			{
	//				result.rotate += parameters[0];
	//			}
	//			// parameters size == 3 not yet implemented
	//		}
	//		if(_stricmp(thisTransform.c_str(),"skewX") == 0)
	//		{
	//			if(parameters.size() == 1)
	//			{
	//				// not yet implemented
	//			}
	//		}
	//		if(_stricmp(thisTransform.c_str(),"skewY") == 0)
	//		{
	//			if(parameters.size() == 1)
	//			{
	//				// not yet implemented
	//			}
	//		}
	//	
	//		transformString = transformString.substr(rightBracketPos + 1);
	//	}

	//	return result; // not yet implemented

	//}

	return Transform();
}

void SvgParser::ParsePresentationals(tinyxml2::XMLElement * element, Stylesheet & stylesheet)
{
	for(const tinyxml2::XMLAttribute * attribute = element->FirstAttribute();
		attribute != 0; attribute = attribute->Next())
	{
		ParseStyleProperty(attribute->Name(), attribute->Value(), stylesheet);
	}
}

void SvgParser::ParseStyleProperty(const char * key, const char * value, Stylesheet & stylesheet)
{
	/*
		‘color-profile’, ‘color-rendering’, ‘color’, ‘cursor’, ‘direction’, ‘display’, 
		‘dominant-baseline’, ‘enable-background’, ‘FILL-OPACITY’, ‘fill-rule’, 'FILL’, 
		‘filter’, ‘flood-color’, ‘flood-opacity’, ‘font-family’, ‘font-size-adjust’, 
		‘font-size’, ‘font-stretch’, ‘font-style’, ‘font-variant’, ‘font-weight’, 
		‘glyph-orientation-horizontal’, ‘glyph-orientation-vertical’, ‘image-rendering’, 
		‘kerning’, ‘letter-spacing’, ‘lighting-color’, ‘marker-end’, ‘marker-mid’, 
		‘marker-start’, ‘mask’, ‘OPACITY’, ‘overflow’, ‘pointer-events’, 
		‘shape-rendering’, ‘stop-color’, ‘stop-opacity’, ‘stroke-dasharray’, 
		‘stroke-dashoffset’, ‘stroke-linecap’, ‘stroke-linejoin’, ‘stroke-miterlimit’, 
		‘STROKE-OPACITY’, ‘STROKE-WIDTH’, ‘STROKE’, ‘text-anchor’, ‘text-decoration’, 
		‘text-rendering’, ‘unicode-bidi’, ‘visibility’, ‘word-spacing’ and ‘writing-mode’
	*/

	if(_stricmp(key,"fill") == 0)
	{
		ParseColor(value, 
			stylesheet.fillR,
			stylesheet.fillG,
			stylesheet.fillB,
			stylesheet.fillA,
			&stylesheet.fillGradient);
	}
	if(_stricmp(key,"fill-opacity") == 0)
	{
		stylesheet.fillA *= (float) atof(value);
	}
	if(_stricmp(key,"opacity") == 0)
	{
		stylesheet.opacity *= (float) atof(value);
	}
	if(_stricmp(key,"stroke") == 0)
	{
		ParseColor(value,
			stylesheet.strokeR,
			stylesheet.strokeG,
			stylesheet.strokeB,
			stylesheet.strokeA,
			&stylesheet.strokeGradient);
		stylesheet.strokeWidth = 1.0f;
	}
	if(_stricmp(key,"stroke-opacity") == 0)
	{
		stylesheet.strokeA *= (float) atof(value);
	}
	if(_stricmp(key,"stroke-width") == 0)
	{
		stylesheet.strokeWidth = (float) atof(value);
	}
	if(_stricmp(key,"stroke-linecap") == 0)
	{
		if(_stricmp(value,"butt") == 0)   stylesheet.capType = GeoBuilder::CapButt;
		if(_stricmp(value,"square") == 0) stylesheet.capType = GeoBuilder::CapSquare;
		if(_stricmp(value,"round") == 0)  stylesheet.capType = GeoBuilder::CapRound;
	}
	if(_stricmp(key,"stroke-linejoin") == 0)
	{
		if(_stricmp(value,"miter") == 0) stylesheet.cornerType = GeoBuilder::CornerMiter;
		if(_stricmp(value,"bevel") == 0) stylesheet.cornerType = GeoBuilder::CornerBevel;
		if(_stricmp(value,"round") == 0) stylesheet.cornerType = GeoBuilder::CornerRound;
	}
	if(_stricmp(key,"stroke-miterlimit") == 0)
	{
		stylesheet.miterLimit = (float) atof(value);
	}
}

void SvgParser::ParseStopStyleProperty(const char * key, const char * value, Gradient * gradient)
{
	float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
	if(_stricmp(key,"stop-color") == 0)
	{
		ParseColor(value,r,g,b,a,0);
		gradient->colors.emplace_back(r,g,b,a);
	}
	if(_stricmp(key,"stop-opacity") == 0)
	{
		Gradient::Color & newColor = gradient->colors.back();
		newColor.a *= (float) atof(value);
	}
}

void SvgParser::ParseColor(const char * value, float & red, float & green, float & blue, float & alpha, Gradient * gradient)
{
	float r = 1.0f, g = 1.0f, b = 1.0f;

	/* AQUA, BLACK, BLUE, fuchsia, gray, GREEN, lime, maroon, 
	navy, olive, purple, RED, silver, teal, WHITE, and YELLOW */

	if(_stricmp(value,"aqua") == 0)  { r = 0.0f; g = 1.0f; b = 1.0f; }
	if(_stricmp(value,"black") == 0) { r = 0.0f; g = 0.0f; b = 0.0f; }
	if(_stricmp(value,"blue") == 0)  { r = 0.0f; g = 0.0f; b = 1.0f; }
	if(_stricmp(value,"green") == 0) { r = 0.0f; g = 1.0f; b = 0.0f; }
	if(_stricmp(value,"red") == 0)   { r = 1.0f; g = 0.0f; b = 0.0f; }
	if(_stricmp(value,"yellow") == 0){ r = 1.0f; g = 1.0f; b = 0.0f; }
	if(_stricmp(value,"white") == 0) { r = 1.0f, g = 1.0f, b = 1.0f; }

	const char * urlChar = strstr(value,"url(");
	if(urlChar != 0)
	{
		std::string valString(value);
		valString = valString.substr(4);
		unsigned idStart = valString.find('#');
		unsigned idEnd = valString.find(')');
		if(idStart != std::string::npos && idEnd != std::string::npos)
		{
			valString = valString.substr(idStart + 1, idEnd - idStart - 1);
			SvgDefs::iterator it = definitions.find(valString);
			if(it != definitions.end())
			{
				if(it->second->type == TypeGradient)
				{
					if(gradient)
					{
						Gradient * referencedGradient = static_cast<Gradient*>(it->second);
						*gradient = *referencedGradient;
					}
				}
			}
		}
	}
	else
	{
		const char * hexChar = strchr(value,'#');
		if(hexChar != 0)
		{
			std::string valString(value);
			unsigned offset = hexChar - value;
			unsigned red   = (unsigned) strtoul(valString.substr(offset + 1,2).c_str(), 0, 16);
			unsigned green = (unsigned) strtoul(valString.substr(offset + 3,2).c_str(), 0, 16);
			unsigned blue  = (unsigned) strtoul(valString.substr(offset + 5,2).c_str(), 0, 16);

			r = (((float)red) / 255.0f);
			g = (((float)green) / 255.0f);
			b = (((float)blue) / 255.0f);
		}
	}

	if(_stricmp(value,"none") == 0)
	{
		red = 0.0f;
		green = 0.0f;
		blue = 0.0f;
		alpha = 0.0f;
	}
	else
	{
		red = r;
		green = g;
		blue = b;
		alpha = 1.0f;
	}
}

void SvgParser::ParseDefinitions(tinyxml2::XMLElement * defsElement)
{
	if(!defsElement->NoChildren())
	{
		tinyxml2::XMLElement * element = defsElement->FirstChildElement();

		while(element)
		{
			if(_stricmp(element->Name(),"LinearGradient") == 0)
			{
				const char * id = element->Attribute("id");
				Gradient * gradient = new Gradient();
				if(definitions.find(id) != definitions.end()) delete definitions[id];
				definitions[id] = gradient;

				const char * hrefChars = element->Attribute("xlink:href");
				if(hrefChars)
				{
					std::string href(hrefChars);
					if(href.length() > 0)
					{
						if(href.find("#") == 0)
						{
							SvgDefs::iterator parentIterator = definitions.find(href.substr(1));
							if(parentIterator != definitions.end() && parentIterator->second->type == TypeGradient)
							{
								Gradient * parentGradient = static_cast<Gradient*>(parentIterator->second);
								*gradient = *parentGradient;
							}
						}
					}
				}

				const char * spreadMethodChars = element->Attribute("spreadMethod");
				if(spreadMethodChars != 0)
				{
					if(_stricmp(spreadMethodChars,"pad") == 0)
					{
						gradient->spreadMethod = Gpu::SamplerParam::AddressClamp;
					}
					if(_stricmp(spreadMethodChars,"reflect") == 0)
					{
						gradient->spreadMethod = Gpu::SamplerParam::AddressMirror;
					}
					if(_stricmp(spreadMethodChars,"repeat") == 0)
					{
						gradient->spreadMethod = Gpu::SamplerParam::AddressWrap;
					}
				}

				tinyxml2::XMLElement * stop = element->FirstChildElement();

				while(stop)
				{
					const char * styleString = stop->Attribute("style");
					if(styleString)
					{
						std::stringstream style(styleString);
						std::string key, value;

						while(GetNextCssKeyValue(style,key,value))
						{
							ParseStopStyleProperty(key.c_str(), value.c_str(), gradient);
						}
					}

					float offset = (float) atof(stop->Attribute("offset"));
					gradient->offsets.push_back(offset);

					stop = stop->NextSiblingElement();
				}

				const char * x1chars = element->Attribute("x1");
				const char * y1chars = element->Attribute("y1");
				const char * x2chars = element->Attribute("x2");
				const char * y2chars = element->Attribute("y2");

				if(x1chars) gradient->x1 = (float) atof(x1chars);
				if(y1chars) gradient->y1 = (float) atof(y1chars);
				if(x2chars) gradient->x2 = (float) atof(x2chars);
				if(y2chars) gradient->y2 = (float) atof(y2chars);
			}

			if(_stricmp(element->Name(),"RadialGradient") == 0)
			{
				const char * id = element->Attribute("id");
				Gradient * gradient = new Gradient();
				if(definitions.find(id) != definitions.end()) delete definitions[id];
				definitions[id] = gradient;

				gradient->radial = true;

				const char * transformChars = element->Attribute("gradientTransform");
				Transform transform = ParseTransform(transformChars);

				const char * x1chars = element->Attribute("cx");
				const char * y1chars = element->Attribute("cy");
				const char * x2chars = element->Attribute("r");
				const char * y2chars = element->Attribute("r");

				if(x1chars) gradient->x1 = (float) atof(x1chars);
				if(y1chars) gradient->y1 = (float) atof(y1chars);
				if(x2chars) gradient->x2 = (float) atof(x2chars);
				if(y2chars) gradient->y2 = (float) atof(y2chars);
			}

			element = element->NextSiblingElement();
		}
	}
};

void SvgParser::BuildArc(float arcRadiusX, float arcRadiusY, float arcRotation, bool arcLargeFlag, bool arcSweepFlag,
						 float penPosX, float penPosY, float arcTargetX, float arcTargetY, std::vector<Path::Point> & pathPoints)
{
	// http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes

	float arcMidpointX = (penPosX + arcTargetX) / 2.0f;
	float arcMidpointY = (penPosY + arcTargetY) / 2.0f;
	float halfDeltaX = (penPosX - arcTargetX) / 2.0f;
	float halfDeltaY = (penPosY - arcTargetY) / 2.0f;

	float rotMidpointX = cosf(arcRotation)*halfDeltaX + sinf(arcRotation)*halfDeltaY;
	float rotMidpointY = -sinf(arcRotation)*halfDeltaX + cosf(arcRotation)*halfDeltaY;

	float arcRadiusX2 = powf(arcRadiusX,2.0f);
	float arcRadiusY2 = powf(arcRadiusY,2.0f);
	float rotMidpointX2 = powf(rotMidpointX,2.0f);
	float rotMidpointY2 = powf(rotMidpointY,2.0f);

	float scaleUpCoefficient = (rotMidpointX2/arcRadiusX2) + (rotMidpointY2/arcRadiusY2) + FLT_EPSILON;
	if(scaleUpCoefficient > 1.0f)
	{
		arcRadiusX *= sqrtf(scaleUpCoefficient);
		arcRadiusY *= sqrtf(scaleUpCoefficient);
		arcRadiusX2 = powf(arcRadiusX,2.0f);
		arcRadiusY2 = powf(arcRadiusY,2.0f);
	}
							
	float numerator = arcRadiusX2*arcRadiusY2 - arcRadiusX2*rotMidpointY2 - arcRadiusY2*rotMidpointX2;
	float denominator = arcRadiusX2*rotMidpointY2 + arcRadiusY2*rotMidpointX2;
	float fraction = numerator/denominator;

	if(fraction >= 0.0f)
	{
		float offsetFactor = sqrtf(fraction);
		if(arcLargeFlag == arcSweepFlag) offsetFactor = -offsetFactor;

		float rotCenterX = (offsetFactor*arcRadiusX*rotMidpointY)/arcRadiusY;
		float rotCenterY = -(offsetFactor*arcRadiusY*rotMidpointX)/arcRadiusX;

		float arcCenterX = cosf(arcRotation)*rotCenterX - sinf(arcRotation)*rotCenterY + arcMidpointX;
		float arcCenterY = sinf(arcRotation)*rotCenterX + cosf(arcRotation)*rotCenterY + arcMidpointY;

		float rotStartDX = (rotMidpointX - rotCenterX) / arcRadiusX;
		float rotStartDY = (rotMidpointY - rotCenterY) / arcRadiusY;
		float rotStartHyp = sqrtf(powf(rotStartDX,2.0f) + powf(rotStartDY,2.0f));

		float startAngle = acosf(rotStartDX/rotStartHyp);
		if(startAngle > ((float)M_PI))  startAngle -= ((float)M_PI*2.0);
		if(startAngle < -((float)M_PI)) startAngle += ((float)M_PI*2.0);
		if(rotStartDY < 0) startAngle = -startAngle;

		float rotEndDX = (-rotMidpointX - rotCenterX) / arcRadiusX;
		float rotEndDY = (-rotMidpointY - rotCenterY) / arcRadiusY;
		float rotEndHyp = sqrtf(powf(rotEndDX,2.0f) + powf(rotEndDY,2.0f));

		float deltaAngleNumerator = rotStartDX*rotEndDX + rotStartDY*rotEndDY;
		float deltaAngleDenominator = rotStartHyp * rotEndHyp;
		if(deltaAngleNumerator < -deltaAngleDenominator) deltaAngleNumerator = -deltaAngleDenominator;
		if(deltaAngleNumerator >  deltaAngleDenominator) deltaAngleNumerator =  deltaAngleDenominator;
		float deltaAngle = acosf(deltaAngleNumerator/deltaAngleDenominator);
		float deltaAngleDirectionality = rotStartDX*rotEndDY - rotEndDX*rotStartDY;
		if(deltaAngleDirectionality < 0) deltaAngle = -deltaAngle;

		if(deltaAngle < ((float) M_PI * 2.0))  deltaAngle += ((float) M_PI * 2.0);
		if(deltaAngle > ((float) M_PI * 2.0))  deltaAngle -= ((float) M_PI * 2.0);
		if(!arcSweepFlag && deltaAngle > 0.0f) deltaAngle -= ((float) M_PI * 2.0);
		if(arcSweepFlag && deltaAngle < 0.0f)  deltaAngle += ((float) M_PI * 2.0);
						
		// construct the ellipse here!
		const float ARC_POINTS = 50.0f;
		for(float i = 1.0f; i < ARC_POINTS; ++i)
		{
			float angle = startAngle + ((deltaAngle / (ARC_POINTS -1.0f)) * i);
			float unrotatedX = arcRadiusX * cosf(angle);
			float unrotatedY = arcRadiusY * sinf(angle);
			float x = arcCenterX + (cosf(arcRotation)*unrotatedX - sinf(arcRotation)*unrotatedY);
			float y = arcCenterY + (sinf(arcRotation)*unrotatedX + cosf(arcRotation)*unrotatedY);
			pathPoints.emplace_back(x,y);
		}

	}
	else
	{
		pathPoints.emplace_back(arcTargetX,arcTargetY);
	}
}

void SvgParser::BuildCubicBezier(float x1, float y1, float x2, float y2, float xc1, float yc1, 
								 float xc2, float yc2, std::vector<Path::Point> & points)
{
	float cx = 3 * (xc1 - x1);
	float bx = (3 * (xc2 - xc1)) - cx;
	float ax = x2 - x1 - cx - bx;

	float cy = 3 * (yc1 - y1);
	float by = (3 * (yc2 - yc1)) - cy;
	float ay = y2 - y1 - cy - by;

	const float BEZIER_POINTS = 50.0f;
	for(float i = 1.0f; i <= BEZIER_POINTS; ++i)
	{
		float t = i / BEZIER_POINTS;
		float t2 = t * t;
		float t3 = t2 * t;

		float x = (ax * t3) + (bx * t2) + (cx * t) + x1;
		float y = (ay * t3) + (by * t2) + (cy * t) + y1;
		points.emplace_back(x,y);
	}
}

void SvgParser::BuildQuadBezier(float x1, float y1, float x2, float y2,
								float xc, float yc, std::vector<Path::Point> & points)
{
	const float BEZIER_POINTS = 50.0f;
	for(float i = 1.0f; i <= BEZIER_POINTS; ++i)
	{
		float t = i / BEZIER_POINTS;

		float xc1 = x1 + ((xc - x1) * t);
		float yc1 = y1 + ((yc - y1) * t);
		float xc2 = xc + ((x2 - xc) * t);
		float yc2 = yc + ((y2 - yc) * t);

		float x = xc1 + ((xc2 - xc1) * t);
		float y = yc1 + ((yc2 - yc1) * t);
		points.emplace_back(x,y);
	}
}

void SvgParser::GradientPainter::PaintGradient(Gpu::Api * gpu)
{
	unsigned textureLength = 256;
	unsigned textureWidth  = 2;

	// 1. Build a line mesh with however many coloured vertex pairs

	VertexBuffer<Vertex_PosCol> * v = new VertexBuffer<Vertex_PosCol>(gradient.offsets.size() * 2);
	unsigned numIndices = (gradient.offsets.size() - 1) * 6;
	unsigned * k = new unsigned[numIndices];

	for(unsigned i = 0; i < gradient.offsets.size(); ++i)
	{
		float offset = gradient.offsets[i];
		Gradient::Color color = gradient.colors[i];

		 // UH OH! NO ALPHA!!!!! FIXME
		v->Set((i*2) + 0, Vertex_PosCol((offset-0.5f) * float(textureLength),  float(textureWidth)/2.0f, 0.0f, color.r, color.g, color.b));
		v->Set((i*2) + 1, Vertex_PosCol((offset-0.5f) * float(textureLength), -float(textureWidth)/2.0f, 0.0f, color.r, color.g, color.b));

		if(i > 0)
		{
			// 0, 2, 1 // 2, 3, 1 //
			k[((i-1)*6) + 0] = (i*2) - 2;
			k[((i-1)*6) + 1] = (i*2);
			k[((i-1)*6) + 2] = (i*2) - 1;
			k[((i-1)*6) + 3] = (i*2);
			k[((i-1)*6) + 4] = (i*2) + 1;
			k[((i-1)*6) + 5] = (i*2) - 1;
		}
	}

	LocalMesh * gradientMesh = new LocalMesh(v,k,numIndices);
	Gpu::Mesh * gradientGpuMesh = gradientMesh->GpuOnly(gpu);

	// 2. Using an orthographic camera, draw that line mesh to a surface

	Gpu::Camera orthoCamera;
	orthoCamera.isOrthoCamera = true;
	orthoCamera.fovOrHeight = float(textureWidth);
	orthoCamera.position.x = 0.0f, orthoCamera.position.y = 0.0f; orthoCamera.position.z = -10.0f;
	orthoCamera.target.x = 0.0f; orthoCamera.target.y = 0.0f; orthoCamera.target.z = 0.0f;

	Gpu::Model gradientModel;
	gradientModel.mesh = gradientGpuMesh;

	surface->Clear();
	gpu->DrawGpuModel(&gradientModel, &orthoCamera, 0, 0, surface);

	delete gradientGpuMesh;
}

bool SvgParser::TokenizeDescription(const std::string & desc, std::queue<DescToken> & tokenQueue)
{
	static const char tokens[] = { 
		'M', 'm', //move
		'L', 'l', //lineto
		'Q', 'q', //quadratic
		'C', 'c', //cubic
		'T', 't', //quadratic
		'S', 's', //cubic
		'A', 'a', //arcto
		'Z', 'z', //close
		'h', 'v'  //horiz/vert
	};

	std::string numericToken("");
	for(unsigned i = 0; i < desc.length(); ++i)
	{
		char curChar = desc[i];
		if((curChar >= '0' && curChar <= '9') || curChar == '-' || curChar == '.')
		{
			// character is numeric
			numericToken += curChar;
		}
		else
		{
			if(numericToken.length() > 0)
			{
				float value = std::stof(numericToken);
				if(!_isnan((double)value))
				{
					DescToken token;
					token.op = false;
					token.value = value;
					tokenQueue.push(token);
				}
				numericToken = "";
			}
			for(int j = 0; j < 18; ++j)
			{
				if(curChar == tokens[j])
				{
					DescToken token;
					token.op = true;
					token.opChar = tokens[j];
					tokenQueue.push(token);
					break;
				}
			}
		}
	}
	if(numericToken.length() > 0)
	{
		float value = std::stof(numericToken);
		if(!_isnan((double)value))
		{
			DescToken token;
			token.op = false;
			token.value = value;
			tokenQueue.push(token);
		}
	}
	return true;
}

bool SvgParser::GetNextCssKeyValue(std::stringstream & css, std::string & key, std::string & value)
{
	std::string statement;

	if(std::getline(css,statement,';'))
	{
		unsigned assignmentChar = statement.find_first_of(':');

		if(assignmentChar != std::string::npos)
		{
			key = statement.substr(0,assignmentChar);
			value = statement.substr(assignmentChar + 1, (statement.length() - (assignmentChar + 1)));
			return true;
		}
	}
	return false;
}

void SvgParser::ParseSvg(Files::Directory * directory, char * data, unsigned dataSize)
{
	tinyxml2::XMLDocument svgDoc;

	if(svgDoc.Parse(data, dataSize) != 0) return;
	
	tinyxml2::XMLElement * root = svgDoc.FirstChildElement("svg");

	if(!root) return; // Not an SVG document

	unsigned numMeshes = 0;
	unsigned level = 0;
	tinyxml2::XMLElement * element = root->FirstChildElement();
	while(element)
	{
		if(_stricmp(element->Name(),"g") == 0)
		{
			if(element->NoChildren())
			{
				element = element->NextSiblingElement();
			}
			else
			{
				++level;
				element = element->FirstChildElement();
			}

			// graphic groups have inheritable stylesheets!!!
		}
		else if(_stricmp(element->Name(),"defs") == 0)
		{
			ParseDefinitions(element);
			element = element->NextSiblingElement();
		}
		else 
		{
			if(_stricmp(element->Name(),"rect") == 0 
				|| _stricmp(element->Name(),"circle") == 0 
				|| _stricmp(element->Name(),"image") == 0
				|| _stricmp(element->Name(),"path") == 0)
			{
				graphics.emplace_back();
				Graphic & graphic = graphics.back();

				ParsePresentationals(element, graphic.stylesheet);

				const char * style = element->Attribute("style");
				ParseStyle(style, graphic.stylesheet);

				const char * transform = element->Attribute("transform");
				graphic.transform = ParseTransform(transform);

				if(_stricmp(element->Name(),"rect") == 0)
				{
					ParseRect(element, graphic);
				}
				if(_stricmp(element->Name(),"circle") == 0)
				{
					ParseCircle(element, graphic);
				}
				if(_stricmp(element->Name(),"image") == 0)
				{
					if(assets)
					{
						const char * imgUrlChars = element->Attribute("xlink:href");
						if(imgUrlChars)
						{
							std::string imgUrl(imgUrlChars);
							if(imgUrl.length() > 0)
							{
								if(imgUrl.find("file:///",0) == 0)
								{
									imgUrl = imgUrl.substr(8);
								}
						
								unsigned htmlSpacePos = std::string::npos;
								while((htmlSpacePos = imgUrl.find("%20")) != std::string::npos)
								{
									imgUrl.replace(htmlSpacePos,3," ");
								}

								std::wstring wimgUrl(imgUrl.begin(),imgUrl.end());
								assetTicket = assets->Load(directory, wimgUrl.c_str(), TextureAsset, 0, assetTicket);
								// FIXME: NEED TO TRIGGER AND WAIT FOR TEXTURE LOAD
								//assets->AddTexture(directory, wimgUrl.c_str(), texture);
								//if(texture) graphic.texture = texture;
							}
						}
					}
					
					ParseRect(element,graphic,graphic.texture!=0);
				}
				if(_stricmp(element->Name(),"path") == 0)
				{
					const char * description = element->Attribute("d");
					ParseDescription(description, graphic);
				}

				if(graphic.fill) ++numMeshes;
				if(graphic.stroke) ++numMeshes;
			}

			if(element->NextSiblingElement() == 0 && level > 0)
			{
				--level;
				element = element->Parent()->NextSiblingElement();
			}
			else
			{
				element = element->NextSiblingElement();
			}
		}
	}
}

Gpu::ComplexModel * SvgParser::GetModel(Gpu::Api * gpu)
{
	Files::Directory * frameworkDir = assets->GetFileApi()->GetKnownDirectory(Files::FrameworkDir);
	Gpu::Shader * pathShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"PathShader.xml");

	Gpu::ComplexModel * modelGroup = new Gpu::ComplexModel(graphics.size() * 2);
	unsigned currentModel = 0;
	for(unsigned i = 0; i < graphics.size(); ++i)
	{
		Graphic & graphic = graphics[i];

		if(graphic.fill)
		{
			Gpu::Model & model = modelGroup->models[currentModel];

			model.mesh = graphic.fill->ToGpuMesh(gpu);
			model.color.r = graphic.stylesheet.fillR;
			model.color.g = graphic.stylesheet.fillG;
			model.color.b = graphic.stylesheet.fillB;
			model.color.a = graphic.stylesheet.fillA * graphic.stylesheet.opacity;
			model.position.x += graphic.transform.translateX;
			model.position.y -= graphic.transform.translateY;
			model.rotation.z += graphic.transform.rotate;
			model.scale *= graphic.transform.scaleX;
			model.texture = graphic.texture;
			if(pathShader)
			{
				model.effect = new Gpu::Effect(pathShader);
				if(graphic.stylesheet.fillGradient.offsets.size() > 0)
				{
					Gradient & gradient = graphic.stylesheet.fillGradient;
					model.effect->SetParam(0, 1.0f);
					model.effect->SetParam(1, gradient.x1);
					model.effect->SetParam(2, gradient.y1);
					model.effect->SetParam(3, gradient.x2);
					model.effect->SetParam(4, gradient.y2);
					GradientPainter * gradientPainter = new GradientPainter(gradient, gpu);
					model.effect->SetParam(5, gradientPainter->surface);
					gpu->AddDeviceListener(gradientPainter);
					model.effect->SetSamplerParam(Gpu::SamplerParam::AddressU, gradient.spreadMethod, 1);
				}
				else
				{
					model.effect->SetParam(0, 0.0f);
				}
			}
			//model.destructMesh = true;
			model.backFaceCull = false;
			//model.wireframe = true;

			++currentModel;
		}

		if(graphic.stroke)
		{
			Gpu::Model & model = modelGroup->models[currentModel];

			model.mesh = graphic.stroke->ToGpuMesh(gpu);
			model.color.r = graphic.stylesheet.strokeR;
			model.color.g = graphic.stylesheet.strokeG;
			model.color.b = graphic.stylesheet.strokeB;
			model.color.a = graphic.stylesheet.strokeA * graphic.stylesheet.opacity;
			model.position.x += graphic.transform.translateX;
			model.position.y = graphic.transform.translateY;
			model.rotation.z += graphic.transform.rotate;
			model.scale *= graphic.transform.scaleX;
			if(pathShader)
			{
				model.effect = new Gpu::Effect(pathShader);
				if(graphic.stylesheet.strokeGradient.offsets.size() > 0)
				{
					Gradient & gradient = graphic.stylesheet.strokeGradient;
					model.effect->SetParam(0, 1.0f);
					model.effect->SetParam(1, gradient.x1);
					model.effect->SetParam(2, gradient.y1);
					model.effect->SetParam(3, gradient.x2);
					model.effect->SetParam(4, gradient.y2);
					GradientPainter * gradientPainter = new GradientPainter(gradient, gpu);
					model.effect->SetParam(5, gradientPainter->surface);
					gpu->AddDeviceListener(gradientPainter);
					model.effect->SetSamplerParam(Gpu::SamplerParam::AddressU, gradient.spreadMethod, 1);
				}
				else
				{
					model.effect->SetParam(0, 0.0f);
				}
			}
			//model.destructMesh = true;
			model.backFaceCull = false;
			//model.wireframe = true;

			++currentModel;
		}
	}

	return modelGroup;
}

Gpu::ComplexModel * SvgParser::GetAnimatedStroke(Gpu::Api * gpu, float animProgress)
{
	//Files::Directory * frameworkDir = assets->GetFileApi()->GetKnownDirectory(Files::FrameworkDir);
	//Gpu::Shader * pathShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"PathShader.xml");

	Gpu::ComplexModel * modelGroup = new Gpu::ComplexModel(graphics.size());
	unsigned currentModel = 0;
	for(unsigned i = 0; i < graphics.size(); ++i)
	{
		Graphic & graphic = graphics[i];

		Gpu::Model & model = modelGroup->models[currentModel];

		Shape & shape = graphic.shape;
		LocalMesh * animStrokeMesh = 0;

		switch(graphic.shape.type)
		{
		case ShapeRect:
			animStrokeMesh = builder.BuildRectStroke(shape.x, shape.y, shape.w, shape.h, graphic.stylesheet.strokeWidth);
			break;
		case ShapeCircle:
			animStrokeMesh = builder.BuildEllipseStroke(shape.x, shape.y, shape.w, shape.h, graphic.stylesheet.strokeWidth);
			break;
		case ShapePath:
			animStrokeMesh = builder.BuildStroke(
				shape.pathPoints.data(),
				shape.pathPoints.size(),
				graphic.stylesheet.strokeWidth,
				graphic.stylesheet.cornerType,
				graphic.stylesheet.capType,
				graphic.stylesheet.miterLimit,
				animProgress);
			break;
		}

		if(animStrokeMesh)
		{
			model.mesh = animStrokeMesh->GpuOnly(gpu);
		}

		model.color.r = graphic.stylesheet.strokeR;
		model.color.g = graphic.stylesheet.strokeG;
		model.color.b = graphic.stylesheet.strokeB;
		model.color.a = graphic.stylesheet.strokeA * graphic.stylesheet.opacity;
		model.position.x += graphic.transform.translateX;
		model.position.y = graphic.transform.translateY;
		model.rotation.z += graphic.transform.rotate;
		model.scale *= graphic.transform.scaleX;
		//if(pathShader)
		//{
		//	model.effect = new Gpu::Effect(pathShader);
		//	if(graphic.stylesheet.strokeGradient.offsets.size() > 0)
		//	{
		//		Gradient & gradient = graphic.stylesheet.strokeGradient;
		//		model.effect->SetParam(0, 1.0f);
		//		model.effect->SetParam(1, gradient.x1);
		//		model.effect->SetParam(2, gradient.y1);
		//		model.effect->SetParam(3, gradient.x2);
		//		model.effect->SetParam(4, gradient.y2);
		//		GradientPainter * gradientPainter = new GradientPainter(gradient, gpu);
		//		model.effect->SetParam(5, gradientPainter->surface);
		//		gpu->AddDeviceListener(gradientPainter);
		//		model.effect->SetSamplerParam(Gpu::SamplerParam::AddressU, gradient.spreadMethod, 1);
		//	}
		//	else
		//	{
		//		model.effect->SetParam(0, 0.0f);
		//	}
		//	//model.destructEffect = true;
		//}
		model.destructMesh = true;
		model.backFaceCull = false;
		//model.wireframe = true;

		++currentModel;
	}

	return modelGroup;
}

} // namespace Ingenuity
