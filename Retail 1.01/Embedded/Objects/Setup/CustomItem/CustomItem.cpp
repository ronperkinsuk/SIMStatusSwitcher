// CustomItem.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#define INVALID_HEIGHT			0xFFFFFFFF

//global variables	
HICON				g_hIcon;
UINT				g_plugInHeight;
HINSTANCE			g_hInst;
HWND				g_hWnd;
BOOL				g_bFirstDisplay 		= TRUE;
HWND				g_hBatteryStatic;
HWND				g_hStorageStatic;
HWND				g_hProgramStatic;
HWND				g_hBatteryProgressBar;
HWND				g_hStorageProgressBar;
HWND				g_hProgramProgressBar;
BYTE				g_bBatteryPercentage;
UINT				g_StorageMemUsed;
UINT				g_ProgramMemUsed;
BOOL				g_bSelected = FALSE;
int					g_nSelectedItem = 0;


LRESULT CALLBACK WndProc (HWND hwnd, UINT uimessage, WPARAM wParam, LPARAM lParam) ;

// forward function declarations
static INT InitilizeClasses();
static void RefreshStatuses();

/*************************************************************************/
/* Entry point for the dll												 */
/*************************************************************************/
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:			
		
		// The DLL is being loaded for the first time by a given process.
		// Perform per-process initialization here.  If the initialization
		// is successful, return TRUE; if unsuccessful, return FALSE.
		g_hInst = (HINSTANCE)hModule;
		
		//load the icon
		g_hIcon = (HICON)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_DISPLAYICON),IMAGE_ICON,16,16,LR_DEFAULTCOLOR );   
		
		//initilize the application class, and set the global window handle
		UnregisterClass((LPCTSTR)LoadString(g_hInst, IDS_TODAY_CUSTOM_ITEM_APPNAME,0,0), g_hInst);
		InitilizeClasses();
		g_hWnd = 0;
		
		break;
		
	case DLL_PROCESS_DETACH:
		// The DLL is being unloaded by a given process.  Do any
		// per-process clean up here, such as undoing what was done in
		// DLL_PROCESS_ATTACH.	The return value is ignored.
		
		DestroyIcon(g_hIcon);
		
		UnregisterClass((LPCTSTR)LoadString(g_hInst, IDS_TODAY_CUSTOM_ITEM_APPNAME,0,0), g_hInst);
		g_hInst = NULL;
		break;		   
	}
	
	return TRUE;
}

