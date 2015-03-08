// SlideShow.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <Windows.h>
#include <WinUser.h>
#include "resource1.h"
#include "shlobj.h"
#include <WinDef.h>
#include <vector>

#define MAX_LOADSTRING 100
#define ID_TIMER 1024
#define ID_ShowSettings 6667
#define RemoveGadget	21111
#define ResizeGadgets	21112
#define MoveGadget		21113

using namespace std;
#define EXPORT extern "C" __declspec (dllexport)

char  path_1[] = "C:\Users\Public\Pictures\Sample Pictures\0";
char * times[] = {"5 seconds" , "10 seconds","15 seconds" , "30 seconds","1 minute","2 minutes","5 minutes"};
char * images[] = {"Chrysanthemum.bmp","Desert.bmp","Jellyfish.bmp","Lighthouse.bmp","Tulips.bmp"};
int Count = 5, Pos = 0;
vector<char*> files;
int seconds[] = {5000 , 10000, 15000, 30000, 60000, 120000, 300000};
TRACKMOUSEEVENT mouse_events;
HINSTANCE hInst;
TCHAR* szWindowClass = L"WinApp";			// the main window class name
TCHAR* szTitle=L"SlideShow";

//int timer;
bool show_menu ;
char path[MAX_PATH] , filename[MAX_PATH];
HWND hand , parent;
static HBITMAP bmp1,bmp2,bmp3,bmp4;
RECT rect;
POINT p;

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
EXPORT void ShowGadgetSettingsDlg(void);

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

void SetRegion(HWND hWnd ,bool has_menu)
{
	POINT pp;
	bool inside;
	GetCursorPos(&pp);
	HWND temp = WindowFromPoint(pp);
	inside = (temp == hWnd );
	
	if (has_menu != show_menu)
	{

		HRGN r1 , r2;
		RECT r;

		GetClientRect(hWnd , &r);
		GetClientRect(hWnd , &r);
		r1 = CreateRectRgn(r.left,r.top,r.right - 25,r.bottom);
		r2  = CreateRectRgn(r.right - 20,r.top,r.right,r.top + 80);
		CombineRgn(r2 , r1 , r2 , RGN_OR);
		if (!show_menu)
		{
			SetWindowRgn(hWnd , r2 , FALSE);
			TrackMouseEvent(&mouse_events);
		}
		else
		{
			if (!inside)
				SetWindowRgn(hWnd , r1 , FALSE);
		}
		InvalidateRect(hWnd,&r,TRUE);
	}
	show_menu = has_menu;

}

void ChangeSize(HWND hWnd , bool size)
{
	GetWindowRect(hWnd , &rect);
	switch (size)
	{
	case true:
		SetWindowPos(hWnd,NULL, rect.left + 100, rect.top, rect.right - rect.left - 100, rect.bottom - rect.top - 20, SWP_NOZORDER);
		break;
	case false:
		SetWindowPos(hWnd,NULL, rect.left - 100, rect.top, rect.right - rect.left + 100, rect.bottom - rect.top + 20, SWP_NOZORDER);
		break;
	}
}


HWND InitInstance(HINSTANCE hInstance)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   if(FindWindow(szWindowClass,szTitle))
	   return NULL;

   hWnd = CreateWindow(szWindowClass,szTitle, WS_POPUP ,
      CW_USEDEFAULT, 0, 150,120,parent, NULL, hInstance, NULL);
   SetWindowLong(hWnd,GWL_EXSTYLE,WS_EX_LAYERED);
   SetLayeredWindowAttributes(hWnd,RGB(255,255,255),255,LWA_ALPHA);

   if (!hWnd)
   {
      return FALSE;
   }



   //inicializar variables glbales
   
   show_menu = true;
   mouse_events.dwFlags = TME_LEAVE ;
   mouse_events.hwndTrack = hWnd;
   mouse_events.cbSize = sizeof(TRACKMOUSEEVENT);

   bmp1 = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP1));
   bmp2 = LoadBitmapW(hInst,MAKEINTRESOURCE(IDB_BITMAP2));
   bmp3 = LoadBitmapW(hInst,MAKEINTRESOURCE(IDB_BITMAP3));
   bmp4 = LoadBitmapW(hInst,MAKEINTRESOURCE(IDB_BITMAP4));

   SetRegion(hWnd , false);
   ShowWindow(hWnd,SW_SHOW);
   UpdateWindow(hWnd);

   return hWnd;
}


