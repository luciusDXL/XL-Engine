#include "../Text/textSystem.h"
#include "../imageLoader.h"
#include "../input.h"
#include "../Math/math.h"
#include "uiSystem.h"
#include "draw2D.h"
#include <vector>

using namespace std;

namespace UISystem
{
	GraphicsDevice* s_gdev;
	ImageLoader* s_imageLoader;
	FontHandle s_font18;
	FontHandle s_font24;
	FontHandle s_font32;

	struct Icon
	{
		TextureHandle texture;
		int width;
		int height;
	};

	vector<Icon*> s_icons;
	Icon* s_background;
	Icon* s_logo;

	TextureHandle s_uiTexture;

	const char* c_iconFiles[]=
	{
		"computer.png",
		"dashboard.png",
		"joystick.png",
		"settings.png",
		"search.png",
		"tools.png",
	};

	int s_windowWidth;
	int s_windowHeight;
	int s_currentLayer = 0;
	char s_iconPath[512] = "UI/icons/";

	//imGUI
	static bool s_enabled = false;

	static s32  s_hotItem;
	static s32  s_activeItem;
	static s32  s_kdbItem;
	static s32  s_lastWidget = 0;
	static u32  s_colorMask = 0xffffffff;
	static bool s_clearState;

	int createIcon(IconID id, int size);
	void setColorMask(u32 mask=0xffffffff);
	void handleFocus(s32 id, s32 layer, s32 x, s32 y, s32 w, s32 h);
	bool handleDefaultKeyboard(s32 id, bool& retValue);
	bool handleMouseLogic(s32 id);
	bool handleMouseLogicHold(s32 id);
	bool regionHit(s32 x, s32 y, s32 w, s32 h);
	void draw_rect_decoration(s32 id, s32 layer, s32 x, s32 y, s32 w, s32 h);
	void draw_rect_decoration_image(s32 id, s32 layer, TextureHandle image, s32 x, s32 y, s32 w, s32 h);

	bool init(GraphicsDevice* gdev, int screenWidth, int screenHeight)
	{
		s_gdev = gdev;
		s_windowWidth = screenWidth;
		s_windowHeight = screenHeight;

		s_font18 = TextSystem::loadFontASCII("Inconsolata.otf", 18);
		s_font24 = TextSystem::loadFontASCII("Inconsolata.otf", 24);
		s_font32 = TextSystem::loadFontASCII("Inconsolata.otf", 32);

		s_imageLoader = new ImageLoader();
		createIcon(ICON_VIDEO, 32);
		createIcon(ICON_DASHBOARD, 32);
		createIcon(ICON_CONTROLS, 32);
		createIcon(ICON_SETTINGS, 32);
		createIcon(ICON_SEARCH, 32);
		createIcon(ICON_TOOLS, 32);

		const SamplerState samplerState=
		{
			WM_CLAMP, WM_CLAMP, WM_CLAMP,							//clamp on every axis
			TEXFILTER_LINEAR, TEXFILTER_POINT, TEXFILTER_POINT,		//filtering
			false													//no mipmapping
		};

		if (s_gdev->getMaximumTextureSize() < 2048 || !s_gdev->supportsFeature(CAP_NON_POWER_2_TEX))
		{
			s_imageLoader->loadImage("UI/background512.png");
		}
		else
		{
			s_imageLoader->loadImage("UI/background.png");
		}

		s_background = new Icon;
		s_background->texture = s_gdev->createTextureRGBA(s_imageLoader->getWidth(), s_imageLoader->getHeight(), (u32*)s_imageLoader->getImageData(), samplerState);
		s_background->width  = s_imageLoader->getWidth();
		s_background->height = s_imageLoader->getHeight();

		if (s_gdev->getMaximumTextureSize() < 1024 || !s_gdev->supportsFeature(CAP_NON_POWER_2_TEX))
		{
			s_imageLoader->loadImage("UI/XL_Engine_2_Small.png");
		}
		else
		{
			s_imageLoader->loadImage("UI/XL_Engine_2.png");
		}
		s_logo = new Icon;
		s_logo->texture = s_gdev->createTextureRGBA(s_imageLoader->getWidth(), s_imageLoader->getHeight(), (u32*)s_imageLoader->getImageData(), samplerState);
		s_logo->width   = s_imageLoader->getWidth();
		s_logo->height  = s_imageLoader->getHeight();

		s_imageLoader->loadImage("UI/UI.png");
		s_uiTexture = s_gdev->createTextureRGBA(s_imageLoader->getWidth(), s_imageLoader->getHeight(), (u32*)s_imageLoader->getImageData(), samplerState);

		s_clearState = false;
		clear();

		return true;
	}

