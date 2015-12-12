#pragma once

#include "../Graphics/graphicsDevice.h"
#include "../Text/textSystem.h"

struct DrawRectBuf
{
	TextureHandle texture;
	Color color;
	int layer;
	//pos
	int x, y;
	int w, h;
	//uvs
	float u, v;
	float du, dv;
};
struct DrawTextBuf
{
	Color color;
	FontHandle font;
	int layer;
	int x, y;
	char msg[4096];
};

#ifndef MIN
#define MIN(a, b) (a)<(b) ? (a) : (b)
#endif

#ifndef MAX
#define MAX(a, b) (a)>(b) ? (a) : (b)
#endif

namespace Draw2D
{
	bool init(GraphicsDevice* gdev);
	void destroy();

	DrawRectBuf* getDrawRect();
	DrawTextBuf* getDrawText();
	void draw();
};
