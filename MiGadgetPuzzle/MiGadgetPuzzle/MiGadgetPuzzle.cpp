// MiGadgetPuzzle.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MiGadgetPuzzle.h"
#include "resource.h"
#include <time.h>

#define EXPORT extern "C" __declspec (dllexport)


#define MAX_LOADSTRING 100
#define ANCHO1 10
#define ANCHO2 20
#define ALTO1 20
#define ALTO2 30

#define ANCHOTABLA 108
#define ALTOTABLA 108

#define ANCHOCASILLA ANCHOTABLA/4
#define ALTOCASILLA ALTOTABLA/4

#define ID_BOTON_INTERROGACION (HMENU)109
#define ID_BOTON_RIEGA (HMENU)110
#define ID_BOTON_TIEMPO (HMENU)111
#define MoveGadget		21113
#define SHOWMENU 6667

//Mis variables
POINT p;
int xActual,yActual,xDown,yDown;

bool ganoUsuario=false;
bool termino=false;
bool clickApretado=false;
bool revolver=true;
bool apretoInterrogacion=false;
POINT vacio; //cuadricula que esta vacia
int posVacioRandom;
static HBITMAP hFondo; //pa pintar
static HBITMAP hActual;
static BITMAP bmFondo;
static BITMAP bmActual;
int xWindow=1100;
int yWindow=300;
int anchoWindow=130;
int altoWindow=140;

int xClient=8;
int yClient=20;

static POINT matrizOrdenada[16]={{0,0},{0,1},{0,2},{0,3},
								{1,0},{1,1},{1,2},{1,3},
								{2,0},{2,1},{2,2},{2,3},
								{3,0},{3,1},{3,2},{3,3}};

static POINT** miMatriz;
static POINT** matrizRegada;
static int* bitmaps;
static int bitmapActual;
static int inicio;

// Global Variables:
HINSTANCE hInst;
HWND hWndGlobal;
TCHAR* szWindowClass = L"WinApp";			// the main window class name
TCHAR* szTitle=L"Puzzle";
static HWND parent;	

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	DialogProc(HWND, UINT, WPARAM, LPARAM);

bool TocoBoton(int x, int y,HWND hWnd);
void InicializaPuzzle(HWND hWnd);
void RiegaMatriz();
void CreaMatriz();
void InicializaBitmaps();
void Pinta(HDC hdc, HDC fondo,HDC imagen);
void PonNumeros(HWND dialog);
bool EstaDentro(int x, int y, POINT* diClick);
int DamePosDeCuad(POINT diClick);
int EsAdyacenteAlVacio(POINT diClick);
void ActualizaMovimientos(int ady,POINT cuadricula);
bool TerminoPuzzle();

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
   DWORD styles;
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   if(FindWindow(szWindowClass,szTitle))
   {
	   return NULL;
   }

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP  ,
	   CW_USEDEFAULT, 0, 130,138, NULL, NULL, hInstance, NULL);

   SetWindowLong(hWnd,GWL_EXSTYLE,WS_EX_LAYERED);
   SetLayeredWindowAttributes(hWnd,RGB(255,255,255),255,LWA_ALPHA);

   if (!hWnd)
   {
      return FALSE;
   }

   styles = GetWindowLong(hWnd, GWL_STYLE); //en styles se guardan todos los estilos de la ventana
   styles &= ~WS_CAPTION & ~WS_THICKFRAME; //se quitan los estilos que se quiera
   SetWindowLong(hWnd, GWL_STYLE, styles); //se le pasa a la ventana sus nuevos estilos

	SetTimer(hWnd,1,100,NULL);

	ShowWindow(hWnd, SW_SHOW);
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
	wcex.hIcon			= NULL;  
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL; 

	return RegisterClassEx(&wcex);
}

//metodos que exporta la dll
EXPORT void GetGadgetName(TCHAR* nombre)
{
   nombre=TEXT("Puzzle");
}

EXPORT HICON GetGadgetIcon()
{
	return LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1));
}

EXPORT BOOL GadgetHasSettings ()
{
	return true;
}

EXPORT void ShowGadgetSettingsDlg ()
{
	inicio=bitmapActual;
	DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOGIMAGENES),NULL,DialogProc);
}