	void setCurrentLayer(int layer)
	{
		s_currentLayer = layer;
	}

	FontHandle getFont(int size)
	{
		if (size == 18)
			return s_font18;
		else if (size == 24)
			return s_font24;
		else if (size == 32)
			return s_font32;
		return INVALID_FONT_HANDLE;
	}

	void destroy()
	{
		size_t iconCount = s_icons.size();
		for (size_t i=0; i<iconCount; i++)
		{
			delete s_icons[i];
		}
		s_icons.clear();

		delete s_background;
		s_background = NULL;

		delete s_logo;
		s_logo = NULL;
		
		delete s_imageLoader;
		s_imageLoader = NULL;
	}

	s32 addIcon(const char* imageName)
	{
		s32 id = (s32)s_icons.size();

		char fullpath[512];
		sprintf(fullpath, "%s%s", s_iconPath, imageName);
		s_imageLoader->loadImage(fullpath);

		const SamplerState samplerState=
		{
			WM_CLAMP, WM_CLAMP, WM_CLAMP,							//clamp on every axis
			TEXFILTER_LINEAR, TEXFILTER_POINT, TEXFILTER_POINT,		//filtering
			false													//no mipmapping
		};

		Icon* icon = new Icon;
		icon->texture = s_gdev->createTextureRGBA(s_imageLoader->getWidth(), s_imageLoader->getHeight(), (u32*)s_imageLoader->getImageData(), samplerState);
		icon->width   = s_imageLoader->getWidth();
		icon->height  = s_imageLoader->getHeight();
		
		s_icons.push_back(icon);

		return id;
	}

	void predraw()
	{
		DrawRectBuf* rect = Draw2D::getDrawRect();
		rect->x = 0;
		rect->y = 0;
		rect->w = s_windowWidth;
		rect->h = s_windowHeight;
		rect->color = PACK_RGBA(0xa0, 0xe0, 0xff, 0x80);
		rect->texture = s_background->texture;

		rect = Draw2D::getDrawRect();
		rect->x = MAX( 0, (s_windowWidth  - s_logo->width)  >> 1 );
		rect->y = MAX( 0, (s_windowHeight - s_logo->height) >> 1 );
		rect->w = MIN( s_logo->width, s_windowWidth );
		rect->h = MIN( s_logo->height, s_windowHeight );
		rect->color = PACK_RGBA(0x80, 0xff, 0xff, 0x20);
		rect->texture = s_logo->texture;
	}

	//State preparation before the drawing
	void begin()
	{
		if (s_clearState)
		{
			s_clearState = false;
			s_kdbItem    = 0;
		}
		s_hotItem = 0;
	}

	//State cleanup after the drawing
	void finish()
	{
		if (Input::getMouseButtonState(Input::MouseLeft) == false)
		{
			s_activeItem = false;
		}
	}

	//Clears the state when a new gui layout should be drawn
	//After calling this function no further widgets are drawn and at the
	//beginning both uistate.hotitem and uistate.kbditem are reset.
	void clear()
	{
		s_clearState = true;

		s_hotItem = 0;
		s_kdbItem = 0;
	}

	int createIcon(IconID id, int size)
	{
		char fullpath[512];
		sprintf(fullpath, "%s%d/%s", s_iconPath, size, c_iconFiles[id]);
		s_imageLoader->loadImage(fullpath);

		const SamplerState samplerState=
		{
			WM_CLAMP, WM_CLAMP, WM_CLAMP,							//clamp on every axis
			TEXFILTER_LINEAR, TEXFILTER_POINT, TEXFILTER_POINT,		//filtering
			false													//no mipmapping
		};

		Icon* icon = new Icon;
		icon->texture = s_gdev->createTextureRGBA(s_imageLoader->getWidth(), s_imageLoader->getHeight(), (u32*)s_imageLoader->getImageData(), samplerState);
		icon->width   = s_imageLoader->getWidth();
		icon->height  = s_imageLoader->getHeight();

		int outID = (int)s_icons.size();
		s_icons.push_back(icon);

		return outID;
	}

