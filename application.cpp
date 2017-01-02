#define _CRT_SECURE_NO_DEPRECATE
#include <ctype.h>
#include <stdio.h>
#include "application.h"

char gBuffer[kMaxChars] = {0}; // Character buffer to fill
int gBufferLen = 0; // Length of buffer

const char kAboutText[] = "Fontastic Version 2\n"
						  "Fontastic is a utility program for creating bitmap fonts to be used in 2D/3D applications.\n"
						  "© 2005-2007 GameTutorials, LLC";
						  
const char kErrMsgCustom[] = "The custom .txt file should be saved as a UNICODE file by NotePad.  It should contain "
							 "only valid Unicode characters, one after other, without any delimiters or line breaks.  "
							 "At max the file can contain 256 custom unicode characters.";

const float kSpacing = 1.0f; // Amount of space in pixels between letters

// True if c equals any of these:  ! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ `
inline bool IsPunctuation(char c)
{
	return ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96));
}

// Constructor
CApp::CApp()
{
	zeroVars();
	mChooseFont.lpLogFont = &mLogFont;		
	// mTexture -- CTexture constructor handles
}

bool CApp::init(HINSTANCE hinst, const SWinInfo &winInfo)
{
	// Guard against multiple initialization
	if(mHwnd)
		return false;
		
	// Init font
	if(!mFont.init())
		return false;
	
	// Load resources	
	if(!loadResources(hinst))
		return false;

	// Init the main window object
	if(!CWindow::init(hinst, winInfo))
		return false;

	// Set the icons
	if(!setIcons(mIcon, mIcon))
		return false;
		
	// Set the menu
	if(!setMenu(mMenu))
		return false;
		
	// Set
		
	// Set menu flags	
	setMenuItemSize(ID_SIZE_128X128);
	setMenuItemAlias(ID_ANTI_FALSE);
	setMenuItemChar(ID_CHARATERS_ALPHAS);
	setMenuItemCase(ID_CASE_ALL);
	setMenuItemColorKey(ID_COLORKEY_FALSE);
	setMenuItemOutline(ID_OUTLINE_FALSE);
	setMenuItemOutlineWidth(ID_OUTLINEWIDTH_1);
	
	setupTexture();
	mFont.setFont(NULL);
	renderFont();
		return true;
}

LRESULT CApp::wndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_ACTIVATE:
			mRedraw = true;
				break;
	
		case WM_MOVE:
			mRedraw = true;
				break;
				
		case WM_PAINT:
			mRedraw = true;
			draw();
				break;
				
		case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
				case ID_FILE_EXIT:
					SendMessage(this->getHWND(), WM_CLOSE, 0, 0);
						return 0;
						
				case ID_FILE_SAVE40029:
					if(saveGTFT() == false)
						displayError("Failure to save file");
					
					return 0;
						
				case ID_FONT_SELECT:
					chooseFont();
					mRedraw = true;
						return 0;

				case ID_COLORKEY_TRUE:
				case ID_COLORKEY_FALSE:
					setMenuItemColorKey((int)wparam);
					pickColor(ID_FONT_COLORKEY);
					renderFont();
					mRedraw = true;
						return 0;
						
				case ID_OUTLINE_TRUE:
				case ID_OUTLINE_FALSE:
					setMenuItemOutline((int)wparam);
					pickColor(ID_FONT_OUTLINE);
					setupTexture();
					mFont.setFont(mCurWin32Font);
					renderFont();
					mRedraw = true;
						return 0;
						
				case ID_OUTLINEWIDTH_1:
				case ID_OUTLINEWIDTH_2:
				case ID_OUTLINEWIDTH_4:
					setMenuItemOutlineWidth((int)wparam);
					renderFont();
					mRedraw = true;
						return 0;					
						
				case ID_ANTI_TRUE:
				case ID_ANTI_FALSE:
					setMenuItemAlias((int)wparam);
					renderFont();
					mRedraw = true;
						return 0;					
						
				case ID_SIZE_128X128:
				case ID_SIZE_256X256:
				case ID_SIZE_512X512:
				case ID_SIZE_1024X1024:
				case ID_SIZE_LINEAR:
					setMenuItemSize((int)wparam);
					setupTexture();
					mFont.setFont(mCurWin32Font);
					renderFont();
					mRedraw = true;
						return 0;			

				case ID_HELP_ABOUT:
					MessageBox(this->getHWND(), kAboutText, "About GameTutorials Fontastic", MB_OK | MB_ICONINFORMATION);
						return 0;
						
				case ID_CHARACTERS_CUSTOM: // Intentional drop through
					if(loadCustomCharSet() == false) 
					{
						MessageBox(this->getHWND(), kErrMsgCustom, "Error!  Incompatible .txt file", MB_OK | MB_ICONERROR);
							return 0;
					}
						
				case ID_CHARATERS_ALPHAS:
				case ID_CHARATERS_NUMERIC:
				case ID_CHARATERS_ALNUM:
				case ID_CHARATERS_ASCII:
				case ID_CHARATERS_ALPHA_NUM_PUN:
					setMenuItemChar((int)wparam);
					setupTexture();
					mFont.setFont(mCurWin32Font);
					renderFont();
					mRedraw = true;
						return 0;

				case ID_CASE_ALL:
				case ID_CASE_UPPERONLY:
				case ID_CASE_LOWERONLY:
					setMenuItemCase((int)wparam);
					setupTexture();
					mFont.setFont(mCurWin32Font);
					renderFont();
					mRedraw = true;
						return 0;
						
				default:
					assert(!"Command unknown");
						break;
			}
		}
		break;
		
		case WM_DESTROY:
		
			PostQuitMessage(0);
				return 0;
	}
	
	// By default
	return CWindow::wndProc(msg, wparam, lparam);
}

