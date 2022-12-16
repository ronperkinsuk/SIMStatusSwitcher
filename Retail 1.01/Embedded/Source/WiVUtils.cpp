//////////////////////////////////////////////////////////////////////
//
// WiVUtils.cpp: Implementation of the CWiVUtils class.
//
//////////////////////////////////////////////////////////////////////

extern struct SSS_GLOBALS *SSSGlobals;

#include "SSSCommon.h"
#include "WiVUtils.h"
#include "WiVLicense.h"
#include "WiVReg.h"
#include "shguim.h"

// DCS - The following missing definition has been added to ril.h
//#define RIL_READYSTATE_SIMPHONEBOOK                     (0x00000010)      // The SIM Phone book is ready for access
#include "ril.h"

namespace WiV
{

static	HMODULE		m_hmInstance;
static	CWiVUtils	*m_pThis;
static 	bool		m_blAutoFontSize = false;
//static	bool		m_blCRCTableEmpty = true;
//static	LPDWORD		m_lpdwCRC32Table = NULL;

static	TCHAR		m_aszStrings[SSS_NUM_STRINGS][WIV_MAX_STRING + 1];
static	DWORD		m_adwDWords[SSS_NUM_DWORDS + 1];
static	DWORD		m_adwOther[SSS_NUM_OTHER + 1];

static	TCHAR		m_szInstallPath[MAX_PATH + 1];
static	TCHAR		m_szWindowsPath[MAX_PATH + 1];
static	TCHAR		m_szTodayItemName[MAX_ITEMNAME + 1];


//======================================================================
// Public methods.
//======================================================================

//----------------------------------------------------------------------
// Default constructor
//----------------------------------------------------------------------
CWiVUtils::CWiVUtils()
{
}

//----------------------------------------------------------------------
// Default destructor
//----------------------------------------------------------------------
CWiVUtils::~CWiVUtils()
{
}

//----------------------------------------------------------------------
// Constructor - Instantiates CWiVUtils object and set the instance.
//----------------------------------------------------------------------
CWiVUtils::CWiVUtils(HINSTANCE hmInstance)
{
	m_hmInstance = hmInstance;
	m_pThis = this;
}

//======================================================================
// Private methods.
//======================================================================

//======================================================================
// Called by TransposeDlg to advance to the next item in the dialog template.
//======================================================================
LPBYTE CWiVUtils::WalkDialogData(const LPBYTE lpData)
{
	TraceEnter(_D("CWiVUtils::WalkDialogData"), tlInternal);

	LPWORD lpWord = (LPWORD)lpData;
	if (*lpWord == 0xFFFF)
	{
		TraceLeave(_D("CWiVUtils::WalkDialogData"), (DWORD)(lpWord + 2), tlInternal);
		return (LPBYTE)(lpWord + 2);
	}
	while (*lpWord != 0x0000)
	{
		lpWord++;
	}

	TraceLeave(_D("CWiVUtils::WalkDialogData"), (DWORD)(lpWord + 1), tlInternal);
	return (LPBYTE)(lpWord + 1);

}

//======================================================================
// Post-processing step for each dialog item.
//    Static controls and buttons: change text and bitmaps.
//    Listboxes and combo boxes: ensures that the selected item is visible.
//======================================================================
void CWiVUtils::FixupDialogItem(const HMODULE hInst, const HWND hDlg, LPDLGITEMTEMPLATE lpDlgItem, LPWORD lpClass, LPWORD lpData)
{
	TraceEnter(_D("CWiVUtils::FixupDialogItem"), tlInternal);

	if (lpClass[0] == 0xFFFF)
	{
		switch (lpClass[1])
		{
			case 0x0080: // button
			case 0x0082: // static
			{
				if (lpData[0] == 0xFFFF)
				{
					if ((lpDlgItem->style & SS_ICON) == SS_ICON)
					{
						HICON hOld = (HICON)SendDlgItemMessageW(hDlg, lpDlgItem->id, STM_GETIMAGE, IMAGE_ICON, 0);
						HICON hNew = LoadIcon(hInst, MAKEINTRESOURCE(lpData[1]));
						SendDlgItemMessageW(hDlg, lpDlgItem->id, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hNew);
						DestroyIcon(hOld);
					}
					else if ((lpDlgItem->style & SS_BITMAP) == SS_BITMAP)
					{
						HBITMAP hOld = (HBITMAP)SendDlgItemMessageW(hDlg, lpDlgItem->id, STM_GETIMAGE, IMAGE_BITMAP, 0);
						HBITMAP hNew = LoadBitmap(hInst, MAKEINTRESOURCE(lpData[1]));
						SendDlgItemMessageW(hDlg, lpDlgItem->id, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNew);
						DeleteObject(hOld);
					}
				}
				else // lpData[0] is not 0xFFFF (it's text).
				{
					SetDlgItemTextW(hDlg, lpDlgItem->id, (LPCTSTR)lpData);
				}
			}
			break;

			case 0x0083: // list box
			{
				INT nSel = SendDlgItemMessageW(hDlg, lpDlgItem->id, LB_GETCURSEL, 0, 0);
				if (nSel != LB_ERR) 
				{
					SendDlgItemMessageW(hDlg, lpDlgItem->id, LB_SETCURSEL, nSel, 0);
				}
			}
			break;

			case 0x0085: // combo box
			{
				INT nSel = SendDlgItemMessageW(hDlg, lpDlgItem->id, CB_GETCURSEL, 0, 0);
				if (nSel != CB_ERR) 
				{
					SendDlgItemMessageW(hDlg, lpDlgItem->id, CB_SETCURSEL, nSel, 0);
				}
			}
			break;
		}
	}

	TraceLeave(_D("CWiVUtils::FixupDialogItem"), tlInternal);

	return;
}

//======================================================================
// Global non-member methods.
//======================================================================

BOOL PointInControl(const HWND hWnd, const UINT uID, LPARAM lParam)
{
	POINT	point;
	RECT	rect;
	BOOL	bResult;

    TraceEnter(_D("PointInControl"), tlInternal);

	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);

