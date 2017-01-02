#include <assert.h>
#include "GT_Input.h"

// Static variables
bool CWinInput::mKeys[kMaxKeys] = {0};
float CWinInput::mKeyDelayTimes[kMaxKeys] = {0};
POINT CWinInput::mPos = {0};
float CWinInput::mDelayTime = kDefaultKeyDelayTime;

// Poll the controls
void CWinInput::poll()
{
	// Grab the state of each key
	for(int i = 0; i < kMaxKeys; ++i)
		mKeys[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
		
	GetCursorPos(&mPos); // Get cursor position
}

// Poll the controls, record time slice
void CWinInput::poll(float dt)
{
	// Grab the state of each key
	for(int i = 0; i < kMaxKeys; ++i)
	{
		mKeys[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
		
		if(mKeyDelayTimes[i] > 0.0f)
			mKeyDelayTimes[i] -= dt;
	}

	GetCursorPos(&mPos); // Get cursor position
}

// Set a key's status
void CWinInput::setKey(int which, bool onOrOff)
{
	assert(which >= 0 && which < kMaxKeys); // Safety first
	mKeys[which] = onOrOff;
}

// Set a key's status
void CWinInput::setKey(WPARAM which, bool onOrOff)
{
	assert((unsigned int)which < kMaxKeys); // Safety first
	mKeys[(unsigned int)which] = onOrOff;
}

// Sets the mouse's position in screen coordinates
void CWinInput::setMousePos(int x, int y)
{
	assert(x >= 0); // Should always be positive
	assert(y >= 0); // Should always be positive
	
	mPos.x = x;
	mPos.y = y;
	
	SetCursorPos(x, y);
}

// Sets the mouse's position in screen coordinates
void CWinInput::setMousePos(const POINT &p)
{
	mPos = p;
	SetCursorPos(p.x, p.y);
}

// Sets the mouse's position in client coordinates
void CWinInput::setMousePosInWindow(HWND hwnd, int x, int y)
{
	assert(hwnd != NULL);
	assert(x >= 0 && y >= 0);
	
	POINT p = { x, y };	
	
	if(ClientToScreen(hwnd, &p))
		mPos = p; 
}

// Returns true if the key is down, false otherwise
bool CWinInput::isAlphaDown(char k)
{
	if(!isalpha(k))
	{
		OutputDebugString("Not an ASCII alphabet character!");
			return false; // Not a alpha character
	}
	else
	{
		if(islower(k))
			k = toupper(k);
			
		return mKeys[k];
	}
}

// Returns true if the key is down, false otherwise
bool CWinInput::isNumericDown(char k)
{
	if(!isdigit(k))
	{
		OutputDebugString("Not an ASCII numeric character!");
			return false; // Not a numeric character
	}
	else
	{
		return mKeys[k];
	}
}

// Returns true if the key down, false otherwise
bool CWinInput::isKeyDown(int k, bool useDelay)
{
	assert(k >= 0 && k < kMaxKeys);
	
	if(useDelay)
	{
		if(mKeys[k] && mKeyDelayTimes[k] <= 0.0f)
		{
			mKeyDelayTimes[k] = mDelayTime;
				return true;
		}
		else
			return false;	
	}
	else
		return mKeys[k]; 
}

// Return mouse position in client area of window passed in
POINT CWinInput::getMousePosInWindow(HWND hwnd)
{
	POINT pos = mPos;
	
	ScreenToClient(hwnd, &pos);
		return pos;
}
		
			
	