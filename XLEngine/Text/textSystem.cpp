#include "textSystem.h"
#include "../crc32.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>

using namespace std;

namespace TextSystem
{
	struct Glyph
	{
		float u0;
		float v0;
		float u1;
		float v1;
		int width;
		int height;
		int left;
		int top;
		int advanceX;
		int advanceY;
		u8* bitmap;
	};

	struct Font
	{
		char name[256];
		int  drawFlags;
		int  size;

		int cacheWidth;
		int cacheHeight;
		Color* imageCache;
		TextureHandle texture;

		int* charMap;
		Glyph* glyphs;
	};

	static FT_Library s_library = NULL;
	static vector<Font> s_fonts;
	static GraphicsDevice* s_gdev;
	static Font* s_curFont = NULL;
	static Color s_currentColor = 0xffffffff;
	static u32   s_textureHash = 0;

	u32 nextPow2(u32 x);

	bool init(GraphicsDevice* gdev)
	{
		s_gdev = gdev;
		if (FT_Init_FreeType(&s_library)) 
		{
			fprintf(stderr, "Could not init freetype library\n");
			return false;
		}
		
		const char* texName = "baseTex";
		s_textureHash = CRC32::get( (u8*)texName, strlen(texName) );
		return true;
	}

	void destroy()
	{
		size_t fontCount = s_fonts.size();
		for (size_t f=0; f<fontCount; f++)
		{
			delete [] s_fonts[f].imageCache;
			delete [] s_fonts[f].glyphs;
			delete [] s_fonts[f].charMap;
		}
		s_fonts.clear();

		FT_Done_FreeType(s_library);
		s_library = NULL;
	}

	//////////////////////////////////
	// Font/Text drawing functions.
	//////////////////////////////////
	FontHandle loadFontASCII(const char* name, int height, int drawFlags/*=FDRAW_NORMAL*/)
	{
		//for now just fit the font into a 16 x 16 grid...
		//so get the largest glyph first...
		char fontPath[1024];
		sprintf(fontPath, "UI/fonts/%s", name);

		FT_Face face;
		if( FT_New_Face(s_library, fontPath, 0, &face) ) 
		{
			fprintf(stderr, "Could not open font\n");
			return INVALID_FONT_HANDLE;
		}

		FT_Set_Pixel_Sizes(face, 0, height);

		//load the characters...
		int numChars = 0;
		int maxWidth = 0, maxHeight = 0;
		Glyph* glyphs = new Glyph[128];
		char map[128];
		for (int c=0; c<128; c++) { map[c] = -1; }
		for (int c=0; c<128; c++)
		{
			if (FT_Load_Char(face, (char)c, FT_LOAD_RENDER)) 
			{
				continue;
			}

			FT_GlyphSlot src = face->glyph;
			Glyph& dst = glyphs[numChars];
			dst.u0 = 0;
			dst.v0 = 0;
			dst.u1 = 0;
			dst.v1 = 0;
			dst.width  = src->bitmap.width;
			dst.height = src->bitmap.rows;
			dst.left   = src->bitmap_left;
			dst.top    = src->bitmap_top;
			dst.advanceX = src->advance.x;
			dst.advanceY = src->advance.y;
			dst.bitmap = new unsigned char[dst.width * dst.height];
			memcpy(dst.bitmap, src->bitmap.buffer, dst.width * dst.height);

			if (dst.width > maxWidth) { maxWidth = dst.width; }
			if (dst.height > maxHeight) { maxHeight = dst.height; }

			map[c] = numChars;
			numChars++;
		}

		//now determine a texture size... for now just do the naive thing.
		int texwidth = 512, texheight = 0;
		int numGlyphHorz = texwidth / maxWidth;
		int numGlyphVert = (numChars+numGlyphHorz-1) / numGlyphHorz;
		texheight = nextPow2( numGlyphVert * maxHeight );

		//finally pack the texture...
		Font font;
		font.cacheWidth  = texwidth;
		font.cacheHeight = texheight;
		font.drawFlags   = drawFlags;
		font.glyphs      = glyphs;
		strcpy(font.name, name);
		font.size        = height;
		font.imageCache  = new Color[ texwidth * texheight ];
		memset(font.imageCache, 0, sizeof(Color)*texwidth * texheight);

		for (int c=0; c<numChars; c++)
		{
			int cx = c % numGlyphHorz;
			int cy = c / numGlyphHorz;
			int tx = cx * maxWidth;
			int ty = cy * maxHeight;

			font.glyphs[c].u0 = float(tx) / float(texwidth);
			font.glyphs[c].v0 = float(ty) / float(texheight);
			font.glyphs[c].u1 = float(tx+font.glyphs[c].width)  / float(texwidth);
			font.glyphs[c].v1 = float(ty+font.glyphs[c].height) / float(texheight);

			const u8* bitmap = font.glyphs[c].bitmap;
			for (int y=0; y<font.glyphs[c].height; y++)
			{
				Color* cache = &font.imageCache[(y+ty)*texwidth + tx];
				for (int x=0; x<font.glyphs[c].width; x++, cache++)
				{
					int alpha = bitmap[ y*font.glyphs[c].width+x ];
					Color color = PACK_RGBA_TEX(0xff, 0xff, 0xff, alpha);
					*cache = color;
				}
			}

			delete[] font.glyphs[c].bitmap;
			font.glyphs[c].bitmap = NULL;
		}

		font.charMap = new int[256];
		for (int c=0; c<256; c++)
		{
			int m = -1;
			if (c < 128)
			{
				m = map[c];
			}
			font.charMap[c] = m;
		}
		font.texture = s_gdev->createTextureRGBA(texwidth, texheight, (u32*)font.imageCache);
		FontHandle handle = FontHandle( s_fonts.size() );

		s_fonts.push_back(font);

		return handle;
	}