	GetStaticRect(ItemHandleFromID(hWnd, uID), &rect);

	bResult = PtInRect(&rect, point);

    TraceLeave(_D("PointInControl"), (DWORD)bResult, tlInternal);

	return bResult;
}

DWORD GetWindowStyle(const HWND hWnd)
{
	DWORD dwStyle;
	TraceEnter(_D("GetWindowStyle"), tlInternal);
	dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	TraceLeave(_D("GetWindowStyle"), dwStyle, tlInternal);
	return dwStyle;
}

bool GetFont(HFONT *hFont, const HWND hWnd, const LONG lWeight, DWORD dwStyle)
{
	LOGFONT lf;
	TEXTMETRIC tm;
	HDC hdc;
//	static HFONT hFont;
	DWORD	dwRequired;
    LONG	dwFontSize = 12;
	
	TraceEnter(_D("GetFont"), tlInternal);

	hdc = GetDC(hWnd);
	GetTextMetrics(hdc, &tm);
	memset(&lf, 0, sizeof (lf));

	if (IsSecondEdition() && m_blAutoFontSize)
	{
	    SHGetUIMetrics(SHUIM_FONTSIZE_PIXEL, &dwFontSize, sizeof(dwFontSize), &dwRequired);
	    lf.lfHeight = -dwFontSize;
	}
	else
	{
		if (dwStyle & WIV_FONT_SMALL)
		{
			lf.lfHeight = -1 * (11 * GetDeviceCaps(hdc, LOGPIXELSY) / 72) / 2;

		}else if (dwStyle & WIV_FONT_MEDIUM)
		{
			lf.lfHeight = -1 * (14 * GetDeviceCaps(hdc, LOGPIXELSY) / 72) / 2;

		}else if (dwStyle & WIV_FONT_LARGE)
		{
			lf.lfHeight = -1 * (20 * GetDeviceCaps(hdc, LOGPIXELSY) / 72) / 2;
		}
		else
		{
			lf.lfHeight = -1 * (17 * GetDeviceCaps(hdc, LOGPIXELSY) / 72) / 2;
		}
	}

//	lf.lfHeight = -1 * (17 * GetDeviceCaps(hdc, LOGPIXELSY) / 72) / 2;
	lf.lfWeight = lWeight;
	lf.lfUnderline = ((dwStyle & WIV_FONT_UNDERLINE) == 0 ? 0 : 1);
	lf.lfItalic = ((dwStyle & WIV_FONT_ITALIC) == 0 ? 0 : 1);
	lf.lfStrikeOut = ((dwStyle & WIV_FONT_STRIKEOUT) == 0 ? 0 : 1);
	lf.lfPitchAndFamily = tm.tmPitchAndFamily;
	lf.lfCharSet = tm.tmCharSet;
	lstrcpy(lf.lfFaceName, _T("Tahoma"));

	*hFont = CreateFontIndirect(&lf);

	ReleaseDC(hWnd, hdc);

	TraceLeave(_D("GetFont"), (DWORD)true, tlInternal);

	return true;
}


bool IsButtonChecked(const HWND hWnd, const UINT nID)
{
	return (BST_CHECKED == SendMessage(ItemHandleFromID(hWnd, nID), BM_GETCHECK, 0, 0));
}

void SetCheckButton(const HWND hWnd, const UINT nID, bool bCheck)
{
	SendMessage(ItemHandleFromID(hWnd, nID), BM_SETCHECK, bCheck ? BST_CHECKED : BST_UNCHECKED, 0);
}

