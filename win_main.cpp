// Done by TheTutor -- 
/*	
	
*/

#include "application.h"
#include <crtdbg.h>

// Width and height of the window (specifically the client rect)
const int kWinWid = 800;
const int kWinHgt = 600;

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprev, PSTR cmdline, int ishow)
{

	{
	MSG msg;
	SWinInfo winInfo = {0};
	
	// Set winInfo
	winInfo.titleBarText = "Fontastic";
	winInfo.width = kWinWid;
	winInfo.height = kWinHgt;
	winInfo.winStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER;
	winInfo.winExStyle = WS_EX_APPWINDOW;
		
	if(!gApp->init(hinstance, winInfo))
		return EXIT_FAILURE;
	
    while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				break;
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if(gApp->isActive())
		{
			gApp->update();
			gApp->draw();
			
			Sleep(1);
		}
	
	} // end of while(1)
	
	}
#ifdef _DEBUG
	_CrtCheckMemory();
	_CrtDumpMemoryLeaks();
#endif
	    return EXIT_SUCCESS; // Application was a success
}