/*************************************************************************/
/* Initilize the DLL by creating a new window							 */
/*************************************************************************/
HWND InitializeCustomItem(TODAYLISTITEM *ptli, HWND hwndParent) 
{
	LPCTSTR appName = (LPCTSTR)LoadString(g_hInst,IDS_TODAY_CUSTOM_ITEM_APPNAME,0,0);
	
	//create a new window
	g_hWnd = CreateWindow(appName,appName,WS_VISIBLE | WS_CHILD,
		CW_USEDEFAULT,CW_USEDEFAULT,240,0,hwndParent, NULL, g_hInst, NULL) ;
	
	// create the battery percentage stuff
	g_hBatteryStatic = CreateWindow(TEXT("STATIC"), TEXT("Battery:"), 
		WS_CHILD, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
		,g_hWnd, NULL, g_hInst, NULL);
	g_hBatteryProgressBar = CreateWindow(PROGRESS_CLASS, TEXT("Battery Progress Bar"), 
		WS_CHILD | PBS_SMOOTH, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
		,g_hWnd, NULL, g_hInst, NULL);
	SendMessage(g_hBatteryProgressBar,PBM_SETSTEP,1,NULL); 

	// create the program memory progress bar
	g_hProgramStatic = CreateWindow(TEXT("STATIC"), TEXT("Memory:"), 
		WS_CHILD, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
		,g_hWnd, NULL, g_hInst, NULL);
	g_hProgramProgressBar = CreateWindow(PROGRESS_CLASS, TEXT("Program Progress Bar"), 
		WS_CHILD | PBS_SMOOTH, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
		,g_hWnd, NULL, g_hInst, NULL);
	SendMessage(g_hProgramProgressBar,PBM_SETSTEP,1,NULL); 

	// create the storage space progress bar
	g_hStorageStatic = CreateWindow(TEXT("STATIC"), TEXT("Storage:"), 
		WS_CHILD, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
		,g_hWnd, NULL, g_hInst, NULL);
	g_hStorageProgressBar = CreateWindow(TRACKBAR_CLASS, TEXT("Storage Track Bar"), 
		WS_CHILD | TBS_AUTOTICKS | TBS_BOTTOM, 
		CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
		,g_hWnd, NULL, g_hInst, NULL);	
	SendMessage(g_hStorageProgressBar, TBM_SETRANGE, 1, MAKELPARAM(0,100));
	
	// attach our winproc to the newly created window
	SetWindowLong(g_hWnd, GWL_WNDPROC, (LONG) WndProc);
	
	//display the window
	ShowWindow (g_hWnd, SW_SHOWNORMAL);
	UpdateWindow (g_hWnd) ;  
	
	return g_hWnd;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT uimessage, WPARAM wParam, LPARAM lParam) 
{
	switch (uimessage)
	{		  
	case WM_TODAYCUSTOM_QUERYREFRESHCACHE: 
		{
			MEMORYSTATUS MemStatus;
			STORE_INFORMATION StoreInfo;
			INT iItemHeight;
			SYSTEM_POWER_STATUS_EX BatteryStatus;
			
			UINT iCurrentStore;
			DOUBLE dCurrentStore;
			BOOL	bReturn = FALSE;
			
			
			TODAYLISTITEM *ptli = (TODAYLISTITEM *)wParam;


			if (WaitForSingleObject(SHELL_API_READY_EVENT, 0) == WAIT_TIMEOUT)
			{
				return FALSE;
			}
			
			BOOL bRes = GetSystemPowerStatusEx(&BatteryStatus,TRUE);
			if ( bRes )
				g_bBatteryPercentage = BatteryStatus.BatteryLifePercent;
			else
				g_bBatteryPercentage = 0;
	
			iItemHeight = 40;
			
			GlobalMemoryStatus(&MemStatus);
			if(g_ProgramMemUsed != MemStatus.dwMemoryLoad)
			{
				g_ProgramMemUsed = MemStatus.dwMemoryLoad;
				bReturn = TRUE;
			}
			
			GetStoreInformation(&StoreInfo);
			dCurrentStore = ((((DOUBLE)StoreInfo.dwStoreSize - (DOUBLE)StoreInfo.dwFreeSize) / (DOUBLE)StoreInfo.dwStoreSize) * 100);
			iCurrentStore = (INT)dCurrentStore;
			
			if(g_StorageMemUsed != iCurrentStore)
			{
				g_StorageMemUsed = iCurrentStore;
				bReturn = TRUE;
			}
			
			if (NULL == ptli)
				bReturn = TRUE;
			
			if (0 == ptli->cyp)
			{
				ptli->cyp = iItemHeight;
				bReturn = TRUE;
			}
			
			return bReturn;
	
		}		
		
	case WM_CREATE: 		
		break;
		
	case WM_LBUTTONUP: 
		{
			RECT rcBattery;
			SetRect(&rcBattery,20,4,89,30);
			RECT rcMem;
			SetRect(&rcMem,90,4,150,30);
			RECT rcStorage;
			SetRect(&rcStorage,160,4,220,30);
			POINT point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);

			PROCESS_INFORMATION pi;
			if (PtInRect(&rcBattery, point))
				::CreateProcess(_T("\\Windows\\ctlpnl.exe"), _T("cplmain.cpl,3"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
			else if (PtInRect(&rcMem, point))
				::CreateProcess(_T("\\Windows\\ctlpnl.exe"), _T("cplmain.cpl,4"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
			else if (PtInRect(&rcStorage, point))
				::CreateProcess(_T("\\Windows\\ctlpnl.exe"), _T("cplmain.cpl,4,1"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
			else
				MessageBox(NULL, TEXT("Detected Screen Tap!"), TEXT("Custom Today Item:"), MB_OK);
		}
		break;		  
		
	case WM_PAINT: 
		PAINTSTRUCT 	ps;
		RECT			rcWindBounds; 
		RECT			rcMyBounds;
		HDC 			hDC;
		HFONT			hFontOld;
		TCHAR			szTextBuffer[32];
		COLORREF		crText;

		RefreshStatuses();

		GetWindowRect( hwnd, &rcWindBounds); 
		
		hDC = BeginPaint(hwnd, &ps);
		
		// create a custom rectangle relative to the client area
		rcMyBounds.left = 0;
		rcMyBounds.top = 0;
		rcMyBounds.right = rcWindBounds.right - rcWindBounds.left;
		rcMyBounds.bottom = rcWindBounds.bottom - rcWindBounds.top; 		 
		
		SetBkMode(hDC, TRANSPARENT);
		DrawIcon(hDC, 2, 0, g_hIcon);
		
		LOGFONT lf;
		HFONT hSysFont;
		HFONT hFont;

		hSysFont = (HFONT) GetStockObject(SYSTEM_FONT);
		GetObject(hSysFont, sizeof(LOGFONT), &lf);
		lf.lfWeight = FW_NORMAL;
		lf.lfHeight = (long) -((6.0 * (double)GetDeviceCaps(hDC, LOGPIXELSY) / 72.0)+.5);

		hFont = CreateFontIndirect(&lf);
	
		hFontOld = (HFONT) SelectObject(hDC, hFont);

		crText = SendMessage(GetParent(hwnd), TODAYM_GETCOLOR, (WPARAM) TODAYCOLOR_TEXT, NULL);

		SetTextColor(hDC, crText);

		if ( g_bBatteryPercentage <= 100 )
			wsprintf(szTextBuffer, TEXT("Battery: %i%%"), g_bBatteryPercentage);
		else
			wsprintf(szTextBuffer, TEXT("Battery: AC"));
		SetWindowText(g_hBatteryStatic,szTextBuffer);
		SetWindowPos(g_hBatteryStatic, g_hWnd, 
			rcMyBounds.left + 20, 
			rcMyBounds.top + 4, 
			60, 10, SWP_SHOWWINDOW);
		::SendMessage(g_hBatteryStatic,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(TRUE,0));
		SendMessage(g_hBatteryProgressBar, PBM_SETPOS, g_bBatteryPercentage, NULL);
		SetWindowPos(g_hBatteryProgressBar, g_hWnd, 
			rcMyBounds.left + 20, 
			rcMyBounds.top + 20, 
			60, 10, SWP_SHOWWINDOW);
		
		//////////////////////// MEMORY STATUS
		wsprintf(szTextBuffer, TEXT("Memory: %i%%"), g_ProgramMemUsed);
		SetWindowText(g_hProgramStatic,szTextBuffer);
		SetWindowPos(g_hProgramStatic, g_hWnd, 
			rcMyBounds.left + 90, 
			rcMyBounds.top + 4, 
			60, 10, SWP_SHOWWINDOW);
		::SendMessage(g_hProgramStatic,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(TRUE,0));
		
		// update and paint program status bar
		SendMessage(g_hProgramProgressBar, PBM_SETPOS, g_ProgramMemUsed, NULL);
		SetWindowPos(g_hProgramProgressBar, g_hWnd, 
			rcMyBounds.left + 90, 
			rcMyBounds.top + 20, 
			60, 10, SWP_SHOWWINDOW);
		
		//////////////////////// STORAGE STATUS
		wsprintf(szTextBuffer, TEXT("Storage: %i%%"), g_StorageMemUsed);
		SetWindowText(g_hStorageStatic,szTextBuffer);
		SetWindowPos(g_hStorageStatic, g_hWnd, 
			rcMyBounds.left + 160, 
			rcMyBounds.top + 4, 
			60, 10, SWP_SHOWWINDOW);
		::SendMessage(g_hStorageStatic,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(TRUE,0));
		
		// update and paint storage track bar
		SendMessage(g_hStorageProgressBar, TBM_SETPOS, TRUE, g_StorageMemUsed);
		SetWindowPos(g_hStorageProgressBar, g_hWnd, 
			rcMyBounds.left + 160, 
			rcMyBounds.top + 20, 
			60, 10, SWP_SHOWWINDOW);

		SelectObject(hDC, hFontOld);

		DeleteObject(hFont);

		EndPaint(hwnd, &ps);
		break;
		
	case WM_DESTROY :		  
		return 0 ;
		
	case WM_ERASEBKGND:
		TODAYDRAWWATERMARKINFO dwi;
		dwi.hdc = (HDC)wParam;
		GetClientRect(hwnd, &dwi.rc);
		
		dwi.hwnd = hwnd;
		SendMessage(GetParent(hwnd), TODAYM_DRAWWATERMARK, 0,(LPARAM)&dwi);
		return TRUE;

		// Just for fun...
	case WM_HSCROLL:
		{
			WORD wNotificationCode = LOWORD(wParam);
			if ( wNotificationCode == TB_THUMBPOSITION ||
				 wNotificationCode == TB_THUMBTRACK )
			{
				WORD wPos = HIWORD(wParam);
				TCHAR szBuffer[32];
				wsprintf(szBuffer,TEXT("Pos = %i"),wPos);
				MessageBox(NULL, szBuffer, TEXT("Custom Today Item:"), MB_OK);
			}
		}
		break;

	case WM_TODAYCUSTOM_RECEIVEDSELECTION:
		MessageBox(NULL,TEXT("GOT SELECTION"),TEXT("Custom Today Item:"), MB_OK);
		g_bSelected = TRUE;
		g_nSelectedItem = 1;
		return TRUE;

	case WM_TODAYCUSTOM_USERNAVIGATION:
		MessageBox(NULL,TEXT("GOT USER NAVIGATION"),TEXT("Custom Today Item:"), MB_OK);
		InvalidateRect(hwnd, NULL, FALSE);
		
		if (wParam == VK_UP)   
			g_nSelectedItem--;
		if (wParam == VK_DOWN) 
			g_nSelectedItem++;
		
		if (g_nSelectedItem < 0 || g_nSelectedItem >= 3)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
		
	case WM_TODAYCUSTOM_ACTION:
		MessageBox(NULL,TEXT("GOT ACTION"),TEXT("Custom Today Item:"), MB_OK);
		if ( g_nSelectedItem > 0 && g_nSelectedItem <= 3 )
		{
			PROCESS_INFORMATION pi;
			switch ( g_nSelectedItem )
			{
			case 1:
				::CreateProcess(_T("\\Windows\\ctlpnl.exe"), _T("cplmain.cpl,3"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
				break;
			case 2:
				::CreateProcess(_T("\\Windows\\ctlpnl.exe"), _T("cplmain.cpl,4"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
				break;
			case 3:
				::CreateProcess(_T("\\Windows\\ctlpnl.exe"), _T("cplmain.cpl,4,1"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
				break;
			}
		}
		break;
		
		
	}
	return DefWindowProc (hwnd, uimessage, wParam, lParam) ;
}

/*************************************************************************/
/* Create and register our window class for the today item				 */
/*************************************************************************/
INT InitilizeClasses()
{
	WNDCLASS		 wc; 
	memset(&wc, 0, sizeof(wc));
	
	wc.style		 = 0;				   
	wc.lpfnWndProc	 = (WNDPROC) WndProc;
	wc.hInstance	 = g_hInst;
	wc.hIcon		 = 0;
	wc.hCursor		 = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = (LPCTSTR)LoadString(g_hInst, IDS_TODAY_CUSTOM_ITEM_APPNAME,0,0);
	
	//register our window
	if(!RegisterClass(&wc))
	{ 
		return 0 ;
	}
	return 1;
}


void RefreshStatuses()
{
	MEMORYSTATUS MemStatus;
	STORE_INFORMATION StoreInfo;
	INT iItemHeight;
	SYSTEM_POWER_STATUS_EX BatteryStatus;
	
	UINT iCurrentStore;
	DOUBLE dCurrentStore;
	
	BOOL bRes = GetSystemPowerStatusEx(&BatteryStatus,TRUE);
	if ( bRes )
	{
		if ( BatteryStatus.ACLineStatus == 1 )
			g_bBatteryPercentage = 101;
		else
			g_bBatteryPercentage = BatteryStatus.BatteryLifePercent;
	}
	else
		g_bBatteryPercentage = 0;
	
	// adjust the height of the today item based on showing one or two bars
	iItemHeight = 40;
	
	// determine % memory space and see if it has changed
	GlobalMemoryStatus(&MemStatus);
	if(g_ProgramMemUsed != MemStatus.dwMemoryLoad)
	{
		g_ProgramMemUsed = MemStatus.dwMemoryLoad;
	}
	
	// determine % storage space and see if it has changed
	GetStoreInformation(&StoreInfo);
	dCurrentStore = ((((DOUBLE)StoreInfo.dwStoreSize - (DOUBLE)StoreInfo.dwFreeSize) / (DOUBLE)StoreInfo.dwStoreSize) * 100);
	iCurrentStore = (INT)dCurrentStore;
	
	if(g_StorageMemUsed != iCurrentStore)
	{
		g_StorageMemUsed = iCurrentStore;
	}
}
