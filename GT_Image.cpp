// Done by TheTutor -- 2002 (Updated Last: 4/27/04)
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include "GT_Image.h"
#include "GT_Util.h"

// Load needed lib
#pragma comment(lib, "msimg32.lib")

#pragma pack(push, 1)
	struct STGAHeader
	{
		char  idFieldLength; // 1 - 255 bytes of optional identification before pixel bits (usually 0)
		char  colorMapType;
		char  dataType; // Type of TGA file -- For us always TGA_RGB
		short colorMapOrigin;
		short colorMapLength;
		char  colorMapEntrySize;
		short x; // x pos of image origin
		short y; // y pos of image origin
		short width;
		short height;
		char  bitsPerPixel;
		char  descriptor;
	};
#pragma pack(pop)

// Windows bitmaps are in BGR format instead of RGB.
// These defines will help let us index into a "color" more clearly
enum {
	eBlue = 0,
	eGreen = 1,
	eRed = 2,
	eAlpha = 3,
};

const int kMaxARGB = 255; // Maximum ARGB value

// Returns true if (R1,G1,B1) == (R2,G2,B2)
inline bool SameRGB(uchar R1, uchar G1, uchar B1, uchar R2, uchar G2, uchar B2)
{
	return ((R1 == R2) && (G1 == G2) && (B1 == B2));
}

// JHACK -- Only clips on the X, CLAMPS on the Y
inline void Clip(int &dx, int &dy, int dw, int dh, int &sx, int &sy, int &sw, int &sh)
{
	if(dx < 0)
	{
		sx = -dx;
		dx = 0;
	}

	if(dy < 0)
	{
		//sy = -dy;
		dy = 0;
	}

	if(dx + sw > dw)
	{
		sw -= ((dx + sw) - dw);
	}

	if(dy + sh > dh)
	{
		//sh -= ((dy + sh) - dh);
		dy = dh - sh;
	}
}

// Constructor
CImage::CImage()
{
	zeroVars();
	GdiFlush(); // Make sure that writing to CImage is okay
}

// Copy Constructor
CImage::CImage(const CImage &image)
{
	zeroVars();
	GdiFlush(); // Make sure that writing to CImage is okay
	*this = image;
}

// Assignment Operator
CImage& CImage::operator =(const CImage &image)
{
	// Check for assigning to self
	if(this == &image) return *this;

	// Set the size of the image (allocate space for the image)
	init(image.getWidth(), image.getHeight(), image.getChannels());
	
	// Copy over pixel data
	memcpy(mPixels, image.mPixels, image.getStride() * image.getHeight());
		return *this;
}

// Returns true for success -- false otherwise
bool CImage::init(int wid, int hgt, int chn)	
{
		// Error Check -- We only support 24 and 32-bit bitmaps
		if(!(chn == 3 || chn == 4))
			return false;
 
	// If CImage has already been set -- clear it out first
	freeMem();

	// Create a compatible HDC
	mHDC = CreateCompatibleDC(NULL);
	
		// Error Check
		if(!mHDC)
			return false;

	mWidth = wid;
	mHeight = hgt;
	mChannels = chn;
	
	// Set stride
	mStride = mWidth * mChannels;

	while((mStride % 4) != 0) // Ensure stride is DWORD aligned
		mStride++;
		
	BITMAPINFO info = {0};

	// Initialize the parameters that we care about
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = mWidth;
	info.bmiHeader.biHeight = mHeight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = mChannels * 8;
	info.bmiHeader.biCompression = BI_RGB;

	// Create a DIBSection. Note that this returns us two
	// things - an HBITMAP handle and a memory pointer, in
	// pixels.
	mBmp = CreateDIBSection(mHDC, &info, DIB_RGB_COLORS,
							   (void**)&mPixels, 0, 0);
		// Error Check
		if(!mBmp)
			return false;		

	// Select our DIBSection bitmap into our DC.
	// This allows us to draw to the bitmap using GDI functions.
	mOldBmp = (HBITMAP)SelectObject(mHDC,mBmp);

		// Error Check
		if (!mOldBmp)
			return false;

	return true;

} // end of init(int width, int height, int channels)

// Loads a Win32 resource
bool CImage::loadWin32Resource(int resourceID)
{
	HBITMAP hbitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceID),
										 IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
		
		// Error check						
		if(!hbitmap)
			return false;
		
	HDC hdc = CreateCompatibleDC(NULL);
	
		// Error check
		if(!hdc)
			return false;
			
	HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hbitmap);
	
		// Error check
		if(!oldBmp)
			return false;
			
	
	BITMAP info = {0};
	
	// Get image's properties
	if(GetObject(hbitmap, sizeof(BITMAP), &info) != sizeof(BITMAP))
		return false;
		
	// Set the size of the CImage
	if(!init(info.bmWidth, info.bmHeight, info.bmBitsPixel / 8))
		return false;
		
	// Copy over pixel data
	BitBlt(mHDC, 0, 0, getWidth(), getHeight(), hdc, 0, 0, SRCCOPY);
	
	// Free up resource
	SelectObject(hdc, oldBmp);
	DeleteObject(hbitmap);
	DeleteDC(hdc);
		return true; // Load was successful	 
}

