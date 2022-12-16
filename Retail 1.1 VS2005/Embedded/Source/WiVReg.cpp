//////////////////////////////////////////////////////////////////////
//
// WiVReg.cpp: Implementation of the CWiVReg class.
//
//////////////////////////////////////////////////////////////////////

#include "SSSCommon.h"
#include "WiVUtils.h"
#include "WiVReg.h"

using namespace WiV;

extern struct SSS_GLOBALS *SSSGlobals;

// The one and only instance of CWiVReg
static	CWiVReg	WiVReg;
static	HKEY	m_hkeyRoot;

DWORD CWiVReg::OpenRootKey(PHKEY phKey, LPCTSTR lpszRoot, HKEY hRequiredHive)
{
	int		rc;
	HKEY	hKey, hKey1;
	TCHAR	szKey[WIV_MAX_PATH + 1];
	DWORD	dwResult;
//	HKEY	hHive = HKEY_LOCAL_MACHINE;

	TraceEnter(_D("CWiVReg::OpenRootKey"), tlInternal);

	SSSGlobals = GlobalsLoad(SSSGlobals);

	if (!g_pSSSData)
	{
		TraceError(_D("CWiVReg::OpenRootKey: SSS data is NULL"));
		TraceLeave(_D("CWiVReg::OpenRootKey"), DWORD(0), tlInternal);
		return 0;
	}

	if (hRequiredHive != 0)
	{
		TraceInternal(_D("CWiVReg::OpenRootKey: Hive = 0x%08X, Root key is <%s>"), hRequiredHive, szKey);
//		hHive = hRequiredHive;
		// Setup "CLSID" key name
		_tcsncpy(szKey, g_szClsIDKey, WIV_MAX_PATH);
	}
	else
	{
		TraceInternal(_D("CWiVReg::OpenRootKey: Hive = 0x%08X, Root key is <%s>"), HKEY_LOCAL_MACHINE, szKey);
		// Setup "Software" key name
		_tcsncpy(szKey, g_szSoftwareKey, WIV_MAX_PATH);
	}


	// Open key
	if (RegOpenKeyEx ((hRequiredHive != 0 ? hRequiredHive : HKEY_LOCAL_MACHINE), szKey, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		TraceInternal(_D("CWiVReg::OpenRootKey: Open key <%s> failed, rc = <%08x"), szKey, rc);
		TraceLeave(_D("CWiVReg::OpenRootKey"), DWORD(0), tlInternal);
		return 0;
	}

	// Setup "Company\Product" key name
	if (lpszRoot != NULL)
	{
		_tcsncpy(szKey, lpszRoot, WIV_MAX_PATH);
	}
	else
	{
#ifdef WIV_DEBUG
		_tcsncpy(szKey, g_pSSSData->szCompanyName, (WIV_MAX_PATH - _tcslen(szKey)));
		_tcsncat(szKey, _T("\\"), (WIV_MAX_PATH - _tcslen(szKey)));
		_tcsncat(szKey, g_pSSSData->szProductName, (WIV_MAX_PATH - _tcslen(szKey)));
#else
		_tcsncpy(szKey, g_szCompanyName, (WIV_MAX_PATH - _tcslen(szKey)));
		_tcsncat(szKey, _T("\\"), (WIV_MAX_PATH - _tcslen(szKey)));
		_tcsncat(szKey, g_szProductName, (WIV_MAX_PATH - _tcslen(szKey)));
#endif
	}

	TraceInternal(_D("CWiVReg::OpenRootKey: Key name is <%s>"), szKey);

	// Open full registry key, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szKey, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		CloseRootKey(hKey);
		TraceInternal(_D("CWiVReg::OpenRootKey: Open key <%s> failed, rc = <%08x>"), szKey, rc);
		TraceLeave(_D("CWiVReg::OpenRootKey"), DWORD(0), tlInternal);
		return 0;
	}

	*phKey = hKey1;
	CloseRootKey(hKey);

	TraceInternal(_D("CWiVReg::OpenRootKey: Key opened, phKey is <%08x>, dwResult is <%08x>"), *phKey, dwResult);
	
	TraceLeave(_D("CWiVReg::OpenRootKey"), dwResult, tlInternal);
	return dwResult;

}

