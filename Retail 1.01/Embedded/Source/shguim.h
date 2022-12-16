/*++

Copyright (c) 2003 Microsoft Corporation

Module Name:

    shguim.h

Abstract:

    Include file for SHGetUIMetric.

--*/

#ifndef __SHGUIM_H__
#define __SHGUIM_H__

#include <windows.h>

//
// Call RegisterWindowMessage on this string if you are interested in knowing 
// when the UI metrics have changed.  wParam will be 0, lParam will be one of the 
// SHUIMETRICTYPE values to indicate what value has changed.  Call SHGetUIMetrics 
// to find out the new value.
//
#define SH_UIMETRIC_CHANGE    TEXT("SH_UIMETRIC_CHANGE")

//
// Enumeration of metrics you can ask for.  Note that you will only receive a 
// notification for SHUIM_FONTSIZE_POINT when any these three values changes.
//
typedef enum tagSHUIMETRIC
{
    SHUIM_INVALID = 0,          // Illegal
    SHUIM_FONTSIZE_POINT,       // Application font size (hundredths of a point) -- buffer is pointer to DWORD
    SHUIM_FONTSIZE_PIXEL,       // Application font size (in pixels) -- buffer is pointer to DWORD
    SHUIM_FONTSIZE_PERCENTAGE,  // Application font size as percentage of normal -- buffer is pointer to DWORD
} SHUIMETRIC;

typedef HRESULT (*PSHGETUIMETRICS)(SHUIMETRIC, PVOID, DWORD, DWORD*);
 
//////////////////////////////////////////////////////////////////////////////
// FUNCTION: SHGetUIMetrics
//
// PURPOSE: retrieves the shell's UI metrics.  Although this function does not
//     exist in the Pocket PC 2003 SDK, it exists AYGSHELL.DLL in newer 
//     versions of the Pocket PC.  This function simply LoadLibrary's AYGSHELL 
//     and calls the function if it exists.
//
// ON ENTRY:
//     SHUIMETRIC shuim: the metric to retrieve.
//     PVOID pvBuffer: the retrieved data for the metric.
//     DWORD cbBufferSize: the size of pvBuffer (should be sizeof(DWORD)).
//     DWORD* pcbRequired: retrieves the minimum size of the buffer necessary
//        to get the specified system UI metric.  This can be NULL.
//

__inline HRESULT SHGetUIMetrics(SHUIMETRIC shuim, PVOID pvBuffer, DWORD cbBufferSize, DWORD *pcbRequired)
{
	HMODULE hLib;
	hLib = LoadLibrary(_T("AYGSHELL"));
	if (hLib != NULL)
	{
		PSHGETUIMETRICS pSHGetUIMetrics = (PSHGETUIMETRICS)GetProcAddress(hLib, _T("SHGetUIMetrics"));
		
		FreeLibrary(hLib);

		if (pSHGetUIMetrics)
		{
			return pSHGetUIMetrics(shuim, pvBuffer, cbBufferSize, pcbRequired);
		}
	}
    return E_FAIL;
}

#endif