	/////////////////////////////////////
	//controls
	/////////////////////////////////////
	void drawWindow(int x, int y, int w, int h, int layer, bool caption, bool exitHot)
	{
		//base
		DrawRectBuf base;
		base.texture = s_uiTexture;
		base.layer = layer;
		base.x = x;
		base.y = y;
		base.w = 16;
		base.h = 16;
		base.u = 0.0f;
		base.v = 0.0f;
		base.du = 16.0f/256.0f;
		base.dv = 16.0f/256.0f;
		base.color = PACK_RGBA(0xff, 0xff, 0xff, 0xc0);

		//draw the background.
		DrawRectBuf* rect = Draw2D::getDrawRect();
		*rect = base;
		rect->u = 16.0f / 256.0f;
		rect->v = 32.0f / 256.0f;
		rect->w = w;
		rect->h = h;

		//draw the corners
		//upper left
		rect = Draw2D::getDrawRect();
		*rect = base;
		rect->x = x-15;
		rect->y = y-15;
		rect->u = 32.0f/256.0f;
		//upper right
		rect = Draw2D::getDrawRect();
		*rect = base;
		rect->x = x+w-1;
		rect->y = y-15;
		rect->u = 48.0f/256.0f;
		//lower left
		rect = Draw2D::getDrawRect();
		*rect = base;
		rect->x = x-15;
		rect->y = y+h-1;
		rect->u = 32.0f/256.0f;
		rect->v = 16.0f/256.0f;
		//lower right
		rect = Draw2D::getDrawRect();
		*rect = base;
		rect->x = x+w-1;
		rect->y = y+h-1;
		rect->u = 48.0f/256.0f;
		rect->v = 16.0f/256.0f;

		//draw the edges
		//top
		rect = Draw2D::getDrawRect();
		*rect = base;
		rect->x = x;
		rect->y = y-16;
		rect->w = w;
		rect->h = 16;
		rect->u = 16.0f/256.0f;
		rect->v =  0.0f/256.0f;
		//bottom
		rect = Draw2D::getDrawRect();
		*rect = base;
		rect->x = x;
		rect->y = y+h;
		rect->w = w;
		rect->u =  0.0f/256.0f;
		rect->v = 32.0f/256.0f;
		//left
		rect = Draw2D::getDrawRect();
		*rect = base;
		rect->x = x-16;
		rect->y = y;
		rect->h = h;
		rect->u =  0.0f/256.0f;
		rect->v = 16.0f/256.0f;
		//right
		rect = Draw2D::getDrawRect();
		*rect = base;
		rect->x = x+w;
		rect->y = y;
		rect->h = h;
		rect->u = 16.0f/256.0f;
		rect->v = 16.0f/256.0f;

		//draw the seperator.
		if (caption)
		{
			//left
			rect = Draw2D::getDrawRect();
			*rect = base;
			rect->x = x;
			rect->y = y+20;
			rect->w = 16;
			rect->h = 16;
			rect->u = 32.0f/256.0f;
			rect->v = 32.0f/256.0f;

			//right
			rect = Draw2D::getDrawRect();
			*rect = base;
			rect->x = x+w-16;
			rect->y = y+20;
			rect->w = 16;
			rect->h = 16;
			rect->u = 48.0f/256.0f;
			rect->v = 32.0f/256.0f;

			//middle
			rect = Draw2D::getDrawRect();
			*rect = base;
			rect->x = x+16;
			rect->y = y+20;
			rect->w = w-32;
			rect->h = 16;
			rect->u = 64.0f/256.0f;
			rect->v = 32.0f/256.0f;

			//icon
			rect = Draw2D::getDrawRect();
			*rect = base;
			if (exitHot)
			{
				rect->color = PACK_RGBA(0, 255, 255, 255);
			}
			rect->x = x+w-20;
			rect->y = y+3;
			rect->w = 16;
			rect->h = 16;
			rect->u = 64.0f/256.0f;
			rect->v =  0.0f/256.0f;
		}
	}

	void staticText(int layer, const char* str, int x, int y, Color color)
	{
		DrawTextBuf* text = Draw2D::getDrawText();
		text->color = color;
		text->x = x;
		text->y = y;
		text->font = s_font18;
		text->layer = layer;
		strcpy(text->msg, str);
	}

