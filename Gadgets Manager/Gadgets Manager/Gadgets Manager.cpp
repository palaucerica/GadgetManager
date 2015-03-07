// Comienzo_de_la_Tarea.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Gadgets Manager.h"
#include "shellapi.h"
#include <windows.h>
#include <strsafe.h>
#include "Commdlg.h"
#include <string.h>
#include <vector>

#define MAX_LOADSTRING 100
#define NOT_ICON1 40000
#define MYWM_NOTIFYICON 40001
#define MSG_MYNOTIFYICON 6666
#define ShowMenu 6667
#define MyTimer 6668
#define ResizeGadget  21112
#define RemoveGadget  21111

using namespace std;
// Global Variables:


typedef HWND (*ShowGadget)(HWND);
typedef void (*ShowSettings)(void);
typedef bool (*HasSettings)(void);
typedef HICON (*GadgetIcon)(void);
typedef char* (*GadgetName)(void);

struct GadgetsItem
{
	ShowSettings settings;
	HWND hwnd;
	HICON hicon;
	char * name;
	bool HasOptions, markalwaysvisible , SizeNormal;
	int opacity;
};

static OPENFILENAME ofn;
WCHAR szFile[260];
HWND hw , LastWin;
bool DialogVisible , movegad , hotkey=false;
vector<GadgetsItem> gadgets;
HMENU menu1;
static POINT posgad;
HICON hIcon;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HBITMAP bmp1,bmp2,bmp3,bmp4;
RECT recta;


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProc1(HWND , UINT, WPARAM , LPARAM );
BOOL CALLBACK DlgProc2(HWND , UINT, WPARAM , LPARAM );
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);



 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GADGETSMANAGER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GADGETSMANAGER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON3));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND win = FindWindow(0,L"Tarea");
	if (win != NULL)
		return FALSE;

   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd =CreateWindowEx(WS_EX_TOOLWINDOW,szWindowClass, L"Tarea", WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   hw = hWnd;
   //GadgetsItem tt;
   //gadgets.push_back(tt);

   bmp1 = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP5));
   bmp2 = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP6));
   bmp3 = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP7));
   bmp4 = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP8));
   
	GetWindowRect( GetDesktopWindow() , & recta);
	posgad.x=recta.right-250;
	posgad.y=recta.top;
   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   return TRUE;
}

int FindGadget(HWND hWnd)
{
	for (int i = 0; i < gadgets.size(); i++)
		if ( gadgets[i].hwnd == hWnd)
			return i;
	return -1;
}

void ShowGadgetMenu_1(GadgetsItem gad,POINT p, HWND hWnd)
{
	menu1 = GetSubMenu( LoadMenu(hInst,MAKEINTRESOURCE(IDR_MENU2)) ,0 );
	if (!gad.HasOptions) DeleteMenu(menu1 , ID_Y_OPTIONS , MF_BYCOMMAND);
	if (gad.markalwaysvisible) 
		CheckMenuItem(menu1,ID_Y_ALWAYSVISIBLE,MFS_CHECKED);
	if (gad.SizeNormal) 
		CheckMenuItem(menu1,ID_SIZE_SMALL,MFS_CHECKED);
	else
		CheckMenuItem(menu1,ID_SIZE_BIG,MFS_CHECKED);
	switch(gad.opacity)
	{
	case 10:
		CheckMenuItem(menu1,ID_OPACITY_10,MFS_CHECKED);
		break;
	case 20:
		CheckMenuItem(menu1,ID_OPACITY_20,MFS_CHECKED);
		break;
	case 40:
		CheckMenuItem(menu1,ID_OPACITY_40,MFS_CHECKED);
		break;
	case 60:
		CheckMenuItem(menu1,ID_OPACITY_60,MFS_CHECKED);
		break;
	case 80:
		CheckMenuItem(menu1,ID_OPACITY_80,MFS_CHECKED);
		break;
	case 100:
		CheckMenuItem(menu1,ID_OPACITY_100,MFS_CHECKED);
		break;
	}
	TrackPopupMenu(menu1,TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL );
}

void AddGadgets()
{
	if ( !DialogVisible)
	CreateDialog( hInst, MAKEINTRESOURCE(IDD_DIALOG1), hw, DlgProc1 );
}