void SetRadioButton(const HWND hWnd, const UINT nIDFirst, const UINT nIDLast, const UINT nIDToCheck)
{
	::CheckRadioButton(hWnd, nIDFirst, nIDLast, nIDToCheck);
}

void GetStaticRect(HWND hStatic, LPRECT lpRect, long lVOffset, long lHOffset)
{
	RECT	rect;

	TraceEnter(_D("GetStaticRect"), tlInternal);

	GetWindowRect(hStatic, &rect);

	lpRect->left = rect.left;
	lpRect->top = rect.top - lVOffset;
	lpRect->right = rect.right;
	lpRect->bottom = rect.bottom - lVOffset;

	TraceLeave(_D("GetStaticRect"), tlInternal);

	return;
}

HBRUSH GetBkBrush( const HWND hWnd, const UINT nID, HBITMAP hBmBk )
{
	HWND hWndCtrl;
	RECT rcCtrl;
	HBRUSH	hBrush;
	HBRUSH	hBrushCtrl = NULL;

	TraceEnter(_D("GetBkBrush"), tlInternal);

	if( NULL == hBmBk )
	{
		hBrush = GetSysColorBrush(COLOR_WINDOW);
		TraceLeave(_D("GetBkBrush"), (DWORD)hBrush, tlInternal);
		return hBrush;
	}

	hWndCtrl = ::GetDlgItem( hWnd, nID );

	if( NULL == hWndCtrl )
	{
		TraceLeave(_D("GetBkBrush"), (DWORD)NULL, tlInternal);
		return NULL;
	}

	GetStaticRect( hWndCtrl, &rcCtrl);

	::ScreenToClient(hWnd, (LPPOINT)&rcCtrl);
	::ScreenToClient(hWnd, ((LPPOINT)&rcCtrl)+1);

	HDC hDC = ::GetDC(hWnd);

	HDC hMemDCBk = CreateCompatibleDC( hDC );
	HDC hMemDCCtrl = CreateCompatibleDC( hDC );

	HBITMAP hBmCtrl = CreateCompatibleBitmap( hDC, _W(rcCtrl), _H(rcCtrl) );

	HBITMAP hBmOldBk;
	HBITMAP hBmOldCtrl;

	hBmOldBk = (HBITMAP) ::SelectObject( hMemDCBk, hBmBk );
	hBmOldCtrl = (HBITMAP) ::SelectObject( hMemDCCtrl, hBmCtrl );

	::BitBlt( hMemDCCtrl, 0, 0, _W(rcCtrl), _H(rcCtrl), hMemDCBk, _X(rcCtrl), _Y(rcCtrl), SRCCOPY );

	hBrushCtrl = ::CreatePatternBrush( hBmCtrl );

	::SelectObject(hMemDCCtrl, hBmOldCtrl );
	::SelectObject(hMemDCBk, hBmOldBk );
	
	::DeleteObject( hBmCtrl );

	::DeleteDC( hMemDCBk );
	::DeleteDC( hMemDCCtrl );
	::ReleaseDC( hWnd, hDC );

	TraceLeave(_D("GetBkBrush"), (DWORD)hBrushCtrl, tlInternal);

	return hBrushCtrl;
}

HFONT GetTitleFont(HWND hWnd)
{
	LOGFONT lf;
	TEXTMETRIC tm;
	HDC hdc;
	HFONT hfFont;
	
	TraceEnter(_D("GetDefaultTitleFont"), tlInternal);

	hdc = GetDC(hWnd);
	GetTextMetrics(hdc, &tm);
	memset(&lf, 0, sizeof (lf));

	lf.lfHeight = -1 * (17 * GetDeviceCaps(hdc, LOGPIXELSY) / 72) / 2;
	lf.lfWeight = FW_SEMIBOLD;
	lf.lfPitchAndFamily = tm.tmPitchAndFamily;
	lstrcpy(lf.lfFaceName, _T("Tahoma"));

	hfFont = CreateFontIndirect(&lf);

	ReleaseDC(hWnd, hdc);

	TraceLeave(_D("GetDefaultTitleFont"), tlInternal);

	return hfFont;
}