EXPORT HWND GetGadgetWindow (HWND hwnd)//preguntar por el HWND
{	
	parent = hwnd;
	MyRegisterClass(hInst);
	hWndGlobal = InitInstance(hInst);
	return hWndGlobal;
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
		{				
			hFondo=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_FONDO));
			hActual=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_PAJARO));
			GetObject(hFondo,sizeof(BITMAP),&bmFondo);
			GetObject(hActual,sizeof(BITMAP),&bmActual);
			InicializaPuzzle(hWnd);
			break;
		}
	case WM_TIMER:
		if(wParam==1)
		{
		termino=TerminoPuzzle();
		InvalidateRect(hWnd,NULL,true);
		break;
		}
		if(wParam==MoveGadget)
		{
			GetWindowRect(hWnd , &r);
			GetCursorPos(&p1);
			SetWindowPos(hWnd,NULL,r.left + p1.x - p.x , r.top + p1.y - p.y,0,0,SWP_NOSIZE | SWP_NOZORDER);
			p.x = p1.x; p.y = p1.y;
			InvalidateRect(hWnd ,NULL , FALSE);
			break;
		}
		
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch(wmId)
		{
		case ID_BOTON_INTERROGACION:
			apretoInterrogacion=!apretoInterrogacion;
			InvalidateRect(hWnd,NULL,true);
			break;
		case ID_BOTON_RIEGA:
			revolver=!revolver;
			if(revolver)
				RiegaMatriz();
			else
			{
				ganoUsuario=false;
				//hay que resolver
				int k=0;		
				int a=0;
				int b=0;
				for(int i=0;i<4;i++)
				{
					for(int j=0;j<4;j++,k++)
					{
						matrizRegada[j][i].x=matrizOrdenada[k].x;
						matrizRegada[j][i].y=matrizOrdenada[k].y;
					}
				}
			}
			InvalidateRect(hWnd,NULL,true);
			break;
		case ID_BOTON_TIEMPO:	
			//aqui tengo que poner el tiempo
			//DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOGIMAGENES),hWnd,DialogProc);	
			break;
		}
		break;
	case WM_RBUTTONDOWN:
		SendMessage(parent,SHOWMENU,0,0);
		break;
	case WM_LBUTTONUP:
		//clickApretado=false;
		KillTimer(hWnd,MoveGadget);
		//apretoInterrogacion=false;
		break;
	case WM_LBUTTONDOWN:
		{		
			int x=LOWORD(lParam); 
			int y=HIWORD(lParam);
			
			if(TocoBoton(x,y,hWnd))
				break;
			
			if(((y<ALTO1) || (x<ANCHO1) || (x>ANCHO1+ANCHOTABLA) || (y>ALTO1+ALTOTABLA)))
			{
				GetCursorPos(&p);
				SetTimer(hWnd,MoveGadget,10,NULL);
			}

			if(!apretoInterrogacion)
			{
				POINT cuadricula;
				//cojo los puntos al reves para no confundirme
				if(EstaDentro(x,y,&cuadricula))
				{
					int ady=EsAdyacenteAlVacio(cuadricula);
					//en cuadricula tengo la cudricula donde dio click
					ActualizaMovimientos(ady,cuadricula);							
				}
				InvalidateRect(hWnd,NULL,true);
			}
		}
		
		break;
	/*case WM_MOUSEMOVE:
		if(clickApretado)
		{
			xActual = LOWORD(lParam);
			yActual = HIWORD(lParam);

			p.x +=xActual - xDown;
			p.y +=yActual - yDown;
			SetWindowPos(hWnd,NULL, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		
		break;*/
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
		
			hFondo=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_FONDO));
			hActual=LoadBitmap(hInst,MAKEINTRESOURCE(bitmaps[bitmapActual]));

			HDC compa = CreateCompatibleDC(hdc);
			SelectObject(compa,hFondo);
			HDC compa1 = CreateCompatibleDC(hdc);
			SelectObject(compa1,hActual);

			Pinta(hdc,compa,compa1);

			DeleteDC(compa);
			DeleteDC(compa1);

			EndPaint(hWnd, &ps);
			break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	int wmId, wmEvent;
	HBITMAP hAux;
	
	
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			GetCursorPos(&p);

			//pongo la imagen que esta en en el rompecabezas
			hAux=LoadBitmap(hInst,MAKEINTRESOURCE(bitmaps[bitmapActual]));
			SendDlgItemMessage(hwndDlg,IDC_MOSTRADOR,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hAux);

			//pongo el numero por donde se quedo
			PonNumeros(hwndDlg);

			//para poner la imagen a los botones anterior y siguiente
			hAux=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ANTERIOR));
			SendDlgItemMessage(hwndDlg,IDC_ANTERIOR,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hAux);

			hAux=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SIGUIENTE));
			SendDlgItemMessage(hwndDlg,IDC_SIGUIENTE,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hAux);
			
			SetWindowPos(hwndDlg,NULL,p.x-150 , p.y-250,0,0,SWP_NOSIZE | SWP_NOZORDER);

			break;
		}
	case WM_COMMAND:
		{
			wmId    = LOWORD(wParam);
		    wmEvent = HIWORD(wParam);
			switch(wmId)
			{
				case IDC_ANTERIOR:
					{
						bitmapActual--;
						if(bitmapActual==-1)
							bitmapActual=10;
						hAux=LoadBitmap(hInst,MAKEINTRESOURCE(bitmaps[bitmapActual]));
						SendDlgItemMessage(hwndDlg,IDC_MOSTRADOR,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hAux);

						PonNumeros(hwndDlg);
						break; 
					}
				case IDC_SIGUIENTE:
					{
						bitmapActual++;
						if(bitmapActual==11)
							bitmapActual=0;
					    hAux=LoadBitmap(hInst,MAKEINTRESOURCE(bitmaps[bitmapActual]));
						SendDlgItemMessage(hwndDlg,IDC_MOSTRADOR,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hAux);

						PonNumeros(hwndDlg);
						break;
					}
				case IDOK:
					{
						RiegaMatriz();
						InvalidateRect(GetParent(hwndDlg),NULL,true);
						EndDialog (hwndDlg, FALSE);
						break;
					}
				case IDCANCEL:
					bitmapActual=inicio;
					EndDialog (hwndDlg, FALSE);
					break;
			}
		}
	}
	return false;
}

