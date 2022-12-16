//////////////////////////////////////////////////////////////////////
//
// WiVUtils.h: Interface for the CWiVUtils class.
//
//////////////////////////////////////////////////////////////////////
#ifndef INC_WIV_UTILS_H
#define INC_WIV_UTILS_H

#include <windows.h>

namespace WiV
{

class CWiVUtils
{

// Friend functions
friend bool			TransposeDlg(const HINSTANCE hInst, const HWND hDlg, LPCWSTR iddTemplate);
//friend int			GetTodayItemFlags();
friend SSS_GLOBALS	*GlobalsLoad(SSS_GLOBALS * pSSSGlobals, const bool blInit = false);

#ifdef WIV_DEBUG
#endif

private:

void			FixupDialogItem(const HINSTANCE hInst, const HWND hDlg,
								LPDLGITEMTEMPLATE lpDlgItem,
								LPWORD lpClass, LPWORD lpData);
LPBYTE			WalkDialogData(const LPBYTE lpData);


#ifdef WIV_DEBUG
#endif

public:

// Constructors/Destructors
CWiVUtils();
~CWiVUtils();
CWiVUtils(HINSTANCE hmInstance);

#ifdef WIV_DEBUG
#endif

}; // class CWiVUtils

// Global non-member functions

bool			IsSecondEdition();
bool			InWideMode();
bool			IsTapAndHold (HWND hWnd, POINT pt, bool blNoAnimate = false);
UINT			IDFromIconSet(const DWORD dwIconSet, const UINT uIconType);
//bool			TransposeDlg(const HINSTANCE hInst, const HWND hDlg, LPCWSTR iddTemplate);
HBRUSH			GetBkBrush( const HWND hWnd, const UINT nID, HBITMAP hBmBk );
RECT			DrawStaticText(HWND hWnd, const UINT nID, LPCTSTR lpszText,
							   const DWORD dwFlags,
							   const DWORD dwFontWeight = FW_NORMAL,
							   const DWORD dwStyle = WIV_FONT_NONE,
							   const COLORREF crColour = GetSysColor(COLOR_STATICTEXT));
RECT			AppendStaticText(HWND hWnd, RECT rect, LPCTSTR lpszText,
								 const DWORD dwFlags,
								 const DWORD dwFontWeight = FW_NORMAL,
								 const DWORD dwStyle = WIV_FONT_NONE,
								 const COLORREF crColour = GetSysColor(COLOR_STATICTEXT));
void			ClearStaticText(HWND hWnd, const UINT nID,
								const HBRUSH hBackColour = GetSysColorBrush(COLOR_STATIC));
void			GetStaticRect(HWND hStatic, LPRECT lpRect,
							  long lVOffset = WIV_OPTIONS_TITLE_BAR_HEIGHT,
							  long lHOffset = 0);
HWND			ItemHandleFromID(const HWND hWnd, const UINT nID);
void			DrawTitle(HMODULE hmInstance, HWND hWnd, HDC hDC, HFONT hfFont,
						  LPCTSTR szTitle, const bool bAllowCancel = false);
void			DrawCancelIcon(const HMODULE hmInstance, const HDC hDC,
							   const bool blDepressed = false);
HFONT			GetTitleFont(HWND hWnd);
bool			GetFont(HFONT *hFont, const HWND hWnd, const LONG lWeight,
						DWORD dwStyle = WIV_FONT_NONE);
DWORD			GetWindowStyle(const HWND hWnd);
bool			IsButtonChecked(const HWND hWnd, const UINT nID);
void			SetCheckButton(const HWND hWnd, const UINT nID, bool bCheck = TRUE);
void			SetRadioButton(const HWND hWnd, const UINT nIDFirst, const UINT nIDLast,
								const UINT nIDToCheck);
BOOL			PointInControl(const HWND hWnd, const UINT uID, LPARAM lParam);
void			GetDrawRect(const HWND hWnd, const UINT uiID, LPCTSTR lpszString,
							LPRECT lpRect, UINT uAlignment = DT_CENTER);
LPCSTR			UtoA(LPCTSTR szUnicode, LPSTR szAscii, const int nLen = -1);
LPCTSTR			AtoU(LPCSTR szAscii, LPTSTR szUnicode, const int nLen = -1);
LPCTSTR			BtoS(const LPBYTE lpbData, const int nLength,
					 LPCTSTR lpszPad = WIV_EMPTY_STRING);

int				GetJulianDay(const int nDay, const int nMonth, const int nYear);
int				GetJulianDate(const int nDay, const int nMonth, const int nYear);

//void			GenCRC32Table();
//DWORD			CalcCRC32(LPBYTE lpbBuffer, UINT uiLen, DWORD dwCurrentCRC);

//SSS_GLOBALS		*GlobalsLoad(SSS_GLOBALS * pSSSGlobals, const bool blInit = false);
int				GlobalLoadStrings();
int				GlobalLoadDwords();
int				GlobalLoadOthers();
LPCTSTR			GlobalGetString(const UCHAR uIndex);
const DWORD		GlobalGetDword(const UCHAR uIndex);
const DWORD		GlobalGetOther(const UCHAR uIndex);

LPCTSTR			GetInstallPath();
void			SetInstallPath(LPCTSTR lpszInstallPath);
LPCTSTR			GetWindowsPath();
void			SetWindowsPath(LPCTSTR lpszWindowsPath);
LPCTSTR			GetTodayItemName();
void			SetTodayItemName(LPCTSTR lpszTodayItemName);
//int				GetTodayItemFlags();

#ifdef WIV_DEBUG
#endif

#ifdef WIV_ENCRYPT_BLD
bool			WiVGenEncryptedData();
#endif

} // namespace WiV

#endif // INC_WIV_UTILS_H
