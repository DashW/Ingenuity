#pragma once

#include "AssetMgr.h"

namespace Ingenuity {
namespace Image {

enum FileType
{
	FileTypeBMP,
	FileTypeJPG,
	FileTypePNG,
	FileTypeTGA,

	FileTypeUnknown
};

enum Format
{
	Format_4x8intRGBA,
	Format_4x8intBGRA,
	Format_3x8intRGB,
	Format_1x8intGrey,

	Format_Total
};

struct Color
{
	float r, g, b, a;
	Color() :
		r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
	Color(float r, float g, float b, float a) :
		r(r), g(g), b(b), a(a) {}
};

struct Buffer : public IAsset
{
	virtual ~Buffer() {}
	virtual AssetType GetAssetType() { return ImageAsset; }
	virtual IAsset * GetAsset() { return this; }
	virtual void * GetData() = 0;
	virtual unsigned GetDataSize() = 0;
	virtual void GetImageSize(unsigned & w, unsigned & h) = 0;
	virtual Color GetPixelColor(unsigned u, unsigned v) = 0;
	virtual void SetPixelColor(unsigned u, unsigned v, Color color) = 0;

protected:
	Buffer() {}
};

class Api
{
protected:
	Api() {}
public:
	virtual ~Api() {}

	virtual Buffer * CreateImage(char * data, unsigned dataSize, unsigned w, unsigned h, Format format = Format_4x8intRGBA) = 0;
	virtual Buffer * CreateImage(char * buffer, unsigned bufferLength) = 0;
	//virtual Buffer * Convert(Buffer * image, Format format) = 0;
	//virtual void SaveImage(ImageBuffer * image, const wchar_t * path, ImageFileType fileType) = 0;
};

struct Loader : public SimpleLoader
{
	Api * imaging;

	Loader(Api * imaging, Files::Api * files, Files::Directory * directory, const wchar_t * path) :
		SimpleLoader(files, directory, path, ImageAsset), imaging(imaging) {}

	virtual void Respond() override
	{
		if(buffer)
		{
			asset = imaging->CreateImage(buffer, bufferLength);
		}
	}
};

} // namespace Image
} // namespace Ingenuity
