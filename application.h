#ifndef APPLICATION_H
#define APPLICATION_H

#include "GT_Color.h"
#include "GT_Window.h"
#include "GT_Input.h"
#include "GT_Font.h"
#include "resource.h"
#include <vector>

const int kMaxChars = 256;
const int kGTFontVersionNum = 2;  // **NOTE** If you change this, make sure to update kAboutText
const int kCustomColorNum = 16;
const int kClearImgColor = ARGB(128,0,0,0);

const int k1PixelKernelSize = 3 * 3;
const int k1PixKernelX[k1PixelKernelSize] = { -1, 0, 1, 
											  -1, 0, 1, 
											  -1, 0, 1,  };
											 
const int k1PixKernelY[k1PixelKernelSize] = { -1, -1, -1, 
											  0, 0, 0, 
											  1, 1, 1,  };

const int k2PixelKernelSize = 5 * 5;
const int k2PixKernelX[k2PixelKernelSize] = { 0, -1, 0, 1, 0,
											  -2, -1, 0, 1, 2,
											  -2, -1, 0, 1, 2
											  -2, -1, 0, 1, 2, 
											  0, -1, 0, 1, 0 };
											 
const int k2PixKernelY[k2PixelKernelSize] = { 0, -2, -2, -2, 0,
											 -1, -1, -1, -1, -1,
											 0, 0, 0, 0, 0,
											 1, 1, 1, 1, 1, 
											 0, 2, 2, 2, 0 };
										   
const int k4PixKernelSize = 9 * 9;
const int k4PixKernelX[k4PixKernelSize] = { 0, 0, -2, -1, 0, 1, 2, 0, 0,
											0, -3, -2, -1, 0, 1, 2, 3, 0,
											-4, -3, -2, -1, 0, 1, 2, 3, 4,
											-4, -3, -2, -1, 0, 1, 2, 3, 4,
											-4, -3, -2, -1, 0, 1, 2, 3, 4,
											-4, -3, -2, -1, 0, 1, 2, 3, 4,
											-4, -3, -2, -1, 0, 1, 2, 3, 4,
											0, -3, -2, -1, 0, 1, 2, 3, 0,
											0, 0, -2, -1, 0, 1, 2, 0, 0 };
											
const int k4PixKernelY[k4PixKernelSize] = { 0, 0, -4, -4, -4, -4, -4, 0, 0,
											0, -3, -3, -3, -3, -3, -3, -3, 0,
											-2, -2, -2, -2, -2, -2, -2, -2, -2,
											-1, -1, -1, -1, -1, -1, -1, -1, -1,
											0, 0, 0, 0, 0, 0, 0, 0, 0,
											1, 1, 1, 1, 1, 1, 1, 1, 1,
											2, 2, 2, 2, 2, 2, 2, 2, 2,
											0, 3, 3, 3, 3, 3, 3, 3, 0,
											0, 0, 4, 4, 4, 4, 4, 0, 0 };

class CAppFont : public CFont
{
	public:
	
		CAppFont() { mOrigFont = NULL; }
		
		bool init()
		{
			mImg = new CImage;
			
			if (mImg != NULL)
			{
				return mImg->init(128,128,4);
			}

			return false;
		}

		bool setFont(HFONT f)
		{		
			if(f)
			{
				if(mOrigFont == NULL)
				{
					mOrigFont = (HFONT)SelectObject(mImg->getDC(), f);

					if(!mOrigFont)
						return false;
				}
				else
				{
					SelectObject(mImg->getDC(), f);
				}
			}
			
			// Set text color
			if(SetTextColor(mImg->getDC(), RGB(255, 255, 255)) == CLR_INVALID)
				return false;

			SetBkColor(mImg->getDC(), RGB(255, 255, 255));

			// Set the background mode to transparent
			if(!SetBkMode(mImg->getDC(), TRANSPARENT))
				return false;

			return true;
		}
		
