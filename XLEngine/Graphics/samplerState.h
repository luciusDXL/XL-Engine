#pragma once

enum WrapMode
{
	WM_WRAP = 0,
	WM_CLAMP,
	WM_MIRROR,
	WM_COUNT,
};

enum TextureFilter
{
	TEXFILTER_POINT = 0,
	TEXFILTER_LINEAR,
	TEXFILTER_ANISOTROPIC,
	TEXFILTER_COUNT
};

struct SamplerState
{
	WrapMode wrapU;
	WrapMode wrapV;
	WrapMode wrapW;

	TextureFilter minFilter;
	TextureFilter magFilter;
	TextureFilter mipFilter;

	bool enableMipMapping;
};