void SetGadgetPos(HWND hWnd)
{
	RECT rectb;
	
	
		GetWindowRect(hWnd, &rectb );
		SetWindowPos(hWnd,NULL,posgad.x, posgad.y+5 , 0,0,SWP_NOSIZE | SWP_NOZORDER);

		if(posgad.y+rectb.bottom+5<recta.bottom)
			posgad.y=posgad.y+rectb.bottom+5;

}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent , a;
	PAINTSTRUCT ps;
	HDC hdc;
	static NOTIFYICONDATA nid;
	WCHAR * msg = L"Haga click derecho para ver opciones";
	static HMENU hMenuExit ;
	POINT p;
	MENUITEMINFO meninf;
	HCURSOR hcursor , hant;
	RECT rect;

	switch (message)
	{

	case ShowMenu:
		GetCursorPos(&p);
		LastWin = WindowFromPoint(p);
		a = FindGadget(  LastWin);
		ShowGadgetMenu_1(gadgets[a] , p , hWnd);
		break;
	case ResizeGadget:
		LastWin = (HWND)lParam;
		a = FindGadget(LastWin);
		gadgets[a].SizeNormal = !gadgets[a].SizeNormal;
		SendMessage(LastWin,ResizeGadget,gadgets[a].SizeNormal,lParam);
		break;
	case RemoveGadget:
		//LastWin = (HWND)lParam;
		//a = FindGadget( LastWin );
		//gadgets.erase(gadgets.begin() + a);
		break;
	case WM_LBUTTONDOWN:
		if (movegad)
		{
			movegad = false;
			ReleaseCapture();
		}
		break;
	case WM_CREATE:
		hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON3));
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hWnd;
		nid.uID = NOT_ICON1;
		nid.uFlags =NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_INFO;
		nid.uCallbackMessage = MYWM_NOTIFYICON;
		nid.hIcon = hIcon;
		nid.uTimeout = 60000;
		wcscpy(nid.szTip,L"Gadgets Manager");
		wcscpy(nid.szInfo,msg);
		wcscpy(nid.szInfoTitle, L"Gadgets Manager"); //Titulo del Globo
		nid.dwInfoFlags = NIIF_INFO; //Banderas de estilo del globo
		Shell_NotifyIcon( NIM_ADD, &nid );//Para poner el icono en la barra
		hMenuExit = GetSubMenu( LoadMenu( hInst, MAKEINTRESOURCE( IDR_MENU1 )), 0 );
		RegisterHotKey(hWnd , NULL,MOD_CONTROL ,'O' ); 
		SetFocus(hWnd);
		break;
	case WM_HOTKEY:
		a = (!hotkey)? SW_HIDE : SW_SHOW;
			for (int i = 0; i < gadgets.size(); i++)
				ShowWindow( gadgets[i].hwnd , a);
			hotkey = !hotkey;
		break;

	case MYWM_NOTIFYICON:
		switch(lParam){
			case WM_RBUTTONDOWN:
						GetCursorPos (&p);
						TrackPopupMenu( hMenuExit, TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL );
						break;
		}
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		
		switch (wmId)
		{

		case ID_Y_ALWAYSVISIBLE:
			a = FindGadget(LastWin);
			gadgets[a].markalwaysvisible = !gadgets[a].markalwaysvisible;
			if (gadgets[a].markalwaysvisible)
				SetWindowPos(gadgets[a].hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
			else
				SetWindowPos(gadgets[a].hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);

			break;
		case ID_SIZE_BIG:
			a = FindGadget(LastWin);
			if (gadgets[a].SizeNormal)
			{
				gadgets[a].SizeNormal = false;
				SendMessage(LastWin,ResizeGadget,false,lParam);
			}
			break;
		case ID_SIZE_SMALL:
			a = FindGadget(LastWin);
			if (!gadgets[a].SizeNormal)
			{
				gadgets[a].SizeNormal = true;
				SendMessage(LastWin,ResizeGadget,true,lParam);
			}
			break;
		case ID_Y_ADDGAGETS:
			AddGadgets();
			break;
		case ID_Y_OPTIONS:
			a = FindGadget(LastWin);
			gadgets[a].settings();
			break;
		case ID_OPACITY_10:
			a = FindGadget(LastWin);
			gadgets[a].opacity = 10;
			SetLayeredWindowAttributes(LastWin, NULL , 200, LWA_ALPHA);
			break;
		case ID_OPACITY_20:
			a = FindGadget(LastWin);
			gadgets[a].opacity = 20;
			SetLayeredWindowAttributes(LastWin, NULL , 170, LWA_ALPHA);
			break;
		case ID_OPACITY_40:
			a = FindGadget(LastWin);
			gadgets[a].opacity = 40;
			SetLayeredWindowAttributes(LastWin, NULL , 140, LWA_ALPHA);
			break;
		case ID_OPACITY_60:
			a = FindGadget(LastWin);
			gadgets[a].opacity = 60;
			SetLayeredWindowAttributes(LastWin, NULL , 110, LWA_ALPHA);
			break;
		case ID_OPACITY_80:
			a = FindGadget(LastWin);
			gadgets[a].opacity = 80;
			SetLayeredWindowAttributes(LastWin, NULL , 80, LWA_ALPHA);
			break;
		case ID_OPACITY_100:
			a = FindGadget(LastWin);
			gadgets[a].opacity = 100;
			SetLayeredWindowAttributes(LastWin, NULL , 50, LWA_ALPHA);			
			break;
		case ID_Y_MOVE:
			movegad = true;
			hcursor = LoadCursor(NULL , MAKEINTRESOURCE(IDC_SIZEALL));
			hant = SetCursor( hcursor );
			SetCapture(hWnd);
			SetFocus(hWnd);
			break;
		case ID_Menu:
			a = FindGadget(LastWin);
			gadgets.erase(gadgets.begin() + a);
			DestroyWindow(LastWin);
			break;

		case ID_X_SALIR:
			Shell_NotifyIcon( NIM_DELETE, &nid );//Para quitar el icono de la barra
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		case ID_X_ADICIONARGADGET:
			AddGadgets();
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		break;

	case WM_KEYDOWN:
		if (movegad)
		{
			GetWindowRect(LastWin,&rect);
			switch( (int)wParam )
			{
			case VK_UP:
				SetWindowPos(LastWin,NULL,rect.left,rect.top - 3,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER);
				break;
			case VK_LEFT:
				SetWindowPos(LastWin,NULL,rect.left - 3,rect.top ,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER);
				break;
			case VK_RIGHT:
				SetWindowPos(LastWin,NULL,rect.left + 3,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER);
				break;
			case VK_DOWN:
				SetWindowPos(LastWin,NULL,rect.left,rect.top + 3,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER);
				break;
			case VK_RETURN:
				movegad = false;
				SetCursor( LoadCursor(NULL , IDC_ARROW) );
				ReleaseCapture();
				break;
			}
			if (movegad) SetFocus(hWnd);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		//Shell_NotifyIcon( NIM_DELETE, &nid );//Para quitar el icono de la barra
		
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void AddGadget(LPCWSTR name)
{
	HINSTANCE hm;
	GadgetsItem temp;
	hm = LoadLibrary(name);
	temp.hwnd = ((ShowGadget)GetProcAddress(hm,"GetGadgetWindow"))(hw);
	if(temp.hwnd==NULL)
		return;
	temp.HasOptions = ((HasSettings)GetProcAddress(hm,"GadgetHasSettings"))();
	temp.settings =  (ShowSettings)GetProcAddress(hm,"ShowGadgetSettingsDlg");
	temp.opacity = 0;
	temp.hicon = ((GadgetIcon)GetProcAddress(hm,"GetGadgetIcon"))();
	temp.markalwaysvisible = false;
	temp.SizeNormal = true;
	temp.name = ((GadgetName)GetProcAddress(hm,"GetGadgetName"))();
	gadgets.push_back(temp);
	//SetFocus(temp.hwnd);
	//FreeLibrary(hm);
	SetGadgetPos(temp.hwnd);

}


BOOL CALLBACK DlgProc1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	int wmId, wmEvent;
	PAINTSTRUCT p;
	HDC dc;
	RECT rect;
	HBRUSH br = CreateSolidBrush( RGB(134,165,233) );	
	HINSTANCE hm;
	HWND buttons;
	HDC comp;

	switch (message)
	{
	case WM_PAINT:
		dc = BeginPaint(hDlg,&p);
		GetClientRect(hDlg,&rect);
		FillRect(dc,&rect,br);
		EndPaint(hDlg,&p);
		//buttons = GetDlgItem(hDlg,IDC_BUTTON2);
		SendDlgItemMessage(hDlg,IDC_BUTTON2,BM_SETIMAGE ,IMAGE_BITMAP,(LPARAM)bmp1);
		SendDlgItemMessage(hDlg,IDC_BUTTON3,BM_SETIMAGE ,IMAGE_BITMAP,(LPARAM)bmp2);
		SendDlgItemMessage(hDlg,IDC_BUTTON4,BM_SETIMAGE ,IMAGE_BITMAP,(LPARAM)bmp3);
		SendDlgItemMessage(hDlg,IDC_BUTTON5,BM_SETIMAGE ,IMAGE_BITMAP,(LPARAM)bmp4);
		break;
	case WM_INITDIALOG:
		DialogVisible = true;
		SetLayeredWindowAttributes(hDlg, RGB(2,2,255) , 220, LWA_ALPHA);
		return TRUE;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch(wmId){
		case IDCANCEL:
			DialogVisible = false;
			EndDialog(hDlg,wmId);
			break;

		case IDC_BUTTON1:
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = L"All (*.dll)\0*.dll\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn)==TRUE) 
				AddGadget(ofn.lpstrFile);

			break;
		case IDC_BUTTON2:
			AddGadget(L"SlideShow_Gadget.dll");
			break;
		case IDC_BUTTON3:
			AddGadget(L"MiGadgetPuzzle.dll");
			break;
		case IDC_BUTTON4:
			AddGadget(L"MiGadgetTicTacToe.dll");
			break;
		case IDC_BUTTON5:
			AddGadget(L"CPU_Gadget.dll");
			break;

		}
		break;
	}
	return FALSE;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}