#pragma once

#include "../Graphics/graphicsDevice.h"
#include "../types.h"
#include "../Text/textSystem.h"

enum IconID
{
	ICON_VIDEO = 0,
	ICON_DASHBOARD,
	ICON_CONTROLS,
	ICON_SETTINGS,
	ICON_SEARCH,
	ICON_TOOLS,
};

struct UI_Sound
{
	const char* filename;
	void* data;
	u32   size;
};

namespace UISystem
{
	bool init(GraphicsDevice* gdev, int screenWidth, int screenHeight);
	void destroy();

	//general
	void begin();
	void finish();
	void clear();
	FontHandle getFont(int size);
	void setCurrentLayer(int layer=0);
	void setMouseOverSound(UI_Sound* sound);

	//controls
	bool window(int id, int layer, const char* caption, int x, int y, int w, int h);
	bool button(s32 id, s32 layer, const char* caption, s32 x, s32 y, s32 w, s32 h);
	bool buttonImage(s32 id, s32 layer, TextureHandle image, s32 x, s32 y, s32 w, s32 h);
	bool buttonIcon(s32 id, s32 layer, s32 iconID, s32 x, s32 y);

	void staticText(int layer, const char* str, int x, int y, Color color=PACK_RGBA(190, 255, 255, 255));

	s32 addIcon(const char* imageName);

	//
	void predraw();
};
