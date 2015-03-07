// MiGadgetTicTacToe.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MiGadgetTicTacToe.h"
#include "resource.h"
#include <Windows.h>
#include <WinUser.h>
#include <WinDef.h>
#include <stdlib.h>

#define MAX_LOADSTRING 100
#define ANCHOCASILLA 55
#define ALTOCASILLA 55
#define XINI 7
#define YINI 5
#define SEPAR 2
#define SHOWMENU 6667
#define RemoveGadget	21111
#define ResizeGadgets	21112
#define MoveGadget		21113

#define EXPORT extern "C" __declspec (dllexport)

// Global Variables:
POINT p;
int xActual,yActual,xDown,yDown;

bool apretado=false;
bool empiezaUsuario=false;
bool gane=false;
bool perdi=false;
static int cantJugadas=0;
POINT casillaActual={0,0};
HBITMAP hBitmap;
BITMAP bm;
HBITMAP hCero;
BITMAP bmCero;
HBITMAP hCruz;
BITMAP bmCruz;
static int** matriz;
HDC actual;
HDC compa;
HDC compaCruz;
HDC compaCero;

HINSTANCE hInst;
HWND hWndGlobal;
TCHAR* szWindowClass = L"WinApp";			// the main window class name
TCHAR* szTitle=L"TicTacToe";
static HWND parent;

//declaracion de metodos
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM,LPARAM);
bool DamePunto(int x, int y);
POINT ChequeaColumnas(int id);
POINT ChequeaDiagonal(int id);
POINT ChequeaFilas(int id);
POINT JugadaInteligente();
POINT HazJugadaAtaque();
POINT ChequeaAmenaza();
POINT ChequeaAtaque();
POINT Jugada();
bool TresEnLinea(int id);
bool Perdi();
bool Gane();
bool TerminoJuego();
void InicializaMatriz();
void CreaMatriz();

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

HWND InitInstance(HINSTANCE hInstance)
{
   HWND hWnd;
   DWORD styles;

   hInst = hInstance; // Store instance handle in our global variable

   if(FindWindow(szWindowClass,szTitle))
   {
	   return NULL;
   }

   hWnd = CreateWindow(szWindowClass,szTitle, WS_POPUP,
      CW_USEDEFAULT, 0, 181,181,parent, NULL, hInstance, NULL);

   SetWindowLong(hWnd,GWL_EXSTYLE,WS_EX_LAYERED);
   SetLayeredWindowAttributes(hWnd,RGB(255,255,255),255,LWA_ALPHA);

   SetTimer(hWnd,1,1000,NULL);  //para saber cuando termino el juego
   styles = GetWindowLong(hWnd, GWL_STYLE); //en styles se guardan todos los estilos de la ventana
   styles &= ~WS_CAPTION & ~WS_THICKFRAME; //se quitan los estilos que se quiera
   SetWindowLong(hWnd, GWL_STYLE, styles); //se le pasa a la ventana sus nuevos estilos	

   hBitmap=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP1)); //para poner el fondo
   GetObject(hBitmap,sizeof(BITMAP),&bm);
   hCero=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP2)); //para los ceros
   GetObject(hCero,sizeof(BITMAP),&bmCero);
   hCruz=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP3)); //para las cruces
   GetObject(hCruz,sizeof(BITMAP),&bmCruz);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd,SW_SHOW);
   UpdateWindow(hWnd);

   return hWnd;
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
	wcex.hIcon			= NULL;  //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_HAND);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;  //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	
	POINT p1;
	RECT r;

	switch (message)
	{
	case WM_CREATE:
		CreaMatriz();
		break;
	case WM_TIMER:
		if (wParam == MoveGadget) //para mover la ventana
		{
			GetWindowRect(hWnd , &r);
			GetCursorPos(&p1);
			SetWindowPos(hWnd,NULL,r.left + p1.x - p.x , r.top + p1.y - p.y,0,0,SWP_NOSIZE | SWP_NOZORDER);
			p.x = p1.x; p.y = p1.y;
			InvalidateRect(hWnd ,NULL , FALSE);
			break;
		} 
		if(wParam==1) //para saber cuando se acabo el juego
		{
			if(TerminoJuego())
			{
				InicializaMatriz();
				InvalidateRect(hWnd,NULL,true);
			}
		}
		break;
	case WM_LBUTTONUP:
		KillTimer(hWnd,MoveGadget);
		break;
	case WM_RBUTTONDOWN:
		SendMessage(parent,SHOWMENU,0,0);
		break;
	case WM_LBUTTONDOWN:
		{
			GetCursorPos(&p);
			SetTimer(hWnd,MoveGadget,10,NULL);
			int x=LOWORD(lParam); 
			int y=HIWORD(lParam);	

			POINT jugada;
			if(!TerminoJuego())
			{
				if(DamePunto(x,y))
				{
					matriz[casillaActual.y][casillaActual.x]=1;
					cantJugadas++;
					jugada=Jugada();
					if(jugada.x!=-1)
					{					
						matriz[jugada.y][jugada.x]=2;
						cantJugadas++;
					}
				}
			}
			InvalidateRect(hWnd,NULL,true);
		break;
		}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		actual=hdc;
		compa=CreateCompatibleDC(hdc); compaCero=CreateCompatibleDC(hdc); compaCruz=CreateCompatibleDC(hdc);
		SelectObject(compa,hBitmap); SelectObject(compaCruz,hCruz); SelectObject(compaCero,hCero);
		StretchBlt(hdc,0,0,180,180,compa,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);//para pintar el fondo

		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
			{
				if(matriz[i][j]==1)
				{
					StretchBlt(hdc,XINI+ANCHOCASILLA*j+SEPAR*j,YINI+i*ALTOCASILLA+i*SEPAR,ANCHOCASILLA,ALTOCASILLA,compaCruz,0,0,bmCruz.bmWidth,bmCruz.bmHeight,SRCCOPY);
				}
				if(matriz[i][j]==2)
				{
					StretchBlt(hdc,XINI+ANCHOCASILLA*j+SEPAR*j,YINI+i*ALTOCASILLA+i*SEPAR,ANCHOCASILLA,ALTOCASILLA,compaCero,0,0,bmCero.bmWidth,bmCero.bmHeight,SRCCOPY);
				}
			}

		DeleteDC(compa);
		DeleteDC(compaCero);
		DeleteDC(compaCruz);

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd,1);
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//metodos que exporta la dll
EXPORT void GetGadgetName(TCHAR* nombre)
{
   nombre=TEXT("TicTacToe");
}

