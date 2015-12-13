#pragma once
#include "../types.h"

///////////////////////////////////////////////////
// Enums
///////////////////////////////////////////////////
enum BlendMode
{
	BLEND_OVER = 0,		//src*Alpha + dst*(1-Alpha)
	BLEND_ALPHA_ADD,	//src*Alpha + dst
	BLEND_ADD,			//src + dst
	BLEND_COUNT
};

enum ShaderID
{
	SHADER_QUAD_UI = 0,
	SHADER_COUNT
};


///////////////////////////////////////////////////
// Typedefs and structures
///////////////////////////////////////////////////
typedef u32 Color;
typedef u32 TextureHandle;

struct iPoint
{
	s32 x;
	s32 y;
};

struct fPoint
{
	f32 x;
	f32 y;
};

struct Quad
{
	iPoint p0;
	iPoint p1;
	fPoint uv0;
	fPoint uv1;
	Color color;
};


///////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////
#define INVALID_TEXTURE_HANDLE 0xffffffff