	bool window(int id, int layer, const char* caption, int x, int y, int w, int h)
	{
		//abort if clear state is set.
		if (s_clearState) { return false; }

		//16 pixel alignment for graphics
		w = (w>>4)<<4;
		h = (h>>4)<<4;

		//handle mouse and keyboard focus [exit button]
		handleFocus(id, layer, x+w-20, y, 20, 20);

		drawWindow(x, y, w, h, layer, caption!=NULL, s_hotItem==id);
		if (caption)
		{
			staticText(layer, caption, x+8, y-2);
		}

		//mouse logic
		if (handleMouseLogic(id))
		{
			return true;
		}

		//keyboard logic
		if (s_kdbItem == id)
		{
			bool retValue;
			if (handleDefaultKeyboard(id, retValue))
			{
				return retValue;
			}
		}

		return false;
	}

	//Draws a button with a given caption (returns 1 if it was pressed)
	bool button(s32 id, s32 layer, const char* caption, s32 x, s32 y, s32 w, s32 h)
	{
		//abort if clear state is set.
		if (s_clearState) { return false; }

		//handle mouse and keyboard focus
		handleFocus(id, layer, x, y, w, h);

		//draw the control rect with appropriate settings.
		draw_rect_decoration(id, layer, x, y, w, h);
		
		//caption
		if (caption)
		{
			int strWidth  = TextSystem::getStringWidth(s_font18, caption);
			int strHeight = 18;

			int offsetX = MAX( w-strWidth,  0 );
			int offsetY = MAX( h-strHeight, 0 );

			staticText(layer, caption, x + (offsetX>>1), y - (offsetY>>1) - 3, 0xffffffff);
		}

		//mouse logic
		if (handleMouseLogic(id))
		{
			return true;
		}

		//keyboard logic
		if (s_kdbItem == id)
		{
			bool retValue;
			if (handleDefaultKeyboard(id, retValue))
			{
				return retValue;
			}
		}

		return false;
	}

	//Draws a button with a given caption (returns 1 if it was pressed)
	bool buttonImage(s32 id, s32 layer, TextureHandle image, s32 x, s32 y, s32 w, s32 h)
	{
		//abort if clear state is set.
		if (s_clearState) { return false; }

		//handle mouse and keyboard focus
		handleFocus(id, layer, x, y, w, h);

		//draw the control rect with appropriate settings.
		draw_rect_decoration_image(id, layer, image, x, y, w, h);

		//mouse logic
		if (handleMouseLogic(id))
		{
			return true;
		}

		//keyboard logic
		if (s_kdbItem == id)
		{
			bool retValue;
			if (handleDefaultKeyboard(id, retValue))
			{
				return retValue;
			}
		}

		return false;
	}

	//Draws a button with a given caption (returns 1 if it was pressed)
	bool buttonIcon(s32 id, s32 layer, s32 iconID, s32 x, s32 y)
	{
		//abort if clear state is set.
		if (s_clearState) { return false; }

		Icon* icon = s_icons[iconID];
		int w = icon->width;
		int h = icon->height;

		//handle mouse and keyboard focus
		handleFocus(id, layer, x, y, w, h);

		//draw the control rect with appropriate settings.
		TextureHandle image = icon->texture;
		draw_rect_decoration_image(id, layer, image, x, y, w, h);

		//mouse logic
		if (handleMouseLogic(id))
		{
			return true;
		}

		//keyboard logic
		if (s_kdbItem == id)
		{
			bool retValue;
			if (handleDefaultKeyboard(id, retValue))
			{
				return retValue;
			}
		}

		return false;
	}

	////////////////////////////////

	//Draws a rectangular decoration for the widget with id 
	//Performs the decoration dependent on hotness and activeness. 
	void draw_rect_decoration(s32 id, s32 layer, s32 x, s32 y, s32 w, s32 h)
	{
		if (s_kdbItem == id)
		{
			DrawRectBuf* rect = Draw2D::getDrawRect();
			rect->color = PACK_RGBA(100, 36, 36, 150) & s_colorMask;
			rect->layer = layer;
			rect->x = x-1;
			rect->y = y-1;
			rect->w = w+2;
			rect->h = h+2;
		}
		else
		{
			DrawRectBuf* rect = Draw2D::getDrawRect();
			rect->color = PACK_RGBA(36, 36, 36, 255) & s_colorMask;
			rect->layer = layer;
			rect->x = x-1;
			rect->y = y-1;
			rect->w = w+2;
			rect->h = h+2;
		}

		Color color;
		if (s_hotItem == id)
		{
			if (s_activeItem == id)
			{
				color = PACK_RGBA(200, 200, 200, 255) & s_colorMask;
			}
			else
			{
				color = PACK_RGBA(150, 150, 150, 255) & s_colorMask;
			}
		}
		else
		{
			color = PACK_RGBA(128, 128, 128, 255) & s_colorMask;
		}

		DrawRectBuf* rect = Draw2D::getDrawRect();
		rect->color = color;
		rect->layer = layer;
		rect->x = x;
		rect->y = y;
		rect->w = w;
		rect->h = h;
	}