		void renderOutline(int outlineSize, int outlineARGB, int bgARGB)
		{
			if(!mImg)
				return;
				
			assert(outlineARGB != bgARGB);
		
			int kernelSize = 0;
			const int *kernelX = NULL;
			const int *kernelY = NULL;

			switch(outlineSize)
			{
				case 1:
				{
					kernelSize = k1PixelKernelSize; 
					kernelX = k1PixKernelX;
					kernelY = k1PixKernelY;
				}
				break;

				case 2:
				{
					kernelSize = k2PixelKernelSize;
					kernelX = k2PixKernelX;
					kernelY = k2PixKernelY;
				}
				break;

				case 4:
				{
					kernelSize = k4PixKernelSize;
					kernelX = k4PixKernelX;
					kernelY = k4PixKernelY;
				}
				break;
			}

			for(int y = 0; y < mImg->getHeight(); ++y)
			{
				for(int x = 0; x < mImg->getWidth(); ++x)
				{
					int pixel = mImg->getPixel(x,y);
						
					if(pixel != bgARGB)
					{
						for(int k = 0; k < kernelSize; ++k)
						{
							int xxx = x + kernelX[k];
							int yyy = y + kernelY[k];

							int pix = mImg->getPixel(xxx,yyy);

							if (pix == 0)
								continue; // Out of bounds

							if ((GET_R(pix) == 255) && (GET_G(pix) == 255) && (GET_B(pix) == 255))
								continue; // Skip true letter pixels

							if (pix == outlineARGB)
								continue; // Skip outline color
							
							mPoints.push_back(PT(xxx, yyy));
						}
					}
				}
			}
			
			while(mPoints.empty() == false)
			{
				PT p = mPoints.back();
				
				mImg->setPixel(p.x, p.y, outlineARGB);
				mPoints.pop_back();
			}		
		}
		
		virtual ~CAppFont()
		{
			if(mOrigFont && mImg) // Put back original
			{
				SelectObject(mImg->getDC(), mOrigFont);
			}
		}

	private:
	
		class PT
		{
			public:
			
				PT(int X, int Y) : x(X), y(Y) {}
				
				int x;
				int y;
		};

		HFONT mOrigFont;
		std::vector<PT> mPoints;

};

class CApp : public CWindow
{
	public:
	
		CApp();
		
		virtual bool init(HINSTANCE hinstance, const SWinInfo &winInfo);
		virtual LRESULT wndProc(UINT msg, WPARAM wparam, LPARAM lparam);
		
		void update();	
		void draw();
		
		// Upper left corner of where to start drawing from
		void drawDebugBoxes(int x, int y); 
		
		virtual ~CApp();
		
	protected:
	
		//SFont mCharUVSet[kMaxChars];
		CHOOSEFONT mChooseFont;
		LOGFONT mLogFont;
		CAppFont mFont;
		
		COLORREF mCustomColors[kCustomColorNum]; // Used by color picker
		CColor mColorKey;
		CColor mOutline;
			
		HICON mIcon;
		HMENU mMenu;
		HFONT mCurWin32Font;
		
		int mSizeFlag;
		int mAliasFlag;
		int mCharUVSetFlag;
		int mCaseFlag;
		int mColorKeyFlag;
		int mOutlineFlag;
		int mOutlineWidFlag;
		
		int mOutlineSize; // Number of pixels to outline by
		bool mRedraw; // Set to true when we need to redraw, false otherwise
		
		wchar_t *mCustomSet; // A custom set of characters to be rendered
		int mCustomSetCharCount; // Number of characters in the custom set

		wchar_t mASCIISet[kMaxChars];

		void zeroVars()
		{
			memset(&mChooseFont, 0, sizeof(CHOOSEFONT));
			memset(&mLogFont, 0, sizeof(LOGFONT));
			memset(mCustomColors, 0, sizeof(COLORREF) * kCustomColorNum);
			
			mColorKey.set((uchar)0,0,0,0);
			mOutline.set((uchar)0,0,0,0);
			
			mIcon = NULL;
			mMenu = NULL;
			mCurWin32Font = NULL;
			mSizeFlag =  mAliasFlag =  mCharUVSetFlag = mCaseFlag = mColorKeyFlag = mOutlineFlag = 0;
			mOutlineSize = 0;
			mRedraw = true;
			
			mCustomSet = NULL;
			mCustomSetCharCount = 0;

			for(int i = 0; i < kMaxChars; ++i)
			{
				mASCIISet[i] = (wchar_t)i;
			}
		}
		