void DrawTitle(HMODULE hmInstance, HWND hWnd, HDC hDC, HFONT hfFont, LPCTSTR szTitle, bool bAllowCancel)
{
	TraceEnter(_D("DrawTitle"), tlInternal);

	// Draw title only if it was defined
	if (_tcslen(szTitle) == 0)
	{
		TraceLeave(_D("DrawTitle: Title is blank"), tlInternal);
		return;
	}

	RECT	rect;
	RECT	rectText;

	GetClientRect(hWnd, &rect);

	// Draw text
	HFONT hOldFont = (HFONT)SelectObject(hDC, hfFont);
	SetTextColor(hDC, g_crBlueTitle);

	rectText.left = 8;
	rectText.top = 0;
	rectText.right = _W(rect);
	rectText.bottom = WIV_OPTIONS_TITLE_HEIGHT;
	DrawText(hDC, szTitle, -1, &rectText, DT_VCENTER);
	SelectObject(hDC, hOldFont);

	// Draw the Cancel icon in un-depressed state
	if (bAllowCancel) DrawCancelIcon(NULL, hDC);
	if (bAllowCancel) DrawCancelIcon(hmInstance, hDC, false);

	// And finally underline title
	HPEN hOldPen, hPen = (HPEN)GetStockObject(BLACK_PEN);
	rect.top = rect.top + WIV_OPTIONS_TITLE_HEIGHT;
	rect.bottom = rect.top;

	hOldPen = (HPEN)SelectObject(hDC, hPen);
	Polyline(hDC, (LPPOINT)&rect, 2);
	SelectObject(hDC, hOldPen);

	
	TraceLeave(_D("DrawTitle"), tlInternal);
	return;
}

void DrawCancelIcon(const HMODULE hmInstance, const HDC hDC, const bool blDepressed)
{
	HICON hiIcon = NULL;
	static bool blIsDepressed = true;

	TraceEnter(_D("DrawCancelIcon"), tlInternal);

	if (hmInstance == NULL)
	{
		blIsDepressed = true;
		TraceLeave(_D("DrawCancelIcon"), tlInternal);
		return;
	}

	if (blDepressed == blIsDepressed)
	{
		TraceLeave(_D("DrawCancelIcon"), tlInternal);
		return;
	}

	blIsDepressed = blDepressed;

	TraceInternal(_D("DrawCancelIcon: About to draw Cancel icon"));

	if (blDepressed)
	{
		hiIcon = (HICON)LoadImage(hmInstance, MAKEINTRESOURCE(IDI_ICON_CANCELIN), IMAGE_ICON, 18, 18, LR_DEFAULTCOLOR);
	}
	else
	{
		hiIcon = (HICON)LoadImage(hmInstance, MAKEINTRESOURCE(IDI_ICON_CANCELOUT), IMAGE_ICON, 18, 18, LR_DEFAULTCOLOR);
	}

	TraceInternal(_D("DrawCancelIcon: hiIcon = %08X, state = %d"), hiIcon, blDepressed);

	if (hiIcon)
	{
		BOOL bRes;
		
		TraceInternal(_D("DrawCancelIcon: Drawing Icon"));
		SetBkMode(hDC, TRANSPARENT);
		if (InWideMode())
		{
			bRes = ::DrawIcon(hDC, 298, 3, hiIcon);
		}
		else
		{
			bRes = ::DrawIcon(hDC, 218, 3, hiIcon);
		}

		TraceInternal(_D("DrawCancelIcon: DrawIcon result = %d"), bRes);
		::DestroyIcon(hiIcon);
	}

	TraceLeave(_D("DrawCancelIcon"), tlInternal);
	return;
}

HWND ItemHandleFromID(const HWND hWnd, const UINT nID)
{
	HWND hwIDWnd = NULL;

	TraceEnter(_D("ItemHandleFromID"), tlInternal);

	if (hWnd)
		hwIDWnd = GetDlgItem(hWnd, nID);

	TraceLeave(_D("ItemHandleFromID"), (DWORD)hwIDWnd, tlInternal);
	return hwIDWnd;
}

RECT DrawStaticText(HWND hWnd, const UINT nID, LPCTSTR lpszText, const DWORD dwFlags,
					const DWORD dwFontWeight, const DWORD dwStyle, const COLORREF crColour)
{
	RECT	rect;
	HWND	hStatic;

	TraceEnter(_D("DrawStaticText"), tlInternal);
//	TraceInternal(_D("DrawStaticText <%s>"), lpszText);

	hStatic	= ItemHandleFromID(hWnd, nID);
	GetStaticRect(hStatic, &rect);

	// Draw text only if it was defined
	if (_tcslen(lpszText) == 0)
	{
		TraceLeave(_D("DrawStaticText"), tlInternal);
		return rect;
	}

	TraceLeave(_D("DrawStaticText"), tlInternal);
	return AppendStaticText(hWnd, rect, lpszText, dwFlags, dwFontWeight, dwStyle, crColour);
}

