#ifndef GT_FONT_H
#define GT_FONT_H

#include <map>
#include "GT_Image.h"
#include "GT_Color.h"

// Comment this in when compiling for font tool, otherwise you 
// can comment out
#define GT_FONT_TOOL

const int kGTFontVersionNumber = 3;
const int kMaxFontChars = 256;

class CAtlas
{
public:

	CAtlas()
	{
		ulx = uly = brx = bry = 0.0f;
	}

	CAtlas(float _ulx, float _uly, float _brx, float _bry)
	{
		ulx = _ulx;
		uly = _uly;
		brx = _brx;
		bry = _bry;
	}

	int ULX() const { return F2I(ulx); }
	int ULY() const { return F2I(uly); }
	int BRX() const { return F2I(brx); }
	int BRY() const { return F2I(bry); }

	bool isEmpty() const { return (ulx == 0.0f) && (uly == 0.0f) && (brx == 0.0f) && (bry == 0.0f); }

	// Public data
	float ulx, uly;
	float brx, bry;
};




class CFont
{
	public:

		typedef std::map<int,CAtlas>::iterator Iter;
		typedef std::map<int,CAtlas>::const_iterator ConstIter;
		typedef std::pair<int, CAtlas> Pair;
	
		CFont() : mImg(NULL) {}
			
		// Loads both the .bmp and .gtft file with name "font"
		// "font" should not have either extension appended to it
		// Returns true on success, false otherwise
		virtual bool loadFont(const char *font);
		
		// Loads only the font .bmp
		virtual bool loadFontImg(const char *imgName);
		
		// Loads only the font .gtft
		virtual bool loadFontInfo(const char *infoName);
		
		#ifdef GT_FONT_TOOL
		
			struct SSaveFontInfo
			{
				CColor outline;
				CColor colorKey;
				bool isOutlined;
			};

			virtual bool saveFontImg(const char *imgName, const SSaveFontInfo &info); // Save image
			virtual bool saveFontInfo(const char *infoName); // Save font information

			virtual void drawDebugLines(); // Draw alternating red/green boxes around each character
		
		#endif

		void clearGlphys()
		{
			mGlyphMap.clear();
		}

		void addGlyph(int c, const CAtlas &atlas)
		{
			Iter i = mGlyphMap.find(c);

			if (i == mGlyphMap.end())
			{
				mGlyphMap.insert(Pair(c, atlas));
			}
			else
			{
				i->second = atlas;
			}
		}
		
		// Data Access ***
		
			const CAtlas& getInfo(int c)
			{
				Iter i = mGlyphMap.find(c);

				if (i != mGlyphMap.end())
				{
					return i->second;
				}
				else
				{
					return mDefaultAtlas;
				}
			}

			const CImage* getFontImg() const { return mImg; }
			CImage* getFontImg() { return mImg; }

			int getGlyphCount() { return mGlyphMap.size(); }
		
		// *** End Data Access
		
		virtual ~CFont();
	
	protected:

		CAtlas mDefaultAtlas;
		std::map<int, CAtlas> mGlyphMap;
	
		CImage *mImg;
};

#endif