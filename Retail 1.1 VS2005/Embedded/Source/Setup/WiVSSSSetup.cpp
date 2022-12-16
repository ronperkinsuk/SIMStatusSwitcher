
// Sample CESetup DLL

#include <windows.h>
#include <tchar.h>
#include "WiVSSSSetup.h"


const TCHAR szTITLE[]       = TEXT("WiViT SIM Status Switcher");
const TCHAR szINST_INIT[]   = TEXT("Install_Init\n\nContinue?");
const TCHAR szINST_EXIT[]   = TEXT("Install_Exit\n\nContinue?");
const TCHAR szUNINST_INIT[] = TEXT("Uninstall_Init\n\nContinue?");
const TCHAR szUNINST_EXIT[] = TEXT("Uninstall_Exit");
static HMODULE	m_hmInstance;

BOOL WINAPI DllMain(HANDLE hMod, DWORD dwReason, LPVOID lpvReserved)
{
	m_hmInstance = (HMODULE)hMod;
	return TRUE;
}

codeINSTALL_INIT Install_Init(
  HWND    hwndParent,
  BOOL    fFirstCall,
  BOOL    fPreviouslyInstalled,
  LPCTSTR pszInstallDir)
{
	TCHAR	szTemp[256] = _T("");
	_snwprintf(szTemp, 255, _T("Install to <%s>\r\nContinue?"), pszInstallDir);

	if (IDYES != MessageBox(hwndParent, szTemp, szTITLE, MB_YESNO))
	{
		return codeINSTALL_INIT_CANCEL;
	}

	if (!SetupReg(hwndParent, pszInstallDir, fFirstCall, fPreviouslyInstalled))
	{
		return codeINSTALL_INIT_CANCEL;
	}
	
	return codeINSTALL_INIT_CONTINUE;
}

codeINSTALL_EXIT Install_Exit(
  HWND    hwndParent,
  LPCTSTR pszInstallDir,
  WORD    cFailedDirs,
  WORD    cFailedFiles,
  WORD    cFailedRegKeys,
  WORD    cFailedRegVals,
  WORD    cFailedShortcuts)
{
  if (IDOK == MessageBox(hwndParent, szINST_EXIT, szTITLE, MB_OKCANCEL))
    return codeINSTALL_EXIT_DONE;
  else
    return codeINSTALL_EXIT_UNINSTALL;
}

codeUNINSTALL_INIT Uninstall_Init(HWND hwndParent, LPCTSTR pszInstallDir)
{
  if (IDOK == MessageBox(hwndParent, szUNINST_INIT, szTITLE, MB_OKCANCEL))
    return codeUNINSTALL_INIT_CONTINUE;
  else
    return codeUNINSTALL_INIT_CANCEL;
}

codeUNINSTALL_EXIT Uninstall_Exit(HWND hwndParent)
{
  MessageBox(hwndParent, szUNINST_EXIT, szTITLE, MB_OK);
  return codeUNINSTALL_EXIT_DONE;
}

bool SetupReg(HWND hwParentWnd, LPCTSTR lpszInstallDir, BOOL blFirstCall, BOOL blPreviouslyInstalled)
{
	bool	blRetVal = false;
	int		rc;
	HKEY	hKey;
	HKEY	hKey1;
	HKEY	hKey2;
	TCHAR	szKey[MAX_PATH + 1];
	TCHAR	szValue[MAX_PATH + 1];
	DWORD	dwValue;
	DWORD	dwResult;
	
	_snwprintf(szKey, MAX_PATH, _T("Software"));

	// Open [HKLM/Software] key
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		return false;
	}

	_snwprintf(szKey, MAX_PATH, _T("Microsoft/Today/Items/SIM Status Switcher"));

	// Open full [HKLM/Software/Microsoft/Today/Items/SIM Status Switcher] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("%s\\WiVSSS.dll"), lpszInstallDir);
	dwResult = RegSetValueEx(hKey1, _T("DLL"), NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	dwValue = 1;
	dwResult = RegSetValueEx(hKey1, _T("Options"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 

	dwValue = 4;
	dwResult = RegSetValueEx(hKey1, _T("Type"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 

	dwValue = 1;
	dwResult = RegSetValueEx(hKey1, _T("Selectability"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	
	RegCloseKey(hKey1);
	
	_snwprintf(szKey, MAX_PATH, _T("WiViT/SIM Status Switcher/Version"));

	// Open full [HKLM/Software/WiViT/SIM Status Switcher/Version] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("mailto:support@wivit.com"));
	dwResult = RegSetValueEx(hKey1, _T("SupportURL"), NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	_snwprintf(szValue, MAX_PATH, _T("WiVSSS"));
	dwResult = RegSetValueEx(hKey1, _T("ShortName"), NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	_snwprintf(szValue, MAX_PATH, _T("http://www.wivit.com/products/SSS.shtml"));
	dwResult = RegSetValueEx(hKey1, _T("BuyURL"), NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	_snwprintf(szValue, MAX_PATH, _T("http://www.wivit.com"));
	dwResult = RegSetValueEx(hKey1, _T("WebURL"), NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	_snwprintf(szValue, MAX_PATH, _T("0706"));
	dwResult = RegSetValueEx(hKey1, _T("Build"), NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	_snwprintf(szValue, MAX_PATH, _T("WiVSSS"));
	dwResult = RegSetValueEx(hKey1, _T("1"), NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	_snwprintf(szValue, MAX_PATH, _T("WiVSSS"));
	dwResult = RegSetValueEx(hKey1, _T("01"), NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	RegCloseKey(hKey1);
	RegCloseKey(hKey);
	
	// Open [HKCR] key
	if (RegOpenKeyEx (HKEY_CLASSES_ROOT, NULL, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		return false;
	}

	_snwprintf(szKey, MAX_PATH, _T(".lang"));

	// Open full [HKCR/.lang] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("langfile"));
	dwResult = RegSetValueEx(hKey1, NULL, NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	RegCloseKey(hKey1);

	_snwprintf(szKey, MAX_PATH, _T("langfile"));

	// Open full [HKCR/langfile] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("LANG File"));
	dwResult = RegSetValueEx(hKey1, NULL, NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	_snwprintf(szKey, MAX_PATH, _T("DefaultIcon"));

	// Open full [HKCR/langfile/DefaultIcon] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey1, szKey, 0, NULL, 0, 0, NULL, &hKey2, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey1);
		RegCloseKey(hKey);
		return false;
	}

	RegCloseKey(hKey2);

	_snwprintf(szKey, MAX_PATH, _T("DefaultIcon"));

	// Open full [HKCR/langfile/DefaultIcon] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey1, szKey, 0, NULL, 0, 0, NULL, &hKey2, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey1);
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("\\%s\\WiVSSS.dll,-300"), lpszInstallDir);
	dwResult = RegSetValueEx(hKey2, NULL, NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	RegCloseKey(hKey2);

	_snwprintf(szKey, MAX_PATH, _T("Shell/Open/Command"));

	// Open full [HKCR/langfile/Shell/Open/Command] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey1, szKey, 0, NULL, 0, 0, NULL, &hKey2, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey1);
		RegCloseKey(hKey);
		return false;
	}


	_snwprintf(szValue, MAX_PATH, _T("pword.exe -opendoc %%1"));
	dwResult = RegSetValueEx(hKey, NULL, NULL, REG_SZ, (LPBYTE)szValue, _tcslen(szValue) + 1); 

	RegCloseKey(hKey2);
	RegCloseKey(hKey1);
	RegCloseKey(hKey);
	
	return blRetVal;
}