RECT AppendStaticText(HWND hWnd, RECT rect, LPCTSTR lpszText, const DWORD dwFlags,
					const DWORD dwFontWeight, const DWORD dwStyle, const COLORREF crColour)
{
	HDC		hDC;
	HFONT	hOldFont;
	HFONT	hFont;
	int		nOldBk;
	RECT	aRect;

	TraceEnter(_D("AppendStaticText"), tlInternal);

	// Draw text only if it was defined
	if (_tcslen(lpszText) == 0)
	{
		TraceLeave(_D("AppendStaticText"), tlInternal);
		return rect;
	}

	hDC = GetDC(hWnd);
	GetFont(&hFont, hWnd, dwFontWeight, dwStyle);
	
	hOldFont = (HFONT)SelectObject(hDC, hFont);
	nOldBk = GetBkMode(hDC);

	// Draw text
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, crColour);

	aRect = rect;

	DrawText(hDC, lpszText, -1, &aRect, dwFlags | DT_CALCRECT);

	DrawText(hDC, lpszText, -1, &rect, dwFlags);

	rect.left = aRect.right;

	SelectObject(hDC, hOldFont);
	SetBkMode(hDC, nOldBk);
	ReleaseDC(hWnd, hDC);
	if (hFont) DeleteObject(hFont);
	
	TraceLeave(_D("AppendStaticText"), tlInternal);
	return rect;
}

void ClearStaticText(HWND hWnd, const UINT nID, const HBRUSH hBackColour)
{
	HDC		hDC;
	RECT	rect;
	HWND	hStatic;
	int		nOldBk;

	TraceEnter(_D("ClearStaticText"), tlInternal);

	hStatic	= ItemHandleFromID(hWnd, nID);
	GetStaticRect(hStatic, &rect);

	hDC = GetDC(hWnd);

	nOldBk = GetBkMode(hDC);
	SetBkMode(hDC, TRANSPARENT);

	FillRect(hDC, &rect, hBackColour);
	
	SetBkMode(hDC, nOldBk);

	ReleaseDC(hWnd, hDC);

	TraceLeave(_D("ClearStaticText"), tlInternal);
	return;
}

void GetDrawRect(const HWND hWnd, const UINT uiID, LPCTSTR lpszString, LPRECT lpRect, UINT uAlignment)
{
	RECT	rectTxt;
	HDC		hDC;

	TraceEnter(_D("GetDrawRect"), tlInternal);

	GetStaticRect(ItemHandleFromID(hWnd, uiID), lpRect);
	TraceInternal(_D("GetDrawRect: Static: lpRect->.left = %d, lpRect->right = %d, lpRect->top = %d, lpRect->bottom = %d"), lpRect->left, lpRect->right, lpRect->top, lpRect->bottom);

	// Get the height and length of the string.
	memset(&rectTxt, 0, sizeof(RECT));
	rectTxt = *lpRect;
	hDC = GetDC(hWnd);
	DrawText (hDC, lpszString, -1, &rectTxt, DT_CALCRECT | uAlignment | DT_SINGLELINE);
	TraceInternal(_D("GetDrawRect: String = <%s>, rectTxt.left = %d, rectTxt.right = %d, rectTxt.top = %d, rectTxt.bottom = %d"), lpszString, rectTxt.left, rectTxt.right, rectTxt.top, rectTxt.bottom);
	ReleaseDC(hWnd, hDC);

	if (uAlignment == DT_CENTER)
	{
		lpRect->left = (((lpRect->right - lpRect->left) - _W(rectTxt))/2) + 19;
	}

	TraceInternal(_D("GetDrawRect: After mod: lpRect->left = %d, lpRect->right = %d, lpRect->top = %d, lpRect->bottom = %d"), lpRect->left, lpRect->right, lpRect->top, lpRect->bottom);
	
	TraceLeave(_D("GetDrawRect"), tlInternal);
	return;
}

//----------------------------------------------------------------------
// BtoS - Returns a readable unicode string from binary data.
//----------------------------------------------------------------------
LPCTSTR BtoS(const LPBYTE lpbData, register const int nLength, LPCTSTR lpszPad)
{
	static	TCHAR szOut[WIV_MAX_BUFFER + 1] = WIV_EMPTY_STRING;
	TCHAR	szPad[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
	int		nCount = min(nLength, WIV_MAX_BUFFER);

	TraceEnter(_D("BtoS"), tlInternal);

	_zclr(szOut);

	// Convert binary data into 'XXXXXXX...' format string,
	// inserting pad string if specified. e.g. 'X, X, X, X...'
	// is output if ', ' is specified as pad string.
	for (register int i = 0; i < nCount; i++)
	{
		register int len = _tcslen(szOut);
		_snwprintf(&szOut[len], WIV_MAX_BUFFER, _T("%s%02X"), szPad, lpbData[i]);
		if(len > (WIV_MAX_BUFFER - 4))
		{
			break;
		}
		if (i == 0) _tcsncpy(szPad, lpszPad, WIV_MAX_STRING);
	}

	TraceLeave(_D("BtoS"), szOut, tlInternal);

	return szOut;
}

//----------------------------------------------------------------------
// IsSecondEdition - Returns true if WM2003 Second Edition is installed.
//----------------------------------------------------------------------
bool IsSecondEdition()
{
	bool			bResult = false;
	OSVERSIONINFO	osvi;

	TraceEnter(_D("IsSecondEdition"), tlInternal);

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	// If Windows Mobile 2002/2003
	if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_CE) && (osvi.dwMajorVersion == 4))
	{
		// If Windows Mobile 2003 SE
		if (osvi.dwMinorVersion > 20)
		{
			TraceInternal(_D("IsSecondEdition: Windows Mobile 2003 SE"));
			bResult = true;
		}
		else
		{
			TraceInternal(_D("IsSecondEdition: Windows Mobile 2002/2003"));
		}

	// If Windows Mobile 2005
	} else if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_CE) && (osvi.dwMajorVersion == 5))
	{
		TraceInternal(_D("IsSecondEdition: Windows Mobile 2005"));
		bResult = true;
	}
	else
	{
		TraceInternal(_D("IsSecondEdition: Windows Mobile osvi.dwMajorVersion = %d"), osvi.dwMajorVersion);
	}

	TraceLeave(_D("IsSecondEdition"), (DWORD)bResult, tlInternal);
	return bResult;
}