DWORD CWiVReg::CloseRootKey(HKEY hKey)
{
	DWORD dwResult;
	TraceEnter(_D("CWiVReg::CloseRootKey"), tlInternal);

	TraceInternal(_D("CWiVReg::CloseRootKey: Calling RegCloseKey for handle <%08x>"), hKey);
	dwResult = RegCloseKey(hKey);
	TraceLeave(_D("CWiVReg::CloseRootKey"), dwResult, tlInternal);
	return dwResult;
}

DWORD CWiVReg::OpenSubKey(HKEY hKey, LPCTSTR szSubKeyName, PHKEY phKey)
{
	int		rc;
	HKEY	hKey1;
	DWORD	dwResult;

	TraceEnter(_D("CWiVReg::OpenSubKey"), tlInternal);
	TraceInternal(_D("CWiVReg::OpenSubKey: Key to open is <%s>"), szSubKeyName);

	// Open the subkey, creating it if it doesn't exist
	if (RegCreateKeyEx (hKey, szSubKeyName, 0, NULL, 0, 0, NULL, &hKey1, &dwResult) != ERROR_SUCCESS)
	{
		rc = GetLastError();
		TraceInternal(_D("CWiVReg::OpenSubKey: Open key <%s> failed, rc = <%08x>"), szSubKeyName, rc);
		TraceLeave(_D("CWiVReg::OpenSubKey"), DWORD(0), tlInternal);
		return 0;
	}

	*phKey = hKey1;

	TraceInternal(_D("CWiVReg::OpenSubKey: Key opened, phKey is <%08x>, dwResult is <%08x>"), *phKey, dwResult);
	
	TraceLeave(_D("CWiVReg::OpenSubKey"), dwResult, tlInternal);
	return dwResult;
}

DWORD CWiVReg::CloseKey(HKEY hKey)
{
	DWORD dwResult;
	TraceEnter(_D("CWiVReg::CloseKey"), tlInternal);

	TraceInternal(_D("CWiVReg::CloseKey: Calling RegCloseKey for handle <%08x>"), hKey);
	dwResult = RegCloseKey(hKey);
	TraceLeave(_D("CWiVReg::CloseKey"), dwResult, tlInternal);
	return dwResult;
}

DWORD CWiVReg::ReadValue(HKEY hKey, LPCTSTR lpszValueName, BYTE * lpbValueData, LPDWORD lpdwValueType, LPDWORD lpdwValueSize)
{
	DWORD	dwResult;

	TraceEnter(_D("CWiVReg::ReadValue"), tlInternal);

	TraceInternal(_D("CWiVReg::ReadValue: Calling RegQueryValueEx for value name <%s>"), lpszValueName);
	dwResult = RegQueryValueEx(hKey, lpszValueName, NULL, lpdwValueType, lpbValueData, lpdwValueSize); 
	TraceInternal(_D("CWiVReg::ReadValue: Back from RegQueryValueEx, dwResult = <%08x>, lpbValueData = <%08x>, dwValueType = <%08x>, dwValueSize = <%08x>"),
					dwResult, *lpbValueData, *lpdwValueType, *lpdwValueSize);

	TraceLeave(_D("CWiVReg::ReadValue"), dwResult, tlInternal);
	return dwResult;
}

