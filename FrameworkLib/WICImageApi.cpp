#include "WICImageApi.h"

#ifdef USE_WIC_IMAGEAPI

#include <wincodec.h>

namespace Ingenuity {

WIC::Buffer::~Buffer()
{
	if(lock) lock->Release();
	if(bitmap) bitmap->Release();
}

WIC::Api::Api() :
factory(0)
{
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
}

WIC::Api::~Api()
{
	if(factory) factory->Release();
}

//ImageBuffer * WICImageApi::LoadImage(const wchar_t * path)
//{
//	if(!factory) return 0;
//	
//	IWICBitmapDecoder * decoder = 0;
//	factory->CreateDecoderFromFilename(path, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
//	if(!decoder) return 0;
//
//	IWICBitmapFrameDecode * bitmapSource = 0;
//	decoder->GetFrame(0, &bitmapSource);
//	if(!bitmapSource)
//	{
//		decoder->Release();
//		return 0;
//	}
//
//	IWICBitmapSource * convertedSource = 0;
//	WICConvertBitmapSource(GUID_WICPixelFormat32bppRGBA, bitmapSource, &convertedSource);
//	bitmapSource->Release();
//	if(!convertedSource)
//	{
//		decoder->Release();
//		return 0;
//	}
//
//	// Create the bitmap from the image frame.
//	IWICBitmap * bitmap = 0;
//	HRESULT hr = factory->CreateBitmapFromSource(
//		  convertedSource,         // Create a bitmap from the image frame
//		  WICBitmapCacheOnDemand,  // Cache metadata when needed
//		  &bitmap);                // Pointer to the bitmap
//
//	convertedSource->Release();
//	decoder->Release();
//
//	if(!bitmap) return 0;
//
//	unsigned width = 0, height = 0;
//	bitmap->GetSize(&width,&height);
//	WICRect lockRect = { 0, 0, width, height };
//
//	IWICBitmapLock * lock = 0;
//	bitmap->Lock(&lockRect, WICBitmapLockWrite, &lock);
//	if(!lock)
//	{
//		bitmap->Release();
//		return 0;
//	}
//
//	return new WICImageBuffer(bitmap, lock, width, height);
//}

Image::Buffer * WIC::Api::CreateImage(unsigned w, unsigned h)
{
	IWICBitmap * bitmap = 0;
	factory->CreateBitmap(w, h, GUID_WICPixelFormat32bppRGBA, WICBitmapCacheOnLoad, &bitmap);
	if(!bitmap) return 0;

	WICRect lockRect = { 0, 0, w, h };

	IWICBitmapLock * lock = 0;
	bitmap->Lock(&lockRect, WICBitmapLockWrite, &lock);
	if(!lock)
	{
		bitmap->Release();
		return 0;
	}

	return new WIC::Buffer(bitmap, lock, w, h);
}

Image::Buffer * WIC::Api::CreateImage(char * buffer, unsigned bufferLength)
{
	if(!factory) return 0;

	// Create input stream for memory
	IWICStream * stream = 0;
	factory->CreateStream(&stream);
	if(!stream) return 0;

	stream->InitializeFromMemory((unsigned char*)(buffer), static_cast<DWORD>(bufferLength));

	IWICBitmapDecoder * decoder = 0;
	factory->CreateDecoderFromStream(stream, 0, WICDecodeMetadataCacheOnDemand, &decoder);
	if(!decoder) return 0;

	IWICBitmapFrameDecode * bitmapSource = 0;
	decoder->GetFrame(0, &bitmapSource);
	if(!bitmapSource)
	{
		decoder->Release();
		return 0;
	}

	IWICBitmapSource * convertedSource = 0;
	WICConvertBitmapSource(GUID_WICPixelFormat32bppRGBA, bitmapSource, &convertedSource);
	bitmapSource->Release();
	if(!convertedSource)
	{
		decoder->Release();
		return 0;
	}

	// Create the bitmap from the image frame.
	IWICBitmap * bitmap = 0;
	factory->CreateBitmapFromSource(
		convertedSource,         // Create a bitmap from the image frame
		WICBitmapCacheOnDemand,  // Cache metadata when needed
		&bitmap);                // Pointer to the bitmap

	convertedSource->Release();
	decoder->Release();

	if(!bitmap) return 0;

	unsigned width = 0, height = 0;
	bitmap->GetSize(&width, &height);
	WICRect lockRect = { 0, 0, width, height };

	IWICBitmapLock * lock = 0;
	bitmap->Lock(&lockRect, WICBitmapLockWrite, &lock);
	if(!lock)
	{
		bitmap->Release();
		return 0;
	}

	return new WIC::Buffer(bitmap, lock, width, height);
}

//void WICImageApi::SaveImage(ImageBuffer * image, const wchar_t * path, ImageApi::ImageFileType fileType)
//{
//	if(!image) return;
//	WICImageBuffer * wicImage = static_cast<WICImageBuffer*>(image);
//
//	wicImage->lock->Release();
//
//	GUID format = GUID_ContainerFormatBmp;
//
//	switch(fileType)
//	{
//	case ImageApi::FileTypeBMP:
//		format = GUID_ContainerFormatBmp;
//		break;
//	case ImageApi::FileTypeJPG:
//		format = GUID_ContainerFormatJpeg;
//		break;
//	case ImageApi::FileTypePNG:
//		format = GUID_ContainerFormatPng;
//		break;
//	case ImageApi::FileTypeTGA:
//		// TARGA not supported!
//		return;
//	}
//
//	IWICBitmapEncoder * encoder = 0;
//	factory->CreateEncoder(format, 0, &encoder);
//
//	IWICStream * stream = 0;
//	factory->CreateStream(&stream);
//
//	stream->InitializeFromFilename(path, GENERIC_WRITE);
//	encoder->Initialize(stream, WICBitmapEncoderNoCache);
//
//	GUID pixelFormat = GUID_WICPixelFormat32bppRGBA;
//
//	IWICBitmapFrameEncode * encodeFrame;
//	encoder->CreateNewFrame(&encodeFrame, 0);
//
//	encodeFrame->Initialize(0);
//	encodeFrame->SetSize(wicImage->width, wicImage->height);
//	encodeFrame->SetPixelFormat(&pixelFormat);
//
//	// http://msdn.microsoft.com/en-us/library/windows/desktop/ff973956.aspx ???
//	//IWICMetadataBlockReader * blockReader = 0;
//	//IWICMetadataBlockWriter * blockWriter = 0;
//
//	encodeFrame->WriteSource(wicImage->bitmap, 0);
//	encodeFrame->Commit();
//	encoder->Commit();
//
//	encodeFrame->Release();
//	encoder->Release();
//	stream->Release();
//	
//	WICRect lockRect = { 0, 0, wicImage->width, wicImage->height };
//	wicImage->bitmap->Lock(&lockRect, WICBitmapLockWrite, &wicImage->lock);
//}

void WIC::Api::GetImageSize(Image::Buffer * image, unsigned & u, unsigned & v)
{
	if(!image) return;
	WIC::Buffer * wicImage = static_cast<WIC::Buffer*>(image);
	u = wicImage->width;
	v = wicImage->height;
}

Image::Color WIC::Api::GetPixelColor(Image::Buffer * image, unsigned u, unsigned v)
{
	if(!image) return Image::Color();
	unsigned numBytes = 0;
	BYTE * bytes = 0;

	WIC::Buffer * wicImage = static_cast<WIC::Buffer*>(image);

	wicImage->lock->GetDataPointer(&numBytes, &bytes);
	if(!bytes) return Image::Color();

	unsigned index = ((v * wicImage->width) + u) * 4; // RGBA, Stride should be 4!

	Image::Color color;

	color.r = float(bytes[index]) / 255.f;
	color.g = float(bytes[index + 1]) / 255.f;
	color.b = float(bytes[index + 2]) / 255.f;
	color.a = float(bytes[index + 3]) / 255.f;

	return color;
}

void WIC::Api::SetPixelColor(Image::Buffer * image, unsigned u, unsigned v, Image::Color color)
{
	if(!image) return;
	WIC::Buffer * wicImage = static_cast<WIC::Buffer*>(image);

	unsigned numBytes = 0;
	BYTE * bytes = 0;

	wicImage->lock->GetDataPointer(&numBytes, &bytes);
	if(!bytes) return;

	unsigned index = ((v * wicImage->width) + u) * 4; // RGBA, Stride should be 4!

	bytes[index] = BYTE(color.r * 255.f);
	bytes[index + 1] = BYTE(color.g * 255.f);
	bytes[index + 2] = BYTE(color.b * 255.f);
	bytes[index + 3] = BYTE(color.a * 255.f);
}

} // namespace Ingenuity

#endif // USE_WIC_IMAGEAPI