#pragma once

#ifdef USE_WIC_IMAGEAPI

#include "ImageApi.h"

// Imaging API using the Microsoft Windows Imaging Component

struct IWICBitmap;
struct IWICBitmapLock;
struct IWICImagingFactory;

namespace Ingenuity {
namespace WIC {

struct Buffer : public Image::Buffer
{
	IWICBitmap * bitmap;
	IWICBitmapLock * lock;
	unsigned width;
	unsigned height;

	Buffer(IWICBitmap * bitmap, IWICBitmapLock * lock, unsigned width, unsigned height) :
		bitmap(bitmap), lock(lock), width(width), height(height) {}
	virtual ~Buffer();
};

class Api : public Image::Api
{
public:
	Api();
	virtual ~Api();

	//virtual ImageBuffer * LoadImage(const wchar_t * path) override;
	virtual Image::Buffer * CreateImage(unsigned w, unsigned h) override;
	virtual Image::Buffer * CreateImage(char * buffer, unsigned bufferLength) override;
	//virtual void SaveImage(ImageBuffer * image, const wchar_t * path, ImageApi::ImageFileType fileType) override;

	virtual void GetImageSize(Image::Buffer * image, unsigned & w, unsigned & h) override;
	virtual Image::Color GetPixelColor(Image::Buffer * image, unsigned u, unsigned v) override;
	virtual void SetPixelColor(Image::Buffer * image, unsigned u, unsigned v, Image::Color color) override;

	IWICImagingFactory * factory;
};

} // namespace WIC
} // namespace Ingenuity

#endif // USE_WIC_IMAGEAPI