DWORD CWiVReg::WriteValue(HKEY hKey,LPCTSTR lpszValueName, BYTE * lpbValueData, DWORD dwValueType, DWORD dwValueSize)
{
	DWORD	dwResult;

	TraceEnter(_D("CWiVReg::WriteValue"), tlInternal);
	
	TraceInternal(_D("CWiVReg::WriteValue: Calling RegSetValueEx for value name <%s>"), lpszValueName);
	dwResult = RegSetValueEx(hKey, lpszValueName, NULL, dwValueType, lpbValueData, dwValueSize); 
	TraceInternal(_D("CWiVReg::ReadValue: Back from RegSetValueEx, dwResult = <%08x>"),	dwResult);

	TraceLeave(_D("CWiVReg::WriteValue"), dwResult, tlInternal);
	return dwResult;
}

DWORD RegKeyOpen(LPCTSTR szKeyName, PHKEY phKey, LPCTSTR lpszRoot, HKEY hRequiredHive)
{
	DWORD dwResult = 0;
	HKEY hKey;

	TraceEnter(_D("RegKeyOpen"), tlInternal);
	TraceInternal(_D("RegKeyOpen: Key to open is <%s>"), szKeyName);

	TraceInternal(_D("RegKeyOpen: Calling OpenRootKey"));
	dwResult =  WiVReg.OpenRootKey(&hKey, lpszRoot, hRequiredHive);
	TraceInternal(_D("RegKeyOpen: Back from OpenRootKey, hKey is <%08x>, dwResult is <%08x>"), hKey, dwResult);

	if (dwResult == REG_CREATED_NEW_KEY)
	{
		TraceInternal(_D("RegKeyOpen: Root key was created"));
		dwResult = ERROR_SUCCESS;
	}
	else
	{
		TraceInternal(_D("RegKeyOpen: Root key was opened"));
		dwResult = ERROR_SUCCESS;
	}

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("RegKeyOpen: Error opening root key <%s>, dwResult = <%08x>"), szKeyName, dwResult);
		TraceLeave(_D("RegKeyOpen"), dwResult, tlInternal);
		return dwResult;
	}

	// If no sub-key provided, return root key handle
	if (szKeyName == NULL)
	{
		*phKey = hKey;
		TraceLeave(_D("RegKeyOpen"), dwResult, tlInternal);
		return dwResult;
	}

	TraceInternal(_D("RegKeyOpen: Calling OpenSubKey"));
	dwResult =  WiVReg.OpenSubKey(hKey, szKeyName, phKey);
	TraceInternal(_D("RegKeyOpen: Back from OpenSubKey, phKey is <%08x>, dwResult is <%08x>"), *phKey, dwResult);

	if (dwResult == REG_CREATED_NEW_KEY)
	{
		TraceInternal(_D("RegKeyOpen: Sub key <%s> was created"), szKeyName);
		dwResult = ERROR_SUCCESS;
	}
	else
	{
		TraceInternal(_D("RegKeyOpen: Sub key <%s> was opened"), szKeyName);
		dwResult = ERROR_SUCCESS;
	}

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("RegKeyOpen: Error opening sub key <%s>, dwResult = <%08x>"), szKeyName, dwResult);
	}

	TraceInternal(_D("RegKeyOpen: Calling CloseRootKey"));
	WiVReg.CloseRootKey(hKey);
	TraceInternal(_D("RegKeyOpen: Back from CloseRootKey"));

	TraceLeave(_D("RegKeyOpen"), dwResult, tlInternal);
	return dwResult;
}

DWORD RegKeyClose(HKEY hKey)
{
	DWORD dwResult;

	TraceEnter(_D("RegKeyClose"), tlInternal);

	TraceInternal(_D("RegKeyClose: Calling WiVReg::CloseKey for handle <%08x>"), hKey);
	dwResult = WiVReg.CloseKey(hKey);

	TraceLeave(_D("RegKeyClose"), dwResult, tlInternal);
	return dwResult;
}

