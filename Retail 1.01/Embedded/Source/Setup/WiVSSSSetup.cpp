
// WiVSSS Setup DLL

#include <windows.h>
#include <tchar.h>
#include "WiVSSSSetup.h"

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
	if ((cFailedDirs + cFailedFiles + cFailedRegKeys + cFailedRegVals + cFailedShortcuts) != 0)
	{
		return codeINSTALL_EXIT_UNINSTALL;
	}

	if (!SetupReg(hwndParent, pszInstallDir))
	{
		return codeINSTALL_EXIT_UNINSTALL;
	}

	return codeINSTALL_EXIT_DONE;
}

codeUNINSTALL_INIT Uninstall_Init(HWND hwndParent, LPCTSTR pszInstallDir)
{
	if (!CleanReg(hwndParent, pszInstallDir))
	{
		return codeUNINSTALL_INIT_CONTINUE;
	}

	return codeUNINSTALL_INIT_CONTINUE;
}

codeUNINSTALL_EXIT Uninstall_Exit(HWND hwndParent)
{
  return codeUNINSTALL_EXIT_DONE;
}

bool SetupReg(HWND hwParentWnd, LPCTSTR lpszInstallDir)
{
	bool	blRetVal = false;
	int		rc;
	HKEY	hKey;
	HKEY	hKey1;
	HKEY	hKey2;
	TCHAR	szKey[MAX_PATH + 1];
	TCHAR	szValue[MAX_PATH + 1];
	DWORD	dwValue;
	DWORD	dwType;
	DWORD	dwResult;
	
	_snwprintf(szKey, MAX_PATH, _T("Software"));

	// Open [HKLM/Software] key
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		return false;
	}
	_snwprintf(szKey, MAX_PATH, _T("WiViT\\SIM Status Switcher\\Options"));
	
	// Open full [HKLM/Software/WiViT/SIM Status Switcher/Options] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szKey, MAX_PATH, _T("Microsoft\\Today\\Items\\SIM Status Switcher"));

	// Open full [HKLM/Software/Microsoft/Today/Items/SIM Status Switcher] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("%s\\WiVSSS.dll"), lpszInstallDir);
	dwResult = RegSetValueEx(hKey1, _T("DLL"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	dwValue = 1;
	dwResult = RegSetValueEx(hKey1, _T("Options"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 

	dwValue = 4;
	dwResult = RegSetValueEx(hKey1, _T("Type"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 

	dwValue = 1;
	dwResult = RegSetValueEx(hKey1, _T("Selectability"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	
	if (RegQueryValueEx(hKey1,_T("Enabled"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 0;
		dwResult = RegSetValueEx(hKey1, _T("Enabled"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}

	RegCloseKey(hKey1);
	
	_snwprintf(szKey, MAX_PATH, _T("WiViT\\SIM Status Switcher\\Version"));

	// Open full [HKLM/Software/WiViT/SIM Status Switcher/Version] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("mailto:support@wivit.com"));
	dwResult = RegSetValueEx(hKey1, _T("SupportURL"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	_snwprintf(szValue, MAX_PATH, _T("WiVSSS"));
	dwResult = RegSetValueEx(hKey1, _T("ShortName"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	_snwprintf(szValue, MAX_PATH, _T("http://www.wivit.com/products/SSS.shtml"));
	dwResult = RegSetValueEx(hKey1, _T("BuyURL"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	_snwprintf(szValue, MAX_PATH, _T("http://www.wivit.com"));
	dwResult = RegSetValueEx(hKey1, _T("WebURL"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	_snwprintf(szValue, MAX_PATH, SSS_VERSION_MAJOR);
	dwResult = RegSetValueEx(hKey1, _T("Major"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	_snwprintf(szValue, MAX_PATH, SSS_VERSION_MINOR);
	dwResult = RegSetValueEx(hKey1, _T("Minor"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	_snwprintf(szValue, MAX_PATH, SSS_VERSION_BUILD);
	dwResult = RegSetValueEx(hKey1, _T("Build"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 
	
	RegCloseKey(hKey1);

	_snwprintf(szKey, MAX_PATH, _T("WiViT\\SIM Status Switcher\\Options"));
	
	// Open full [HKLM/Software/WiViT/SIM Status Switcher/Options] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey);
		return false;
	}

	if (RegQueryValueEx(hKey1,_T("TodayIconTAHAction"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 3;
		dwResult = RegSetValueEx(hKey1, _T("TodayIconTAHAction"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("TodayIconTapAction"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 2;
		dwResult = RegSetValueEx(hKey1, _T("TodayIconTapAction"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("TAHAction"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 1;
		dwResult = RegSetValueEx(hKey1, _T("TAHAction"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("TapAction"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 0;
		dwResult = RegSetValueEx(hKey1, _T("TapAction"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("ButtonAction"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 3;
		dwResult = RegSetValueEx(hKey1, _T("ButtonAction"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("IconSet"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 0;
		dwResult = RegSetValueEx(hKey1, _T("IconSet"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("Line2BoldFont"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 0;
		dwResult = RegSetValueEx(hKey1, _T("Line2BoldFont"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("Line1BoldFont"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 1;
		dwResult = RegSetValueEx(hKey1, _T("Line1BoldFont"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("SingleLineDisplay"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 0;
		dwResult = RegSetValueEx(hKey1, _T("SingleLineDisplay"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("ShowTSP"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 1;
		dwResult = RegSetValueEx(hKey1, _T("ShowTSP"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("ShowNumber"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		dwValue = 1;
		dwResult = RegSetValueEx(hKey1, _T("ShowNumber"), NULL, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)); 
	}

	if (RegQueryValueEx(hKey1,_T("DefaultLanguageID"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		_snwprintf(szValue, MAX_PATH, _T("ENG"));
		dwResult = RegSetValueEx(hKey1, _T("DefaultLanguageID"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 
	}
	
	if (RegQueryValueEx(hKey1,_T("DefaultLanguageName"), NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
	{
		_snwprintf(szValue, MAX_PATH, _T("UK English"));
		dwResult = RegSetValueEx(hKey1, _T("DefaultLanguageName"), NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 
	}

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
	dwResult = RegSetValueEx(hKey1, NULL, NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

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
	dwResult = RegSetValueEx(hKey1, NULL, NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	_snwprintf(szKey, MAX_PATH, _T("DefaultIcon"));

	// Open full [HKCR/langfile/DefaultIcon] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey1, szKey, 0, NULL, 0, 0, NULL, &hKey2, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey1);
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("%s\\WiVSSS.dll,-300"), lpszInstallDir);
	dwResult = RegSetValueEx(hKey2, NULL, NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	RegCloseKey(hKey2);

	_snwprintf(szKey, MAX_PATH, _T("Shell\\Open\\Command"));

	// Open full [HKCR/langfile/Shell/Open/Command] key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey1, szKey, 0, NULL, 0, 0, NULL, &hKey2, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		RegCloseKey(hKey1);
		RegCloseKey(hKey);
		return false;
	}

	_snwprintf(szValue, MAX_PATH, _T("pword.exe -opendoc %%1"));
	dwResult = RegSetValueEx(hKey2, NULL, NULL, REG_SZ, (LPBYTE)szValue, (_tcslen(szValue) + 1) * sizeof(TCHAR)); 

	RegCloseKey(hKey2);
	RegCloseKey(hKey1);
	RegCloseKey(hKey);
	
	blRetVal = true;	
	return blRetVal;
}

bool CleanReg(HWND hwParentWnd, LPCTSTR lpszInstallDir)
{
	bool	blRetVal = false;
	HKEY	hKey;
	TCHAR	szKey[MAX_PATH + 1];
	TCHAR	szName[MAX_PATH + 1];
	DWORD	dwNameSize = MAX_PATH;

	// Delete [HKCR/.lang] key
	_snwprintf(szKey, MAX_PATH, _T(".lang"));
	RegDeleteKey(HKEY_CLASSES_ROOT, szKey);
	
	// Delete [HKCR/langfile] key
	_snwprintf(szKey, MAX_PATH, _T("langfile"));
	RegDeleteKey(HKEY_CLASSES_ROOT, szKey);
	
	// Delete [HKLM/Software/Microsoft/Today/Items/SIM Status Switcher] key
	_snwprintf(szKey, MAX_PATH, _T("Software\\Microsoft\\Today\\Items\\SIM Status Switcher"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, szKey);
	
	// Open [HKLM/Software/WiViT] key
	_snwprintf(szKey, MAX_PATH, _T("Software\\WiViT"));
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, NULL, NULL, &hKey) != ERROR_SUCCESS)
	{
		return true;
	}

	// Delete [HKLM/Software/WiViT/SIM Status Switcher] key
	_snwprintf(szKey, MAX_PATH, _T("SIM Status Switcher"));
	RegDeleteKey(hKey, szKey);
	
	RegFlushKey(hKey);

	// If no other WiViT products installed, Delete [HKLM/Software/WiViT] key
	if (RegEnumKeyEx(hKey, 0, szName, &dwNameSize, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
	{
		RegCloseKey(hKey);
		_snwprintf(szKey, MAX_PATH, _T("Software\\WiViT"));
		RegDeleteKey(HKEY_LOCAL_MACHINE, szKey);
	}
	else
	{
		RegCloseKey(hKey);
	}
	
	blRetVal = true;
	return blRetVal;
}