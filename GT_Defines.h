#ifndef GT_DEFINES_H
#define GT_DEFINES_H

///////////////
// Typedefs //
/////////////

typedef unsigned char uchar;
typedef unsigned int uint;

//////////////////////
// Constant Values //
////////////////////

const float kOneOver255 = 1.0f / 255.0f;
const int kMaxStrLen = 256;
const uint kInvalidIndex = 0xFFFFFFFF;

/////////////
// Marcos //
///////////

// Creates a 32-bit color in ARGB format
#define ARGB(A, R, G, B) ( (int)((A & 0xFF) << 24 | \
								 (R & 0xFF) << 16 | \
								 (G & 0xFF) << 8 | \
								 (B & 0xFF)) )

// Gets either the A, R, G, or B component from 
// a 32-bit color in ARGB format
#define GET_A(c) ((c >> 24) & 0xFF)
#define GET_R(c) ((c >> 16) & 0xFF)
#define GET_G(c) ((c >> 8) & 0xFF)
#define GET_B(c) (c & 0xFF)

// Float to int conversion
#define F2I(x) (int)((x) + 0.5f)

#define MAX(a, b) (((a) > (b))?(a):(b))
#define MIN(a, b) (((a) < (b))?(a):(b))

#endif