bool CImage::loadBMP(const char *file_name)
{
		// Error Check -- Make sure they passed in a valid file name
		if(!file_name)
			return false;

	FILE *file = fopen(file_name, "rb");

		// Error Check -- Make sure the file could be opened
		if(!file)
			return false;

	try
	{
		BITMAPFILEHEADER fileheader;
		
		// Read the BITMAPFILEHEADER
		if(!fread(&fileheader, sizeof(BITMAPFILEHEADER), 1, file))
			throw GT_IMAGE_ERROR_LOADING;

		// Check the type field to make sure we have a .bmp file
		if(memcmp(&fileheader.bfType, "BM", 2))
			throw GT_IMAGE_ERROR_LOADING;

		BITMAPINFOHEADER infoheader;
		
		// Read the BITMAPINFOHEADER.
		if(!fread(&infoheader, sizeof(BITMAPINFOHEADER), 1, file))
			throw GT_IMAGE_ERROR_LOADING;

		// Double Check to make sure we got a correct BITMAPINFOHEADER
		if(infoheader.biSize != sizeof(infoheader))
			throw GT_IMAGE_ERROR_LOADING;

		// Check for unsupported format
		if(infoheader.biPlanes != 1)
			throw GT_IMAGE_ERROR_LOADING;
			
		// One more double check that we have a .bmp we can handle
		if(infoheader.biCompression != BI_RGB)
			throw GT_IMAGE_ERROR_LOADING;

		if(init(infoheader.biWidth,infoheader.biHeight,infoheader.biBitCount / 8) == false)
			throw GT_IMAGE_ERROR_LOADING;

		// Jump to the location where the bitmap data is stored
		// fseek() returns 0 on success
		if(fseek(file, fileheader.bfOffBits, SEEK_SET))
			throw GT_IMAGE_ERROR_LOADING;

		{ // Now read in the pixel bits

			unsigned int bytesUsed = mWidth * mChannels;
			unsigned int padding = mStride - bytesUsed;

			for(int y = 0; y < mHeight; ++y)
			{
				// Get the current line
				uchar *line_ptr = getLinePtr(y);

				// Read the precise number of bytes that the line requires into the bitmap.
				// Don't read the padding bytes, because the in memory alignment requirements
				// may vary
				if(!fread(line_ptr, bytesUsed, 1, file))
					throw GT_IMAGE_ERROR_LOADING;

				// Skip over any padding bytes.
				if(fseek(file, padding, SEEK_CUR))
					throw GT_IMAGE_ERROR_LOADING;
		
			} // end of for(int y = 0; y < height; ++y)
		}
	}
	catch(int error)
	{
		fclose(file);
		throw error;
			return false;
	}
	
	fclose(file);
		return true; // Load was successful

} // end of loadBMP(char *file_name)

bool CImage::loadTGA(const char *file_name)
{
	// Make sure we have a legal file name
	if(!file_name)
		return false;

	FILE *file = fopen(file_name,"rb");

		// Error Check
		if(!file)
			return false;
			
	try
	{
		STGAHeader header = {0};
			
		if(!fread(&header, sizeof(STGAHeader), 1, file))
			throw GT_IMAGE_ERROR_LOADING;

		if(!init(header.width,header.height,header.bitsPerPixel / 8))
			throw GT_IMAGE_ERROR_LOADING;

		if(header.idFieldLength > 0)
			fseek(file,header.idFieldLength,SEEK_CUR); // Seek past optional image description
										  
		{ // Now load in the pixel bits

			int bytesUsed = mWidth * mChannels;

			// Load in all the pixel data
			for(int y = 0; y < mHeight; ++y)
			{	
				// Get the current line
				uchar *line_ptr = getLinePtr(y);

				if(!fread(line_ptr,bytesUsed,1,file))
					throw GT_IMAGE_ERROR_LOADING;

			}
		}
	}
	catch(int error)
	{
		fclose(file);
		throw error;
			return false;
	}

	fclose(file);
		return true; // Load was successful

} // end of loadTGA(char *file_name)

