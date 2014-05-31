#include "FreeImageApi.h"

#ifdef USE_FREE_IMAGEAPI

#define FREEIMAGE_LIB
#include <FreeImage.h>
#undef FREEIMAGE_LIB

#include <glm/glm.hpp>
#include <string>

namespace Ingenuity {

FreeImage::Api::Api()
{
	FreeImage_Initialise();
}

FreeImage::Api::~Api()
{
	FreeImage_DeInitialise();
}

FreeImage::Buffer::~Buffer()
{
	FreeImage_Unload(bitmap);
}

Image::Buffer * FreeImage::Api::CreateImage(unsigned w, unsigned h)
{
	FIBITMAP * bitmap = FreeImage_Allocate(w, h, 32);

	return new FreeImage::Buffer(bitmap);
}

Image::Buffer * FreeImage::Api::CreateImage(char * buffer, unsigned bufferLength)
{
	FIMEMORY * fiMemory = FreeImage_OpenMemory((BYTE*) buffer, bufferLength);

	FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(fiMemory);

	FIBITMAP * bitmap = FreeImage_LoadFromMemory(format, fiMemory, 0);

	FreeImage_CloseMemory(fiMemory);

	if (!bitmap) return 0;
	return new FreeImage::Buffer(bitmap);
}

//void FreeImageApi::SaveImage(ImageBuffer * image, const wchar_t * path, ImageApi::ImageFileType fileType)
//{
//	if(!image) return;
//	FreeImageBuffer * freeImage = static_cast<FreeImageBuffer*>(image);
//
//	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
//	switch(fileType)
//	{
//	case ImageApi::FileTypeBMP:
//		format = FIF_BMP; 
//		break;
//	case ImageApi::FileTypeJPG:
//		format = FIF_JPEG;
//		break;
//	case ImageApi::FileTypePNG:
//		format = FIF_PNG;
//		break;
//	case ImageApi::FileTypeTGA:
//		format = FIF_TARGA;
//		break;
//	}
//
//	if(format != FIF_UNKNOWN)
//	{
//		std::wstring wpath(path);
//		std::string  spath(wpath.begin(), wpath.end());
//
//		FreeImage_Save(format, freeImage->bitmap, spath.c_str());
//	}
//}

void FreeImage::Api::GetImageSize(Image::Buffer * image, unsigned & w, unsigned & h)
{
	if(!image) return;
	FreeImage::Buffer * freeImage = static_cast<FreeImage::Buffer*>(image);

	w = FreeImage_GetWidth(freeImage->bitmap);
	h = FreeImage_GetHeight(freeImage->bitmap);
}

Image::Color FreeImage::Api::GetPixelColor(Image::Buffer * image, unsigned u, unsigned v)
{
	if(!image) return Image::Color();

	FreeImage::Buffer * freeImage = static_cast<FreeImage::Buffer*>(image);

	RGBQUAD colorBytes;

	FreeImage_GetPixelColor(freeImage->bitmap,u,v,&colorBytes);

	Image::Color colorFloats;
	colorFloats.r = float(colorBytes.rgbRed) / 256.0f; // WRONG!!! SHOULD BE 255 TO BE IN THE RANGE 0.0f-1.0f inclusive!!!
	colorFloats.g = float(colorBytes.rgbGreen) / 256.0f;
	colorFloats.b = float(colorBytes.rgbBlue) / 256.0f;
	colorFloats.a = float(colorBytes.rgbReserved) / 256.0f;
	return colorFloats;
}

void FreeImage::Api::SetPixelColor(Image::Buffer * image, unsigned u, unsigned v, Image::Color colorFloats)
{
	if(!image) return;
	FreeImage::Buffer * freeImage = static_cast<FreeImage::Buffer*>(image);

	RGBQUAD colorBytes;

	colorBytes.rgbRed = char(colorFloats.r * 256.0f);
	colorBytes.rgbGreen = char(colorFloats.g * 256.0f);
	colorBytes.rgbBlue = char(colorFloats.b * 256.0f);
	colorBytes.rgbReserved = char(colorFloats.a * 256.0f);

	FreeImage_SetPixelColor(freeImage->bitmap, u, v, &colorBytes);
}

} // namespace Ingenuity

#endif // USE_FREE_IMAGEAPI