void CApp::update()
{
	CWinInput::poll();
}

void CApp::draw()
{
	if(mRedraw)
	{
		FillRect(getDC(), &(this->getClientRect()), (HBRUSH)GetStockObject(WHITE_BRUSH));
		
		int halfClientWid = getClientRect().right / 2;
		int halfClientHgt = getClientRect().bottom / 2;
		int halfTexWid = mFont.getFontImg()->getWidth() / 2;
		int halfTexHgt = mFont.getFontImg()->getHeight() / 2;
		
		RECT rect = { halfClientWid - halfTexWid, halfClientHgt - halfTexHgt,
					  halfClientWid + halfTexWid, halfClientHgt + halfTexHgt };
		
		mFont.getFontImg()->blit(getDC(), rect);
		#ifdef _DEBUG
			drawDebugBoxes(rect.left, rect.top);
		#endif
		mRedraw = false;	
	}
}

void CApp::drawDebugBoxes(int x, int y)
{
return;
//#define _DRAW_DBG_TO_SCREEN_

#ifdef _DRAW_DBG_TO_SCREEN_
	#define DC getDC()
#else
	#define DC mFont.getFontImg()->getDC()
	x = 0;
	y = 0;
#endif

	HPEN hpen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	HPEN oldPen = (HPEN)SelectObject(DC, hpen);
	HBRUSH oldBrush = (HBRUSH)SelectObject(DC, GetStockObject(NULL_BRUSH));

	for(int i = 0; i < kMaxChars; ++i)
	{
		const CAtlas &info = mFont.getInfo(i);

		if(info.isEmpty())
			continue;

		Rectangle(DC, x + info.ULX(), y + info.ULY(), x + info.BRX(), y + info.BRY());
	}

	SelectObject(DC, oldPen);
	SelectObject(DC, oldBrush);
	DeleteObject(hpen);
	
#undef DC
}

bool CApp::getSaveFileName(OPENFILENAME *ofn, const char *filtetText, 
						   const char *filterExt, char *fileName)
{
	char filter[256] = {0};
	
	// Set filter
	sprintf(filter, "%s", filtetText);
	sprintf(filter + strlen(filtetText) + 1, "%s", filterExt);
	
	ofn->lStructSize = sizeof(OPENFILENAME);
	ofn->hwndOwner = mHwnd;
	ofn->nMaxFile = MAX_PATH;
	ofn->lpstrFilter = filter;
	ofn->lpstrFile = fileName;
	ofn->Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

	if(!GetSaveFileName(ofn))
		return false; // Error occurred or they canceled...
		
	return true;
}

bool CApp::getLoadFileName(OPENFILENAME *ofn, const char *filtetText, 
						   const char *filterExt, char *fileName)
{
	char filter[256] = {0};
	
	// Set filter
	sprintf(filter, "%s", filtetText);
	sprintf(filter + strlen(filtetText) + 1, "%s", filterExt);
	
	ofn->lStructSize = sizeof(OPENFILENAME);
	ofn->hwndOwner = mHwnd;
	ofn->nMaxFile = MAX_PATH;
	ofn->lpstrFilter = filter;
	ofn->lpstrFile = fileName;
	ofn->Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

	if(!GetOpenFileName(ofn))
		return false; // Error occurred or they canceled...
		
	return true;
} 