//----------------------------------------------------------------------
// InWideMode - Returns true if in landscape mode
//----------------------------------------------------------------------
bool InWideMode()
{
	bool	bResult = false;
	int		nHeight;

	TraceEnter(_D("InWideMode"), tlInternal);

	if (IsSecondEdition())
	{
		nHeight = GetSystemMetrics(SM_CYSCREEN);
		bResult = (nHeight < 320) ? true : false;
	}

	TraceLeave(_D("InWideMode"), (DWORD)bResult, tlInternal);

	return bResult;
}


//----------------------------------------------------------------------
// IsTapAndHold - Returns true if tap and hold recognised on screen.
//----------------------------------------------------------------------
bool IsTapAndHold (HWND hWnd, POINT pt, bool blNoAnimate)
{
	SHRGINFO	shrg;
	bool		bResult = false;
	DWORD		dwFlags = SHRG_RETURNCMD;
	
	TraceEnter(_D("IsTapAndHold"), tlInternal);

	if (blNoAnimate)
	{
		dwFlags |= SHRG_NOANIMATION;
	}

	shrg.cbSize = sizeof(shrg);
	shrg.hwndClient = hWnd;
    shrg.ptDown.x = pt.x;
    shrg.ptDown.y = pt.y;
	shrg.dwFlags = dwFlags;

	if (SHRecognizeGesture(&shrg) == GN_CONTEXTMENU) 
	{
		TraceInternal(_D("IsTapAndHold: Tap and hold recognised at x = %d, y = %d"), (DWORD)pt.x, (DWORD)pt.y);
		bResult = true;
	}

	TraceLeave(_D("IsTapAndHold"), (DWORD)bResult, tlInternal);

    return bResult; 
}

UINT IDFromIconSet(const DWORD dwIconSet, const UINT uIconType)
{
	return (IDI_PHONE_ICON + (4 * dwIconSet) + uIconType);
}

bool TransposeDlg(const HMODULE hInst, const HWND hDlg, LPCWSTR iddTemplate)
{
	TraceEnter(_D("TransposeDlg"), tlInternal);

	HRSRC hRsrc = FindResource((HMODULE)hInst, iddTemplate, RT_DIALOG);
	if (hRsrc == NULL) 
	{
		TraceLeave(_D("TransposeDlg"), (DWORD)FALSE, tlInternal);
		return FALSE;
	}

	HGLOBAL hGlobal = LoadResource((HMODULE)hInst, hRsrc);
	if (hGlobal == NULL)
	{
		TraceLeave(_D("TransposeDlg"), (DWORD)FALSE, tlInternal);
		return FALSE;
	}

	INT nStatics = 0;
	LPBYTE lpData = (LPBYTE)LockResource(hGlobal);
	LPDLGTEMPLATE lpTemplate = (LPDLGTEMPLATE)lpData;
	HDWP hDWP = BeginDeferWindowPos(lpTemplate->cdit);

	//
	// For more information about the data structures that we are walking,
	// consult the DLGTEMPLATE and DLGITEMTEMPLATE documentation on MSDN.
	//
	lpData += sizeof(DLGTEMPLATE);
	lpData = m_pThis->WalkDialogData(lpData);     // menu
	lpData = m_pThis->WalkDialogData(lpData);     // class
	lpData = m_pThis->WalkDialogData(lpData);     // title

	if (lpTemplate->style & DS_SETFONT)
	{
		lpData += sizeof(WORD);          // font size.
		lpData = m_pThis->WalkDialogData(lpData); // font face.
	}

	for (int i = 0; i < lpTemplate->cdit; i++)
	{
		lpData = (LPBYTE) (((INT)lpData + 3) & ~3);  // force to DWORD boundary.
		LPDLGITEMTEMPLATE lpDlgItem = (LPDLGITEMTEMPLATE)lpData;
		HWND hwndCtl = GetDlgItem(hDlg, lpDlgItem->id);

		if (lpDlgItem->id == 0xFFFF)
		{
			nStatics++;
		}

		//
		// Move the item around.
		//
		{
			RECT r;
			r.left   = lpDlgItem->x;
			r.top    = lpDlgItem->y;
			r.right  = lpDlgItem->x + lpDlgItem->cx;
			r.bottom = lpDlgItem->y + lpDlgItem->cy;
			MapDialogRect(hDlg, &r);
			DeferWindowPos(hDWP, hwndCtl, NULL, 
				r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER);
		}

		lpData += sizeof(DLGITEMTEMPLATE);
		LPWORD lpClass = (LPWORD)lpData;
		lpData = m_pThis->WalkDialogData(lpData);  // class
    
		//
		// Do some special handling for each dialog item (changing text,
		// bitmaps, ensuring visible, etc.
		//
		m_pThis->FixupDialogItem(hInst, hDlg, lpDlgItem, lpClass, (LPWORD)lpData);

		lpData = m_pThis->WalkDialogData(lpData);  // title        
		WORD cbExtra = *((LPWORD)lpData); // extra class data.
		lpData += (cbExtra ? cbExtra : sizeof(WORD));
	}

	EndDeferWindowPos(hDWP);

	TraceLeave(_D("TransposeDlg"), (DWORD)(nStatics < 2 ? TRUE : FALSE), tlInternal);
	return nStatics < 2 ? TRUE : FALSE;
}

