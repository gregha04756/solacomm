#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"

extern "C++" HINSTANCE g_hInst;
HWND g_hODResetLineWnd;
extern "C++" BOOL bSolaConnected;
extern "C++" DWORD dwCommThreadID;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" TCHAR szTitle[];
//extern "C++" float TempVal( BOOL units, short temp );
//extern "C++" short ssTempVal(BOOL units, short temp);
//extern "C++" float HystVal( BOOL units, short temp );
extern "C++" signed short SolaTempVal( BOOL units, short temp );
extern "C++" signed short SolaHystVal( BOOL units, short temp );
//extern "C++" SOLAMBMAP TrendStatus[];
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" CSolaMBMap* pcCHConfiguration;
extern "C++" CSolaMBMap* pcLLConfig;
extern "C++" CSolaMBMap* pcODResetConfig;

LRESULT CALLBACK ODResetWndProc(HWND, UINT, WPARAM, LPARAM);
//void PlotResetCurve(HWND hWindow, HDC hDeviceContext,BOOL bUnits, SOLAODRCURVEPT LodHwt, SOLAODRCURVEPT HodLwt);


template <typename T>
T TempVal (BOOL units,T temp)
{
	return ((((9*temp)/50)+32)*(units == 0))+((temp/10)*(units == 1));
}

template <typename T>
T HystVal (BOOL units,T temp)
{
	return (((9*temp)/50)*(units == 0))+((temp/10)*(units == 1));
}