bool CImage::saveAsBMP(const char *fileName)
{
	// Error Check -- Make sure they passed in a valid file name
	if(!fileName)
		return false;
		
	char *saveFileName = NULL;
	
	// Allocate string long enough for "fileName" + ".bmp" + a terminating NULL
	saveFileName = new char[strlen(fileName) + 5];
	
		// Error check
		if(!saveFileName)
			return false;
			
	// Copy over "fileName"
	strcpy(saveFileName, fileName);
	
	// See if ".bmp" is already in string, if not append it
	if(!strstr(saveFileName, ".bmp"))
		strcat(saveFileName, ".bmp"); // Add on the ".bmp"

	FILE *file = fopen(saveFileName, "wb");

	// Error Check -- Make sure the file could be opened for writing
	if(!file)
		return false;
	
	// We can free up the temporary file name	
	SAFE_DELETE_ARRAY(saveFileName);

	try
	{
		BITMAPFILEHEADER fileheader = {0};
		
		// Fill in the BITMAPFILEHEADER
		fileheader.bfType = 19778; // 19778 == "BM"
		fileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		fileheader.bfSize = fileheader.bfOffBits + (getHeight() * getStride()); // The size of the file in bytes

		// Write the BITMAPFILEHEADER
		if(!fwrite(&fileheader, sizeof(BITMAPFILEHEADER), 1, file))
			throw GT_IMAGE_ERROR_SAVING;

		BITMAPINFOHEADER infoheader = {0};
		
		// Fill in the BITMAPINFOHEADER
		infoheader.biSize = sizeof(BITMAPINFOHEADER);
		infoheader.biPlanes = 1;
		infoheader.biCompression = BI_RGB;
		infoheader.biWidth = getWidth();
		infoheader.biHeight = getHeight();
		infoheader.biBitCount = getChannels() * 8;
		infoheader.biXPelsPerMeter = 2834; // Resolution of 72 pixels per inch
		infoheader.biYPelsPerMeter = 2834; // Resolution of 72 pixels per inch
		
		// Write the BITMAPINFOHEADER
		if(!fwrite(&infoheader, sizeof(BITMAPINFOHEADER), 1, file))
			throw GT_IMAGE_ERROR_SAVING;

		for(int y = 0; y < mHeight; ++y)
		{
			// Get the current line
			uchar *linePtr = getLinePtr(y);

			// Write out the bytes
			if(!fwrite(linePtr, getStride(), 1, file))
				throw GT_IMAGE_ERROR_SAVING;

		} // end of for(int y = 0; y < height; ++y)
	}
	catch(int error)
	{
		fclose(file);
		throw error;
			return false;
	}

	fclose(file);
		return true; // Load was successful
}

bool CImage::saveAsTGA(const char *fileName)
{
	// Error Check -- Make sure they passed in a valid file name
	if(!fileName)
		return false;
		
	char *saveFileName = NULL;
	
	// Allocate string long enough for "fileName" + ".tga" + a terminating NULL
	saveFileName = new char[strlen(fileName) + 5];
	
		// Error check
		if(!saveFileName)
			return false;
			
	// Copy over "fileName"
	strcpy(saveFileName, fileName);
	
	// See if ".bmp" is already in string, if not append it
	if(!strstr(saveFileName, ".tga"))
		strcat(saveFileName, ".tga"); // Add on the ".tga"

	FILE *file = fopen(saveFileName, "wb");

	// Error Check -- Make sure the file could be opened for writing
	if(!file)
		return false;
	
	// We can free up the temporary file name	
	SAFE_DELETE_ARRAY(saveFileName);

	try
	{
		STGAHeader fileheader = {0};

		fileheader.dataType = TGA_RGB;
		fileheader.bitsPerPixel = getChannels() * 8;
		fileheader.width = getWidth();
		fileheader.height = getHeight();
		fileheader.descriptor = (getChannels() == 4) ? 8 : 0;
		
		// Write the header
		if(!fwrite(&fileheader, sizeof(STGAHeader), 1, file))
			throw GT_IMAGE_ERROR_SAVING;

		for(int y = 0; y < mHeight; ++y)
		{
			// Get the current line
			uchar *linePtr = getLinePtr(y);

			// Write out the bytes
			if(!fwrite(linePtr, getStride(), 1, file))
				throw GT_IMAGE_ERROR_SAVING;

		} // end of for(int y = 0; y < height; ++y)
	}
	catch(int error)
	{
		fclose(file);
		throw error;
			return false;
	}

	fclose(file);
		return true; // Load was successful
}

// Set pixel at (x,y) to (R,G,B)
void CImage::setPixel(int x, int y, uchar R, uchar G, uchar B)
{
	assert((x >= 0) && (x < mWidth));
	assert((y >= 0) && (y < mHeight));

	uchar *destLine = getLinePtr(mHeight - y - 1); // Flip for Win32 coordinates

	destLine += (x * mChannels);

	destLine[eRed] = R;
	destLine[eGreen] = G;
	destLine[eBlue] = B;
}

// Set pixel at (x,y) to (R,G,B,A)
// If image is 24-bit the A component will be skipped
void CImage::setPixel(int x, int y, int argb)
{
	assert((x >= 0) && (x < mWidth));
	assert((y >= 0) && (y < mHeight));

	uchar *destLine = getLinePtr(mHeight - y - 1); // Flip for Win32 coordinates

	destLine += (x * mChannels);
	
	destLine[eRed] = GET_R(argb);
	destLine[eGreen] = GET_G(argb);
	destLine[eBlue] = GET_B(argb);
	
	if(mChannels == 4)
		destLine[eAlpha] = GET_A(argb);	
}


