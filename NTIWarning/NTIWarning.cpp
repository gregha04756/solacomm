// NTIWarning.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "NTIWarning.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	WarningDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HWND hwndWnd = NULL;
	WNDCLASSEX wcex;
	ATOM atomResult;
	BOOL bResult;
	int nResult;

	// Initialize global strings
	nResult = ::LoadString(hInstance,IDC_NTIWARNING,szWindowClass,MAX_LOADSTRING);
	nResult = ::LoadString(hInstance,IDS_APP_TITLE,szTitle,MAX_LOADSTRING);

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= ::LoadIcon(hInstance,MAKEINTRESOURCE(IDC_NTIWARNING));
	wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)::CreateSolidBrush(RGB(255,0,0));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= ::LoadIcon(wcex.hInstance,MAKEINTRESOURCE(IDI_SMALL));

	atomResult = ::RegisterClassEx(&wcex);

	hInst = hInstance; // Store instance handle in our global variable
	hwndWnd = ::CreateWindow(szWindowClass,
	   szTitle,
	   WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT,
	   0,
	   500,
	   400,
	   NULL,
	   NULL,
	   hInstance,
	   NULL);

	if ( !hwndWnd )
	{
		return (int)-1;
	}

	bResult = ::ShowWindow(hwndWnd,SW_SHOW);
	bResult = UpdateWindow(hwndWnd);

	// Message pump:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
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
LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	BOOL bResult;
	int wmId, wmEvent;
	int nResult;
	PAINTSTRUCT ps;
	HDC hdc;
	static HBITMAP himgWarning;
	HDC hdcMem;
	HBITMAP hbmT;
	BITMAP bm;
	const int aElement = COLOR_ACTIVECAPTION;
    static DWORD aOldColor;
    static DWORD aNewColor;
	static BOOL bToggle;
	const UINT_PTR uipTimerID = 1;
	static UINT_PTR uipTimer;
	RECT cRect;
	static COLORREF crBkClrSv;
	static COLORREF crBkClrNew;
	static COLORREF crBkClrNow;
	static int cxChar;
	static int cyChar;
	static HWND hwndCancelBtn;
	static HWND hwndOKBtn;
	static HANDLE hReplys[2];

	switch (message)
	{
	case WM_CREATE:
		hReplys[0] = ::OpenEvent(EVENT_ALL_ACCESS,false,_T("AcceptEvent"));
		hReplys[1] = ::OpenEvent(EVENT_ALL_ACCESS,false,_T("RejectEvent"));
		cxChar = LOWORD(::GetDialogBaseUnits()) ;
		cyChar = HIWORD(GetDialogBaseUnits()) ;
		bResult = ::GetClientRect(hWnd,&cRect);
		hwndCancelBtn = ::CreateWindowEx(WS_EX_LEFT,
			_T("button"),
			_T("CANCEL"),
			WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
			(cRect.right-cRect.left-(10*cxChar))/4,
			cRect.bottom-(7*cyChar)/4,
			10*cxChar,
			(7*cyChar)/4,
			hWnd,
			(HMENU)IDC_BTNCANCEL,
			((LPCREATESTRUCT)lParam)->hInstance, NULL) ;
		hwndOKBtn = ::CreateWindowEx(WS_EX_LEFT,
			_T("button"),
			_T("ACCEPT"),
			WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
			(3*(cRect.right-cRect.left-(10*cxChar)))/4,
			cRect.bottom-(7*cyChar)/4,
			10*cxChar,
			(7*cyChar)/4,
			hWnd,
			(HMENU)IDC_BTNACCEPT,
			((LPCREATESTRUCT)lParam)->hInstance, NULL) ;
		himgWarning = ::LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP1));
		if ( !himgWarning )
		{
			bResult = ::SetEvent(hReplys[1]);
			bResult = ::DestroyWindow(hWnd);
			break;
		}
		bToggle = false;
		crBkClrSv = RGB(0xff,0x00,0x00);
		crBkClrNew = RGB(0xff,0xff,0x00);
		crBkClrNow = crBkClrSv;
		uipTimer = ::SetTimer(hWnd,uipTimerID,800,NULL);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDC_BTNACCEPT:
			bResult = ::SetEvent(hReplys[0]);
			bResult = ::DeleteObject(himgWarning);
			bResult = ::KillTimer(hWnd,uipTimer);
			bResult = ::DeleteObject((HGDIOBJ)(::SetClassLong(hWnd,GCL_HBRBACKGROUND,(LONG)::CreateSolidBrush(crBkClrNow))));
			bResult = ::DestroyWindow(hWnd);
			break;
		case IDC_BTNCANCEL:
		case IDM_EXIT:
			bResult = ::SetEvent(hReplys[1]);
			bResult = ::DeleteObject(himgWarning);
			bResult = ::KillTimer(hWnd,uipTimer);
			bResult = ::DeleteObject((HGDIOBJ)(::SetClassLong(hWnd,GCL_HBRBACKGROUND,(LONG)::CreateSolidBrush(crBkClrNow))));
			bResult = ::DestroyWindow(hWnd);
			break;
		default:
			return ::DefWindowProc(hWnd,message,wParam,lParam);
		}
		break;
	case WM_TIMER:
		if ( !bToggle )
		{
			crBkClrNow = crBkClrNew;
		}
		if ( bToggle || NULL == uipTimer)
		{
			crBkClrNow = crBkClrSv;
		}
		bResult = ::DeleteObject((HGDIOBJ)::SetClassLong(hWnd,GCL_HBRBACKGROUND,(LONG)::CreateSolidBrush(crBkClrNow))) ;
		bToggle = ~bToggle;
		bResult = ::GetClientRect(hWnd,&cRect);
		bResult = ::InvalidateRect(hWnd,&cRect,true);
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		bResult = ::GetClientRect(hWnd,&cRect);
		hdcMem = ::CreateCompatibleDC(NULL);
		hbmT = SelectBitmap(hdcMem,himgWarning);
		nResult = ::GetObject(himgWarning,sizeof(bm),&bm);
		bResult = ::BitBlt(hdc,(cRect.right-cRect.left-bm.bmWidth)/2,(cRect.bottom-bm.bmHeight)/2,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCCOPY);
		SelectBitmap(hdcMem,hbmT);
		bResult = ::DeleteDC(hdcMem);
		bResult = ::EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		bResult = ::CloseHandle(hReplys[0]);
		bResult = ::CloseHandle(hReplys[1]);
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hWnd,message,wParam,lParam);
	}
	return 0;
}
