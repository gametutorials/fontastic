#include <assert.h>
#include <stdio.h>
#include "GT_Window.h"

////////////////
// Constants //
//////////////

const char* kWindowClassName = "GTWindowVersion1_1";

/////////////////////
// Static Members //
///////////////////

int CWindow::mRegisterCount = 0;

// CWindow's WinProc
LRESULT CALLBACK GTWinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// Constructor
CWindow::CWindow()	
{
	zeroVars();
}

// Init the CWindow
bool CWindow::init(HINSTANCE hinstance, const SWinInfo &winInfo)
{
		// Error Check
		if(mHinst || !hinstance)
			return false; // Already initialised OR "hinstance" == NULL
	
	try // Try and create this window and add it to the list
	{
		// Set data members
		mHinst = hinstance;
		mStyle = winInfo.winStyle;
		mExStyle = winInfo.winExStyle;
		mActive = true; // Window is becoming active
		mFullScreen = false; // Start in windowed mode
		
		if(!registerWndClass())
			throw "Couldn't register WNDCLASSEX";

		// Set desired client RECT
		mRect.left = mRect.top = 0;
		mRect.right = winInfo.width;
		mRect.bottom = winInfo.height;

		// Adjust window rect so it gives us our desired client rect when we 
		// create the window
		AdjustWindowRectEx(&mRect, mStyle, (winInfo.menu != NULL), mExStyle);
		
		mHwnd = CreateWindowEx(	mExStyle,
								kWindowClassName,
								winInfo.titleBarText,
								mStyle,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								mRect.right - mRect.left,
								mRect.bottom - mRect.top,
								NULL,
								winInfo.menu,
								NULL,
								this ); // Pointer to self
									
		if(!mHwnd)
			throw "Couldn't create the window";	
			
		// Get window's client RECT
		GetClientRect(mHwnd, &mRect);
		
		// Make sure it's what we requested
		if(mRect.bottom != winInfo.height || mRect.right != winInfo.width)
			throw "Window's client RECT does not equal desired client RECT";
			
		// Capture window's HDC
		mHdc = GetDC(mHwnd);

		// Error Check
		if(mHdc == NULL)
			throw "Couldn't get window's HDC";
	}
	catch(char* err)
	{
		OutputError(err);
			return false;
	}
	
	ShowWindow(mHwnd, true);
	UpdateWindow(mHwnd); 
		return true; // Created successfully
}

// CWindow's WNDPROC
LRESULT CWindow::wndProc(UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	{
		case WM_ACTIVATE:
		{
			if((LOWORD(wparam) == WA_INACTIVE) || (HIWORD(wparam) != 0))
				mActive = false;
			else
				mActive = true;
				
			return 0;
		}
				
		case WM_CLOSE:
		
			destroy();
				return 0;
	}

	return DefWindowProc(mHwnd, message, wparam, lparam);
}

// Set the size of the window
bool CWindow::setWindowSize(int wid, int hgt)
{
	assert(wid > 0);
	assert(hgt > 0);
	assert(mHwnd != NULL);

	if(!SetWindowPos(mHwnd, NULL, 0, 0, wid, hgt, SWP_NOZORDER | SWP_NOMOVE))
		return false;
		
	// Get the new client size
	GetClientRect(mHwnd, &mRect);
		
#ifdef _DEBUG
	RECT rect = {0};
	GetWindowRect(mHwnd, &rect);
	assert(rect.right - rect.left == wid);
	assert(rect.bottom - rect.top == hgt);
#endif
		
	return true;
}

// Set the size of the client area of the window
bool CWindow::setClientSize(int wid, int hgt)
{
	assert(wid > 0);
	assert(hgt > 0);
	assert(mHwnd != NULL);
	
	RECT rect = {0, 0, wid, hgt};
	
	// Get desired dimensions	
	if(!AdjustWindowRectEx(&rect, mStyle, FALSE, mExStyle))
		return false;

	if(!SetWindowPos(mHwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE))
		return false;
		
	// Get the new client size
	GetClientRect(mHwnd, &mRect);

	assert(mRect.right == wid);
	assert(mRect.bottom == hgt);
		return true;
}