bool TocoBoton(int x, int y,HWND hWnd)
{
	if(x>=bmFondo.bmWidth-ANCHO2 && y>=5 && y<=5+12 && x<=bmFondo.bmWidth-ANCHO2+12)
	{
		SendMessage(hWnd,WM_COMMAND,(WPARAM)ID_BOTON_RIEGA,0);
		return true;
	}
	if(x>=bmFondo.bmWidth-ANCHO2-17 && y>=5 && y<=5+12 && x<=bmFondo.bmWidth-ANCHO2-17+12)
	{
		SendMessage(hWnd,WM_COMMAND,(WPARAM)ID_BOTON_INTERROGACION,0);
		return true;
	}
	if(x>=ANCHO1+2 && y>=5 && y<=5+12 && x<=ANCHO1+2+12)
	{
		SendMessage(hWnd,WM_COMMAND,(WPARAM)ID_BOTON_TIEMPO,0);
		return true;
	}
	return false;
}

void InicializaPuzzle(HWND hWnd)
{
	//pongo la region del fondo
	HRGN hRgn;
	hRgn = CreateRectRgn(0,0,bmFondo.bmWidth,bmFondo.bmHeight);
	SetWindowRgn(hWnd,hRgn,true);
	
	//pongo los botones
	HWND hBoton;
	hBoton = CreateWindow(TEXT("BUTTON"),TEXT(""),WS_CHILD|BS_BITMAP ,bmFondo.bmWidth-ANCHO2,5,12,12,hWnd,ID_BOTON_RIEGA,hInst,NULL);
	SendMessage(hBoton,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_RIEGA)));
	ShowWindow(hBoton,SW_SHOW);

	hBoton=CreateWindow(TEXT("BUTTON"),TEXT(""),WS_CHILD|BS_BITMAP|BS_PUSHBUTTON ,bmFondo.bmWidth-ANCHO2-17,5,12,12,hWnd,ID_BOTON_INTERROGACION,hInst,NULL);
	SendMessage(hBoton,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_INTERROGACION)));
	ShowWindow(hBoton,SW_SHOW);

	hBoton=CreateWindow(TEXT("BUTTON"),TEXT(""),WS_CHILD|BS_BITMAP|BS_PUSHBUTTON ,ANCHO1+2,5,12,12,hWnd,ID_BOTON_TIEMPO,hInst,NULL);
	SendMessage(hBoton,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_RELOJ)));
	ShowWindow(hBoton,SW_SHOW);

	bitmaps=new int[11];
	InicializaBitmaps();
	bitmapActual=0;
	CreaMatriz();
	RiegaMatriz();
}