	void draw_rect_decoration_image(s32 id, s32 layer, TextureHandle image, s32 x, s32 y, s32 w, s32 h)
	{
		f32 du = 1.0f, dv = 1.0f;
		f32  u = 0.0f,  v = 0.0f;
		if ( !s_gdev->supportsFeature(CAP_NON_POWER_2_TEX) )
		{
			du = f32(w) / f32(Math::nextPow2(w));
			dv = f32(h) / f32(Math::nextPow2(h));

			v = 1.0f - dv;
		}

		if (s_kdbItem == id || s_hotItem == id)
		{
			DrawRectBuf* rect = Draw2D::getDrawRect();
			rect->texture = image;
			rect->color = PACK_RGBA(32, 255, 255, 150) & s_colorMask;
			rect->layer = layer;
			rect->x = x-1;
			rect->y = y-1;
			rect->w = w+2;
			rect->h = h+2;

			rect->u = u;
			rect->v = v;
			rect->du = du;
			rect->dv = dv;
		}

		Color color;
		if (s_hotItem == id)
		{
			if (s_activeItem == id)
			{
				color = PACK_RGBA(0xff, 0xff, 0xff, 0xff) & s_colorMask;
			}
			else
			{
				color = PACK_RGBA(0xc0, 0xff, 0xff, 0xff) & s_colorMask;
			}
		}
		else
		{
			color = PACK_RGBA(0xa0, 0xff, 0xff, 0x80) & s_colorMask;
		}

		DrawRectBuf* rect = Draw2D::getDrawRect();
		rect->texture = image;
		rect->color = color;
		rect->layer = layer;
		rect->x = x;
		rect->y = y;
		rect->w = w;
		rect->h = h;

		rect->u = u;
		rect->v = v;
		rect->du = du;
		rect->dv = dv;
	}

	bool regionHit(s32 x, s32 y, s32 w, s32 h)
	{
		s32 mouseX, mouseY;
		Input::getMousePos(mouseX, mouseY);
		if (mouseX < x || mouseY < y || mouseX >= x + w || mouseY >= y + h)
		{
			return false;
		}

		return true;
	}

	void setColorMask(u32 mask/*=0xffffffff*/)
	{
		s_colorMask = mask;
	}

	void handleFocus(s32 id, s32 layer, s32 x, s32 y, s32 w, s32 h)
	{
		if (layer == s_currentLayer && regionHit(x, y, w, h))
		{
			s_hotItem = id;
			if (s_kdbItem != id)
			{
				s_lastWidget = id;
			}
			s_kdbItem = id;

			if (s_activeItem == 0 && Input::getMouseButtonState(Input::MouseLeft))
			{
				s_activeItem = id;
			}
		}

		if (layer == s_currentLayer && s_kdbItem == 0)
		{
			s_kdbItem = id;
			s_lastWidget = id;
		}
	}

	bool handleDefaultKeyboard(s32 id, bool& retValue)
	{
		bool inputConsumed = false;

		if (Input::keyPressedWithRepeat(Input::KEY_RETURN))	//return
		{
			retValue = true;
			inputConsumed = true;
		}
		else if (Input::keyPressedWithRepeat(Input::KEY_TAB))	//tab
		{
			s_kdbItem = 0;

			retValue = false;
			inputConsumed = true;
		}

		return inputConsumed;
	}

	bool handleMouseLogic(s32 id)
	{
		if (Input::getMouseButtonState(Input::MouseLeft) == false && s_hotItem == id && s_activeItem == id)
		{
			return true;
		}

		return false;
	}

	bool handleMouseLogicHold(s32 id)
	{
		if (Input::getMouseButtonState(Input::MouseLeft) == true && s_hotItem == id && s_activeItem == id)
		{
			return true;
		}

		return false;
	}
};