void PlotResetCurve(HWND& hWindow,HDC& hDeviceContext,BOOL bUnits,SOLAODRCURVEPT LodHwt,SOLAODRCURVEPT HodLwt)
{
	float fODTemp;
	float fSetpoint;
	float fStep;
	HRESULT hRes;
	HGDIOBJ hgo;
	TCHAR szTemp[8];
	const TCHAR szXLabelF[] = _T("OD Temperature F");
	const TCHAR szYLabelF[] = _T("Setpoint F");
	const TCHAR szXLabelC[] = _T("OD Temperature C");
	const TCHAR szYLabelC[] = _T("Setpoint C");
	TEXTMETRIC tm;
	BOOL bResult;
	int xaxisoffset = 0;
	int yaxisoffset = 0;
	int i;
	int j;
	int x;
	int y;
	int nXDiv;
	int nYDiv;
	int nTickLength;
	int nXAxLen;
	int nYAxLen;

	const double xaofactor = 0.1;
	const double yaofactor = 0.075;
	const double xaefactor = 1.05;
	const double yaefactor = 1.05;
	double dXscale;
	double dYscale;
	double temp;
	RECT crect;
	POINT pt;
	PLOGFONT plf;


	if ( (HodLwt.x-LodHwt.x) < 10 )
	{
		::MessageBox(hWindow, _T("Outdoor delta T too small"),szTitle,MB_OK);
		return;
	}
	if ( (LodHwt.y-HodLwt.y) < 10 )
	{
		::MessageBox(hWindow, _T("Water delta T too small"),szTitle,MB_OK);
		return;
	}
	plf = (PLOGFONT) new LOGFONT;
	plf->lfHeight = -::MulDiv(8,::GetDeviceCaps(hDeviceContext,LOGPIXELSY),72);
	plf->lfWidth = 0;
	plf->lfOrientation = 0;
	plf->lfEscapement = 0;
	plf->lfWeight = FW_NORMAL;
	plf->lfItalic = false;
	plf->lfCharSet = ANSI_CHARSET;
	plf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	plf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	plf->lfQuality = DEFAULT_QUALITY;
	plf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	plf->lfStrikeOut = false;
	plf->lfUnderline = false;
	plf->lfItalic = false;
	hRes = ::StringCchCopy(plf->lfFaceName, 8, _T("Verdana"));
	HFONT hfnt = ::CreateFontIndirect(plf);
	HFONT hfntPrev = (HFONT) ::SelectObject(hDeviceContext, hfnt);
	bResult = ::GetTextMetrics(hDeviceContext, &tm);
	bResult = GetClientRect(hWindow,&crect);
	xaxisoffset = tm.tmHeight*4;
	yaxisoffset = tm.tmAveCharWidth*12;
	nXAxLen =  (crect.right-crect.left)-(2*yaxisoffset);
	nYAxLen = (crect.bottom-crect.top)-(2*xaxisoffset);
	dXscale = ((double)nXAxLen)/(TempVal(bUnits,(double)HodLwt.x)-TempVal(bUnits,(double)LodHwt.x));
	dYscale = ((double)nYAxLen)/(TempVal(bUnits,(double)LodHwt.y)-TempVal(bUnits,(double)HodLwt.y));
	temp = ((double)LodHwt.x)*dXscale;
	// Draw axes
	bResult = ::MoveToEx(hDeviceContext,yaxisoffset,crect.bottom-xaxisoffset,&pt);
	bResult = ::LineTo(hDeviceContext,yaxisoffset,crect.top+(xaxisoffset/2));
	bResult = ::MoveToEx(hDeviceContext,yaxisoffset,crect.bottom-xaxisoffset,&pt);
	bResult = ::LineTo(hDeviceContext,crect.right-(yaxisoffset/2),crect.bottom-xaxisoffset);
	// Draw axes labels and scales
	if ( 0 == bUnits )
	{
		x = ((crect.right-crect.left)-(tm.tmAveCharWidth*(sizeof(szXLabelF)/sizeof(TCHAR))))/2;
		bResult = ::TextOut(hDeviceContext,x,crect.bottom-tm.tmHeight,szXLabelF,(sizeof(szXLabelF)-1)/sizeof(TCHAR));
	}
	if ( 1 == bUnits )
	{
		x = ((crect.right-crect.left)-(tm.tmAveCharWidth*(sizeof(szXLabelC)/sizeof(TCHAR))))/2;
		bResult = ::TextOut(hDeviceContext,x,crect.bottom-tm.tmHeight,szXLabelC,(sizeof(szXLabelC)-1)/sizeof(TCHAR));
	}
	// Draw tick marks on X axis
	y = crect.bottom - xaxisoffset;
	nTickLength = tm.tmHeight;
	x = crect.left + yaxisoffset;
	fStep = ((bUnits==0)*5.0F)+((bUnits==1)*2.0F);
	if ( (6*tm.tmAveCharWidth) > 0 )
	{
		nXDiv = ((95*nXAxLen)/100)/(6*tm.tmAveCharWidth);
	}
	if ( nXDiv > 0 )
	{
		fStep = (float)HystVal(bUnits,(float)(HodLwt.x-LodHwt.x))/(float)nXDiv;
	}
	fODTemp = TempVal(bUnits,(float)LodHwt.x);
	for ( i = 0; fODTemp < TempVal(bUnits,(float)HodLwt.x)+fStep; i++ )
	{
		bResult = ::MoveToEx(hDeviceContext,x,y,&pt);
		bResult = ::LineTo(hDeviceContext,x,y+nTickLength);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%5.1f"),(float)fODTemp);
		hRes = ::StringCchLength(szTemp,sizeof(szTemp)/sizeof(TCHAR),(size_t*)&j);
		bResult = ::TextOut(hDeviceContext,x-((j*tm.tmAveCharWidth)/2),y+nTickLength,szTemp,j);
		x += (int)(((double)fStep)*dXscale);
		fODTemp += fStep;
	}
	hgo = ::SelectObject(hDeviceContext, hfntPrev);
	bResult = ::DeleteObject(hfnt);
	// Label Y axis
	plf->lfOrientation = 900;
	plf->lfEscapement = 900;
	hfnt = ::CreateFontIndirect(plf);
	hfntPrev = (HFONT) ::SelectObject(hDeviceContext, hfnt);
	bResult = ::GetTextMetrics(hDeviceContext, &tm);
	x = crect.left;
	y = crect.bottom - xaxisoffset - (nYAxLen/2);
	if ( 0 == bUnits )
	{
		bResult = ::TextOut(hDeviceContext,x,y,szYLabelF,(sizeof(szYLabelF)-1)/sizeof(TCHAR));
	}
	if ( 1 == bUnits )
	{
		bResult = ::TextOut(hDeviceContext,x,y,szYLabelC,(sizeof(szYLabelC)-1)/sizeof(TCHAR));
	}
	hgo = ::SelectObject(hDeviceContext, hfntPrev);
	bResult = ::DeleteObject(hfnt);
	// Draw tick marks on Y axis
	plf->lfOrientation = 0;
	plf->lfEscapement = 0;
	hfnt = ::CreateFontIndirect(plf);
	hfntPrev = (HFONT) ::SelectObject(hDeviceContext, hfnt);
	bResult = ::GetTextMetrics(hDeviceContext, &tm);
	x = yaxisoffset;
	y = crect.bottom - xaxisoffset;
	fStep = ((bUnits==0)*5.0F)+((bUnits==1)*2.0F);
	if ( (2*tm.tmHeight) > 0 )
	{
		nYDiv = ((95*nYAxLen)/100)/(2*tm.tmHeight);
	}
	if ( nYDiv > 0 )
	{
		fStep = HystVal(bUnits,(float)(LodHwt.y-HodLwt.y))/(float)nYDiv;
	}
	fSetpoint = TempVal(bUnits,(float)HodLwt.y);
	for ( i = 0; fSetpoint < TempVal(bUnits,(float)LodHwt.y)+fStep; i++ )
	{
		bResult = ::MoveToEx(hDeviceContext, x, y, &pt);
		bResult = ::LineTo(hDeviceContext,x-nTickLength,y);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%4.1f"),(float)fSetpoint);
		hRes = ::StringCchLength(szTemp,sizeof(szTemp)/sizeof(TCHAR),(size_t*)&j);
		bResult = ::TextOut(hDeviceContext,x-nTickLength-(j*tm.tmAveCharWidth),y-(tm.tmHeight/2),szTemp,j);
		y -= (int)(((double)fStep)*dYscale);
		fSetpoint += fStep;
	}
	hgo = ::SelectObject(hDeviceContext, hfntPrev); 
	bResult = ::DeleteObject(hfnt);
	// Draw reset line
	x = yaxisoffset;
	y = crect.bottom - xaxisoffset - (int)(dYscale*HystVal(bUnits,(double)(LodHwt.y-HodLwt.y)));
	bResult = ::MoveToEx(hDeviceContext,x,y,&pt);
	x = yaxisoffset + (int)(dXscale*HystVal(bUnits,(double)(HodLwt.x-LodHwt.x)));
	y = crect.bottom - xaxisoffset;
	bResult = ::LineTo(hDeviceContext,x,y);
	delete plf;
}



