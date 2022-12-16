//////////////////////////////////////////////////////////////////////
//
// WiVReg.h: Interface for the CWiVReg class.
//
//////////////////////////////////////////////////////////////////////
#ifndef INC_WIV_REG_H
#define INC_WIV_REG_H

#include <windows.h>

class CWiVReg
{

private:

	// Friend functions
	friend DWORD	RegKeyOpen(LPCTSTR szKeyName, PHKEY phKey, LPCTSTR lpszRoot = NULL, HKEY hRequiredHive = 0);
	friend DWORD	RegKeyClose(HKEY hKey);

	friend DWORD	RegGetValue(HKEY hKey, LPCTSTR lpszValueName, LPTSTR lpszValueData, const DWORD dwValueSize, LPCTSTR szDefaultValue = NULL);
	friend bool		RegGetValue(HKEY hKey, LPCTSTR lpszValueName, const bool bDefaultValue);
	friend DWORD	RegGetValue(HKEY hKey, LPCTSTR lpszValueName, const DWORD dwDefaultValue);
	friend DWORD    RegGetValue(HKEY hKey, LPCTSTR lpszValueName, LPBYTE lpbValueData, const DWORD dwValueSize, const LPBYTE lpbDefaultValue = NULL);

	friend DWORD	RegSetValue(HKEY hKey, LPCTSTR lpszValueName, LPCTSTR lpszValueData);
	friend DWORD	RegSetValue(HKEY hKey, LPCTSTR lpszValueName, const bool bValueData);
	friend DWORD	RegSetValue(HKEY hKey, LPCTSTR lpszValueName, const DWORD dwValueData);
	friend DWORD	RegSetValue(HKEY hKey, LPCTSTR lpszValueName, BYTE *lpbValueData, const DWORD dwValueSize);

#ifdef WIV_DEBUG
	friend bool		GetTraceActive();
	friend DWORD	GetTraceLevel();
	friend DWORD	GetTracePath(LPTSTR lpszTracePath);
#endif

	DWORD			OpenRootKey(PHKEY phKey, LPCTSTR lpszRoot = NULL, HKEY hRequiredHive = 0);
	DWORD			CloseRootKey(HKEY hKey);
	DWORD			OpenSubKey(HKEY hKey, LPCTSTR szSubKeyName, PHKEY phKey);
	DWORD			CloseKey(HKEY hKey);
	DWORD			ReadValue(HKEY hKey, LPCTSTR lpszValueName, BYTE * lpbValueData, LPDWORD lpdwValueType, LPDWORD lpdwValueSize);
	DWORD			WriteValue(HKEY hKey,LPCTSTR lpszValueName, BYTE * lpbValueData, DWORD dwValueType, DWORD dwValueSize);

public:

	// Constructor/Destructor
	CWiVReg()
	{
	}

	~CWiVReg()
	{
	}
}; // class CWiVReg

// Global non-member functions

#endif // INC_WIV_REG_H
