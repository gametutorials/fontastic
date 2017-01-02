#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include "GT_Util.h"
#include "GT_Font.h"

// Loads both the .bmp and .gtft file with name "font"
bool CFont::loadFont(const char *font)
{
	char temp[kMaxStrLen] = {0};

	try
	{
		if(strstr(font, ".bmp") || strstr(font, ".gtft"))
			throw "File name had an extension appended, should not contain an extension";
			
		if(_snprintf(temp, kMaxStrLen, "%s.gtft.bmp", font) <= 0)
			throw "File name longer than max allowed";
			
		if(!loadFontImg(temp))
			throw "Couldn't load font .bmp file";
			
		// Append new name
		if(_snprintf(temp, kMaxStrLen, "%s.gtft", font) <= 0)
			throw "File name longer than max allowed";
			
		if(!loadFontInfo(temp))
			throw "Couldn't load font .gtft file";		
	}
	catch(char *str)
	{
		OutputError(str);
			return false;
	}	

	return true;
}

// Loads only the font .bmp
bool CFont::loadFontImg(const char *imgName)
{
	// Error check
	if(!imgName)
		return false;
		
	if(!mImg)
	{
		mImg = new CImage;
		
		// Error check
		if(!mImg)
			return false;
	}
	
	return mImg->loadBMP(imgName);
}

/*
	Spec

	1 (int) == Version Number
	1 (int) == Number of glyphs
	for each glyph
	{
		1 (int) == glyph
		4 (float) == { ulx, uly, brx, bry }
	}
*/

// Loads only the font .gtft
bool CFont::loadFontInfo(const char *infoName)
{
	// Error check
	if(!infoName)
		return false;
		
	FILE *file = NULL;
	
	int versionNum = 0;
	int glyphCount = 0;
	bool status = false; // Assume failure
	
	file = fopen(infoName, "rb");
	
	// Error check
	if(!file)
		return false;

	if(fread(&versionNum, sizeof(int), 1, file) != 1)
		goto CLEANUP;
		
	if(versionNum != kGTFontVersionNumber)
		 goto CLEANUP;

	if(fread(&glyphCount, sizeof(int), 1, file) != 1)
		goto CLEANUP;
		
	if(glyphCount <= 0)
		 goto CLEANUP;

	// Blow away current font info
	clearGlphys();

	for(int i = 0; i < glyphCount; ++i)
	{
		int glyph;
		float uvs[4];

		if(fread(&glyph, sizeof(int), 1, file) != 1)
			goto CLEANUP;

		if(fread(uvs, sizeof(float) * 4, 1, file) != 1)
			goto CLEANUP;

		CAtlas atlas(uvs[0], uvs[1], uvs[2], uvs[3]);

		addGlyph(glyph, atlas);		
	}
	
	status = true; // If we made it here, we're successful
	
CLEANUP:
	fclose(file);
		return status; 
}

#ifdef GT_FONT_TOOL

// Save the font image 
bool CFont::saveFontImg(const char *imgName, const SSaveFontInfo &info)
{
	assert(imgName != NULL);
	assert(mImg != NULL);

	// Convert to 32-bit image and save
	CImage saveImg;

	saveImg.init(mImg->getWidth(), mImg->getHeight(), 4);
	mImg->blit(saveImg.getDC(), saveImg.getRect());

	// Convert any non (255, 255, 255) color (expect for pure outline and colorkey) to
	// (255,255,255,alpha) with alpha conversion
	for(int y = 0; y < saveImg.getHeight(); ++y)
	{
		for(int x = 0; x < saveImg.getWidth(); ++x)
		{
			int pixel = saveImg.getPixel(x,y);

			if (pixel == info.outline.getARGB())
				continue;

			if (pixel == info.colorKey.getARGB())
				continue;

			if (pixel == ARGB(255, 255, 255, 255))
				continue;

			int a = 255;

			//  If no outline, convert anti-aliased pixels into alpha
			//  Otherwise, we have an outline so fill in anti-aliased pixels with pure white
			if (info.isOutlined == false)
			{
				int a = GET_R(pixel) + GET_G(pixel) + GET_B(pixel);
				a /= 3;
			}

			pixel = ARGB(a, 255, 255, 255);
			saveImg.setPixel(x,y,pixel);				
		}
	}

	//return saveImg.saveAsBMP(imgName);
	return saveImg.saveAsTGA(imgName);
}

// *** See spec above loadFontInfo ***
bool CFont::saveFontInfo(const char *infoName)
{
	assert(infoName != NULL);
	assert(mImg != NULL);

	bool status = true; // Assume success
	FILE *file = NULL;

	file = fopen(infoName, "wb");

	// Error check
	if(!file)
		return false;

	try
	{
		if(fwrite(&kGTFontVersionNumber, sizeof(int), 1, file) != 1)
			throw "Error writing out version #";

		int glyphCount = getGlyphCount();

		if(fwrite(&glyphCount, sizeof(int), 1, file) != 1)
			throw "Error writing out glyph count";
			
		for (Iter iii = mGlyphMap.begin(); iii != mGlyphMap.end(); iii++)
		{
			int glyph = iii->first;

			if(fwrite(&glyph, sizeof(int), 1, file) != 1)
				throw "Error writing out glyph";

			float uvs[4] = {0};

			// Flip 'V' for UV coordinates
			uvs[0] = iii->second.ulx / mImg->getWidth();
			uvs[1] = (mImg->getHeight() - iii->second.bry) / mImg->getHeight();
			uvs[2] = iii->second.brx / mImg->getWidth();
			uvs[3] = (mImg->getHeight() - iii->second.uly) / mImg->getHeight();

			if(fwrite(uvs, sizeof(float) * 4, 1, file) != 1)
				throw "Error writing out uvs";

		}
	}
	catch(char *str)
	{
		assert(!str);
		OutputError(str);
		status = false;
	}

	// Close the file
	fclose(file);
	return status;
}

void CFont::drawDebugLines()
{
	assert(mImg != NULL);

	bool toggle = true;

	for (ConstIter iii = mGlyphMap.begin(); iii != mGlyphMap.end(); iii++)
	{
		const CAtlas *a = &(iii->second);

		if(iii->second.isEmpty())
			continue;

		int startX = a->ULX();
		int startY = a->ULY();
		int endX = a->BRX();
		int endY = a->BRY();
		int color = ARGB(255, 255, 0, 0);

		toggle = !toggle;

		if(toggle)
			color = ARGB(255, 0, 255, 0);

		for(int x = startX; x <= endX; ++x)
		{
			mImg->setPixel(x, startY, color);
			mImg->setPixel(x, endY, color);
		}

		for(int y = startY; y <= endY; ++y)
		{
			mImg->setPixel(startX, y, color);
			mImg->setPixel(endX, y, color);
		}
	}			
}

#endif

CFont::~CFont()
{
	SAFE_DELETE(mImg);
}