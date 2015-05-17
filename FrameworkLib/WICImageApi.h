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

	virtual void * GetData() override;
	virtual unsigned GetDataSize() override;
	virtual void GetImageSize(unsigned & w, unsigned & h) override;
	virtual Image::Color GetPixelColor(unsigned u, unsigned v) override;
	virtual void SetPixelColor(unsigned u, unsigned v, Image::Color color) override;
};

class Api : public Image::Api
{
public:
	Api();
	virtual ~Api();

	//virtual ImageBuffer * LoadImage(const wchar_t * path) override;
	virtual Image::Buffer * CreateImage(char * data, unsigned dataSize, unsigned w, unsigned h, Image::Format format = Image::Format_4x8intRGBA) override;
	virtual Image::Buffer * CreateImage(char * buffer, unsigned bufferLength) override;
	//virtual Image::Buffer * Convert(Image::Buffer * image, Image::Format format) override;
	//virtual void SaveImage(ImageBuffer * image, const wchar_t * path, ImageApi::ImageFileType fileType) override;

	IWICImagingFactory * factory;
};

} // namespace WIC
} // namespace Ingenuity

#endif // USE_WIC_IMAGEAPI