// String
DWORD RegGetValue(HKEY hKey, LPCTSTR lpszValueName, LPTSTR lpszValueData, const DWORD dwValueSize, LPCTSTR szDefaultValue)
{
	DWORD	dwResult;
	BYTE	bData[WIV_MAX_STRING + 1];
	DWORD	dwType = REG_SZ;
	DWORD	dwSize;

	TraceEnter(_D("RegGetValue(String)"), tlInternal);

	dwSize = min(dwValueSize, WIV_MAX_STRING);
	TraceInternal(_D("RegGetValue(String): dwSize = %08X"), dwSize);

	TraceInternal(_D("RegGetValue(String): Calling WiVReg.ReadValue for <%s>"), lpszValueName);
	dwResult = WiVReg.ReadValue(hKey, lpszValueName, bData, &dwType, &dwSize);
	TraceInternal(_D("RegGetValue(String): Back from WiVReg.ReadValue, result = %08X"), dwResult);
	if (dwResult != ERROR_SUCCESS)
	{
		TraceInternal(_D("RegGetValue(String): dwSize should be = %08X"), dwSize);
		if (dwType == REG_SZ)
		{
			if (szDefaultValue != NULL)
			{
				_tcscpy(lpszValueData, szDefaultValue);
				dwResult = ERROR_SUCCESS;
			}
		}
	}
	else
	{
		_tcsncpy(lpszValueData, (LPCTSTR)bData, dwSize);
		TraceInternal(_D("RegGetValue(String): BlpszValueData = <%s>"), lpszValueData);
	}

	TraceLeave(_D("RegGetValue(String)"), dwResult, tlInternal);
	return dwResult;
}

// Boolean
bool RegGetValue(HKEY hKey, LPCTSTR lpszValueName, const bool bDefaultValue)
{
	DWORD	dwResult;
	bool	bData;
	DWORD	dwType = REG_DWORD;
	DWORD	dwSize = sizeof(DWORD);
	DWORD	dwData = 0;

	TraceEnter(_D("RegGetValue(Bool)"), tlInternal);
	dwResult = WiVReg.ReadValue(hKey, lpszValueName, (BYTE *)&dwData, &dwType, &dwSize);

	if (dwResult != ERROR_SUCCESS)
	{
		if (dwType == REG_DWORD)
		{
			bData = bDefaultValue;
		}
	}

	else
	{
	}

	bData = (dwData == 0 ? false : true);

	TraceLeave(_D("RegGetValue(Bool)"), bData, tlInternal);
	return bData;
}

// Double word
DWORD RegGetValue(HKEY hKey, LPCTSTR lpszValueName, const DWORD dwDefaultValue)
{
	DWORD	dwResult;
	DWORD	dwType = REG_DWORD;
	DWORD	dwSize = sizeof(DWORD);
	DWORD	dwData = 0;

	TraceEnter(_D("RegGetValue(DWORD)"), tlInternal);

	dwResult = WiVReg.ReadValue(hKey, lpszValueName, (BYTE *)&dwData, &dwType, &dwSize);

	if (dwResult != ERROR_SUCCESS)
	{
		if (dwType == REG_DWORD)
		{
			dwData = dwDefaultValue;
		}
	}

	TraceLeave(_D("RegGetValue(DWORD)"), dwData, tlInternal);
	return dwData;
}