// Set CImage to color passed in
void CImage::setToColor(uchar red, uchar green, uchar blue)
{
	int x = 0, y = 1; // Loop variables
	uchar *line_ptr = getLinePtr(0); // Get pointer to first line of pixels
	
	for(x = 0; x < mWidth; ++x)
	{
		line_ptr[eRed] = red;
		line_ptr[eGreen] = green;
		line_ptr[eBlue] = blue;
			
		line_ptr += mChannels;
	}
	
	line_ptr = getLinePtr(0); // Move back to start of line 0
	
	for(; y < mHeight; ++y)
		memcpy(getLinePtr(y), line_ptr, mStride);
		
}

// Set CImage to color passed in
void CImage::setToColor(int argb)
{
	int x = 0, y = 1; // Loop variables
	uchar *line_ptr = getLinePtr(0); // Pointer to the first line of pixels

	// If it's a 32-bit image
	if(mChannels == 4)
	{
		// Set first line of pixels
		for(x = 0; x < mWidth; ++x)
		{
			memcpy(line_ptr, &argb, 4);
			line_ptr += mChannels;
		}
	}
	else
	{
		uchar red = GET_R(argb);
		uchar green = GET_G(argb);
		uchar blue = GET_B(argb);
		
		// Set the first line to the color passed in
		for(x = 0; x < mWidth; ++x)
		{
			*line_ptr++ = blue;
			*line_ptr++ = green;
			*line_ptr++ = red;
		}
	}
	
	line_ptr = getLinePtr(0); // Move back to start of line 0
	
	// Copy first line to rest of image
	for(; y < mHeight; ++y)
		memcpy(getLinePtr(y), line_ptr, mStride);
}

// Fill the rect area with the color passed in
void CImage::fillRect(const RECT &rect, uchar R, uchar G, uchar B)
{
	assert(rect.left >= 0 && rect.right <= mWidth);
	assert(rect.top >= 0 && rect.bottom <= mHeight);

	int x; // for the x loops

	uchar *line_ptr;

	for(int y = rect.top; y < rect.bottom; ++y)
	{	
		line_ptr = getLinePtr(mHeight - y - 1); // Flip for Win32 coordinates
		line_ptr += (rect.left * mChannels);

		for(x = rect.left; x < rect.right; ++x)
		{
			*line_ptr++ = B;
			*line_ptr++ = G;
			*line_ptr++ = R;
		}
	}

} // end of fillRect(const RECT &rect, uchar R, uchar G, uchar B)

// Fill the rect area with the color passed in
void CImage::fillRect(const RECT &rect, int argb)
{
	assert(rect.left >= 0 && rect.right <= mWidth);
	assert(rect.top >= 0 && rect.bottom <= mHeight);
	
	int x; // for the x loops

	uchar *line_ptr; // pointer to a line of pixels
	
	// If we're filling in a 32-bit image
	if(mChannels == 4)
	{
		for(int y = rect.top; y < rect.bottom; ++y)
		{	
			line_ptr = getLinePtr(mHeight - y - 1); // Flip for Win32 coordinates
			line_ptr += (rect.left * mChannels);

			for(x = rect.left; x < rect.right; ++x)
			{
				memcpy(line_ptr, &argb, 4);
				line_ptr += mChannels;
			}

		}
	}
	else // Image is 24-bit
	{
		// Get R, G, and B component
		uchar red = GET_R(argb);
		uchar green = GET_G(argb);
		uchar blue = GET_B(argb);
	
		for(int y = rect.top; y < rect.bottom; ++y)
		{	
			line_ptr = getLinePtr(mHeight - y - 1); // Flip for Win32 coordinates;
			line_ptr += (rect.left * mChannels);

			for(x = rect.left; x < rect.right; ++x)
			{
				*line_ptr++ = blue;
				*line_ptr++ = green;
				*line_ptr++ = red;
			}
		}
	}

} // end of fillRect(const RECT &rect, int argb)

// Blit CImage to "dest" starting at upper-left coordinate (x,y)
void CImage::blit(HDC destDC, int x, int y, DWORD rop) const
{
	BitBlt(destDC,x,y,getWidth(),getHeight(),mHDC,0,0,rop);
}

// Blit CImage to "destDC" using coordinates specified by "rect"
void CImage::blit(HDC destDC, const RECT &rect, DWORD rop) const
{
	BitBlt(destDC,rect.left,rect.top,(rect.right - rect.left),(rect.bottom - rect.top),
		   mHDC,0,0,rop);
}

