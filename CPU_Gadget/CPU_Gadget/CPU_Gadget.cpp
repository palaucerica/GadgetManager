// CPU_Gadget.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <windows.h>
#include "Resource.h"
#define ShowSettings    6667
#define MyTimer		6668

#define EXPORT extern "C" __declspec (dllexport)

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR* szWindowClass = L"WinApp";			// the main window class name
TCHAR* szTitle=L"CPU";
HINSTANCE hInst;

POINT center1 , center2,p;

SIZE size1 , size2 ,current;
static RECT rect;
static HWND hand , parent;
static int xDown , yDown, xActual, yActual;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	hInst = hModule;
	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}


HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   if(FindWindow(szWindowClass,szTitle))
	   return NULL;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
      CW_USEDEFAULT, 0, 200, 160, parent, NULL, hInstance, NULL);
   SetWindowLong(hWnd,GWL_EXSTYLE,WS_EX_LAYERED);
   SetLayeredWindowAttributes(hWnd,RGB(255,255,255),255,LWA_ALPHA);

   if (!hWnd)
   {
      return FALSE;
   }


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}



void SetRegion(HWND hWnd , bool & siz)
{
	HRGN h1 , h2;
	if (siz)
	{
		h1 = CreateEllipticRgn(rect.left + 2,rect.top + 20, rect.left + 82,rect.top + 100);
		h2 = CreateEllipticRgn(rect.left + 70,rect.top + 5,rect.left+ 125,rect.top+66);
		current = size1;
	} 
	else
	{
		current = size2;
		h1 = CreateEllipticRgn(rect.left + 15,rect.top + 30, rect.left + 121,rect.top + 139);
		h2 = CreateEllipticRgn(rect.left + 104,rect.top + 12,rect.left+ 183,rect.top+92);
	}

	CombineRgn(h1 , h1 , h2 , RGN_OR);
	SetWindowRgn(hWnd , h1 , TRUE);
	siz = !siz;

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc , hdctemp;
	HRGN h1 , h2;
	HBRUSH hbrush;
	static HBITMAP hbmp ;
	static bool size = true , click = false;
	int x , y;
	SYSTEM_POWER_STATUS sps;
	MEMORYSTATUS memstate;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION spi;
	

	switch (message)
	{
	case WM_CREATE:
		GetClientRect(hWnd ,& rect);
		size1.cx = 130; size1.cy = 103;
		size2.cx = 198; size2.cy = 150;
		current = size1;
		SetTimer(hWnd , MyTimer , 500 , NULL);
		break;

	case WM_RBUTTONDOWN:
		SendMessage(parent,ShowSettings,wParam,lParam);

	case WM_TIMER:

		GetSystemPowerStatus(&sps);
		GlobalMemoryStatus(&memstate);
		
		break;
	case WM_SIZE:

		SetRegion(hWnd , size);
		if (size)
		{
			center1.x = 67; center1.y = 84;
			center2.x = 145; center2.y = 50;
			hbmp = LoadBitmap(hInst , MAKEINTRESOURCE(IDB_BITMAP2));
			InvalidateRect(hWnd,NULL,TRUE);
		}
		else
		{
			center1.x = 40; center1.y = 60;
			center2.x = 96; center2.y = 35;
			hbmp = LoadBitmap(hInst , MAKEINTRESOURCE(IDB_BITMAP1));
			InvalidateRect(hWnd,NULL,TRUE);
		}
		
		break;
	case WM_LBUTTONUP:
		if (click) ReleaseCapture();
		click = false;
		break;
	case WM_LBUTTONDOWN:
		if (!click)
		{
			click = true;
			SetCapture(hWnd);
			p.x=xDown = LOWORD(lParam);
			p.y=yDown = HIWORD(lParam);
		}
		break;
	case WM_MOUSEMOVE:
		if (click)
		{
			xActual = LOWORD(lParam);
			yActual = HIWORD(lParam);
			p.x +=xActual - xDown;
			p.y +=yActual - yDown;
			SetWindowPos(hWnd,NULL, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE); 
		}
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...

	//	hbmp =   LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP2) );
		
		hdctemp = CreateCompatibleDC(hdc);
		
		DeleteObject(SelectObject(hdctemp, (HBITMAP) hbmp));
		BitBlt (hdc, 0, 0, current.cx, current.cy, hdctemp, 0, 0, SRCCOPY) ;
		DeleteDC (hdctemp) ;

		h1 = CreateEllipticRgn( center1.x - 3 , center1.y-3,center1.x+3,center1.y+3);
		h2 = CreateEllipticRgn( center2.x - 2 , center2.y-2,center2.x+2,center2.y+2);
		FillRgn(hdc,h1,CreateSolidBrush( RGB(0,0,0)) );
		FillRgn(hdc,h2,CreateSolidBrush( RGB(0,0,0)) );

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



EXPORT HICON GetGadgetIcon()
{
	return LoadIcon(hInst,MAKEINTRESOURCE(IDI_CPU));
}


EXPORT bool GadgetHasSettings()
{
	return false;
}


EXPORT void GetGadgetName(char* buff)
{
	buff = "CPU";
}

EXPORT void ShowGadgetSettingsDlg()
{
	
}


EXPORT HWND GetGadgetWindow(HWND hwnd)
{
	parent = hwnd;
	MyRegisterClass(hInst);
	hand = InitInstance(hInst,SW_SHOW);
	return hand;
}