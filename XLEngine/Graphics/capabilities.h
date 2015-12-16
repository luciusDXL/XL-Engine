#pragma once

enum CapabilityFlags
{
	CAP_SUPPORT_SHADERS = (1<<0),
	CAP_NON_POWER_2_TEX = (1<<1),
	CAP_RENDER_TARGET   = (1<<2),
};

struct Capabilities
{
	u32 flags;
	u32 maxTextureSize2D;
};