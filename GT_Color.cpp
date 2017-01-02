#include "GT_Color.h"
#include "GT_MathGlobal.h"

const float kColorTolerance = 0.01f;

// Equals operator
bool CColor::operator ==(const CColor &c) const
{
	return (Equivalent(r, c.r, kColorTolerance) &&
			Equivalent(g, c.g, kColorTolerance) &&
			Equivalent(b, c.b, kColorTolerance) &&
			Equivalent(a, c.a, kColorTolerance));
}

// Not equals operator
bool CColor::operator !=(const CColor &c) const
{
	return !(*this == c);
}	

// Set the CColor
void CColor::set(int argb)
{
	a = GET_A(argb) / 255.0f;
	r = GET_R(argb) / 255.0f;
	g = GET_G(argb) / 255.0f;
	b = GET_B(argb) / 255.0f;
}

// Set the CColor
void CColor::set(uchar A, uchar R, uchar G, uchar B)
{
	a = A / 255.0f;
	r = R / 255.0f;
	g = G / 255.0f;
	b = B / 255.0f;
}

// Set the CColor
void CColor::set(float A, float R, float G, float B)
{
	a = A;
	r = R;
	g = G;
	b = B;
}

// Normalize the color between 0.0f and 1.0f
void CColor::normalize()
{
	float high = 0.0f;
	
	// Make sure all components are positive
	for(int i = 0; i < 4; ++i)
		rgba[i] = fabs(rgba[i]);
	
	// Loop through all and find the greatest component
	for(int i = 0; i < 4; ++i)
	{
		if(rgba[i] > high)
			high = rgba[i];
	}
	
	// If a color component is greater than 1.0f
	if(high > 1.0f)
	{
		// Normalize all colors to that value
		for(int i = 0; i < 4; ++i)
			rgba[i] /= high;
	}
}

// Clamps the color to the range of (0.0f - 1.0f)
void CColor::clamp()
{
	for(int i = 0; i < 4; ++i)
	{
		if(rgba[i] < 0.0f)
			rgba[i] = 0.0f;
		else if(rgba[i] > 1.0f)
			rgba[i] = 1.0f;
	}
}

// Returns packed ARGB version of the color
int CColor::getARGB() const
{
	return ARGB((uchar)(a * 255.0f),
				(uchar)(r * 255.0f),
				(uchar)(g * 255.0f),
				(uchar)(b * 255.0f));
}

// Returns packed ABGR version of the color
int CColor::getABGR() const
{
	return ARGB((uchar)(a * 255.0f),
				(uchar)(b * 255.0f), // We want Blue here
				(uchar)(g * 255.0f),
				(uchar)(r * 255.0f)); // We want Red here
}

// *** Friend Functions ***

// LERP between "start" and "end" at time dt (between 0.0f - 1.0f)
CColor LerpColor(const CColor &start, const CColor &end, float dt)
{
	float oneMinusDT = 1.0f - dt;
	
	return CColor(oneMinusDT * start.a + end.a * dt,
				  oneMinusDT * start.r + end.r * dt,
				  oneMinusDT * start.g + end.g * dt,
				  oneMinusDT * start.b + end.b * dt);
}