bool CApp::saveGTFT()
{
	const char kFilterText[] = "GT Font Files";
	const char kFilterExt[] = "*gtft";
	const char kExt[] = ".gtft";

	OPENFILENAME ofn = {0};
	char fileName[MAX_PATH] = {0};
	bool status = true; // Assume success

	mRedraw = true; // We'll need to redraw when we're done

	if(!getSaveFileName(&ofn, kFilterText, kFilterExt, fileName))
		return true; // Must have canceled... We hope %)
	
	// If there's no extension, append one	
	if(!strstr(fileName, kExt))
		strcat(fileName, kExt);
		
	if(!mFont.saveFontInfo(fileName))
		return false;

	CFont::SSaveFontInfo fi;

	fi.outline = mOutline;
	fi.colorKey = mColorKey;
	fi.isOutlined = (mOutlineFlag == ID_OUTLINE_TRUE);

	return mFont.saveFontImg(fileName, fi);
}

bool CApp::loadCustomCharSet()
{
	const char kFilterText[] = "Custom Unicode Characters";
	const char kFilterExt[] = "*txt";
	const char kExt[] = ".txt";

	OPENFILENAME ofn = {0};
	char fileName[MAX_PATH] = {0};
	bool status = true; // Assume success

	mRedraw = true; // We'll need to redraw when we're done

	if(!getLoadFileName(&ofn, kFilterText, kFilterExt, fileName))
		return true; // Must have canceled... We hope %)
	
	// If there's no extension, append one	
	if(!strstr(fileName, kExt))
		strcat(fileName, kExt);
		
	// Load custom file
	FILE *f = NULL;
	unsigned int fileSize = 0;
	
	f = fopen(fileName, "rb");
	
	if(f == NULL)
		return false;
		
	#define CLOSE_AND_EXIT(b) { fclose(f); return (b); }
		
	if(fseek(f,0,SEEK_END) != 0)
		CLOSE_AND_EXIT(false); 

	fileSize = ftell(f);
	
	if(fileSize <= 2)
		CLOSE_AND_EXIT(false);
	
	mCustomSetCharCount = fileSize;
	mCustomSetCharCount /= sizeof(wchar_t); // Convert to total number of wchart_t
	mCustomSetCharCount--; // Only care about total number of characters
		
	// Seek back to the beginning
	if(fseek(f,0,SEEK_SET) != 0)
		CLOSE_AND_EXIT(false);
	
	SAFE_DELETE_ARRAY(mCustomSet);
	mCustomSet = new wchar_t[mCustomSetCharCount];
	memset(mCustomSet, 0, mCustomSetCharCount * sizeof(wchar_t));
	
	if(mCustomSet == NULL)
		CLOSE_AND_EXIT(false);
		
	if(fread(mCustomSet, sizeof(wchar_t), 1, f) != 1)
		CLOSE_AND_EXIT(false);
		
	if(mCustomSet[0] != 0xFEFF) // Version byte for a UNICODE .txt file made by NotePad ?????
		CLOSE_AND_EXIT(false);
		
    size_t r = fread(mCustomSet, mCustomSetCharCount * sizeof(wchar_t), 1, f);
    
    // Make sure we read in the file correctly
    if((r != 1) && (feof(f) == 0))
        CLOSE_AND_EXIT(false);
	
	CLOSE_AND_EXIT(true);
	#undef CLOSE_AND_EXIT
}

bool CApp::chooseFont()
{
	mRedraw = true;
	
	mChooseFont.lStructSize = sizeof(CHOOSEFONT);
	mChooseFont.hwndOwner = getHWND();
	mChooseFont.Flags = CF_SCREENFONTS | CF_SELECTSCRIPT | CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST;
	
	if(!ChooseFont(&mChooseFont))
		return false;

	if(!setFont(mChooseFont.lpLogFont))
		return false;

	if(mSizeFlag == ID_SIZE_LINEAR)
	{
		setupTexture();
		mFont.setFont(mCurWin32Font); // setupTexture destroys font, must restore
	}
		
	if(!renderFont())
		return false;
		
	return true;
}