	void setFont(FontHandle handle)
	{
		if (handle == INVALID_FONT_HANDLE) { return; }

		s_curFont = &s_fonts[handle];
	}

	void setColor(Color color)
	{
		s_currentColor = color;
	}

	int getStringWidth(FontHandle font, const char* msg)
	{
		if (font == INVALID_FONT_HANDLE)
		{
			return 0;
		}

		int x = 0;
		Font* curFont = &s_fonts[font];

		const char* text = msg;
		for (; *text != 0; text++)
		{
			const char c = *text;
			if (c < 0) { continue; }

			const int m = curFont->charMap[c];
			if (m < 0) { continue; }

			const Glyph& glyph = curFont->glyphs[m];
			x += (glyph.advanceX >> 6);
		}

		return x;
	}

	void print(int x, int y, const char* msg, ...)
	{
		if (s_curFont == NULL) { return; }

		//first build up the string.
		static char outMsg[4096];
		va_list arg;
		va_start(arg, msg);
		int res = vsprintf(outMsg, msg, arg);
		va_end(arg);

		//set the texture.
		s_gdev->setShaderResource( s_curFont->texture, s_textureHash );

		//now draw each quad...
		const char* text = outMsg;
		for (; *text != 0; text++)
		{
			const char c = *text;
			if (c < 0) { continue; }

			const int m = s_curFont->charMap[c];
			if (m < 0) { continue; }

			const Glyph& glyph = s_curFont->glyphs[m];

			//setup the quad to draw.
			Quad quad;
			quad.p0.x = x + glyph.left;
			quad.p0.y = y - glyph.top + s_curFont->size;
			quad.p1.x = quad.p0.x + glyph.width;
			quad.p1.y = quad.p0.y + glyph.height;
			quad.uv0.x = glyph.u0;
			quad.uv0.y = glyph.v0;
			quad.uv1.x = glyph.u1;
			quad.uv1.y = glyph.v1;
			quad.color = s_currentColor;

			s_gdev->drawQuad( quad );

			x += (glyph.advanceX >> 6);
			y += (glyph.advanceY >> 6);
		}
	}

	u32 nextPow2(u32 x)
	{
		x = x-1;
		x = x | (x>>1);
		x = x | (x>>2);
		x = x | (x>>4);
		x = x | (x>>8);
		x = x | (x>>16);
		return x + 1;
	}
};