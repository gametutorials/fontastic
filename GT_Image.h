#ifndef	GT_IMAGE_H
#define	GT_IMAGE_H

#include <windows.h>
#include <assert.h>
#include "GT_Defines.h"

// The only type of .tga files this library supports
#define TGA_RGB 2

// Exception thrown if errors occur
#define GT_IMAGE_ERROR_LOADING 1
#define GT_IMAGE_ERROR_SAVING 2

// An image class which supports 24-bit images
// Minimal error checking so optimized for speed
class CImage
{
	public:

		enum EFlipType
		{
			eHorizontal, // Flips around the vertical axis (right to left, left to right)
			//eVertical, // Flips around the horizontal axis (top to bottom, bottom to top)
		};

		CImage(); // Constructor() -- Zero's out CImage
		CImage(const CImage &image); // Copy constructor

		CImage& operator =(const CImage& image); // Assignment operator

		// Returns true for success -- false otherwise
		// If init() is called on a CImage that already has memory associated with it
		// that memory is freed and the new size is implemented
		bool init(int width, int height, int channels = 3);
		
		// Loads a Win32 resource with specified resourceID -- Returns true on success, false otherwise
		// If loadWin32Resource() is called on a CImage that already has memory associated with
		// it, that memory is freed and the resource is loaded
		bool loadWin32Resource(int resourceID);

		// Loads a bmp with specified fileName -- Returns true on success, false otherwise
		// If loadBMP() is called on a CImage that already has memory associated with
		// it, that memory is freed and the the .bmp is loaded
		bool loadBMP(const char *fileName);

		// Loads a .tga with specified fileName -- Returns true on success, false otherwise
		// Free's existing CImage if already allocated
		bool loadTGA(const char *fileName);
		
		// Saves the CImage a .bmp file with "fileName"
		// There is no need to add the extension ".bmp" to "fileName"
		// Returns true on success, false otherwise
		bool saveAsBMP(const char *fileName);

		// Saves the CImage as a .tga file with "fileName"
		// There is no need to add the extension ".tga" to "fileName"
		// Returns true on success, false otherwise
		bool saveAsTGA(const char *fileName);

		// Sets the pixel (x,y) to the color passed in
		void setPixel(int x, int y, uchar R, uchar G, uchar B);
		void setPixel(int x, int y, int argb);
												   
		// Sets the CImage to the specified color
		void setToColor(uchar R, uchar G, uchar B);
		void setToColor(int argb);
		
		// Fills the rectangle area in the CImage to the color passed in
		void fillRect(const RECT &rect, uchar R, uchar G, uchar B);
		void fillRect(const RECT &rect, int argb);

		// Blits CImage to "dest" starting at upper-left coordinates (x,y) 
		// using raster operation code "rop" -- Uses GDI BitBlt()
		void blit(HDC destDC, int x, int y, DWORD rop = SRCCOPY) const;
		
		// Blits CImage to "destDC" using dimensions in "destRect"
		// using raster operation code "rop" -- Uses GDI BitBlt()
		void blit(HDC destDC, const RECT &destRect, DWORD rop = SRCCOPY) const;
		
		// Blits CImage to "dest" scaling the image as necessary
		// Uses bilinear filtering to do the scaling
		void scaleBlit(CImage &dest);
		
		// Transparent blits to "destDC" using dimensions in "destRect" with specified
		// transparency color -- Uses GDI TransparentBlt()
		void transparentBlit(HDC destDC, const RECT &destRect, uchar R, uchar G, uchar B) const;
		void transparentBlit(HDC destDC, const RECT &destRect, int color = ARGB(0,0,0,0)) const;
		
		// Transparent blits to "dest" using starting at upper left corner (x,y) using
		// the specified transparency color	
		void transparentBlit(CImage &dest, int x, int y, uchar R, uchar G, uchar B) const;
		void transparentBlit(CImage &dest, int x, int y, int color = ARGB(0,0,0,0)) const;
		
		// Transparent blits "dest" starting at upper left hand corner (x,y) using
		// the RGB in "color", also draws image using alpha component found in "color"
		void alphaTransBlit(CImage &dest, int x, int y, int color = ARGB(255, 0, 0, 0)) const; 

		// Blits to "dest" using alpha starting at upper-left coordinates (x,y)
		// **NOTE** CImage must be 32-bit or unexpected results will occur
		void alphaBlit(CImage &dest, int x, int y) const;
		
		// Blits to "dest" using the constant alpha value passed in, starting
		// at upper-left coordinates (x,y)
		void constAlphaBlit(CImage &dest, int x, int y, uchar alpha) const;

		// Fills the CImage with "src" by the dimensions specified in "srcRect"
		// starting at upper-left corner (destX, destY) using transparency color (R,G,B)
		// Does not alter alpha in source or destination
		void transparentFill(CImage &src, const RECT &srcRect, int destX, int destY,
							 uchar R, uchar G, uchar B);

		// Fills the CImage with "src" by the dimensions specified in "srcRect"
		// starting at upper-left corner (destX, destY)
		// Achieves "opaqueness" by only drawing every other pixel
		// Does not alter alpha in source or destination
		void opaqueFill(CImage &src, const RECT &srcRect, int destX, int destY);
		
		// Fills the CImage with "src" by the dimensions specified in "srcRect"
		// starting at upper-left corner (destX, destY) using per pixel alpha in "src"
		// Does not alter alpha in source or destination
		// **NOTE** "src" must be a 32-bit CImage or unexpected results will occur
		void alphaFill(CImage &src, const RECT &srcRect, int destX, int destY);
				
		void blendDown(); // Averages all lines together from top to bottom
		void blendUp(); // Averages all lines together from bottom to top

		void flip(EFlipType type);

		void decrement(int amount); // Decrements each color component by "amount"
		void increment(int amount); // Increments each color component by "amount"
		
		RECT getRect() const; // Returns a RECT of CImage with upper-left corner being (0,0)
		
		// Returns a pixel at row Y, column X in the CImage
		// Alpha will equal 255 if it's a 24-bit image
		// If (x,y) is out of range the return value is 0
		int getPixel(int x, int y) const;
		
		// Data Access Functions ************

			inline int getWidth() const { return mWidth; } 
			inline int getHeight() const { return mHeight; }
			inline int getChannels() const { return mChannels; }
			inline int getStride() const { return mStride; }
			inline HDC getDC() const { return mHDC; }
			
			// Returns a pointer to the first pixel in the line of pixels
			// with "which" row number
			// ---------- Line (0) Top of image
			// |		|
			// |		|
			// |		|
			// ---------- Line (Height - 1) Bottom of image
			uchar* getLinePtr(int which)
			{
				assert(which >= 0 && which < mHeight);
				return (mPixels + mStride * which);
			}
			
			// const version of getLinePtr()
			uchar* getLinePtr(int which) const
			{
				assert(which >= 0 && which < mHeight);
				return (mPixels + mStride * which);
			}

		// ****** End of Data Access Functions

		// Deconstructor()
		~CImage();
		
	private:

		int mWidth;
		int mHeight;
		int mChannels;
		int mStride;

		HBITMAP mBmp;
		HBITMAP mOldBmp;

		HDC mHDC;

		uchar *mPixels;

		void freeMem(); // Frees all mem associated with CImage
		void zeroVars()
		{
			mWidth = mHeight = mChannels = mStride = 0;
			mBmp = mOldBmp = NULL;
			mHDC = NULL;
			mPixels = NULL;
		}
};

#endif

/*----------------------------*\
|  TheTutor                    |
|  thetutor@gametutorials.com  |
|  © 2000-2003 GameTutorials   |
\*----------------------------*/