void RiegaMatriz()
{
	revolver=true;
	int count=16;
	int aux=0;
	srand((unsigned)time(NULL)); //pongo semilla al random

	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			aux=rand()*100%count;
			matrizRegada[i][j]=matrizOrdenada[aux];
			matrizOrdenada[aux]=matrizOrdenada[--count];
		}
	}

	//para restablecer la matriz ordenada
	POINT temp[16]={{0,0},{0,1},{0,2},{0,3},
					{1,0},{1,1},{1,2},{1,3},
					{2,0},{2,1},{2,2},{2,3},
					{3,0},{3,1},{3,2},{3,3}};

	for(int i=0;i<16;i++)
	{
		matrizOrdenada[i] = temp[i];
	}
}

void CreaMatriz()
{
	matrizRegada = new POINT* [4];
	miMatriz =new POINT* [4];
	for(int i=0; i<4; i++)
	{
		matrizRegada[i] = new POINT[4];
		miMatriz[i] =new POINT[4];
	}
}

void InicializaBitmaps()
{
	bitmaps[0]=IDB_PAJARO;
	bitmaps[1]=IDB_TORTUGA;
	bitmaps[2]=IDB_GIRASOL;
	bitmaps[3]=IDB_ESTRELLA;
	bitmaps[4]=IDB_FLOR;
	bitmaps[5]=IDB_ROBOT;
	bitmaps[6]=IDB_PINTURA;
	bitmaps[7]=IDB_PEZ;
	bitmaps[8]=IDB_CIELO;
	bitmaps[9]=IDB_CAMPO;
	bitmaps[10]=IDB_NUMEROS;
}

void Pinta(HDC hdc, HDC fondo,HDC imagen)
{
	StretchBlt(hdc,0,0,bmFondo.bmWidth,bmFondo.bmHeight,fondo,0,0,bmFondo.bmWidth,bmFondo.bmHeight,SRCCOPY); //pinto el fondo
	if(apretoInterrogacion)
	{
		StretchBlt(hdc,ANCHO1,ALTO1,bmFondo.bmWidth-ANCHO2,bmFondo.bmHeight-ALTO2,imagen,0,0,bmActual.bmWidth,bmActual.bmHeight,SRCCOPY); //pinto la imagen actual
		
	}
	else
	{
		
		//else
		//{
			int k=0;		
			for(int i=0;i<4;i++)
			{
				for(int j=0;j<4;j++)
				{
					if(matrizRegada[i][j].x!=3 || matrizRegada[i][j].y!=3) //para dejar el cuadrado vacio
					{
						StretchBlt(hdc,ANCHO1+j*ANCHOCASILLA,ALTO1+i*ALTOCASILLA,ANCHOCASILLA,ALTOCASILLA,imagen,matrizRegada[i][j].x*bmActual.bmWidth/4,matrizRegada[i][j].y*(bmActual.bmHeight/4),bmActual.bmWidth/4,bmActual.bmHeight/4,SRCCOPY);
					}
					else
					{
						posVacioRandom=k;
						vacio.x=j; //columna
						vacio.y=i; //fila
					}
				}
			}
			for(int i=0;i<5;i++)
			{
				//para pintar las columnas
				MoveToEx(hdc,ANCHO1+i*ANCHOCASILLA,ALTO1,NULL);
				LineTo(hdc,ANCHO1+i*ANCHOCASILLA,ALTO1+ALTOTABLA);
			}

			for(int i=0;i<5;i++)
			{
				//para pintar las filas
				MoveToEx(hdc,ANCHO1,ALTO1+i*ALTOCASILLA,NULL);
				LineTo(hdc,ANCHO1+ANCHOTABLA,ALTO1+i*ALTOCASILLA);
			}
			if(termino)
			{
				TextOut(hdc,ANCHO1+10,ALTO1+10,TEXT("Ha ganado"),10);
			}
		//}
	}
}

