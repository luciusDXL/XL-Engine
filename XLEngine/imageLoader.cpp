#include "imageLoader.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#define IL_USE_PRAGMA_LIBS
#include <IL/il.h>

#define MAX_IMAGE_WIDTH 4096

ImageLoader::ImageLoader(void)
{
	// We're going to allocate a buffer for image loading.
	m_imageData      = (u8 *)malloc(MAX_IMAGE_WIDTH*MAX_IMAGE_WIDTH*4);
	m_imageData_Work = (u8 *)malloc(MAX_IMAGE_WIDTH*MAX_IMAGE_WIDTH*4);
	m_width  = 0;
	m_height = 0;
	m_path[0] = 0;

	// Initialize IL
	ilInit();

	// We want all images to be loaded in a consistent manner
	ilEnable(IL_ORIGIN_SET);
}

ImageLoader::~ImageLoader(void)
{
	if ( m_imageData )
	{
		free(m_imageData);
		m_imageData = NULL;
	}
	if ( m_imageData_Work )
	{
		free(m_imageData_Work);
		m_imageData_Work = NULL;
	}
}

void ImageLoader::freeImageData()
{
	m_width  = 0;
	m_height = 0;
}

void ImageLoader::setPath(const char* path)
{
	strcpy(m_path, path);
}

bool ImageLoader::saveImageRGBA(const char* image, u8* data, u32 width, u32 height)
{
	ILuint handle;

	// In the next section, we load one image
	ilGenImages(1, &handle);
	ilBindImage(handle);

	ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, data);
	ilEnable(IL_FILE_OVERWRITE);
	const ILboolean saved = ilSaveImage(image);

	// Finally, clean the mess!
	ilDeleteImages(1, &handle);

	return saved ? true : false;
}

bool ImageLoader::loadImage(const char* image)
{
	freeImageData();

	// Create the final path...
	char filePath[260];
	sprintf(filePath, "%s%s", m_path, image);

	// Now let's switch over to using devIL...
	ILuint handle;

	// In the next section, we load one image
	ilGenImages(1, &handle);
	ilBindImage(handle);
	const ILboolean loaded = ilLoadImage(filePath);

	if (loaded == IL_FALSE)
	{
		ILenum error = ilGetError();
		return false; // Error encountered during loading
	}

	// Let’s spy on it a little bit
	m_width  = (u32)ilGetInteger(IL_IMAGE_WIDTH);  // getting image width
	m_height = (u32)ilGetInteger(IL_IMAGE_HEIGHT); // and height

	// Make sure our buffer is big enough.
	assert( m_width <= MAX_IMAGE_WIDTH && m_height <= MAX_IMAGE_WIDTH );

	// Finally get the image data
	ilCopyPixels(0, 0, 0, m_width, m_height, 1, IL_RGBA, IL_UNSIGNED_BYTE, m_imageData_Work);

	// (Now flip the image for OpenGL... does not seem to be necessary so just copy for now)
	memcpy( m_imageData, m_imageData_Work, 4*m_width*m_height );

	// Finally, clean the mess!
	ilDeleteImages(1, &handle);

	return true;
}