EXPORT HICON GetGadgetIcon()
{
	return LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1));
}

EXPORT BOOL GadgetHasSettings ()
{
	return false;
}

EXPORT void ShowGadgetSettingsDlg ()
{
	return;
}

EXPORT HWND GetGadgetWindow (HWND hwnd)//preguntar por el HWND
{	
	parent = hwnd;
	MyRegisterClass(hInst);
	hWndGlobal = InitInstance(hInst);
	return hWndGlobal;
}

bool DamePunto(int x, int y)
{
	x-=7; //porque la imagen tiene un borde
	y-=5;
	x=x/ANCHOCASILLA;
	y=y/ALTOCASILLA;
	if(x<3 && y<3 && matriz[y][x]==0)
	{
		casillaActual.x=x;
		casillaActual.y=y;
		return true;
		
	}
	else return false;
		
}

POINT ChequeaColumnas(int id)
{
	POINT res={-1,-1};
	int peligro;
	for(int i=0;i<3;i++)
	{
		peligro=0;
		for(int j=0;j<3;j++)
		{
			if(matriz[j][i]==0)
			{
				res.x=i; //porque estoy verificando las columnas
				res.y=j;
			}
			if(matriz[j][i]==id)
				peligro++;
			
		}
		if(peligro==2 && res.x==i) //para saber si  tengo una casilla vacia en la misma columna del peligro
			return res;
	}
	res.x=-1;
	res.y=-1;
	return res;
}

POINT ChequeaDiagonal(int id)
{
	POINT res={-1,-1};
	int peligro=0;
	bool cogi=false;
	for(int i=0;i<3;i++)
	{
		if(matriz[i][i]==0)
		{
			res.x=i;
			res.y=i;
			cogi=true;
		}
		if(matriz[i][i]==id)
			peligro++;
	}
	if(peligro==2 && res.x!=-1) //para saber si tengo una casilla vacia en la diagonal
		return res;
	peligro=0;
	cogi=false;
	int j=0;
	for(int i=2;i>=0;i--,j++)
	{
		if(matriz[i][j]==0)
		{
			res.x=j;
			res.y=i;
			cogi=true;
		}
		if(matriz[i][j]==id)
			peligro++;
	}
	if(peligro!=2)
	{
		res.x=-1;
		res.y=-1;	
	}
	return res;
}

POINT ChequeaFilas(int id)
{
	POINT res={-1,-1};
	int peligro;
	for(int i=0;i<3;i++)
	{
		peligro=0;
		for(int j=0;j<3;j++)
		{
			if(matriz[i][j]==0)
			{
				res.x=j;
				res.y=i;
			}
			if(matriz[i][j]==id)
				peligro++;
		}
		if(peligro==2 && res.y==i) //para saber si tengo una casilla vacia en la misma fila del peligro
			return res;
	}
	res.x=-1;
	res.y=-1;
	return res;
}