// Convert a Unicode string to an ASCII string
LPCSTR UtoA(LPCTSTR szUnicode, LPSTR szAscii, const int nLen)
{
	// Convert from unicode to ascii
	WideCharToMultiByte(CP_ACP, 0, szUnicode, nLen, szAscii, WIV_MAX_BUFFER, NULL, NULL);

	return szAscii;
}

// Convert an ASCII string to a Unicode String
LPCTSTR AtoU(LPCSTR szAscii, LPTSTR szUnicode, const int nLen)
{
	// Convert from ascii to unicode
	MultiByteToWideChar(CP_ACP, 0, szAscii, nLen, szUnicode, WIV_MAX_BUFFER);

	return szUnicode;
}

int GetJulianDay(const int nDay, const int nMonth, const int nYear)
{
	int nJDay;

	TraceEnter(_D("GetJulianDay"), tlInternal);

	nJDay =  GetJulianDate(nDay, nMonth, nYear);

	nJDay -= WIV_EPOCH;

	TraceInternal(_D("GetJulianDay: Julian day for %d/%d/%d = %d (0x%08X)"), nDay, nMonth, nYear, nJDay, nJDay);

	TraceLeave(_D("GetJulianDay"), (DWORD)nJDay, tlInternal);
	return nJDay;
}

// Given Y,M,D, compute the Julian date
int GetJulianDate(const int nDay, const int nMonth, const int nYear)
{
	int nJDay;
	int m1;
	int y1;

	TraceEnter(_D("GetJulianDate"), tlInternal);

	m1 = (nMonth - 14) / 12;
	y1 = nYear + 4800;

	nJDay = (1461 * (y1+m1) / 4);
	nJDay += (367 * (nMonth - 2 - 12 * m1) / 12);
	nJDay -= ((3 * ((y1 + m1 + 100) / 100)) / 4);
	nJDay += nDay;
	nJDay -= 32075;

	TraceInternal(_D("GetJulianDate: Julian date for %d/%d/%d = %d (0x%08X)"), nDay, nMonth, nYear, nJDay, nJDay);
	
	TraceLeave(_D("GetJulianDate"), (DWORD)nJDay, tlInternal);
	return nJDay;
}			

// Get Today Item Name
LPCTSTR GetTodayItemName()
{
	return m_szTodayItemName;
}

// Set Today Item Name
void SetTodayItemName(LPCTSTR lpszTodayItemName)
{
	_tcsncpy(m_szTodayItemName, lpszTodayItemName, MAX_ITEMNAME);
}

// Get Install Path
LPCTSTR GetInstallPath()
{
	return m_szInstallPath;
}

// Set Install Path
void SetInstallPath(LPCTSTR lpszInstallPath)
{
	_tcsncpy(m_szInstallPath, lpszInstallPath, MAX_PATH);
}

// Get DLL Path
LPCTSTR GetWindowsPath()
{
	return m_szWindowsPath;
}

// Set DLL Path
void SetWindowsPath(LPCTSTR lpszWindowsPath)
{
	_tcsncpy(m_szWindowsPath, lpszWindowsPath, MAX_PATH);
}