bool its_bmp(char * cad)
{
	int l = strlen(cad);
	char * ext = "pmb.";
	if (l < 5) return false;
	for (int i = 0; l < 4; l++)
		if (cad[l - i]!=ext[i]) return false;
	return true;
}

void Get_Files_From_Path(char * path)
{
	WIN32_FIND_DATAA data;
	HANDLE next;

	files.clear();
	strcat(path , "\\*");
	next = FindFirstFileA(path , &data);

	if (next!=INVALID_HANDLE_VALUE)
	{
		if ( its_bmp(data.cFileName) )
		files.push_back(data.cFileName);
		while (FindNextFileA(next , &data) !=0)
			if ( its_bmp(data.cFileName) )
			files.push_back(data.cFileName);
		FindClose(next);
	}
	Pos = 0;
	Count = files.size();
}


BOOL CALLBACK DialogModalProc(HWND hDlg,UINT iMsg,WPARAM wParam,LPARAM lParam){


	static BROWSEINFOA browser;
	static char buffer[MAX_PATH];
	LPCITEMIDLIST items;
	static int timer=0;

	switch(iMsg){
		
	case WM_INITDIALOG:
		
		timer = 0;//SendDlgItemMessageA(hDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
		for (int i = 0; i < 7; i++)
			SendDlgItemMessageA(hDlg,  IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)times[i]);
		SendDlgItemMessageA(hDlg,  IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)"My Pictures");
		if ( strcmp(path,path_1) == 0 )
			SendDlgItemMessageA(hDlg,IDC_COMBO2,CB_SELECTSTRING,-1,(LPARAM)"My Pictures");
		else
			SendDlgItemMessageA(hDlg,IDC_COMBO2,CB_SELECTSTRING,-1,(LPARAM)path);

		SendDlgItemMessageA(hDlg,IDC_COMBO1,CB_SELECTSTRING,-1,(LPARAM)times[timer]);
		
		browser.hwndOwner = hDlg;
		browser.iImage = NULL;
		browser.lParam = NULL;
		browser.lpfn = NULL;
		browser.lpszTitle = "Select Folder";
		browser.pidlRoot = NULL;
		browser.pszDisplayName = buffer;
		
		return true;

	case WM_COMMAND:
		{
		switch(LOWORD(wParam)){

		case IDOK:
			
			timer = SendDlgItemMessageA(hDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
			KillTimer(hand , ID_TIMER );
			SetTimer( hand , ID_TIMER , seconds[timer] , NULL);
			GetDlgItemTextA(hDlg,IDC_COMBO2,path,MAX_PATH);
			if (strcmp(path,"My Pictures") == 0)
				strcpy(path,path_1);
			//Get_Files_From_Path(path);
			EndDialog(hDlg,true);
			break;

		case IDC_BUTTON1:

			items = SHBrowseForFolderA( & browser );
			if (items!=NULL)
			{
				SHGetPathFromIDListA(items , buffer);
				if (SendDlgItemMessageA(hDlg,IDC_COMBO2,CB_FINDSTRINGEXACT,-1,(LPARAM)buffer) == CB_ERR)
					SendDlgItemMessageA(hDlg,IDC_COMBO2,CB_INSERTSTRING,0,(LPARAM)buffer);
			}
			SendDlgItemMessageA(hDlg , IDC_COMBO2 , CB_SELECTSTRING , -1 , (LPARAM)buffer);
			
			break;

		case IDCANCEL:
			EndDialog(hDlg,false);
			break;

		}
		return true;
		}

	default:
		return false;
	}
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc , hdctemp;
	RECT r;
	HRGN reg , reg1;
	static HBITMAP hbmp;
	static BITMAP bmp;
	static POINT p1;

	switch(message)
	{

	case WM_CREATE:
		strcpy(path,path_1);
		//Get_Files_From_Path(path);
		SetTimer(hWnd,ID_TIMER,5000,NULL);
		break;
	case WM_MOUSELEAVE:
		Sleep(500);
		SetRegion(hWnd,false);
		break;
	case ResizeGadgets:
		ChangeSize( hWnd , (bool)wParam );
		SetRegion(hWnd , true);
		break;
	case WM_TIMER:
		if (wParam == MoveGadget)
		{
			GetWindowRect(hWnd , &r);
			GetCursorPos(&p1);
			SetWindowPos(hWnd,NULL,r.left + p1.x - p.x , r.top + p1.y - p.y,0,0,SWP_NOSIZE | SWP_NOZORDER);
			p.x = p1.x; p.y = p1.y;
			InvalidateRect(hWnd ,NULL , FALSE);
			break;
		} 

		Pos++;
		if (Count!=0) Pos %= Count;
		InvalidateRect(hWnd ,NULL , TRUE);
		break;
		
		
	case WM_RBUTTONDOWN:
		GetClientRect(hWnd , & r);
		p1.x = LOWORD(lParam);
		if (p1.x < r.right - 20)
			SendMessage(parent,ID_ShowSettings,wParam,lParam);
		break;
	case WM_LBUTTONUP:
		KillTimer(hWnd , MoveGadget );
		break;
	case WM_LBUTTONDOWN:
		GetWindowRect(hWnd , &rect);
		GetClientRect(hWnd , & r);
		p1.x = LOWORD(lParam);
		p1.y = HIWORD(lParam);
		if (p1.x >= r.right - 20 && p1.y<=20) 
		{
			SendMessage(parent,RemoveGadget,wParam,(LPARAM)hWnd);
			DestroyWindow(hWnd);
		}
			
		if (p1.x >= r.right - 20 && p1.y<=40) 
		{
			SendMessage(parent,ResizeGadgets,wParam,(LPARAM)hWnd);
			break;
		}
		if (p1.x >= r.right - 20 && p1.y > 60) 
		{
			DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOG1),hWnd,DialogModalProc);
			break;
		}
		GetCursorPos(&p);
		SetTimer(hWnd,MoveGadget,10,NULL);
		break;
	case WM_MOUSEMOVE:
		SetRegion(hWnd,true);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		GetClientRect(hWnd,&r);
		hdctemp = CreateCompatibleDC(hdc);
		if (show_menu)
		{
			SelectObject(hdctemp, (HBITMAP)bmp1);
			StretchBlt(hdc,r.right-20,0,20,20,hdctemp,0,0,16,16,SRCCOPY);
			SelectObject(hdctemp, (HBITMAP)bmp2);
			StretchBlt(hdc,r.right-20,20,20,20,hdctemp,0,0,16,16,SRCCOPY);
			SelectObject(hdctemp, (HBITMAP)bmp3);
			StretchBlt(hdc,r.right-20,40,20,20,hdctemp,0,0,128,128,SRCCOPY);
			SelectObject(hdctemp, (HBITMAP)bmp4);
			StretchBlt(hdc,r.right-20,60,20,20,hdctemp,0,0,16,16,SRCCOPY);
		}

		hbmp = (HBITMAP)LoadImageA(NULL,images[Pos] , IMAGE_BITMAP , 0,0,LR_LOADFROMFILE);
		SelectObject(hdctemp , hbmp);
		SetMapMode(hdctemp , GetMapMode(hdc));
		GetObject(hdctemp,sizeof(BITMAP),&bmp);
		StretchBlt(hdc,0,0,r.right - 25 , r.bottom , hdctemp , 0,0,1024,768,SRCCOPY);

		DeleteDC(hdctemp);
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
	return LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON2) );
}


EXPORT bool GadgetHasSettings()
{
	return true;
}


EXPORT void GetGadgetName(char* buff)
{
	buff = "Slide Show";
}


EXPORT void ShowGadgetSettingsDlg()
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOG1),hand,DialogModalProc);
}


EXPORT HWND GetGadgetWindow(HWND hwnd)
{
	parent = hwnd;
	MyRegisterClass(hInst);
	hand = InitInstance(hInst);
	return hand;
}

