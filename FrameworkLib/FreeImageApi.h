#pragma once

#ifdef USE_FREE_IMAGEAPI

#include "ImageApi.h"

struct FIBITMAP;

namespace Ingenuity {
namespace FreeImage {

struct Buffer : public Image::Buffer
{
	FIBITMAP * bitmap;

	Buffer(FIBITMAP * bitmap) : bitmap(bitmap) {}
	virtual ~Buffer();
};

class Api : public Image::Api
{
public:
	Api();
	virtual ~Api();

	virtual Image::Buffer * CreateImage(unsigned w, unsigned h) override;
	virtual Image::Buffer * CreateImage(char * buffer, unsigned bufferLength) override;
	//virtual void SaveImage(ImageBuffer * image, const wchar_t * path, ImageApi::ImageFileType fileType) override;

	virtual void GetImageSize(Image::Buffer * image, unsigned & w, unsigned & h) override;
	virtual Image::Color GetPixelColor(Image::Buffer * image, unsigned u, unsigned v) override;
	virtual void SetPixelColor(Image::Buffer * image, unsigned u, unsigned v, Image::Color color) override;
};

} // namespace FreeImage
} // namespace Ingenuity

#endif // USE_FREE_IMAGEAPI