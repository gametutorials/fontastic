#ifndef GT_COLOR_H
#define GT_COLOR_H

#include "GT_Defines.h"

// Defines a color 
class CColor
{
	public:
	
		// Constructors
		CColor(int argb = ARGB(255, 0, 0, 0)) { set(argb); }
		CColor(uchar A, uchar R, uchar G, uchar B) { set(A, R, G, B); }
		CColor(float A, float R, float G, float B) { set(A, R, G, B); }
		
		// More constructors... Alpha defaults to 1.0f
		CColor(uchar R, uchar G, uchar B) { set(255, R, G, B); }
		CColor(float R, float G, float B) { set(1.0f, R, G, B); }
		
		// Copy Constructor
		CColor(const CColor &c) : a(c.a), r(c.r), g(c.g), b(c.b) {}
		
		// Assignment operator
		CColor& operator =(const CColor &c)
		{
			a = c.a;
			r = c.r;
			g = c.g;
			b = c.b;
				return *this;
		}
		
		// Multiplication operator
		CColor operator *(const CColor &c) const
		{
			return CColor(a * c.a, r * c.r, g * c.g, b * c.b);
		}
		
		// Addition operator
		CColor operator +(const CColor &c) const
		{
			return CColor(a + c.a, r + c.r, g + c.g, b + c.b);
		}
		
		// Subtraction operator
		CColor operator -(const CColor &c) const
		{ 
			return CColor(a - c.a, r - c.r, g - c.g, b - c.b);
		}
		
		// Equals operator
		bool operator ==(const CColor &c) const;
		
		// Not equals operator
		bool operator !=(const CColor &c) const;
		
		// Sets the CColor to the specified color components
		// If alpha is not specified, it defaults to 1.0f
		void set(int argb);
		void set(uchar A, uchar R, uchar G, uchar B);
		void set(float A, float R, float G, float B);
			
		void normalize(); // Normalizes each color component to be between 0.0f - 1.0f
		void clamp(); // Clamps all components to a value between 0.0f - 1.0f
		
		// Returns an int with an ARGB layout, with each component
		// represented by an unsigned char between 0 - 255
		int getARGB() const;
		
		// Returns an int with an ARGB layout, with each component
		// represented by an unsigned char between 0 - 255
		int getABGR() const;
		
		// Returns the CColor at time "dt", linearly interpolated between 
		// "start" and "end"
		friend CColor LerpColor(const CColor &start, const CColor &end, float dt);
	
		// Public data
		union
		{
			struct {
				float r, g, b, a;
			};
			
			float rgba[4];
		};
};

#endif