bool CApp::setFont(LOGFONT *f)
{
	if(!f)
		return false;
		
	// Delete current font
	if(mCurWin32Font)
	{
		DeleteObject(mCurWin32Font);
		mCurWin32Font = NULL;
	}

	// Create the font
	mCurWin32Font = CreateFontIndirect(f); // mChooseFont.lpLogFont

	if(!mCurWin32Font)
		return false;
	
	return mFont.setFont(mCurWin32Font);
}

CApp::~CApp()
{
	BOOL b;

	if(mIcon)
	{
		b = DestroyIcon(mIcon);
		assert(b == TRUE);
		
		mIcon = NULL;
	}
	
	SAFE_DELETE_ARRAY(mCustomSet);
	
	// mMenu is automatically freed by window
}

// Protected Functions //

// Size the texture
bool CApp::setupTexture()
{
	int size = 0;
	
	if(mSizeFlag == ID_SIZE_128X128)
		size = 128;
	else if(mSizeFlag == ID_SIZE_256X256)
		size = 256;
	else if(mSizeFlag == ID_SIZE_512X512)
		size = 512;
	else if(mSizeFlag == ID_SIZE_1024X1024)
		size = 1024;
	else if(mSizeFlag == ID_SIZE_LINEAR)
		size = -1; // Linear
		
	if(size == 0)
		return false;

	int w = size;
	int h = size;

	if(size == -1) // If it's linear
		calcLinearImgSize(w, h);

	if((mFont.getFontImg()->getWidth() != w) || (mFont.getFontImg()->getHeight() != h))
	{
		// Size the image
		if(!mFont.getFontImg()->init(w, h, 4))
			return false;
	}
		
	return true;
}

// Anti-Alias the texture
bool CApp::antiAliasTexture()
{
	if(mAliasFlag == ID_ANTI_FALSE)
		return true; // Function call didn't fail, just no work to do
	
	const float kPercent = 1.0f / 8.0f;
	
	CImage *img = mFont.getFontImg();
	CImage temp = *img;
	int colorKey = (mColorKeyFlag == ID_COLORKEY_TRUE) ? mColorKey.getARGB() : kClearImgColor;
	int p2 = 0;
								
	#define EDGE_BLEND(x,y,p,m)	p2 = img->getPixel((x), (y)); \
								if(p2 && (p2 != colorKey)) { \
									int A = F2I(GET_A(p2) * kPercent); \
									int R = F2I(GET_R(p2) * kPercent); \
									int G = F2I(GET_G(p2) * kPercent); \
									int B = F2I(GET_B(p2) * kPercent); \
									p += ARGB(A, R, G, B); \
									mask |= m; \
								}
								
	for(int y = 0; y < img->getHeight(); ++y)
	{
		for(int x = 0; x < img->getWidth(); ++x)
		{
			int pixel = img->getPixel(x,y);
			int neighborCount = 0;
			int p = 0;
			int mask = 0;
			
			const int kCorner = (1 << 0);
			const int kLeftRight = (1 << 1);
			const int kUpDown = (1 << 2);
			
			if(pixel == colorKey)
			{
				EDGE_BLEND(x - 1, y - 1, p, kCorner);
				EDGE_BLEND(x - 1, y, p, kLeftRight);
				EDGE_BLEND(x - 1, y + 1, p, kCorner);
				EDGE_BLEND(x, y - 1, p, kUpDown);
				EDGE_BLEND(x, y + 1, p, kUpDown);
				EDGE_BLEND(x + 1, y - 1, p, kCorner);
				EDGE_BLEND(x + 1, y, p, kLeftRight);
				EDGE_BLEND(x + 1, y + 1, p, kCorner);				
			}
			
			if(mask == (kCorner | kLeftRight | kUpDown)) {
				temp.setPixel(x,y,p);
			}
		}
	}
	
	*img = temp;
		return true;
}

// Set color key in texture
bool CApp::colorKeyTexture()
{
	CImage *img = mFont.getFontImg();
	
	if(!img)
		return false;

	bool outlineOn = (mOutlineFlag == ID_OUTLINE_TRUE);
	bool colorKeyOn = (mColorKeyFlag == ID_COLORKEY_TRUE);

	for(int y = 0; y < img->getHeight(); ++y)
	{
		for(int x = 0; x < img->getWidth(); ++x)
		{
			int color = img->getPixel(x, y);

			if(color == kClearImgColor)
				img->setPixel(x, y, (colorKeyOn) ? (mColorKey.getARGB()) : ARGB(0,0,0,0));
			else // Set alpha to 255
			{
				color |= 0xFF000000;
				img->setPixel(x, y, color);
			}
		}
	}

	return true;
}