		bool getSaveFileName(OPENFILENAME *ofn, const char *filtetText, 
							 const char *filterExt, char *fileName);
		bool getLoadFileName(OPENFILENAME *ofn, const char *filtetText, 
							 const char *filterExt, char *fileName);
		bool saveGTFT();
		bool loadCustomCharSet();
		
		bool chooseFont();
		bool setFont(LOGFONT *f);
		bool setupTexture();
		bool antiAliasTexture();
		bool colorKeyTexture();
		bool renderFont();
		bool renderLetters(); // Helper function to render each letter of chosen set
		bool testLetter(int c); // Returns true if 'c' is in range of chosen ASCII set, false otherwise
		bool testCase(int c); // Return true if 'c' is in range of chosen case set
		
		void setMenuItemSize(int which); // Checks "which" size menu item
		void setMenuItemAlias(int which); // Checks "which" anti-alias menu item
		void setMenuItemChar(int which); // Check "which" character menu item
		void setMenuItemCase(int which); // Check "which" case menu item
		void setMenuItemColorKey(int which); // Check "which" color key menu item
		void setMenuItemOutline(int which); // Check "which" outline menu item
		void setMenuItemOutlineWidth(int which); // Check "which" outline width menu item
		
		bool loadResources(HINSTANCE hinst); // Loads menu and icon
		void calcLinearImgSize(int &wid, int &hgt);
		void pickColor(int flag); // Uses color picker to a color determined by "flag"
		void displayError(const char *err); // Display error messages
};

extern CApp *gApp;

#endif


/*
	int last = outlineARGB;
			
			// Check horizontal pixels						
			for(int y = 0; y < mImg->getHeight(); ++y)
			{				
				for(int x = 0; x < mImg->getWidth(); ++x)
				{
					int pixel = mImg->getPixel(x,y);
					
					// Check behind
					if((pixel != bgARGB) && (pixel != outlineARGB) && (last == bgARGB))
					{
						int xxx = x - outlineSize;
						int count = outlineSize;
						
						if(xxx < 0)
						{
							count += xxx;
							xxx = 0;
						}
						
						for(; count; xxx++, count--)
						{
							if(mImg->getPixel(xxx, y) == bgARGB)
								mImg->setPixel(xxx, y, outlineARGB);
						}						
					}
					// Check in front
					else if((pixel == bgARGB) && (last != bgARGB) && (last != outlineARGB))
					{
						int xxx = x;
						int count = outlineSize;
						
						if(xxx + count >= mImg->getWidth())
							count = (xxx + count) - mImg->getWidth();
							
						for(; count; xxx++, count--)
						{
							if(mImg->getPixel(xxx, y) == bgARGB)
								mImg->setPixel(xxx, y, outlineARGB);
						}
					}
					
					last = pixel;
				}
			}
			
			last = outlineARGB;
			
			// Check vertical pixels						
			for(int x = 0; x < mImg->getWidth(); ++x)
			{				
				for(int y = 0; y < mImg->getHeight(); ++y)
				{
					int pixel = mImg->getPixel(x,y);
					
					// Check behind
					if((pixel != bgARGB) && (pixel != outlineARGB) && (last == bgARGB))
					{
						int yyy = y - outlineSize;
						int count = outlineSize;
						
						if(yyy < 0)
						{
							count += yyy;
							yyy = 0;
						}
						
						for(; count; yyy++, count--)
						{
							if(mImg->getPixel(x, yyy) == bgARGB)
								mImg->setPixel(x, yyy, outlineARGB);
						}						
					}
					// Check in front
					else if((pixel == bgARGB) && (last != bgARGB) && (last != outlineARGB))
					{
						int yyy = y;
						int count = outlineSize;
						
						if(yyy + count >= mImg->getWidth())
							count = (yyy + count) - mImg->getWidth();
							
						for(; count; yyy++, count--)
						{
							if(mImg->getPixel(x, yyy) == bgARGB)
								mImg->setPixel(x, yyy, outlineARGB);
						}
					}
					
					last = pixel;
				}
			}
*/