DWORD WINAPI ODResetLineThread(LPVOID lpParam)
{
	TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
	TCHAR szODResetLineTitle[MAX_LOADSTRING];			// the main window class name

 	// TODO: Place code here.
	MSG msg;
	WNDCLASSEX wcex;

	HWND hParent = ((LPODRESETLINEPARMS)lpParam)->hODResetWnd;
	delete (LPODRESETLINEPARMS)lpParam;

	// Initialize global strings
	LoadString(g_hInst, IDS_ODRESETLINEWND, szWindowClass, MAX_LOADSTRING);
	HRESULT hRes = ::StringCchPrintf(szODResetLineTitle,sizeof(szODResetLineTitle)/sizeof(TCHAR),_T("%s ODR Line"),szTitle);

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= ODResetWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SOLACOMM));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if ( !RegisterClassEx(&wcex) )
	{
		return FALSE;
	}

	g_hODResetLineWnd = CreateWindow(	szWindowClass,
										szODResetLineTitle,
										WS_OVERLAPPED | WS_SYSMENU,
										100, 100, 500, 325, NULL, NULL, g_hInst, NULL);

	if (!g_hODResetLineWnd)
	{
		return FALSE;
	}

	ShowWindow(g_hODResetLineWnd, SW_SHOW);
	UpdateWindow(g_hODResetLineWnd);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

//	g_hODResetLineWnd = NULL;
	BOOL bResult = ::UnregisterClass(szWindowClass,g_hInst);
	LRESULT lRes = ::PostMessage(hParent,WM_APPODRESETLINEQUIT,(WPARAM)0,(LPARAM)0);

	return (int) msg.wParam;
}

