#pragma once

typedef unsigned long long	u64;
typedef unsigned int		u32;
typedef unsigned short		u16;
typedef unsigned char		u8;

typedef long long			s64;
typedef int					s32;
typedef short				s16;
typedef char				s8;

typedef float				f32;
typedef double				f64;

#ifdef _WIN32
	#define XL_STDCALL __stdcall
#else
	#define XL_STDCALL
#endif
