#ifndef WINOBJ_H
#define WINOBJ_H

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include "GT_Util.h"

// A flag to pass to the setStyle() method to ignore
// setting the window's style and/or extended style
const int kIgnoreStyle = 0xFFFFFFFF;

// Structure to fill to pass to to CWindow::init() to create the window
struct SWinInfo
{
	const char* titleBarText; // Window's title bar text

	HMENU menu; // NULL if no menu, otherwise a valid menu handle
	
	DWORD winStyle; // Window's style
	DWORD winExStyle; // Window's extended style
	
	int width; // Desired client width of window
	int height; // Desired client height of window
};

// This is a base window object that handles basic Win32 functionality for us
class CWindow
{
	public:
		
		CWindow(); // Constructor
		
		// Inits a CWindow -- Returns true on success, false otherwise
		// **NOTE** Call this first on any derived class from CWindow
		virtual bool init(HINSTANCE hinstance, const SWinInfo &winInfo);
		
		// Window procedure for the CWindow
		virtual LRESULT wndProc(UINT msg, WPARAM wparam, LPARAM lparam);
				
		// Sets window dimensions to the specified WINDOW width/height
		// Returns true for success, false otherwise
		virtual bool setWindowSize(int wid, int hgt);
		
		// Sets window dimensions to the specified CLIENT width/height
		// Returns true for success, false otherwise
		virtual bool setClientSize(int wid, int hgt);
		
		// Sets the window icon and small icon to the HICON's passed in
		// If either icon is NULL, it is ignored
		// Returns true for success, false otherwise
		virtual bool setIcons(HICON icon = NULL, HICON smallIcon = NULL);
		
		// Sets the window menu to the menu handle passed in
		// Returns true for success, false otherwise
		virtual bool setMenu(HMENU hmenu);		
		
		// Sets the style and extended style of the window to the values passed in
		// Returns true on success, false otherwise
		virtual bool setStyle(DWORD style = kIgnoreStyle, DWORD exStyle = kIgnoreStyle);
		
		virtual bool toggleFullScreen(); // Toggles between window mode and full screen mode
										// Returns true for success, false otherwise
		
		// Destroys the window								
		virtual bool destroy();
										
		// Returns status of being full screen or not						
		bool isFullScreen() { return mFullScreen; }
		
		// Returns status of being active or not
		bool isActive() { return mActive; }

		// Data Access ***

			HINSTANCE getHINST() { return mHinst; }
			HWND getHWND() { return mHwnd; }
			HDC getDC() { return mHdc; }

			RECT getClientRect() { return mRect; }
			
			int getClientWid() { return mRect.right; }
			int getClientHgt() { return mRect.bottom; }
			
			DWORD getStyle() { return mStyle; }
			DWORD getStyleEx() { return mExStyle; }

		// *** End Data Access

		// Deconstructor
		virtual ~CWindow();

	protected: // -- The data --

		HINSTANCE mHinst; // A copy of the HINSTANCE that was used to make the window
		HWND mHwnd; // A copy of the window's HWND
	
		HDC mHdc; // The window's device context
		
		RECT mRect; // The windows client size
		
		DWORD mStyle;
		DWORD mExStyle;

		bool mFullScreen; // True if in full screen mode, false otherwise
		bool mActive; // True if window is active, false otherwise
		
	private:
			
		void zeroVars(); // "Zeros" out all the variables
	
		static int mRegisterCount; // Count of how many times we've registered the WNDCLASSEX
		bool registerWndClass(); // Registers the WNDCLASSEX
		
		// Private copy constructor and assignment operator so 
		// no accidental copies are made
		CWindow(const CWindow &obj) { }
		CWindow& operator =(const CWindow &obj) { return *this; }
};

#endif

// © 2000-2003 GameTutorials