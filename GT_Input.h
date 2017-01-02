#ifndef GT_INPUT_H
#define GT_INPUT_H

#include <windows.h>

const int kMaxKeys = 256;
const float kDefaultKeyDelayTime = 1.0f; // Amount of time to wait to register a repeated key press

class CWinInput
{
	public:
	
		CWinInput() { /* do nothing */ }
		
		static void poll(); // Poll the inputs
		static void poll(float dt); // Poll the inputs, record time slice since last call to poll
		
		// Sets a key, using the Win32 virtual key codes, to either
		// down (true), or up (false)
		static void setKey(int key, bool onOrOff);
		static void setKey(WPARAM key, bool onOrOff);
		
		// Sets the mouse position in screen coordinates
		static void setMousePos(int x, int y);
		static void setMousePos(const POINT &p);
		
		// Sets the mouse position in client coordinates
		static void setMousePosInWindow(HWND hwnd, int x, int y);
		
		// Sets the state of the mouse buttons
		static void setLeftMB(bool onOrOff) { mKeys[VK_LBUTTON] = onOrOff; }
		static void setRightMB(bool onOrOff) { mKeys[VK_RBUTTON] = onOrOff; }
		static void setMiddleMB(bool onOrOff) { mKeys[VK_MBUTTON] = onOrOff; }
		
		// Set delay time
		static void setKeyDelayTime(float delay) { mDelayTime = delay; }
		
		// Checks if the key is pressed or not
		static bool isAlphaDown(char key); // For ASCII characters 'a' - 'z' and 'A' - 'Z'
		static bool isNumericDown(char key); // For ASCII characters '0' - '9'
		static bool isLeftMBDown() { return mKeys[VK_LBUTTON]; }
		static bool isRightMBDown() { return mKeys[VK_RBUTTON]; }
		static bool isMiddleMBDown() { return mKeys[VK_MBUTTON]; }
		
		// Checks if the Win32 virtual key is pressed or not
		// If "useDelay" equals true, returns true only if key is pressed and
		// "mKeyDelayTime" has passed since the last key press 
		static bool isKeyDown(int key, bool useDelay = false); 
		
		// Returns mouse position in reference to client area of window
		static POINT getMousePosInWindow(HWND hwnd);	
		
		// Data Access ***
		
			static POINT getMousePos() { return mPos; }
			
		// *** End Data Access
		
	private:
	
		static bool mKeys[kMaxKeys]; // Array of virtual keys whether they are down (true) or up (false)
		static float mKeyDelayTimes[kMaxKeys]; // Amount of time to delay for repeated key strokes
		
		static POINT mPos; // Mouse position
		static float mDelayTime; // Amount of time to delay between repeated key strokes
};

#endif