LRESULT CALLBACK ODResetWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static TCHAR szNewTitle[MAX_LOADSTRING];
	HRESULT hRes;
	BOOL bResult;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	const int btnwidth = 50;
	const int btnht = 20;
	const TCHAR szODRLineName[2][3] = {_T("CH"),_T("LL")};
	static int nODRLineNum;
	static int nNextODRLineNum;
	static HWND hBtn;
	RECT crect;

	switch (message)
	{
	case WM_CREATE:
		{
			RECT crect;
			bResult = ::GetClientRect(hWnd,&crect);
			hBtn = ::CreateWindow(	_T("BUTTON"),   // predefined class 
									_T(""),       // button text 
									WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles 
									crect.right-btnwidth,         // starting x position 
									crect.top,         // starting y position 
									btnwidth,        // button width 
									btnht,        // button height 
									hWnd,       // parent window 
									NULL,       // No menu 
									(HINSTANCE) ::GetWindowLong(hWnd, GWL_HINSTANCE), 
									NULL);      // pointer not needed
			if ( !hBtn )
			{
				bResult = ::DestroyWindow(hWnd);
			}
			nODRLineNum = 0;
			nNextODRLineNum = nODRLineNum+1 < sizeof(szODRLineName)/sizeof(szODRLineName[0]) ? nODRLineNum+1 : 0;
			bResult = ::SetWindowText(hBtn,&szODRLineName[nNextODRLineNum][0]);
			hRes = ::StringCchPrintf(szNewTitle,sizeof(szNewTitle)/sizeof(TCHAR),_T("%s %s ODR Line"),szTitle,&szODRLineName[nODRLineNum][0]);
			bResult = ::SetWindowText(hWnd,szNewTitle);
		}
		break;
	case WM_APPODRESETCFGUPD:
		bResult = ::GetClientRect(hWnd,&crect);
		bResult = ::InvalidateRect(hWnd,NULL,true);
		break;
	case WM_APPODRESETLINEQUIT:
		bResult = DestroyWindow(hWnd);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		if ( wmEvent == BN_CLICKED && (HWND)lParam == hBtn )
		{
			nODRLineNum = ++nODRLineNum < sizeof(szODRLineName)/sizeof(szODRLineName[0]) ? nODRLineNum : 0;
			nNextODRLineNum = nODRLineNum+1 < sizeof(szODRLineName)/sizeof(szODRLineName[0]) ? nODRLineNum+1 : 0;
			bResult = ::SetWindowText(hBtn,&szODRLineName[nNextODRLineNum][0]);
			hRes = ::StringCchPrintf(szNewTitle,sizeof(szNewTitle)/sizeof(TCHAR),_T("%s %s ODR Line"),szTitle,&szODRLineName[nODRLineNum][0]);
			bResult = ::SetWindowText(hWnd,szNewTitle);
			bResult = ::GetClientRect(hWnd,&crect);
			bResult = ::InvalidateRect(hWnd,&crect,true);
		}
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			bResult = DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			SOLAODRCURVEPT x1y1, x2y2;
			switch (nODRLineNum)
			{
			case 0:
				x1y1.x = pcODResetConfig->GetValue((unsigned short)0x0201);
				x1y1.y = pcCHConfiguration->GetValue((unsigned short)0x00D3);
				x2y2.x = pcODResetConfig->GetValue((unsigned short)0x0200);
				x2y2.y = pcODResetConfig->GetValue((unsigned short)0x0202);
				break;
			case 1:
				x1y1.x = pcODResetConfig->GetValue((unsigned short)0x0206);
//				x1y1.y = pcTrendStatus->GetLPMap(12)->sValue;
				x1y1.y = pcLLConfig->GetValue((unsigned short)0x0222);
				x2y2.x = pcODResetConfig->GetValue((unsigned short)0x0205);
				x2y2.y = pcODResetConfig->GetValue((unsigned short)0x0207);
			break;
			}
			PlotResetCurve(hWnd, hdc, (BOOL)(pcSystemConfiguration->GetValue((unsigned short)0x00B2)), x1y1, x2y2);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