// Sets the window's icons
bool CWindow::setIcons(HICON icon, HICON smallIcon)
{
	if(icon)
		SendMessage(mHwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
		
	if(smallIcon)
		SendMessage(mHwnd, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
		
	return true;
}

// Sets the window's menu
bool CWindow::setMenu(HMENU hmenu)
{
	if(hmenu)
	{
		if(!SetMenu(mHwnd, hmenu))
			return false;
			
		// Get the new client size
		GetClientRect(mHwnd, &mRect);
			return true;			
	}
	else
		return false;
}

// Sets the style of window
bool CWindow::setStyle(DWORD style, DWORD exStyle)
{
	bool styleSet = false; // Assume style will not be set

	// If we have a style to set
	if(style != kIgnoreStyle)
	{
		SetLastError(0); // Clear out error log
		
		// Always make sure style will be visible
		int ret = SetWindowLongPtr(mHwnd, GWL_STYLE, style);

		// If there has been an error
		if(ret == 0 && (GetLastError() != 0))
			return false;
			
		mStyle = style; // Set style
		styleSet = true;
	}

	// If we have an extended style to set
	if(exStyle != kIgnoreStyle) 
	{
		SetLastError(0);

		int ret = SetWindowLongPtr(mHwnd, GWL_EXSTYLE, exStyle);

		// If there has been an error
		if(ret == 0 && (GetLastError() != 0))
			return false;
			
		mExStyle = exStyle; // Set extended style
		styleSet = true;
	}
	
	// If we set a style, call this function to make sure window is updated
	if(styleSet)
	{
		if(!SetWindowPos(mHwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
			return false;
			
		GetClientRect(mHwnd, &mRect); // Update client RECT
		ShowWindow(mHwnd, SW_SHOW); // Make sure the window is shown
			return true;
	}
	
	return false; // No style was changed
}

// Toggles between full screen and windowed mode
bool CWindow::toggleFullScreen()
{
	static WINDOWPLACEMENT windowPos = {0};
	static DWORD prevExStyle = mExStyle;
	static DWORD prevStyle = mStyle;

	if(mFullScreen)
	{	
		if(!setStyle(prevStyle, prevExStyle))
			return false;
		
		// Get window width and height	
		int wid = windowPos.rcNormalPosition.right - windowPos.rcNormalPosition.left;
		int hgt = windowPos.rcNormalPosition.bottom - windowPos.rcNormalPosition.top;
		
		if(!setWindowSize(wid, hgt))
			return false;
		
		if(!SetWindowPlacement(mHwnd, &windowPos))
			return false;
	}
	else
	{	
		// Store off previous styles
		prevExStyle = mExStyle;
		prevStyle = mStyle;
		
		// Store off window placement
		GetWindowPlacement(mHwnd, &windowPos);
		
		DEVMODE devMode = {0};
	
		// Get the current display settings
		if(!EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&devMode))
			return false;
			
		// Attempt to change style
		if(setStyle(WS_POPUP) == false)
			return false;
			
		int wid = devMode.dmPelsWidth;
		int hgt = devMode.dmPelsHeight;
				
		if(!setClientSize(wid, hgt)) // Set the client size of the window
			return false;
			
		// Move the window to (0,0)
		SetWindowPos(mHwnd, NULL, 0, 0, 0, 0, SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOSIZE);
	}
	
	mFullScreen = !mFullScreen; // Toggle flag
		return true; // Change made successfully
}

// Destroy the window
bool CWindow::destroy()
{
	if(mHdc)
	{
		int r = ReleaseDC(mHwnd, mHdc);
		assert(r != 0);
		
		mHdc = NULL;
	}

	if(DestroyWindow(mHwnd))
	{
		mHwnd = NULL;
			return true;
	}
	else
		return false; // Couldn't destroy window
}
	

// Deconstructor
CWindow::~CWindow()
{
	assert(mHdc == NULL && "HDC was not released");

	// If the window was initialized, decrement the register count and if
	// it equals zero, unregister the class
	if(--mRegisterCount == 0)
	{
		BOOL b = UnregisterClass(kWindowClassName, mHinst);
		assert(b == TRUE);
	}
}

// *** Private Methods ***

void CWindow::zeroVars()
{
	mHinst = NULL;
	mHwnd = NULL; 
	mHdc = NULL;
	
	mRect.left = mRect.right = mRect.bottom = mRect.top = 0;

	mStyle = mExStyle = 0;
	mFullScreen = false;
	mActive = false;
}

// Register the WNDCLASSEX
bool CWindow::registerWndClass()
{
	if(mRegisterCount++ == 0)
	{
		WNDCLASSEX wndclassex = {0};

		// Init fields we care about
		wndclassex.cbSize = sizeof(WNDCLASSEX); // Always set to size of WNDCLASSEX
		wndclassex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_PARENTDC;
		wndclassex.lpfnWndProc = GTWinProc;
		wndclassex.hInstance = mHinst;
		wndclassex.lpszClassName = kWindowClassName;
		wndclassex.hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(IDC_ARROW),
												IMAGE_CURSOR, 0, 0, LR_SHARED); // Default cursor

		if(!RegisterClassEx(&wndclassex)) // Register the WNDCLASSEX
			return false;
	}
	return true; // It worked!
}

#pragma warning(push)
#pragma warning(disable : 4311 4312)

// WinProc
LRESULT CALLBACK GTWinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	CWindow *current = NULL;

	switch(msg)
	{
		case WM_CREATE:
		
			current = (CWindow*)((LPCREATESTRUCT)lparam)->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)current);
				return 0;			
	}
	
	current = (CWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if(current)
		return current->wndProc(msg, wparam, lparam);
	else
		return DefWindowProc(hwnd, msg, wparam, lparam);
}

#pragma warning(pop)
	