// Binary
DWORD RegGetValue(HKEY hKey, LPCTSTR lpszValueName, LPBYTE lpbValueData, const DWORD dwValueSize, const LPBYTE lpbDefaultValue)
{
	DWORD	dwResult;
	DWORD	dwType = REG_BINARY;
	DWORD	dwSize;

	TraceEnter(_D("RegGetValue(Binary)"), tlInternal);
	dwSize = min(dwValueSize, WIV_MAX_BINARY);

	dwResult = WiVReg.ReadValue(hKey, lpszValueName, lpbValueData, &dwType, &dwSize);
	TraceInternal(_D("RegGetValue(Binary): Back from ReadValue, dwResult = <%08x>, dwValueType = <%08x>, dwValueSize = <%08x>"),
						dwResult, dwType, dwSize);
	TraceInternal(_D("RegGetValue(Binary): Back from ReadValue, lpbValueData = <%s>"), BtoS(lpbValueData, dwValueSize));
	if (lpbDefaultValue != NULL)
	{
		TraceInternal(_D("RegGetValue(Binary): Back from ReadValue, lpbDefaultValue = <%s>"), BtoS(lpbDefaultValue, dwValueSize));
	}

	if (dwResult != ERROR_SUCCESS)
	{
		if (dwType == REG_BINARY)
		{
			if (lpbDefaultValue != NULL)
			{
				memcpy(lpbValueData, lpbDefaultValue, dwSize);
				dwResult = dwSize;
			}
			else
			{
				dwResult = 0;
			}
		}
		else
		{
			dwResult = dwValueSize;
		}
	}
	else
	{
		dwResult = dwSize;
	}

	TraceLeave(_D("RegGetValue(Binary)"), dwResult, tlInternal);
	return dwResult;
}

// String
DWORD RegSetValue(HKEY hKey, LPCTSTR lpszValueName, LPCTSTR lpszValueData)
{
	DWORD	dwResult = 0;
	DWORD	dwSize = (_tcslen(lpszValueData) + 1) * sizeof(TCHAR);

	TraceEnter(_D("RegSetValue(string)"), tlInternal);
	TraceInternal(_D("RegSetValue(string): lpszValueName= <%s>, lpszValueData = <%s>, size = <%X>"), lpszValueName, lpszValueData, dwSize);

	dwResult = WiVReg.WriteValue(hKey, lpszValueName, (BYTE *)lpszValueData, REG_SZ, dwSize);

	TraceLeave(_D("RegSetValue(string)"), dwResult, tlInternal);

	return dwResult;
}

// Boolean
DWORD RegSetValue(HKEY hKey, LPCTSTR lpszValueName, const bool bValueData)
{
	DWORD	dwResult = 0;
	DWORD	bData = (DWORD)bValueData;

	TraceEnter(_D("RegSetValue(bool)"), tlInternal);
	TraceInternal(_D("RegSetValue(bool): lpszValueName= <%s>, bValueData = <%X>"), lpszValueName, bValueData);

	dwResult = WiVReg.WriteValue(hKey, lpszValueName, (BYTE *)&bData, REG_DWORD, sizeof(DWORD));
	TraceLeave(_D("RegSetValue(bool)"), dwResult, tlInternal);

	return dwResult;
}

// Double word
DWORD RegSetValue(HKEY hKey, LPCTSTR lpszValueName, const DWORD dwValueData)
{
	DWORD	dwResult = 0;
	DWORD	dwData = dwValueData;

	TraceEnter(_D("RegSetValue(DWORD)"), tlInternal);
	TraceInternal(_D("RegSetValue(DWORD): lpszValueName = <%s>, dwValueData = <%X>"), lpszValueName, dwValueData);

	dwResult = WiVReg.WriteValue(hKey, lpszValueName, (BYTE *)&dwData, REG_DWORD, sizeof(DWORD));

	TraceLeave(_D("RegSetValue(DWORD)"), dwResult, tlInternal);

	return dwResult;
}

// Binary
DWORD RegSetValue(HKEY hKey, LPCTSTR lpszValueName, BYTE * lpbValueData, const DWORD dwValueSize)
{
	DWORD	dwResult = 0;

	TraceEnter(_D("RegSetValue(Binary)"), tlInternal);
	TraceInternal(_D("RegSetValue(Binary): lpszValueName = <%s>, dwValueSize = <%X>"), lpszValueName, dwValueSize);
	TraceInternal(_D("RegSetValue(Binary): Value data = <%s>"), BtoS(lpbValueData, dwValueSize));

	dwResult = WiVReg.WriteValue(hKey, lpszValueName, lpbValueData, REG_BINARY, dwValueSize);

	TraceLeave(_D("RegSetValue(Binary)"), dwResult, tlInternal);

	return dwResult;
}
