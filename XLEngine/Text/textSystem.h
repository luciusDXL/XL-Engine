#pragma once

#include "../types.h"
#include "../Graphics/graphicsDevice.h"

typedef u32 FontHandle;

#define INVALID_FONT_HANDLE 0xffffffff
#define PACK_RGBA_TEX(r, g, b, a) ( (b) | ((g)<<8) | ((r)<<16) | ((a)<<24) )
#define PACK_RGBA(r, g, b, a) ( (r) | ((g)<<8) | ((b)<<16) | ((a)<<24) )

namespace TextSystem
{
	enum FontDrawFlags
	{
		FDRAW_NORMAL  = 0,
		FDRAW_BOLD    = (1<<0),
		FDRAW_ITALICS = (1<<1),
	};

	bool init(GraphicsDevice* gdev);
	void destroy();

	//load and cache a font
	FontHandle loadFontASCII(const char* name, int height, int drawFlags=FDRAW_NORMAL);

	void setFont(FontHandle handle);
	void setColor(Color color);
	void print(int x, int y, const char* msg, ...);
	u32 print_genQuads(u32& quadCount, int x, int y, const char* msg, ...);
	int  getStringWidth(FontHandle font, const char* msg);

	void bindTexture();
};