// Scale blit using bilinear filtering
void CImage::scaleBlit(CImage &dest)
{
	float yDelta = mHeight / (float)dest.getHeight();
	float xDelta = mWidth / (float)dest.getWidth();
	
	// Weights
	float topWeight, botWeight, rtWeight, ltWeight;
	
	for(int y = 0; y < dest.getHeight(); ++y)
	{
		float srcY = ((y + 0.5f) * yDelta) - 0.5f;
		assert(srcY < mHeight);
		
		// Clamp
		if(srcY < 0.0f)
			srcY = 0.0f;
			
		int topLine = (int)srcY;
		int botLine = (topLine == (mHeight - 1)) ? topLine : topLine + 1;
		
		// Set top and bottom weight
		botWeight = srcY - topLine;
		topWeight = 1.0f - botWeight;
	
		uchar *srcLineTop = getLinePtr(topLine);
		uchar *srcLineBot = getLinePtr(botLine);
		uchar *destLine = dest.getLinePtr(y);
	
		for(int x = 0; x < dest.getWidth(); ++x)
		{
			float srcX = ((x + 0.5f) * xDelta) - 0.5f;
			assert(srcX < mWidth);
			
			// Clamp
			if(srcX < 0.0f)
				srcX = 0.0f;
				
			int ltPixel = (int)srcX;
			int rtPixel = (ltPixel == mWidth - 1) ? ltPixel : ltPixel + 1;
			
			// Set right and left weights
			rtWeight = srcX - ltPixel;
			ltWeight = 1.0f - rtWeight;
			
			// Make ltPixel and rtPixel correct indices
			ltPixel *= mChannels;
			rtPixel *= mChannels;
			
			for(int c = 0; (c < mChannels) && (c < dest.getChannels()); ++c)
			{
				destLine[c] = uchar((float)srcLineTop[ltPixel + c] * topWeight * ltWeight +
								    (float)srcLineTop[rtPixel + c] * topWeight * rtWeight +
								    (float)srcLineBot[ltPixel + c] * botWeight * ltWeight +
								    (float)srcLineBot[rtPixel + c] * botWeight * rtWeight);
			}
			
			destLine += dest.getChannels();
		}
	}
}

void CImage::transparentBlit(HDC destDC, const RECT &rect, uchar R, uchar G, uchar B) const
{
	TransparentBlt(destDC,rect.left,rect.top,(rect.right - rect.left),(rect.bottom - rect.top),
					mHDC, 0, 0, mWidth, mHeight, RGB(R, G, B));
}

void CImage::transparentBlit(HDC destDC, const RECT &rect, int color) const
{
	TransparentBlt(destDC,rect.left,rect.top,(rect.right - rect.left),(rect.bottom - rect.top),
					mHDC, 0, 0, mWidth, mHeight, RGB(GET_R(color), GET_G(color), GET_B(color)));
}

// Draws to "dest", starting at upper left corner (destX, destY), using transparency
// color (R, G, B)
void CImage::transparentBlit(CImage &dest, int destX, int destY, uchar R, uchar G, uchar B) const
{
	int x, y;
	int srcWid = getWidth();
	int srcHgt = getHeight();
	int sX = 0;
	int sY = srcHgt - 1;

	Clip(destX, destY, dest.getWidth(), dest.getHeight(), sX, sY, srcWid, srcHgt);

	int srcOffset = sX * getChannels();
	int destOffset = destX * dest.getChannels();
	
	// Flip for Win32 coords
	destY = (dest.getHeight() - destY - 1);

	for(y = sY; y >= 0; --y)
	{
		uchar *srcLine = getLinePtr(y);
		uchar *destLine = dest.getLinePtr(destY);
		
		// Adjust over
		destLine += destOffset;
		srcLine += srcOffset;
	
		for(x = sX; x < srcWid; ++x)
		{
			if(!SameRGB(srcLine[eRed], srcLine[eGreen], srcLine[eBlue], R, G, B))
			{
				destLine[eRed] = srcLine[eRed];
				destLine[eGreen] = srcLine[eGreen];
				destLine[eBlue] = srcLine[eBlue];
			}
		
			srcLine += getChannels();
			destLine += dest.getChannels();
		}
		
		// Can't draw past out destination image
		if(--destY < 0) 
			return;
	}
}

// Draws to "dest", starting at upper left corner (destX, destY), using transparency "color"
void CImage::transparentBlit(CImage &dest, int destX, int destY, int color) const
{
	transparentBlit(dest, destX, destY, GET_R(color), GET_G(color), GET_B(color));
}

// Draws to "dest" starting at upper left corner (destX, destY) using
// the RGB in "color" as the color key and uses the A in "color" for the 
// constant alpha component
void CImage::alphaTransBlit(CImage &dest, int destX, int destY, int color) const
{
	int x, y;
	int srcWid = getWidth();
	int srcHgt = getHeight();
	int sX = 0;
	int sY = srcHgt - 1;

	Clip(destX, destY, dest.getWidth(), dest.getHeight(), sX, sY, srcWid, srcHgt);
	
	int destOffset = destX * dest.getChannels();
	
	uchar R = GET_R(color);
	uchar G = GET_G(color);
	uchar B = GET_B(color);
	uchar alpha = GET_A(color);

	// Flip for Win32 coords
	destY = (dest.getHeight() - destY - 1);

	// Loop backwards to handle Win32 coords
	for(y = sY; y >= 0; --y)
	{
		uchar *srcLine = getLinePtr(y);
		uchar *destLine = dest.getLinePtr(destY);

		// Adjust over
		destLine += destOffset;

		for(x = sX; x < srcWid; ++x)
		{
			if(!SameRGB(srcLine[eRed], srcLine[eGreen], srcLine[eBlue], R, G, B))
			{
				// Set the RGB
				destLine[eRed] += ((srcLine[eRed] - destLine[eRed]) * alpha + kMaxARGB) >> 8;
				destLine[eGreen] += ((srcLine[eGreen] - destLine[eGreen]) * alpha + kMaxARGB) >> 8;
				destLine[eBlue] += ((srcLine[eBlue] - destLine[eBlue]) * alpha + kMaxARGB) >> 8;
			}

			srcLine += getChannels();
			destLine += dest.getChannels();
		}

		// Can't draw past out destination image
		if(--destY < 0) 
			return;
	}
}
	 
