#pragma once

#include "GeoBuilder.h"
#include <vector>
#include <queue>
#include <map>
#include <string>

namespace tinyxml2
{
class XMLElement;
};

namespace Ingenuity {

namespace Gpu {
struct ComplexModel;
struct Effect;
struct Texture;
struct DrawSurface;
}

struct LocalMesh;
class AssetMgr;

class SvgParser : public IAsset
{
	struct DescToken
	{
		union
		{
			char opChar;
			float value;
		};
		bool op;
	};

	struct Transform
	{
		float scaleX, scaleY;
		float translateX, translateY;
		float rotate;
		Transform() : scaleX(1.0f), scaleY(1.0f),
			translateX(0.0f), translateY(0.0f), rotate(0.0f) {}
	};

	enum SvgObjectType
	{
		TypeGraphic,
		TypeGradient
	};

	struct SvgObject
	{
		SvgObjectType type;

		virtual ~SvgObject() {}
	protected:
		SvgObject(SvgObjectType type) : type(type) {}
	};

	struct Gradient : SvgObject
	{
		struct Color
		{
			float r, g, b, a;
			Color(float r, float g, float b, float a)
				: r(r), g(g), b(b), a(a) {}
		};

		std::vector<Color> colors;
		std::vector<float> offsets;

		Gpu::SamplerParam::AddressMode spreadMethod;
		float x1, y1, x2, y2;
		bool radial;

		Gradient() :
			SvgObject(TypeGradient),
			spreadMethod(Gpu::SamplerParam::AddressClamp),
			x1(0.0f), y1(0.0f), x2(0.0f), y2(0.0f), radial(false)
		{}
		virtual ~Gradient() {}
	};

	struct Stylesheet
	{
		float fillR, fillG, fillB, fillA,
			strokeR, strokeG, strokeB, strokeA,
			strokeWidth, opacity, miterLimit;

		GeoBuilder::StrokeCapType capType;
		GeoBuilder::StrokeCornerType cornerType;

		Gradient fillGradient;
		Gradient strokeGradient;

		Stylesheet() :
			fillR(1.0f),
			fillG(1.0f),
			fillB(1.0f),
			fillA(1.0f),
			strokeR(1.0f),
			strokeG(1.0f),
			strokeB(1.0f),
			strokeA(1.0f),
			strokeWidth(0.0f),
			opacity(1.0f),
			miterLimit(4.0f),
			capType(GeoBuilder::CapButt),
			cornerType(GeoBuilder::CornerMiter)
		{}
	};

	enum SvgShapeType
	{
		ShapeRect,
		ShapeCircle,
		ShapePath
	};

	struct Shape
	{
		SvgShapeType type;

		float x, y, w, h;
		std::vector<Path::Point> pathPoints;

		virtual ~Shape() {}
		Shape() :
			x(0.0f),
			y(0.0f),
			w(0.0f),
			h(0.0f) {}
	};

	struct Graphic : SvgObject
	{
		Shape shape;
		Stylesheet stylesheet;
		Transform transform;
		LocalMesh * fill;
		LocalMesh * stroke;
		Gpu::Texture * texture;

		Graphic() : SvgObject(TypeGraphic), fill(0), stroke(0), texture(0) {}
		virtual ~Graphic() {}
	};

	struct GradientPainter : public Gpu::IDeviceListener
	{
		static const unsigned TEXTURE_LENGTH = 256;
		static const unsigned TEXTURE_WIDTH = 2;

		Gradient gradient;
		Gpu::DrawSurface * surface;

		void PaintGradient(Gpu::Api * gpu);

		virtual void OnLostDevice(Gpu::Api * gpu) {}
		virtual void OnResetDevice(Gpu::Api * gpu) { PaintGradient(gpu); }

		GradientPainter(Gradient gradient, Gpu::Api * gpu)
			: gradient(gradient)
		{
			surface = gpu->CreateDrawSurface(TEXTURE_LENGTH, TEXTURE_WIDTH);
			PaintGradient(gpu);
		}
		~GradientPainter()
		{
			delete surface;
		}
	};

	struct PendingTexture
	{
		unsigned ticket;
		Graphic * Graphic;
	};

	GeoBuilder builder;
	typedef std::map<std::string, SvgObject*> SvgDefs;
	SvgDefs definitions;
	std::vector<Graphic> graphics;
	std::vector<PendingTexture> pendingTextures;
	AssetMgr * assets;
	int assetTicket;

	void ParseRect(tinyxml2::XMLElement * element, Graphic & graphic, bool image = false);
	void ParseCircle(tinyxml2::XMLElement * element, Graphic & graphic);
	void ParseStyle(const char * styleString, Stylesheet & stylesheet);
	void ParseDescription(const char * description, Graphic & graphic);
	Transform ParseTransform(const char * transform);
	void ParsePresentationals(tinyxml2::XMLElement * element, Stylesheet & stylesheet);
	void ParseStyleProperty(const char * key, const char * value, Stylesheet & stylesheet);
	void ParseStopStyleProperty(const char * key, const char * value, Gradient * gradient);
	void ParseColor(const char * value, float & red, float & green, float & blue, float & alpha, Gradient * gradient);
	void ParseDefinitions(tinyxml2::XMLElement * defsElement);

	void BuildArc(float rX, float rY, float angle, bool large, bool sweep,
		float x, float y, float tX, float tY, std::vector<Path::Point> & points);
	void BuildCubicBezier(float x1, float y1, float x2, float y2,
		float xc1, float yc1, float xc2, float yc2, std::vector<Path::Point> & points);
	void BuildQuadBezier(float x1, float y1, float x2, float y2,
		float xc, float yc, std::vector<Path::Point> & points);
	Gpu::DrawSurface * BuildGradientTexture(Gradient * gradient, Gpu::Api * gpu);

	bool TokenizeDescription(const std::string & desc, std::queue<DescToken> & tokenQueue);
	bool GetNextCssKeyValue(std::stringstream & css, std::string & key, std::string & value);

public:
	SvgParser(AssetMgr * assets) : assets(assets), assetTicket(-1) {}
	~SvgParser();

	void SetPixelGranularity(float granularity);

	void ParseSvg(Files::Directory * directory, char * data, unsigned dataSize);
	Gpu::ComplexModel * GetModel(Gpu::Api * gpu);
	Gpu::ComplexModel * GetAnimatedStroke(Gpu::Api * gpu, float animProgress);

	bool IsFinished() { return assetTicket == -1 || assets->IsLoaded(assetTicket); }

	virtual AssetType GetType() override { return SvgAsset; }
	virtual IAsset * GetAsset() override { return this; }
};

struct SvgLoader : public SimpleLoader
{
	AssetMgr * assets;
	Gpu::Api * gpu;
	SvgParser * parser;

	SvgLoader(AssetMgr * assets, Gpu::Api * gpu, Files::Directory * directory, const wchar_t * path) :
		SimpleLoader(assets->GetFileApi(), directory, path, SvgAsset), assets(assets), gpu(gpu), parser(0) {}

	virtual void Respond() override
	{
		if(buffer)
		{
			parser = new SvgParser(assets);
			parser->ParseSvg(directory, buffer, bufferLength);
			asset = parser;
		}
	}

	virtual bool IsAssetReady() override { return complete && parser != 0 && parser->IsFinished(); }
};

} // namespace Ingenuity
