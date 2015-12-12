#pragma once
#include "types.h"

#include <vector>
using namespace std;

class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();

	bool loadImage(const char* image);
	bool saveImageRGBA(const char* image, u8* data, u32 width, u32 height);
	void freeImageData();

	const u8* getImageData() const { return m_imageData; }
	const u32 getWidth()     const { return m_width;  }
	const u32 getHeight()    const { return m_height; }
	const u32 getOffsetX()   const { return m_offsX;  }
	const u32 getOffsetY()   const { return m_offsY;  }

	void setPath(const char* path);
	const char* getPath() const { return m_path; }

private:
	char m_path[260];
	u32  m_width;
	u32  m_height;
	u32  m_offsX;
	u32  m_offsY;
	u8*  m_imageData;
	u8*  m_imageData_Work;
};