void PonNumeros(HWND dialog)
{
	TCHAR *numx1=new TCHAR[3];
	TCHAR *numx2=new TCHAR[3];
	int segDigito=-1;
	int priDigito=bitmapActual+1;
	if(bitmapActual+1==10)
	{
		segDigito=0;
		priDigito=1;
	}
	if(bitmapActual+1==11)
	{
		segDigito=1;
		priDigito=1;
	}
						
	
	HWND primerDig=GetDlgItem(dialog,IDC_STATIC_NUMERO);
	HWND segundoDig=GetDlgItem(dialog,IDC_STATIC_NUMERO2);
	if(segDigito!=-1)
	{
		_itow_s(segDigito,numx2,10,15);//convierte de int a string
							
		SetWindowText(segundoDig,numx2);
	}
	else
	{
		SetWindowText(segundoDig,TEXT(""));
	}
	_itow_s(priDigito,numx1,20,20);//convierte de int a string
	//numx1=numx1+numx2;
	SetWindowText(primerDig,numx1);

	DeleteObject(numx1);
	DeleteObject(numx2);
}

BYTE* Get24BitPixels(HBITMAP pBitmap, WORD *pwWidth, WORD *pwHeight)
{
// a bitmap object just to get bitmap width and height
BITMAP bmpBmp;

// pointer to original bitmap info
LPBITMAPINFO pbmiInfo;

// bitmap info will hold the new 24bit bitmap info
BITMAPINFO bmiInfo;

// width and height of the bitmap
WORD wBmpWidth, wBmpHeight;

// ---------------------------------------------------------
// get some info from the bitmap
// ---------------------------------------------------------
GetObject(pBitmap, sizeof(bmpBmp),&bmpBmp);
pbmiInfo = (LPBITMAPINFO)&bmpBmp;

// get width and height
wBmpWidth = (WORD)pbmiInfo->bmiHeader.biWidth;
wBmpWidth -= (wBmpWidth%4); // width is 4 byte boundary aligned.
wBmpHeight = (WORD)pbmiInfo->bmiHeader.biHeight;

// copy to caller width and height parms
*pwWidth = wBmpWidth;
*pwHeight = wBmpHeight;
// ---------------------------------------------------------

// allocate width * height * 24bits pixels
BYTE *pPixels = new BYTE[wBmpWidth*wBmpHeight*3];
if (!pPixels) return NULL;

// get user desktop device context to get pixels from
HDC hDC = GetWindowDC(NULL);

// fill desired structure
bmiInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
bmiInfo.bmiHeader.biWidth = wBmpWidth;
bmiInfo.bmiHeader.biHeight = -wBmpHeight;
bmiInfo.bmiHeader.biPlanes = 1;
bmiInfo.bmiHeader.biBitCount = 24;
bmiInfo.bmiHeader.biCompression = BI_RGB;
bmiInfo.bmiHeader.biSizeImage = wBmpWidth*wBmpHeight*3;
bmiInfo.bmiHeader.biXPelsPerMeter = 0;
bmiInfo.bmiHeader.biYPelsPerMeter = 0;
bmiInfo.bmiHeader.biClrUsed = 0;
bmiInfo.bmiHeader.biClrImportant = 0;

// get pixels from the original bitmap converted to 24bits
int iRes = GetDIBits(hDC,pBitmap,0,wBmpHeight,(LPVOID)pPixels,&bmiInfo,DIB_RGB_COLORS);

// release the device context
ReleaseDC(NULL,hDC);

// if failed, cancel the operation.
if (!iRes)
{
delete pPixels;
return NULL;
}

// return the pixel array
return pPixels;
}