// Draws "this" CImage to "dest" starting at upper-left corner (destX, destY) using
// per-pixel alpha in "this" image
void CImage::alphaBlit(CImage &dest, int destX, int destY) const
{
	assert(destX >= 0 && destX < dest.getWidth());
	assert(destY >= 0 && destY < dest.getHeight());
	assert(mChannels == 4); // Make sure "this" image has alpha

	uchar *srcLine;
	uchar *destLine;
	
	// Flip for Win32 coords	
	destY = (dest.getHeight() - destY - 1);

	int x, y;
	
	// Offset, to move over by, for the destination line
	int dest_offset = destX * mChannels;
	int dest_width = mWidth; // Assume we can draw entire CImage
	
	// If source CImage is wider than destination, set dest_width to clip source
	if(dest_width > (destX + dest.getWidth()))
		dest_width -= (dest_width - (destX + dest.getWidth())); // Subtract off amount over
		
	assert(dest_width > 0);
	
	// Walk through image backwards for Win32 coords	
	for(y = mHeight - 1; y >= 0; --y)
	{
		srcLine = getLinePtr(y);
		destLine = dest.getLinePtr(destY);

		// Move to starting position in destination line
		destLine += dest_offset;

		for(x = 0; x < dest_width; ++x)
		{
			uchar alpha = srcLine[eAlpha];

			// Set the RGB
			destLine[eRed] += ((srcLine[eRed] - destLine[eRed]) * alpha + kMaxARGB) >> 8;
			destLine[eGreen] += ((srcLine[eGreen] - destLine[eGreen]) * alpha + kMaxARGB) >> 8;
			destLine[eBlue] += ((srcLine[eBlue] - destLine[eBlue]) * alpha + kMaxARGB) >> 8;

			srcLine += mChannels;
			destLine += dest.getChannels();
		}
		
		// Walk backwards for Win32 image, make sure we don't draw past our CImage
		if(--destY < 0)
			return;
	}	
	
}

// Draws "this" CImage to "dest" starting at upper-left corner (destX, destY) using
// constant per-pixel "alpha" 
void CImage::constAlphaBlit(CImage &dest, int destX, int destY, uchar alpha) const
{
	assert(destX >= 0 && destX < dest.getWidth());
	assert(destY >= 0 && destY < dest.getHeight());

	uchar *srcLine;
	uchar *destLine;

	// Flip for Win32 coords	
	destY = (dest.getHeight() - destY - 1);

	int x, y;

	// Offset, to move over by, for the destination line
	int dest_offset = destX * mChannels;
	int dest_width = mWidth; // Assume we can draw entire CImage

	// If source CImage is wider than destination, set dest_width to clip source
	if(dest_width > (destX + dest.getWidth()))
		dest_width -= (dest_width - (destX + dest.getWidth())); // Subtract off amount over

	assert(dest_width > 0);

	// Walk through image backwards for Win32 coords	
	for(y = mHeight - 1; y >= 0; --y)
	{
		srcLine = getLinePtr(y);
		destLine = dest.getLinePtr(destY);

		// Move to starting position in destination line
		destLine += dest_offset;

		for(x = 0; x < dest_width; ++x)
		{
			// Set the RGB
			destLine[eRed] += ((srcLine[eRed] - destLine[eRed]) * alpha + kMaxARGB) >> 8;
			destLine[eGreen] += ((srcLine[eGreen] - destLine[eGreen]) * alpha + kMaxARGB) >> 8;
			destLine[eBlue] += ((srcLine[eBlue] - destLine[eBlue]) * alpha + kMaxARGB) >> 8;

			srcLine += mChannels;
			destLine += dest.getChannels();
		}

		// Walk backwards for Win32 image, make sure we don't draw past our CImage
		if(--destY < 0)
			return;
	}	

}