bool CApp::renderFont()
{
	bool status = true;
	
	// Clear current font texture to the app's color key
	mFont.getFontImg()->fillRect(mFont.getFontImg()->getRect(), kClearImgColor); 
	
	try
	{
		if(!renderLetters())
			throw "Couldn't render font";
		else if(!colorKeyTexture())
			throw "Couldn't set color key for texture";
		else if(!antiAliasTexture())
			throw "Couldn't anti-alias font";
	}
	catch(char *str)
	{
		displayError(str);
		status = false;
	}
	
	return status;
}

bool CApp::renderLetters()
{
	TEXTMETRIC tm = {0};
	float xOffset = (mOutlineFlag == ID_OUTLINE_TRUE) ? (mOutlineSize * 2.0f + kSpacing) : kSpacing;
	float yOffset = (mOutlineFlag == ID_OUTLINE_TRUE) ? (mOutlineSize * 2.0f) : 0;
	float xPos = xOffset;
	float yPos = yOffset + kSpacing;
	float wid = 0;
	float hgt = 0;
	float leftOverHang = 0.0f;
	
	// First zero out font info
	mFont.clearGlphys();

	// Get the height of the font
	GetTextMetrics(mFont.getFontImg()->getDC(), &tm);

	// Character array to draw	
	wchar_t* str = NULL;
	int glyphCount = 0;
	
	if(mCharUVSetFlag == ID_CHARACTERS_CUSTOM)
	{
		glyphCount = mCustomSetCharCount;
		str = mCustomSet;
	}
	else
	{
		glyphCount = kMaxChars;
		str = mASCIISet;
	}
			
	for(int i = 0; i < glyphCount; ++i)
	{
		ABCFLOAT floatWid = {0};
		int ch = str[i];

		if(testLetter(i) == false)
		{
			continue;
		}

		if(!GetCharABCWidthsFloat(mFont.getFontImg()->getDC(), ch, ch, &floatWid))
		{
			SIZE sz = {0};
			GetTextExtentPointW(mFont.getFontImg()->getDC(), (LPCWSTR)(&ch), 1, &sz);
			
			floatWid.abcfB = (float)sz.cx;
		}
		
		// Set width and height
		wid = floatWid.abcfB;

		if (floatWid.abcfA > 0)
			wid += floatWid.abcfA;

		hgt = (float)tm.tmHeight;

		// Add additional buffer width and height
		wid += xOffset;
		hgt += yOffset;

		leftOverHang = max(0.0f, -floatWid.abcfA);
		xPos += leftOverHang; 
			
		// Test to see if there is enough room for this letter
		// If not, wrap to the next line
		if(F2I(xPos + wid) >= mFont.getFontImg()->getWidth())
		{
			xPos = xOffset + leftOverHang;
			yPos += hgt;

			if(F2I(yPos + hgt) >= mFont.getFontImg()->getHeight())
			{	
				throw "Font selected is to big for texture";
					return false;
			}
		}
		
		// Write out letter
		TextOutW(mFont.getFontImg()->getDC(), F2I(xPos), F2I(yPos + yOffset), (LPCWSTR)&ch, 1);
		
		// Set u1, v1
		CAtlas atlas;

		atlas.ulx = (xPos - leftOverHang - (xOffset / 2.0f));
		atlas.uly = yPos;
		
		// Move to end of letter
		xPos += wid;	
						
		// Set u2, v2
		atlas.brx = (xPos + (xOffset / 2.0f));
		atlas.bry = (yPos + hgt);

		mFont.addGlyph(ch, atlas);
	}
	
	if(mOutlineFlag == ID_OUTLINE_TRUE)
		mFont.renderOutline(mOutlineSize, mOutline.getARGB(), kClearImgColor);
		
	return true; // All letters drawn correctly		
}

