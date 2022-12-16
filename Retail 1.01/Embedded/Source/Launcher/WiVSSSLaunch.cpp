// WiVSSSLaunch.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include "resource.h"
#include "WiVSSSLaunch.h"

static	UINT	WM_SSS_DOACTION;
static	DWORD	m_dwAction = 3;
static	HWND	m_hwTodayWnd = NULL;

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	bool	blRetVal = false;
	int		rc;
	HWND	hwDesktopWnd = NULL;
	HKEY	hKey;
	HKEY	hKey1;
	TCHAR	szKey[MAX_PATH + 1];
	TCHAR	szValue[MAX_PATH + 1];
	DWORD	dwType = REG_DWORD;
	DWORD	dwSize = sizeof(DWORD);
	
	
    WM_SSS_DOACTION = RegisterWindowMessage(SSS_WM_DOACTION);

	_snwprintf(szKey, MAX_PATH, _T("Software"));
	
	// Open [HKLM/Software] key
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		ExitProcess(0);	
		return 0;
	}
	
	_snwprintf(szKey, MAX_PATH, _T("WiViT\\SIM Status Switcher\\Options"));
	
	// Open full [HKLM/Software/WiViT/SIM Status Switcher/Options] key
	if (RegOpenKeyEx (hKey, szKey, 0, 0, &hKey1) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		ExitProcess(0);	
		return 0;
	}
	
	// Get the [HKLM/Software/WiViT/SIM Status Switcher/Options/ButtonAction] value
	DWORD dwResult = 0;
	
	_snwprintf(szValue, MAX_PATH, _T("ButtonAction"));
	dwResult = RegQueryValueEx(hKey1, szValue, NULL, &dwType, (LPBYTE)&m_dwAction, &dwSize);

	if (dwResult != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey1);
		RegCloseKey(hKey);
		ExitProcess(0);	
		return 0;
	}

	RegCloseKey(hKey1);
	RegCloseKey(hKey);

	// Get desktop window
	hwDesktopWnd = FindWindow(TEXT("DesktopExplorerWindow"),TEXT("Desktop"));;

	// Go find our Today window
	m_hwTodayWnd = NULL;
	EnumerateWindows(hwDesktopWnd);

	if (m_hwTodayWnd != NULL)
	{
		SendMessage(m_hwTodayWnd, WM_SSS_DOACTION, (WPARAM)m_dwAction, 0);
		ExitProcess(1);	
		return 1;
	}

	ExitProcess(0);	
	return 0;
}

void EnumerateWindows(HWND hWndParent)
{
	if (HWND hwChild = ::GetWindow(hWndParent, GW_CHILD)) do
	{
		TCHAR	szClass[MAX_PATH];

		GetClassName(hwChild, szClass, MAX_PATH);

		if (_tcscmp(szClass, _T("SSSTodayClass")) == 0)
		{
			m_hwTodayWnd = hwChild;
			return;
		}

		EnumerateWindows(hwChild);

	} while (hwChild = ::GetWindow(hwChild, GW_HWNDNEXT));

	return;
} 