// Fills the CImage with "src" by the dimensions specified in "srcRect"
// starting at upper-left corner (destX, destY) using transparency color (R,G,B)
void CImage::transparentFill(CImage &src, const RECT &srcRect, int destX, int destY,
							 uchar R, uchar G, uchar B)
{
	assert(destX >= 0 && destX < mWidth);
	assert(destY >= 0 && destY < mHeight);
	assert(srcRect.left >= 0 && srcRect.right <= src.getWidth());
	assert(srcRect.top >= 0 && srcRect.bottom <= src.getHeight());
	assert(destX + (srcRect.right - srcRect.left) <= mWidth);
	
	// Flip for Win32 coords
	destY = (mHeight - destY - 1);

	uchar *srcLine;
	uchar *destLine;

	int x, y;
	
	int src_offset = srcRect.left * src.getChannels(); // Offset into source line
	int dest_offset = destX * mChannels; // Offset into dest line

	for(y = srcRect.top; y < srcRect.bottom; ++y)
	{
		srcLine = src.getLinePtr(src.getHeight() - y - 1); // Flip for Win32 coords
		destLine = getLinePtr(destY);

		// Move to correct offset for source and dest lines
		srcLine += src_offset;
		destLine += dest_offset;

		for(x = srcRect.left; x < srcRect.right; ++x)
		{
			// If it's not the transparency color, copy it over to the destination
			if(SameRGB(R,G,B,srcLine[eRed],srcLine[eGreen],srcLine[eBlue]) == false)
			{
				destLine[eRed] = srcLine[eRed];
				destLine[eGreen] = srcLine[eGreen];
				destLine[eBlue] = srcLine[eBlue];
			}

			srcLine += src.getChannels();
			destLine += mChannels;
		}

		// Walk backwards for Win32 coords, don't draw past our CImage
		if(--destY < 0)
			return;
	}

} // end of transFill(CImage &src, const RECT &srcRect, int destX, int destY,
  //				  uchar R, uchar G, uchar B)
 
// Fills CImage with every other pixel from "src" using the "srcRect" as the dimensions
// in the source image.  Draws to the CImage starting at upper left corner 
// (destX, destY)
void CImage::opaqueFill(CImage &src, const RECT &srcRect, int destX, int destY)
{
	assert(destX >= 0 && destX < mWidth);
	assert(destY >= 0 && destY < mHeight);
	assert(srcRect.left >= 0 && srcRect.right <= src.getWidth());
	assert(srcRect.top >= 0 && srcRect.bottom <= src.getHeight());
	assert(destX + (srcRect.right - srcRect.left) < mWidth);
	
	uchar *destLine;
	uchar *srcLine;
	
	// Flip for Win32 coords
	destY = (mHeight - destY - 1);
	
	// Toggles between using the 0th column or 1st column in the 
	// destination image, per scan line
	int toggle = 0;
	
	int x, y;
	
	int schan_doubled = src.getChannels() * 2;
	int dchan_doubled = getChannels() * 2;
	
	int src_offset = srcRect.left * src.getChannels();
	int dest_offset = destX * mChannels;
	
	for(y = srcRect.top; y < srcRect.bottom; ++y)
	{
		srcLine = src.getLinePtr(src.getHeight() - y - 1);
		destLine = getLinePtr(destY);
		
		srcLine += src_offset; // Move over in source
		destLine += dest_offset; // Move over in destination
		
		if(toggle)
			destLine += mChannels;
			
		toggle ^= 1;	
		
		for(x = srcRect.left; x < srcRect.right; x += 2)
		{
			destLine[eRed] = srcLine[eRed];
			destLine[eGreen] = srcLine[eGreen];
			destLine[eBlue] = srcLine[eBlue];
			
			srcLine += schan_doubled;
			destLine += dchan_doubled;
		}
		
		// Walk backwards for Win32 coords, don't draw past our CImage
		if(--destY < 0)
			return;
	}
}

// Fills the CImage with "src" by the dimensions specified in "srcRect"
// starting at upper-left corner (destX, destY) using per pixel alpha in "src"
// Does not alter alpha in source or destination
// **NOTE** "src" must be a 32-bit CImage or this method fails
void CImage::alphaFill(CImage &src, const RECT &srcRect, int destX, int destY)
{
	assert(destX >= 0 && destX < mWidth);
	assert(destY >= 0 && destY < mHeight);
	assert(srcRect.left >= 0 && srcRect.right <= src.getWidth());
	assert(srcRect.top >= 0 && srcRect.bottom <= src.getHeight());
	assert(src.getChannels() == 4); // Make sure image has alpha

	uchar *srcLine;
	uchar *destLine;

	int x, y;
	
	// Flip for Win32 coords
	destY = (mHeight - destY - 1);
	
	// Offset, to move over by, for the source and destination lines
	int src_offset = srcRect.left * src.getChannels();
	int dest_offset = destX * mChannels;

	for(y = srcRect.top; y < srcRect.bottom; ++y)
	{
		srcLine = src.getLinePtr(src.getHeight() - y - 1); // Flip for Win32 coords
		destLine = getLinePtr(destY);

		// Move to starting position in source and destination line
		srcLine += src_offset;
		destLine += dest_offset;

		for(x = srcRect.left; x < srcRect.right; ++x)
		{
			uchar alpha = srcLine[eAlpha];

			// Set the RGB
			destLine[eRed] += ((srcLine[eRed] - destLine[eRed]) * alpha + kMaxARGB) >> 8;
			destLine[eGreen] += ((srcLine[eGreen] - destLine[eGreen]) * alpha + kMaxARGB) >> 8;
			destLine[eBlue] += ((srcLine[eBlue] - destLine[eBlue]) * alpha + kMaxARGB) >> 8;

			srcLine += src.getChannels();
			destLine += mChannels;
		}

		// Walk backwards for Win32 coords, don't draw past our CImage
		if(--destY < 0)
			return;
	}	
}