bool CApp::testLetter(int c)
{
	if(mCharUVSetFlag == ID_CHARATERS_ALPHAS)
		return (isalpha(c) != 0) && (testCase(c));
	else if(mCharUVSetFlag == ID_CHARATERS_NUMERIC)
		return (isdigit(c) != 0);
	else if(mCharUVSetFlag == ID_CHARATERS_ALNUM)
	{		
		return ((isalpha(c) && testCase(c)) || isdigit(c));
	}
	else if(mCharUVSetFlag == ID_CHARATERS_ALPHA_NUM_PUN)
	{
		return ((isalpha(c) && testCase(c)) || isdigit(c) || IsPunctuation(c));
	}
	else
		return true;
}

bool CApp::testCase(int c)
{
	assert(isalpha(c));

	if(mCaseFlag == ID_CASE_ALL)
		return true;
	else if(mCaseFlag == ID_CASE_UPPERONLY)
		return (isupper(c) != 0);
	else if(mCaseFlag == ID_CASE_LOWERONLY)
		return (islower(c) != 0);

	return false;
}

void CApp::setMenuItemSize(int which)
{
	assert(which >= ID_SIZE_128X128 && which <= ID_SIZE_LINEAR);
		
	if(mMenu == NULL)
		return; // Nothing to do
		
	mSizeFlag = which;
	
	for(int i = ID_SIZE_128X128; i <= ID_SIZE_LINEAR; ++i)
	{
		if(mSizeFlag == i)
			CheckMenuItem(mMenu, i, MF_CHECKED);
		else
			CheckMenuItem(mMenu, i, MF_UNCHECKED);
	}
}

void CApp::setMenuItemAlias(int which)
{
	assert(which >= ID_ANTI_TRUE && which <= ID_ANTI_FALSE);

	if(mMenu == NULL)
		return; // Nothing to do
		
	// Set flag
	mAliasFlag = which;

	for(int i = ID_ANTI_TRUE; i <= ID_ANTI_FALSE; ++i)
	{
		if(mAliasFlag == i)
			CheckMenuItem(mMenu, i, MF_CHECKED);
		else
			CheckMenuItem(mMenu, i, MF_UNCHECKED);
	}
}

void CApp::setMenuItemColorKey(int which)
{
	assert(which >= ID_COLORKEY_TRUE && which <= ID_COLORKEY_FALSE);

	if(mMenu == NULL)
		return; // Nothing to do
		
	// Set flag
	mColorKeyFlag = which;

	for(int i = ID_COLORKEY_TRUE; i <= ID_COLORKEY_FALSE; ++i)
	{
		if(mColorKeyFlag == i)
			CheckMenuItem(mMenu, i, MF_CHECKED);
		else
			CheckMenuItem(mMenu, i, MF_UNCHECKED);
	}
}

void CApp::setMenuItemOutline(int which)
{
	assert(which >= ID_OUTLINE_TRUE && which <= ID_OUTLINE_FALSE);

	if(mMenu == NULL)
		return; // Nothing to do
		
	// Set flag
	mOutlineFlag = which;

	for(int i = ID_OUTLINE_TRUE; i <= ID_OUTLINE_FALSE; ++i)
	{
		if(mOutlineFlag == i)
			CheckMenuItem(mMenu, i, MF_CHECKED);
		else
			CheckMenuItem(mMenu, i, MF_UNCHECKED);
	}
}

void CApp::setMenuItemOutlineWidth(int which)
{
	assert(which >= ID_OUTLINEWIDTH_1 && which <= ID_OUTLINEWIDTH_4);

	if(mMenu == NULL)
		return; // Nothing to do
		
	// Set flag
	mOutlineWidFlag = which;

	for(int i = ID_OUTLINEWIDTH_1; i <= ID_OUTLINEWIDTH_4; ++i)
	{
		if(mOutlineWidFlag == i)
			CheckMenuItem(mMenu, i, MF_CHECKED);
		else
			CheckMenuItem(mMenu, i, MF_UNCHECKED);
	}
	
	switch(mOutlineWidFlag)
	{
		case ID_OUTLINEWIDTH_1:
			mOutlineSize = 1;
				break;
				
		case ID_OUTLINEWIDTH_2:
			mOutlineSize = 2;
				break;
				
		case ID_OUTLINEWIDTH_4:
			mOutlineSize = 4;
				break;
	}
}

void CApp::setMenuItemChar(int which)
{
	assert(which >= ID_CHARATERS_ALPHAS && which <= ID_CHARACTERS_CUSTOM);
	
	if(mMenu == NULL)
		return; // Nothing to do
		
	// Set Flag
	mCharUVSetFlag = which;

	for(int i = ID_CHARATERS_ALPHAS; i <= ID_CHARACTERS_CUSTOM; ++i)
	{
		if(mCharUVSetFlag == i)
			CheckMenuItem(mMenu, i, MF_CHECKED);
		else
			CheckMenuItem(mMenu, i, MF_UNCHECKED);
	}
}

