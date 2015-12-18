#pragma once
#include "../types.h"
#include <math.h>

namespace Math
{
	const f32 PI        = 3.1415926535897932384626433832795f;
	const f32 twoPI     = 6.283185307179586476925286766559f;
	const f32 PIoverTwo = 1.5707963267948966192313216916398f;
	const f32 degToRadScale = PI / 180.0f;
	const f32 radToDegScale = 180.0f / PI;

	inline bool isPow2(u32 x)
	{
	  return ((x != 0) && !(x & (x - 1))) != 0;
	}

	inline u32 nextPow2(u32 x)
	{
		x = x-1;
		x = x | (x>>1);
		x = x | (x>>2);
		x = x | (x>>4);
		x = x | (x>>8);
		x = x | (x>>16);
		return x + 1;
	}

	template <typename T>
	inline T clamp(T x, T a, T b)
	{
		if ( x < a ) x = a;
		if ( x > b ) x = b;

		return x;
	}

	template <typename T>
	inline T saturate(T x)
	{
		return clamp(x, T(0), T(1));
	}

	template <typename T>
	inline T sign(T x)
	{
		return x >= T(0) ? T(1) : T(-1);
	}

	//simple templates to avoid code duplication.
	//expected inputs: floating point types (to use integers, use float inputs and then convert the result).
	template <typename T>
	inline T lerp(T x, T a, T b)
	{
		return (T(1.0)-x)*a + x*b;
	}

	template <typename T>
	inline T frac(T x)
	{
		T ax = (T)fabs(x);
		T s  = x<T(0.0)?T(-1.0):T(1.0);
		return s*(ax - (T)floor(ax));
	}

	template <typename T>
	inline T degToRad(T degrees)
	{
		return degrees * T(degToRadScale);
	}

	template <typename T>
	inline T radToDeg(T radians)
	{
		return radians * T(radToDegScale);
	}

	template <typename T>
	inline T log2(T x)
	{
		return (T)log10(x) / T(0.30102999566398119521373889472449);
	}

	//Do the closed intervals [a0,a1] and [b0,b1] overlap?
	template <typename T>
	inline bool intervalOverlap(T a0, T a1, T b0, T b1)
	{
		return a0 <= b1 && b0 <= a1;
	}

	//Do the half open intervals [a0,a1) and [b0,b1) overlap?
	template <typename T>
	inline bool intervalOverlapOpen(T a0, T a1, T b0, T b1)
	{
		return a0 < b1 && b0 < a1;
	}

	//cubic polynomial that implements "hermite interpolation" if x is between 0 and 1
	inline f32 cubicPolynomial(f32 x)
	{
		return x*x*(3.0f-2.0f*x);
	}

	//quintic polynomial that implements "hermite interpolation" if x is between 0 and 1.
	inline f32 quinticPolynomial(f32 x)
	{
		return x*x*x*(x*(x*6.0f - 15.0f) + 10.0f);
	}

	//evaluates to 0 if x <= a and 1 if x >= b, linear interpolation between 0 to 1.
	inline f32 lineStep(f32 x, f32 a, f32 b)
	{
		return saturate( (x-a) / (b-a) );
	}

	//standard smoothstep
	//evaluates to 0 if x <= a and 1 if x >= b, hermite cubic interpolation from 0 to 1 between.
	//1st order derivatives evaluate to 0 at a and b.
	inline f32 smoothStep(f32 x, f32 a, f32 b)
	{
		return cubicPolynomial( lineStep(x, a, b) );
	}

	//"improved" smoothstep, C2 continuous. Obviously more expensive to evaluate than smoothstep()
	//evaluates to 0 if x <= a and 1 if x >= b, hermite quintic interpolation from 0 to 1 between.
	//1st and 2nd order derivatives evaluate to 0 at a and b.
	inline f32 smootherStep(f32 x, f32 a, f32 b)
	{
		return quinticPolynomial( lineStep(x, a, b) );
	}

	//a cubic pulse centered on 'c' with width 'w'
	//the result is 0 when x less than c-w or greater than c+w 
	//the result is 1 when x == c
	//the shape is Gaussian-like.
	inline f32 cubicPulse(f32 x, f32 c, f32 w)
	{
		x = fabsf(x - c);
		if (x > w) return 0.0f;

		return 1.0f - cubicPolynomial( x/w );
	}

	//maps a 0..1 range to a power curve where (x=0) and (x=1) are mapped to 0.
	//a and b skew the shape of the curve such that the peak can be off-center.
	//the peak is not scaled to 1.0, use this version when custom scaling is required.
	inline f32 powerCurveNoScale(f32 x, f32 a, f32 b)
	{
		return powf( x, a ) * powf( 1.0f-x, b );
	}

	//maps a 0..1 range to a power curve where (x=0) and (x=1) are mapped to 0.
	//a and b skew the shape of the curve such that the peak can be off-center.
	//the peak is exactly 1.0
	inline f32 powerCurve(f32 x, f32 a, f32 b)
	{
		const f32 k = powf(a+b,a+b) / (powf(a,a)*powf(b,b));
		return k * powerCurveNoScale(x, a, b);
	}
}