void CImage::blendDown()
{
	int x = 0; // for the x loops
	int y = mHeight - 1; // for the y loops
	int c = 0; // for the channel loops

	uchar *prev_line_ptr, *line_ptr; // The line pointers

	line_ptr = getLinePtr(y);

	// Divide bottom line in half
	for(x ^= x; x < mWidth; ++x)
	{
		for(c ^= c; c < mChannels; ++c)
			*line_ptr++ >>= 1;
	}

	// Average all other lines, two at a time, together
	for(--y; y >= 0; --y)
	{
		line_ptr = getLinePtr(y);
		prev_line_ptr = line_ptr + mStride;

		for(x ^= x; x < mWidth; ++x)
		{
			for(c ^= c; c < mChannels; ++c, ++line_ptr, ++prev_line_ptr)
				*line_ptr = (*prev_line_ptr + *line_ptr) >> 1;
		}
	}

} // end of blendDown()

// Blend the CImage from bottom to top
void CImage::blendUp()
{
	int x; // for the x loops
	int y; // for the y loops
	int c; // for the channel loops
		
	uchar *prev_line_ptr, *line_ptr; // The line pointers

	line_ptr = getLinePtr(0); // Get top line

	for(x ^= x; x < mWidth; ++x)
	{
		for(c ^= c; c < mChannels; ++c)
			*line_ptr++ >>= 1;
	}

	for(y ^= y; y < mHeight; ++y)
	{
		line_ptr = getLinePtr(y);
		prev_line_ptr = line_ptr - mStride;

		for(x ^= x; x < mWidth; ++x)
		{
			for(c ^= c; c < mChannels; ++c, ++line_ptr, ++prev_line_ptr)
				*line_ptr = (*prev_line_ptr + *line_ptr) >> 1;
		}

	}

} // end of blendUp()

void CImage::flip(EFlipType type)
{
	if(type == eHorizontal)
	{
		uchar *linePtr = NULL;
		int xl;
		int xr;

		for(int y = 0; y < mHeight; ++y)
		{
			xl = 0;
			xr = mWidth - 1;

			for(; xl < xr; xl++, xr--)
			{
				int p1 = getPixel(xl, y);
				int p2 = getPixel(xr, y);

				setPixel(xr, y, p1);
				setPixel(xl, y, p2);
			}
		}
	}
}

void CImage::decrement(int amount)
{
	int x; // for the x loops
	int c; // for channel loops
	
	uchar *line_ptr;

	for(int y = 0; y < mHeight; ++y)
	{
		line_ptr = getLinePtr(y);
		
		for(x ^= x; x < mWidth; ++x)
		{
			for(c ^= c; c < mChannels; ++c, ++line_ptr)
				*line_ptr -= (*line_ptr >= amount) ? amount : 0;
		}

	}

} // end of decrementDB()

void CImage::increment(int amount)
{
	int x; // for the x loops
	int c; // for channel loops

	uchar *line_ptr;

	for(int y = 0; y < mHeight; ++y)
	{
		line_ptr = getLinePtr(y);
		
		for(x ^= x; x < mWidth; ++x)
		{
			for(c ^= c; c < mChannels; ++c, ++line_ptr)
				*line_ptr += ((*line_ptr + amount) <= kMaxARGB) ? amount : 0;
		}
	}

} // end of incrementDB(int amount)

// Release the memory
void CImage::freeMem() 
{
	if(!mOldBmp)
		return;
	
	if(!SelectObject(mHDC,mOldBmp)) // get the old stuff back
		return;
	
	if(!DeleteObject(mBmp)) // delete the new stuff
		return;

	if(!DeleteDC(mHDC)) // release the device context
		return;

	// Zero out all data associated with CImage
	mOldBmp = mBmp = NULL;
	mHDC = NULL;
	mWidth = mHeight = mChannels = mStride = 0;
	mPixels = NULL;
		return;
}

// Returns a RECT that defines CImage's dimensions with the upper-left corner being (0,0)
RECT CImage::getRect() const
{
	RECT rect;

	rect.left = rect.top = 0;
	rect.right = mWidth;
	rect.bottom = mHeight;
	return rect;
}

// Returns the CColor at (x,y)
int CImage::getPixel(int x, int y) const
{
	if(x < 0 || x >= getWidth()) // Bounds check 'x'
		return 0;
	else if(y < 0 || y >= getHeight()) // Bounds check 'y'
		return 0;

	uchar *line = getLinePtr(mHeight - y - 1); // Flip for Win32 coordinates
	line += x * getChannels();

	if(getChannels() == 3)
		return ARGB(255, line[eRed], line[eGreen], line[eBlue]);
	else
		return ARGB(line[eAlpha], line[eRed], line[eGreen], line[eBlue]);
}

// Deconstructor
CImage::~CImage() { freeMem(); }	
