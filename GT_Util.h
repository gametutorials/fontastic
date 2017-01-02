#ifndef GT_UTIL_H
#define GT_UTIL_H

#include <stdlib.h>
#include "GT_Defines.h"

/////////////
// Marcos //
///////////

// Returns random colors in the ARGB format
#define RAND_RGB() ARGB(255, rand()%256, rand()%256, rand()%256)
#define RAND_ARGB() ARGB(rand()%256, rand()%256, rand()%256, rand()%256)
#define RAND_RED() ARGB(255, rand()%256, 0, 0)
#define RAND_GREEN() ARGB(255, 0, rand()%256, 0)
#define RAND_BLUE() ARGB(255, 0, 0, rand()%256)

// Safely free up memory
#define SAFE_FREE(x) if(x) { free(x); x = NULL; }
#define SAFE_DELETE(x) if(x) { delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x) if(x) { delete[] x; x = NULL; }
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }

///////////////////////
// Helper Functions //
/////////////////////

void Lowercase(char *str); // Makes "str" lowercase
void Uppercase(char *str); // Makes "str" uppercase

// Prints error messages to the Output Window in .NET (Debug Only)
void OutputError(const char *err, ...);

#endif