HRGN Region(HBITMAP pBitmap, BYTE transpR, BYTE transpG, BYTE transpB)
{
  // bitmap width and height
  WORD wBmpWidth,wBmpHeight;  
  // the final region and a temporary region
  HRGN hRgn, hTmpRgn;  
  // 24bit pixels from the bitmap
  BYTE *pPixels = Get24BitPixels(pBitmap, &wBmpWidth, &wBmpHeight);
  
  if (!pPixels) return NULL;  // create our working region
  hRgn = CreateRectRgn(0,0,wBmpWidth,wBmpHeight);
  
  if (!hRgn) { delete pPixels; return NULL; }  
  // ---------------------------------------------------------
  // scan the bitmap
  // ---------------------------------------------------------
  DWORD p=0;
  for (WORD y=0; y<wBmpHeight; y++)
  {
    for (WORD x=0; x<wBmpWidth; x++)
    {
      BYTE jRed   = pPixels[p+2];
      BYTE jGreen = pPixels[p+1];
      BYTE jBlue  = pPixels[p+0];      
	  if (jRed == transpR && jGreen == transpG && jBlue == transpB)
      {
        // remove transparent color from region
        hTmpRgn = CreateRectRgn(x,y,x+1,y+1);
        CombineRgn(hRgn, hRgn, hTmpRgn, RGN_XOR);
        DeleteObject(hTmpRgn);
      }      // next pixel
      p+=3;
    }
  }  // release pixels
  delete pPixels;  // return the region
  return hRgn;
}

bool EstaDentro(int x, int y, POINT* diClick)
{
	if(x>ANCHO1 && x<ANCHO1 + ANCHOTABLA && y>ALTO1 && y<ALTO1+ALTOTABLA)
	{
		x-=ANCHO1;
		y-=ALTO1;
		x=x/(ANCHOCASILLA);
		y=y/(ALTOCASILLA);
		
		diClick->x=x;
		diClick->y=y;
		return (x<4 && y<4) ;
	}
	return false;
}

int DamePosDeCuad(POINT diClick)
{
	return (4*diClick.y)+( 3 -diClick.x) -1;
	
}

int EsAdyacenteAlVacio(POINT diClick)
{
	if((diClick.x==vacio.x && diClick.y==vacio.y) || (diClick.x!=vacio.x && diClick.y!=vacio.y))
		return -1;
	//pregunto si estan en la misma fila y en columnas adyacentes ||  si estan en la misma columna y en filas adyacentes
	if (diClick.y==vacio.y) return 1; //si esta en la misma fila
	if (diClick.x==vacio.x) return 2; //si esta en la misma columna
	return -1;
}

void ActualizaMovimientos(int ady,POINT cuadricula)
{
	if(ady==1)
		{
			//esta en filas iguales
			if(cuadricula.x>vacio.x)
			{
				for(int i=vacio.x;i<cuadricula.x;i++)
				{
					matrizRegada[vacio.y][i]=matrizRegada[vacio.y][i+1];
				}
				matrizRegada[vacio.y][cuadricula.x].x=3;
				matrizRegada[vacio.y][cuadricula.x].y=3;
			}
			else
			{
				for(int i=vacio.x;i>cuadricula.x;i--)
				{
					matrizRegada[cuadricula.y][i]=matrizRegada[cuadricula.y][i-1];
				}
				matrizRegada[cuadricula.y][cuadricula.x].x=3;
				matrizRegada[cuadricula.y][cuadricula.x].y=3;
			}
			vacio=cuadricula;
					
		}
		else 
		{

			if(ady==2)
			{
				//estan en la misma columna
				if(cuadricula.y>vacio.y)
				{

					for(int i=vacio.y;i<cuadricula.y;i++)
					{
						matrizRegada[i][vacio.x]=matrizRegada[i+1][vacio.x];
					}
					matrizRegada[cuadricula.y][cuadricula.x].x=3;
					matrizRegada[cuadricula.y][cuadricula.x].y=3;
				}
			else
			{
				for(int i=vacio.y;i>cuadricula.y;i--)
				{
					matrizRegada[i][cuadricula.x]=matrizRegada[i-1][cuadricula.x];
				}
				matrizRegada[cuadricula.y][cuadricula.x].x=3;
				matrizRegada[cuadricula.y][cuadricula.x].y=3;
			}

			vacio=cuadricula;
			}
		}								
}

bool TerminoPuzzle()
{
	int k=0;
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++,k++)
		{
			if(matrizRegada[j][i].x!=matrizOrdenada[k].x || matrizRegada[j][i].y!=matrizOrdenada[k].y)
				return false;
		}
	return true;
}