// Get Globals
SSS_GLOBALS * GlobalsLoad(SSS_GLOBALS * pSSSGlobals, const bool blInit)
{
	DWORD	dwCount		= sizeof(m_biBuf);
	BOOL	blResult;

	TraceEnter(_D("GlobalsLoad"), tlInternal);

	if (!blInit && (pSSSGlobals != NULL) && (pSSSGlobals == &SSSGlobalData))
	{
		TraceLeave(_D("GlobalsLoad"), (DWORD)&SSSGlobalData, tlInternal);
		return &SSSGlobalData;
	}

	memset(&SSSGlobalData, 0, sizeof(SSS_GLOBALS));

	blResult = Crypt(NULL, (LPBYTE)m_biBuf, (LPBYTE)m_boBuf, &dwCount, false);

	memcpy(&SSSGlobalData, m_boBuf, dwCount);
	
	int nStrings = GlobalLoadStrings();
	int nDwords = GlobalLoadDwords();
	int nOther = GlobalLoadOthers();

	TraceLeave(_D("GlobalsLoad"), (DWORD)&SSSGlobalData, tlInternal);
	return &SSSGlobalData;
}

// Load Strings
int	GlobalLoadStrings()
{
	TCHAR	szUnicode[WIV_MAX_STRING + 1];
	char	szAscii[WIV_MAX_STRING + 1];
	UCHAR	uCount;
	int		nByte = 0;

	TraceEnter(_D("GlobalLoadStrings"), tlInternal);

	for (uCount = 0; uCount < SSS_NUM_STRINGS; uCount++)
	{
		UCHAR uSize = SSSGlobalData.bStringData[nByte++];

		_zclr(szUnicode);
		strncpy(szAscii, (PSZ)&SSSGlobalData.bStringData[nByte], uSize);
		szAscii[uSize]=0;
		MultiByteToWideChar(CP_ACP, 0, szAscii, -1, szUnicode, WIV_MAX_STRING);
		_tcsncpy(m_aszStrings[uCount], szUnicode, WIV_MAX_STRING);
		nByte += uSize;
	}

	TraceInternal(_D("GlobalLoadStrings: Number of strings = %d"), SSS_NUM_STRINGS);

	TraceLeave(_D("GlobalLoadStrings"), (DWORD)SSS_NUM_STRINGS, tlInternal);
	return SSS_NUM_STRINGS;
}

// Load DWORDs
int GlobalLoadDwords()
{
	UCHAR	uCount;
	int		nByte = 0;

	TraceEnter(_D("GlobalLoadDwords"), tlInternal);

	for (uCount = 0; uCount < SSS_NUM_DWORDS; uCount++)
	{
		UCHAR uSize = SSSGlobalData.bDWordData[nByte++];

		memcpy(&m_adwDWords[uCount], &SSSGlobalData.bDWordData[nByte], uSize);
		nByte += uSize;
	}

	TraceLeave(_D("GlobalLoadDwords"), (DWORD)SSS_NUM_DWORDS, tlInternal);
	return SSS_NUM_DWORDS;
}

// Load Other Values
int GlobalLoadOthers()
{
	UCHAR	uCount;
	int		nByte = 0;

	TraceEnter(_D("GlobalLoadOthers"), tlInternal);

	for (uCount = 0; uCount < SSS_NUM_OTHER; uCount++)
	{
		UCHAR uSize = SSSGlobalData.bOtherData[nByte++];

		memcpy(&m_adwOther[uCount], &SSSGlobalData.bOtherData[nByte], uSize);
		nByte += uSize;
	}

	TraceLeave(_D("GlobalLoadOthers"), (DWORD)SSS_NUM_OTHER, tlInternal);
	return SSS_NUM_OTHER;
}

// Get String
LPCTSTR	GlobalGetString(const UCHAR uIndex)
{
	LPCTSTR	szRetVal;

	TraceEnter(_D("GlobalGetString"), tlInternal);

	szRetVal = (((uIndex < 1) || (uIndex > SSS_NUM_STRINGS)) ? WIV_EMPTY_STRING : m_aszStrings[uIndex - 1]);

	TraceLeave(_D("GlobalGetString"), szRetVal, tlInternal);
	
	return szRetVal;
}

// Get DWORD
const DWORD GlobalGetDword(const UCHAR uIndex)
{
	TraceEnter(_D("GlobalGetDword"), tlInternal);

	DWORD dwWord = (((uIndex < 1) || (uIndex > SSS_NUM_DWORDS)) ? 0 : m_adwDWords[uIndex - 1]); 

	TraceLeave(_D("GlobalGetDword"), dwWord, tlInternal);
	return dwWord;
}

// Get Other Values
const DWORD GlobalGetOther(const UCHAR uIndex)
{
	TraceEnter(_D("GlobalGetOther"), tlInternal);

	DWORD dwWord = (((uIndex < 1) || (uIndex > SSS_NUM_OTHER)) ? 0 : m_adwOther[uIndex - 1]);

	TraceLeave(_D("GlobalGetOther"), dwWord, tlInternal);
	return dwWord;
}

} // namespace WiV