POINT JugadaInteligente()
{
	POINT res={-1,-1};
	int posib;
	for(int i=0;i<3;i++)
	{
		posib=0;
		for(int j=0;j<3;j++)
		{
			if(posib==1 && matriz[i][j]==0)
			{
				res.x=j;
				res.y=i;
				
				return res;
			}
			if(matriz[i][j]==2)
				posib++;
		}
	}
	for(int i=0;i<3;i++)
	{
		posib=0;
		for(int j=0;j<3;j++)
		{
			if(posib==1 && matriz[i][j]==0)
			{
				res.x=j;
				res.y=i;
				return res;
			}
			if(matriz[j][i]==2)
				posib++;
		}
	}
	posib=0;
	for(int i=0;i<3;i++)
	{
		if(posib==1 && matriz[i][i]==0)
		{
			res.x=i;
			res.y=i;
			return res;
		}
		if(matriz[i][i]==2)
			posib++;
	}
	return res;
}

POINT HazJugadaAtaque()
{
	POINT res={-1,-1};
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			if(matriz[i][j]==0)
			{
				res.x=j;
				res.y=i;
				return res;
			}
	return res;
}

POINT ChequeaAmenaza()
{
	POINT res=ChequeaColumnas(1);
	if(res.x!=-1) return res;
	res=ChequeaFilas(1);
	if(res.x!=-1) return res;
	res=ChequeaDiagonal(1);
	return res;
}

POINT ChequeaAtaque()
{
	POINT res=ChequeaColumnas(2);
	if(res.x!=-1) return res;
	res=ChequeaFilas(2);
	if(res.x!=-1) return res;
	res=ChequeaDiagonal(2);
	
	return res;
}

POINT Jugada()
{	
	POINT res=ChequeaAtaque(); //si tengo 2 ceros juntos
	if(res.x!=-1) {gane=true; return res;}
	res=ChequeaAmenaza(); //si hay 2 cruces juntas
	if(res.x!=-1) return res;
	res=JugadaInteligente();
	if(res.x!=-1) return res;
	res=HazJugadaAtaque();
	//if(res.x!=-1) return res;
	return res;
}

bool TresEnLinea(int id)
{
	int peligro;
	//para chequear las filas
	for(int i=0;i<3;i++)
	{
		peligro=0;
		for(int j=0;j<3;j++)
		{
			if(matriz[i][j]==id)
				peligro++;
		}
		if(peligro==3)
			return true;
	}

	//para chequear las columnas
	for(int i=0;i<3;i++)
	{
		peligro=0;
		for(int j=0;j<3;j++)
		{
			if(matriz[j][i]==id)
				peligro++;
		}
		if(peligro==3)
			return true;
	}

	//para chequear la diagonal 1
	peligro=0;
	for(int i=0;i<3;i++)
	{
		if(matriz[i][i]==id)
			peligro++;
	}
	if(peligro==3)
		return true;

	//para chequear la diagonal 2
	int j=0;
	peligro=0;
	for(int i=2;i>=0;i--,j++)
	{
		if(matriz[i][j]==id)
			peligro++;
	}
	return peligro==3;
}

bool Perdi()
{
	if(TresEnLinea(1))
	{
		perdi=true;
		gane=false;
	}
	return perdi;
}

bool Gane()
{
	if(TresEnLinea(2)) 
	{
		gane=true;
		perdi=false;
	}
	return gane;
}

bool TerminoJuego()
{
	return (cantJugadas==9 || Perdi() || Gane());
}

void InicializaMatriz()
{
	cantJugadas=0;
	//voy a ver quien empieza jugando
	if(!gane && !perdi)
		empiezaUsuario=!empiezaUsuario; //hice tablas
	else
		empiezaUsuario=!gane; //para saber quien empieza
	perdi=false;
	gane=false;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			matriz[i][j]=0;

	//pregunto si empiezo jugando
	if(!empiezaUsuario)
	{
		POINT res;
		res.x=rand()%3;
		res.y=rand()%3;
		matriz[res.y][res.x]=2;
		cantJugadas++;
		InvalidateRect(hWndGlobal,NULL,true);
		//matriz[0][0]=2;
	}
}

void CreaMatriz()
{
	matriz=new int* [3];
	for(int i=0;i<3;i++)
		matriz[i]=new int [3];
	InicializaMatriz();
}
