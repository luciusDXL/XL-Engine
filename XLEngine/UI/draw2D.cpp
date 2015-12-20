#include "draw2D.h"
#include "../Math/crc32.h"
#include <stdlib.h>
#include <string.h>

namespace Draw2D
{
	#define DRAW_POOL_SIZE 1024

	static GraphicsDevice* s_gdev;
	static DrawRectBuf s_rectPool[DRAW_POOL_SIZE];
	static DrawTextBuf s_textPool[DRAW_POOL_SIZE];
	static int s_rectIndex = 0;
	static int s_textIndex = 0;
	static u32 s_textureHash;

	void clearDraw();

	bool init(GraphicsDevice* gdev)
	{
		s_gdev = gdev;
		const char* texName = "baseTex";
		s_textureHash = CRC32::get( (u8*)texName, strlen(texName) );
		return true;
	}

	void destroy()
	{
	}

	DrawRectBuf* getDrawRect()
	{
		DrawRectBuf* rect = NULL;
		if (s_rectIndex < DRAW_POOL_SIZE)
		{
			rect = &s_rectPool[s_rectIndex];
			rect->texture = INVALID_TEXTURE_HANDLE;
			rect->color = 0xffffffff;
			rect->layer = 0;
			rect->u = 0.0f;
			rect->v = 0.0f;
			rect->du = 1.0f;
			rect->dv = 1.0f;
			s_rectIndex++;
		}

		return rect;
	}

	DrawTextBuf* getDrawText()
	{
		DrawTextBuf* text = NULL;
		if (s_textIndex < DRAW_POOL_SIZE)
		{
			text = &s_textPool[s_textIndex];
			text->color = 0xffffffff;
			text->layer = 0;
			s_textIndex++;
		}

		return text;
	}

	void drawImmediate(const DrawRectBuf& rect)
	{
		const Quad quad=
		{
			{ rect.x,          rect.y                },
			{ rect.x + rect.w, rect.y + rect.h       },
			{ rect.u,          1.0f-rect.v           },
			{ rect.u+rect.du,  1.0f-(rect.v+rect.dv) },
			rect.color
		};
		s_gdev->setShaderResource( rect.texture, s_textureHash );
		s_gdev->drawQuad( quad );
	}

	void draw()
	{
		s_gdev->enableBlending(true);
		s_gdev->setBlendMode( BLEND_OVER );
		s_gdev->setShader( SHADER_QUAD_UI );

		int curLayer = 255;
		if (s_rectIndex > 0)
		{
			curLayer = MIN(curLayer, s_rectPool[0].layer);
		}
		if (s_textIndex > 0)
		{
			curLayer = MIN(curLayer, s_textPool[0].layer);
		}

		int r=0, t=0;
		while (r < s_rectIndex || t < s_textIndex)
		{
			int nextLayerRect = 255;
			int nextLayerText = 255;
			for (; r < s_rectIndex; r++)
			{
				const DrawRectBuf& rect = s_rectPool[r];

				if (rect.layer > curLayer)
				{
					nextLayerRect = rect.layer;
					break;
				}

				const Quad quad=
				{
					{ rect.x,          rect.y                },
					{ rect.x + rect.w, rect.y + rect.h       },
					{ rect.u,          1.0f-rect.v           },
					{ rect.u+rect.du,  1.0f-(rect.v+rect.dv) },
					rect.color
				};
				s_gdev->setShaderResource( rect.texture, s_textureHash );
				s_gdev->drawQuad( quad );
			}

			for (; t < s_textIndex; t++)
			{
				const DrawTextBuf& text = s_textPool[t];
				if (text.layer > curLayer)
				{
					nextLayerText = text.layer;
					break;
				}

				TextSystem::setColor( text.color );
				TextSystem::setFont( text.font );
				TextSystem::print( text.x, text.y, text.msg );
			}

			curLayer = MIN(nextLayerRect, nextLayerText);
		};

		clearDraw();
		s_gdev->enableBlending(false);
	}

	void clearDraw()
	{
		s_rectIndex = 0;
		s_textIndex = 0;
	}
};