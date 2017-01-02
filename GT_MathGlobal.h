#ifndef GT_MATHGLOBAL_H
#define GT_MATHGLOBAL_H

// Global math defines/macros/functions/classes to be used by everything
#pragma once

#include <stdlib.h>
#include <assert.h>
#include <cmath>

//////////////////////
// Constant Values //
////////////////////

const float kPi = 3.14159265f;
const float k2Pi = kPi * 2.0f;
const float kHalfPi = kPi / 2.0f;
const float k1OverPi = 1.0f / kPi;
const float k1Over2Pi = 1.0f / k2Pi;
const float kPiOver180 = kPi / 180.0f;
const float k180OverPi = 180.0f / kPi;

/////////////
// Marcos //
///////////

#define DEG2RAD(x) (x * kPiOver180) // Converts degrees to radians
#define RAD2DEG(x) (x * k180OverPi) // Converts radians to degrees

#define FLT2INT(x) ( (int)(x + 0.5f) )

// Returns a random percent between 0 - 1
#define RAND_PERCENT() ((rand() & 0x7FFF) / ((float)0x7FFF))

////////////////
// Functions //
//////////////

// Checks for two variables to be equal within a certain tolerance
template <typename type>
inline bool Equivalent(type a, type b, type t)
{
	return ((a > b - t) && (a < b + t));
}

// Returns a random number between and including min and max
// Assumes generic case to be integer "min" and "max" input parameters
template <typename type>
type Rand(type min, type max)
{
	assert(min < max); // Make sure min is less than max
	return (type)(min + (max - min) * RAND_PERCENT() + 0.5f);
}

// Exlipict specilization of Rand() for floats
template <> 
inline float Rand<float>(float min, float max)
{
	assert(min < max); // Make sure min is less than max
	return (min + (max - min) * RAND_PERCENT());
}

// Exlipict specilization of Rand() for doubles
template <>
inline double Rand<double>(double min, double max)
{
	assert(min < max); // Make sure min is less than max
	return (min + (max - min) * RAND_PERCENT());
}

//////////////
// Classes //
////////////

// Class for oscillating a number between a "min" and a "max"
template <typename type>
class COscillator
{
	public:

		COscillator() // Default constructor
		{
			memset(&mMin, 0, sizeof(type));
			memset(&mMax, 0, sizeof(type));
			memset(&mVal, 0, sizeof(type));
			memset(&mIncAmt, 0, sizeof(type));
		}

		COscillator(type min, type max, type incAmt)
		{
			setMinMax(min, max);
			setIncAmt(incAmt);
			mVal = mMin + (mMax - mMin) * 0.5f; // Set to center of (max,min)
		}

		// Update the oscillator
		void update()
		{
			mVal += mIncAmt;

			if(mVal > mMax)
			{
				mVal = mMax;
				mIncAmt = -mIncAmt;
			}
			else if(mVal < mMin)
			{
				mVal = mMin;
				mIncAmt = -mIncAmt;
			}
		}

		// Update the oscillator with respect to time
		void update(float dt)
		{
			mVal += mIncAmt * dt;

			if(mVal > mMax)
			{
				mVal = mMax;
				mIncAmt = -mIncAmt;
			}
			else if(mVal < mMin)
			{
				mVal = mMin;
				mIncAmt = -mIncAmt;
			}
		}

		void setMinMax(type min, type max) { mMin = min; mMax = max; }
		void setIncAmt(type incAmt) { mIncAmt = incAmt; }
		void setVal(type val) { mVal = val; } // Set current value

		// Data Access ***

		type getVal() { return mVal; } // Get current value

		// *** End Data Access

	private:

		type mMin;
		type mMax;
		type mVal;
		type mIncAmt;
};

#endif