void CApp::setMenuItemCase(int which)
{
	assert(which >= ID_CASE_ALL && which <= ID_CASE_LOWERONLY);
	
	if(mMenu == NULL)
		return; // Nothing to do
		
	// Set Flag
	mCaseFlag = which;

	for(int i = ID_CASE_ALL; i <= ID_CASE_LOWERONLY; ++i)
	{
		if(mCaseFlag == i)
			CheckMenuItem(mMenu, i, MF_CHECKED);
		else
			CheckMenuItem(mMenu, i, MF_UNCHECKED);
	}
}


bool CApp::loadResources(HINSTANCE hinst)
{
	// Create the icon
	mIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

	// Error check
	if(mIcon == NULL)
		return false;

	mMenu = LoadMenu(hinst, MAKEINTRESOURCE(IDR_MENU1));

	// Error check
	if(mMenu == NULL)
		return false;

	return true;
}

void CApp::calcLinearImgSize(int &wid, int &hgt)
{
	TEXTMETRIC tm = {0};
	float outlineOffset = (mOutlineFlag == ID_OUTLINE_TRUE) ? mOutlineSize : 0.0f;
	float xPos = outlineOffset;
	float leftOverHang = 0.0f;
	float rightOverHang = 0.0f;
	float w = 0;
	
	// Get the height of the font
	GetTextMetrics(mFont.getFontImg()->getDC(), &tm);
	
	hgt = F2I(tm.tmHeight + kSpacing);
	hgt += F2I(outlineOffset);

	for(int c = 0; c < kMaxChars; ++c)
	{
		ABCFLOAT floatWid = {0};

		if(testLetter(c) == false)
			continue;
		
		GetCharABCWidthsFloat(mFont.getFontImg()->getDC(), c, c, &floatWid);

		// Set width and height
		w = (floatWid.abcfA + floatWid.abcfB + floatWid.abcfC);
		
		if(mOutlineFlag == ID_OUTLINE_TRUE)
			w += mOutlineSize;
			
		// Calculate over-hang to the left
		leftOverHang = -1.0f * min(0.0f, floatWid.abcfA);
		rightOverHang = min(0.0f, floatWid.abcfC);

		// Adjust xPos for over-hang
		xPos += leftOverHang;

		if(xPos < 0.0f)
			xPos = 0.0f; // Clamp to left edge 

		// Move to end of letter
		xPos += w;
		xPos -= rightOverHang;
		xPos += kSpacing; // Leave some spacing between letters
	}

	wid = F2I(xPos + kSpacing);
}

void CApp::pickColor(int flag)
{
	if(flag == ID_FONT_COLORKEY)
	{
		if(mColorKeyFlag == ID_COLORKEY_FALSE)
			return; // Nothing to do
	}
	else if(flag == ID_FONT_OUTLINE)
	{
		if(mOutlineFlag == ID_OUTLINE_FALSE)
			return; // Nothing to do
	}
	else
		return; // Illegal flag, nothing to do 
	
	CHOOSECOLOR cc = {0};

	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.lpCustColors = mCustomColors;
	cc.Flags = CC_FULLOPEN | CC_SOLIDCOLOR | CC_ANYCOLOR | CC_RGBINIT;
	cc.rgbResult = (flag == ID_FONT_COLORKEY) ? mColorKey.getARGB() : mOutline.getARGB();

	if(ChooseColor(&cc))
	{
		uchar A = 255;
		uchar R = GET_B((int)cc.rgbResult); // Swizel R and B
		uchar G = GET_G((int)cc.rgbResult);
		uchar B = GET_R((int)cc.rgbResult); // Swizel R and B

		if(flag == ID_FONT_COLORKEY)
			mColorKey.set(A, R, G, B);
		else
			mOutline.set(A, R, G, B);

	}
	
	// else assume they hit cancel
}

void CApp::displayError(const char *err)
{
	assert(mHwnd != NULL);
	
	if(err)
		MessageBox(mHwnd, err, "Font Application Error", MB_OK | MB_ICONERROR);
}

CApp theApp;
CApp *gApp = &theApp;