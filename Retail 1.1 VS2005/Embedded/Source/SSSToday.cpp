//////////////////////////////////////////////////////////////////////
//
// SSSToday.cpp: implementation of the CSSSToday class.
//
//////////////////////////////////////////////////////////////////////

extern struct SSS_GLOBALS *SSSGlobals;

#include "SSSCommon.h"
#include "WiVUtils.h"
#include "WiVLicense.h"
#include "WiVLang.h"
#include "SSSToday.h"
#include "SSSOptions.h"
#include "SSSPhone.h"
#include "shguim.h"
#include "ril.h"

using namespace WiV;

//extern SSS_GLOBALS *SSSGlobals;

// Member Variables

// Handles
static	HMODULE		m_hmInstance;
static	HWND		m_hwTodayWnd;
static	HWND		m_hwParentHwnd;
static	HICON		m_hiIcon;

static	TODAYLISTITEM m_tliTLI;

// Flags
union {
	__int64	i64Flags;
	struct  {
		unsigned int	Flags						: 32;
		unsigned int	License						: 16;
		unsigned int	Spare						: 16;
	}Options;

	struct {
		unsigned int	ForceRefresh				: 1;
		unsigned int	PINPending					: 1;
		unsigned int	PINError					: 1;
		unsigned int	IgnoreButtonUp				: 1;
		unsigned int	CommsStarted				: 1;

		unsigned int	RadioIsOn					: 1;
		unsigned int	SIMReady					: 1;
		unsigned int	PINRequired					: 1;
		unsigned int	TapToEnterPIN				: 1;
		unsigned int	NoAutoPIN					: 1;

		unsigned int	Initializing				: 1;
		unsigned int	AllowAutoPINAfterInit		: 1;
		unsigned int	AllowAutoPINAfterRadioON	: 1;
		unsigned int	ShowPhoneNumber				: 1;
		unsigned int	ShowTSP						: 1;
		unsigned int	SingleLineDisplay			: 1;
		unsigned int	Line1BoldFont				: 1;
		unsigned int	Line2BoldFont				: 1;

		unsigned int	SIMMissing					: 1;
		unsigned int	IncorrectSIM				: 1;

		unsigned int	Unused						: 12;

//#define WIV_LICENSE_CONDITION_GREEN			0x00a1
//#define WIV_LICENSE_CONDITION_AMBER			0x2892
//#define WIV_LICENSE_CONDITION_RED				0x12c4
//#define WIV_LICENSE_CONDITION_WHITE			0x1780

		unsigned int	LicenseGreen1				: 1;
		unsigned int	LicenseAmber1				: 1;
		unsigned int	LicenseRed1					: 1;
		unsigned int	LicenseZero1				: 1;
		unsigned int	LicenseAmber2				: 1;
		unsigned int	LicenseGreen2				: 1;
		unsigned int	LicenseRed2					: 1;
		unsigned int	LicenseAdvised				: 1;
		unsigned int	LicenseDontCare1			: 1;
		unsigned int	LicenseRed3					: 1;
		unsigned int	LicenseInvalid				: 1;
		unsigned int	LicenseAmber3				: 1;
		unsigned int	TapToRegister				: 1;
		unsigned int	PleaseRegister				: 1;
		unsigned int	LicenseZero2				: 2;

		unsigned int	LicenseUnused				: 16;
	}TodayBits;
} TodayFlags;

// Class object references
static	CSSSToday	*m_pThis;
static	CSSSPhone	*m_pPhone;
static	CSSSOptions	*m_pOptions;

// Strings
static	TCHAR		m_szPhonePrompt[SSS_MAX_STATUS_PROMPT + 1];
static	TCHAR		m_szProviderPrompt[SSS_MAX_STATUS_PROMPT + 1];
static	TCHAR		m_szPhoneNumber[SSS_MAX_PHONE_NUMBER + 1];
static	TCHAR		m_szProvider[SSS_MAX_PROVIDER + 1];
static	TCHAR		m_szRegistration[SSS_MAX_PROVIDER + 1];
static	TCHAR		m_szClassName[MAX_PATH + 1];
static	TCHAR		m_szWindowName[MAX_PATH + 1];
static	TCHAR		m_szLineOneText[MAX_PATH + 1];
static	TCHAR		m_szLineTwoText[MAX_PATH + 1];

// Data
static	DWORD		m_dwSIMLockedState;
static	DWORD		m_dwPreviousPhoneState;
static	DWORD		m_dwPreviousSIMLockedState;
static	DWORD		m_dwIconSet;
static	DWORD		m_dwTapAction;
static	DWORD		m_dwTAHAction;
static	DWORD		m_dwTodayIconTapAction;
static	DWORD		m_dwTodayIconTAHAction;

static	UINT		WM_SH_UIMETRIC_CHANGE;
static	UINT		m_uiHeight;

static	POINT		m_ptIconPos;
static	COLORREF	m_crTodayText;
static	HFONT		m_hfNormalTodayFont;
static	HFONT		m_hfBoldTodayFont;

static	DWORD		m_dwConfig = 0;
static	UCHAR		m_uchRN;

static	DWORD		m_dwTodayLicenseData		= 0;
static	DWORD		m_dwTodayLicenseConfig		= 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSSSToday::CSSSToday()
{
	TraceEnter(_D("CSSSToday::CSSSToday (Default Constructor)"));
	TraceLeave(_D("CSSSToday::CSSSToday (Default Constructor)"));
}

CSSSToday::CSSSToday(HMODULE HMODULE, CSSSPhone *pPhone, CSSSOptions *pOptions, LPCTSTR lpszClassName, LPCTSTR lpszWindowName)
{
	TraceEnter(_D("CSSSToday::CSSSToday (Constructor)"));

	m_hmInstance = HMODULE;
	m_pPhone = pPhone;
	m_pOptions = pOptions;
	m_pThis = this;

//	m_hwTodayWnd = NULL;
	m_dwPreviousPhoneState = RIL_EQSTATE_UNKNOWN;
	m_dwPreviousSIMLockedState = RIL_LOCKEDSTATE_UNKNOWN;

	memset(&TodayFlags, 0, sizeof(TodayFlags));
	TodayFlags.TodayBits.Initializing = true;
	TodayFlags.TodayBits.ForceRefresh = false;

	_tcsncpy(m_szClassName, lpszClassName, MAX_PATH);
	_tcsncpy(m_szWindowName, lpszWindowName, MAX_PATH);

	m_dwTodayLicenseData		= (DWORD)m_pThis;
	m_dwTodayLicenseConfig		= (DWORD)m_pPhone;

	TraceDetail(_D("CSSSToday::CSSSToday (Constructor): Calling LicenseRegisterNotification with 0x%08X"), &LicenseTodayNotify);
	LicenseRegisterNotification(&LicenseTodayNotify);
	TraceDetail(_D("CSSSToday::CSSSToday (Constructor): Back from LicenseRegisterNotification"));

	TraceLeave(_D("CSSSToday::CSSSToday (Constructor)"));
}

CSSSToday::~CSSSToday()
{
	TraceEnter(_D("CSSSToday::~CSSSToday (Destructor)"));

	TraceDetail(_D("CSSSToday::~CSSSToday (Destructor): Calling LicenseDeregisterNotification with 0x%08X"), &LicenseTodayNotify);
	LicenseDeregisterNotification(&LicenseTodayNotify);
	TraceDetail(_D("CSSSToday::}CSSSToday (Destructor): Back from LicenseDeregisterNotification"));

	if (m_hiIcon)
	{
		DestroyIcon(m_hiIcon);
		m_hiIcon = NULL;
	}

	if (m_hfNormalTodayFont)
	{
		DeleteObject(m_hfNormalTodayFont);
		m_hfNormalTodayFont = NULL;
	}

	if (m_hfBoldTodayFont)
	{
		DeleteObject(m_hfBoldTodayFont);
		m_hfBoldTodayFont = NULL;
	}

	TraceLeave(_D("CSSSToday::CSSSToday (Destructor)"));
}

//////////////////////////////////////////////////////////////////////
// Create TodayWindow with given parent and style
//////////////////////////////////////////////////////////////////////
BOOL CSSSToday::TodayCreate(HWND hWndParent, TODAYLISTITEM *ptli, DWORD dwStyle)
{
	TraceEnter(_D("CSSSToday::TodayCreate"));
	memcpy(&m_tliTLI, ptli, sizeof(TODAYLISTITEM));

	m_hwParentHwnd = hWndParent;
	
	HWND hWnd = CreateWindow(m_szClassName, m_szWindowName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		GetSystemMetrics(SM_CXSCREEN), 0, m_hwParentHwnd, NULL, m_hmInstance, NULL);

	_snwprintf(m_szPhonePrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_STATUS_LABEL);
	_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_INITIALIZING, SSS_MAX_PHONE_NUMBER);
	_snwprintf(m_szProviderPrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_PROVIDER_LABEL);
	_tcsncpy(m_szProvider, SSS_TEXT_PHONE_PROVIDER_LOOKING, SSS_MAX_PROVIDER);

	//Get default today attributes
	GetTodayDefaults();

    WM_SH_UIMETRIC_CHANGE = RegisterWindowMessage(SH_UIMETRIC_CHANGE);

	TraceLeave(_D("CSSSToday::TodayCreate"), (hWnd != NULL));
	return (hWnd != NULL);
}

//////////////////////////////////////////////////////////////////////
// Register/Unregister TodayWindowClass
//////////////////////////////////////////////////////////////////////
void CSSSToday::RegisterTodayClass(WNDPROC wndProc)
{
	WNDCLASS wndClass;

	TraceEnter(_D("CSSSToday::RegisterTodayClass"));

	if (m_hmInstance)
		UnregisterClass(m_szClassName, m_hmInstance);

	memset(&wndClass, 0, sizeof(wndClass));
	wndClass.hCursor = 0;
	wndClass.hIcon = 0;
	wndClass.hInstance = m_hmInstance;
	wndClass.lpszClassName = m_szClassName;
	wndClass.lpszMenuName = NULL;
	wndClass.style = CS_VREDRAW | CS_HREDRAW;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	wndClass.lpfnWndProc = wndProc;

	RegisterClass(&wndClass);
	TraceLeave(_D("CSSSToday::RegisterTodayClass"));
}

void CSSSToday::UnregisterTodayClass()
{
	TraceEnter(_D("CSSSToday::UnregisterTodayClass"));
	if (m_hmInstance)
		UnregisterClass(m_szClassName, m_hmInstance);
	TraceLeave(_D("CSSSToday::UnregisterTodayClass"));
}

bool CSSSToday::TodayLicenseNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig)
{
	TraceEnter(_D("CSSSToday::LicenseNotify"));

	if (lpLicenseData == NULL)
	{
		TodayFlags.Options.License = dwLicenseConfig;
		TraceDetail(_D("CSSSToday::LicenseNotify: TodayFlags.Options.License = 0x%08X"), TodayFlags.Options.License);
	}
	else
	{
		m_dwTodayLicenseData = (DWORD)lpLicenseData;
		m_dwTodayLicenseConfig = dwLicenseConfig;
	}

	TraceDetail(_D("CSSSToday::LicenseNotify: m_dwTodayLicenseData = 0x%08X, m_dwTodayLicenseConfig = 0x%08X"), m_dwTodayLicenseData, m_dwTodayLicenseConfig);

	TraceLeave(_D("CSSSToday::LicenseNotify"));
	return true;
}

//////////////////////////////////////////////////////////////////////
// Get Methods
//////////////////////////////////////////////////////////////////////
HWND CSSSToday::GetParent()
{
	TraceEnter(_D("CSSSToday::GetParent"));
	TraceLeave(_D("CSSSToday::GetParent"), (DWORD)m_hwParentHwnd);
	return m_hwParentHwnd;
};

HWND CSSSToday::GetTodayWindowHandle()
{
	TraceEnter(_D("CSSSToday::GetTodayWindowHandle"));
	TraceLeave(_D("CSSSToday::GetTodayWindowHandle"), (DWORD)m_hwTodayWnd);
	return m_hwTodayWnd;
};

UINT CSSSToday::GetItemHeight()
{
	TraceEnter(_D("CSSSToday::GetItemHeight"));
	TraceLeave(_D("CSSSToday::GetItemHeight"), m_uiHeight);
	return m_uiHeight;
};

HINSTANCE CSSSToday::GetInstance()
{
	TraceEnter(_D("CSSSToday::GetInstance"));
	TraceLeave(_D("CSSSToday::GetInstance"), (DWORD)m_hmInstance);
	return m_hmInstance;
};

HICON CSSSToday::GetIcon()
{
	TraceEnter(_D("CSSSToday::GetIcon"));
	TraceLeave(_D("CSSSToday::GetIcon"), (DWORD)m_hiIcon);
	return m_hiIcon;
};

LPCTSTR	CSSSToday::GetClassName()
{
	TraceEnter(_D("CSSSToday::GetClassName"));
	TraceLeave(_D("CSSSToday::GetClassName"), m_szClassName);
	return m_szClassName;
};

LPCTSTR	CSSSToday::GetWindowName()
{
	TraceEnter(_D("CSSSToday::GetWindowName"));
	TraceLeave(_D("CSSSToday::GetWindowName"), m_szWindowName);
	return m_szWindowName;
};

//////////////////////////////////////////////////////////////////////
// Set Methods
//////////////////////////////////////////////////////////////////////

void CSSSToday::SetIconSet(const DWORD dwIconSet)
{
	TraceEnter(_D("CSSSToday::SetIconSet"));
	m_dwIconSet = dwIconSet;
	TraceLeave(_D("CSSSToday::SetIconSet"));
}

BOOL CSSSToday::SetIcon(const UINT uID, const int xDrawAt, const int yDrawAt)
{
	TraceEnter(_D("CSSSToday::SetIcon"));
	if (m_hiIcon)
	{
		DestroyIcon(m_hiIcon);
		m_hiIcon = NULL;
	}

	if (m_hmInstance)
	{
		m_hiIcon = (HICON)LoadImage(m_hmInstance, MAKEINTRESOURCE(uID), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

		m_ptIconPos.x = xDrawAt;
		m_ptIconPos.y = yDrawAt;
	}

	TraceLeave(_D("CSSSToday::SetIcon"), (m_hiIcon != NULL));
	return (m_hiIcon != NULL);
}

void CSSSToday::SetInstance(const HMODULE HMODULE)
{
	TraceEnter(_D("CSSSToday::SetInstance"));
	m_hmInstance = HMODULE;
	TraceLeave(_D("CSSSToday::SetInstance"));
}

void CSSSToday::SetClassInfo(LPCTSTR lpszClassName, LPCTSTR lpszWindowName)
{
	TraceEnter(_D("CSSSToday::SetClassInfo"));
	_tcsncpy(m_szClassName, lpszClassName, MAX_PATH);
	_tcsncpy(m_szWindowName, lpszWindowName, MAX_PATH);
	TraceLeave(_D("CSSSToday::SetClassInfo"));
}

void CSSSToday::SetItemHeight(const UINT nHeight)
{
	TraceEnter(_D("CSSSToday::SetItemHeight"));
	m_uiHeight = nHeight;
	TraceLeave(_D("CSSSToday::SetItemHeight"));
}

void CSSSToday::SetAutoPINAfterInit(const bool bAllowAutoPINAfterInit)
{
	TraceEnter(_D("CSSSToday::SetAutoPINAfterInit"));
	TodayFlags.TodayBits.AllowAutoPINAfterInit = bAllowAutoPINAfterInit;
	TraceInfo(_D("CSSSToday::SetAutoPINAfterInit: TodayFlags.TodayBits.AllowAutoPINAfterInit = <%d>"), TodayFlags.TodayBits.AllowAutoPINAfterInit);
	TraceInfo(_D("CSSSToday::SetAutoPINAfterInit: TodayFlags.TodayBits.Initializing = <%d>"), TodayFlags.TodayBits.Initializing);
	TraceLeave(_D("CSSSToday::SetAutoPINAfterInit"));
}

void CSSSToday::SetAutoPINAfterRadioON(const bool bAllowAutoPINAfterRadioON)
{
	TraceEnter(_D("CSSSToday::SetAutoPINAfterRadioON"));
	TodayFlags.TodayBits.AllowAutoPINAfterRadioON = bAllowAutoPINAfterRadioON;
	TraceInfo(_D("CSSSToday::SetAutoPINAfterRadioON: TodayFlags.TodayBits.AllowAutoPINAfterRadioON = <%d>"), TodayFlags.TodayBits.AllowAutoPINAfterRadioON);
	TraceInfo(_D("CSSSToday::SetAutoPINAfterRadioON: TodayFlags.TodayBits.Initializing = <%d>"), TodayFlags.TodayBits.Initializing);
	TraceLeave(_D("CSSSToday::SetAutoPINAfterRadioON"));
}

void CSSSToday::SetShowPhoneNumber(const bool bShowPhoneNumber)
{
	TraceEnter(_D("CSSSToday::SetShowPhoneNumber"));
	TodayFlags.TodayBits.ShowPhoneNumber = bShowPhoneNumber;
	TraceLeave(_D("CSSSToday::SetShowPhoneNumber"));
}

void CSSSToday::SetShowTSP(const bool bShowTSP)
{
	TraceEnter(_D("CSSSToday::SetShowTSP"));
	TodayFlags.TodayBits.ShowTSP = bShowTSP;
	TraceLeave(_D("CSSSToday::SetShowTSP"));
}

void CSSSToday::SetSingleLineDisplay(const bool bSingleLineDisplay)
{
	TraceEnter(_D("CSSSToday::SetShowTSP"));
	TodayFlags.TodayBits.SingleLineDisplay = bSingleLineDisplay;
	TraceLeave(_D("CSSSToday::SetShowTSP"));
}

void CSSSToday::SetLine1BoldFont(const bool bLine1BoldFont)
{
	TraceEnter(_D("CSSSToday::SetLine1BoldFont"));
	TodayFlags.TodayBits.Line1BoldFont = bLine1BoldFont;
	TraceLeave(_D("CSSSToday::SetLine1BoldFont"));
}

void CSSSToday::SetLine2BoldFont(const bool bLine2BoldFont)
{
	TraceEnter(_D("CSSSToday::SetLine2BoldFont"));
	TodayFlags.TodayBits.Line2BoldFont = bLine2BoldFont;
	TraceLeave(_D("CSSSToday::SetLine2BoldFont"));
}

void CSSSToday::SetTapAction(const DWORD dwAction)
{
	TraceEnter(_D("CSSSToday::SetTapAction"));
	m_dwTapAction = dwAction;
	TraceLeave(_D("CSSSToday::SetTapAction"));
}

void CSSSToday::SetTAHAction(const DWORD dwAction)
{
	TraceEnter(_D("CSSSToday::SetTAHAction"));
	m_dwTAHAction = dwAction;
	TraceLeave(_D("CSSSToday::SetTAHAction"));
}

void CSSSToday::SetTodayIconTapAction(const DWORD dwAction)
{
	TraceEnter(_D("CSSSToday::SetTodayIconTapAction"));
	m_dwTodayIconTapAction = dwAction;
	TraceLeave(_D("CSSSToday::SetTodayIconTapAction"));
}

void CSSSToday::SetTodayIconTAHAction(const DWORD dwAction)
{
	TraceEnter(_D("CSSSToday::SetTodayIconTAHAction"));
	m_dwTodayIconTAHAction = dwAction;
	TraceLeave(_D("CSSSToday::SetTodayIconTAHAction"));
}

void CSSSToday::RefreshWindow(BOOL bShow)
{
	TraceEnter(_D("CSSSToday::RefreshWindow"));
	if (m_hwTodayWnd)
	{
		if (bShow)
			ShowWindow(m_hwTodayWnd, SW_SHOWNORMAL);

		RECT rect;
		GetClientRect(m_hwTodayWnd, &rect);
		InvalidateRect(m_hwTodayWnd, &rect, TRUE);
		UpdateWindow(m_hwTodayWnd);
	}

	TraceLeave(_D("CSSSToday::RefreshWindow"));
}

void CSSSToday::PhoneNotify(const DWORD dwNotifyCode, const LPVOID lpData)
{
	DWORD	dwCode = dwNotifyCode;
	DWORD	dwData = (DWORD)lpData;

	TraceEnter(_D("CSSSToday::PhoneNotify"));

	TraceDetail(_D("CSSSToday::PhoneNotify, dwNotifyCode = <%08X>"), dwNotifyCode);
	TraceDetail(_D("CSSSToday::PhoneNotify, TodayFlags.TodayBits.LicenseAdvised = %d"), TodayFlags.TodayBits.LicenseAdvised);
	TraceDetail(_D("CSSSToday::PhoneNotify, TodayFlags.Options.License = 0x%08X"), TodayFlags.Options.License);

	if (!TodayFlags.TodayBits.LicenseAdvised)
	{
		if (m_dwTodayLicenseData == (DWORD)m_pThis)
		{
			m_dwTodayLicenseData = dwNotifyCode;
		}
		if (m_dwTodayLicenseConfig == (DWORD)m_pPhone)
		{
			m_dwTodayLicenseConfig = (DWORD)lpData;
		}
	}

	dwCode = dwNotifyCode ^ m_dwTodayLicenseData;
	dwData = (DWORD)lpData ^ m_dwTodayLicenseConfig;

	TraceDetail(_D("CSSSToday::PhoneNotify, dwCode = <%08X>"), dwCode);

	if (dwCode == g_dwNotifyHandle)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: Handle Notification"));
		if ((LPVOID)dwData != NULL)
		{
			m_dwConfig = *(LPDWORD)dwData;
			TraceDetail(_D("CSSSToday::PhoneNotify: Handle Notification, RIL Handle = <%08X>"), m_dwConfig);
		}
		else
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Handle Notification, RIL Handle has been cleared"));
			m_dwConfig = 0;
		}

	}else if (dwCode == g_dwNotifyICCID)
	{
		TraceDetail(_D("CSSSToday::PhoneNotify: ICCID Notification"));

		TodayFlags.TodayBits.NoAutoPIN = true;

		if ((LPVOID)dwData != NULL)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: ICCID Notification, ICCID = <%s>, Calling GetTodayOptions"), (LPCTSTR)dwData);
			m_pOptions->GetTodayOptions(g_dwGetSecurityOptions | g_dwGetForce);
			TraceDetail(_D("CSSSToday::PhoneNotify: ICCID Notification, Back from GetTodayOptions, PIN = %s"), m_pPhone->GetPIN());
			TraceDetail(_D("TodayFlags.TodayBits.Initializing, = %d, TodayFlags.TodayBits.AllowAutoPINAfterInit = %d, TodayFlags.TodayBits.AllowAutoPINAfterRadioON = %d"),
				TodayFlags.TodayBits.Initializing, TodayFlags.TodayBits.AllowAutoPINAfterInit, TodayFlags.TodayBits.AllowAutoPINAfterRadioON);

			if (TodayFlags.TodayBits.Initializing ? TodayFlags.TodayBits.AllowAutoPINAfterInit : TodayFlags.TodayBits.AllowAutoPINAfterRadioON)
			{
				TodayFlags.TodayBits.NoAutoPIN = false;
				TraceDetail(_D("CSSSToday::PhoneNotify: Calling UnlockSIM"));
				m_pPhone->UnlockSIM();
				TraceDetail(_D("CSSSToday::PhoneNotify: Back from UnlockSIM"));
			}

			TraceDetail(_D("CSSSToday::PhoneNotify: ICCID Notification, Calling UpdateTodayItemData(true)"));
			UpdateTodayItemData(true);
			UpdateOptionsDialog(0, 0);
		}
		else
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: ICCID Notification, ICCID has been cleared"));
		}

	}else if (dwCode == g_dwNotifyIMSI)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: IMSI Notification"));
		if ((LPVOID)dwData != NULL)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: IMSI Notification, IMSI = <%s>"), (LPCTSTR)dwData);
		}
		else
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: IMSI Notification, IMSI has been cleared"));
		}

		UpdateOptionsDialog(0, 0);

	}else if (dwCode == g_dwNotifyRadioPresence)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: Radio Presence Notification"));
		if (*(LPDWORD)dwData == RIL_RADIOPRESENCE_NOTPRESENT)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Radio Presence Notification, Radio is not present"));
		}
		else
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Radio Presence Notification, Radio is present"));
		}

	}else if (dwCode == g_dwNotifyRadioState)
	{
		DWORD	dwRadioSupport;
		DWORD	dwEquipmentState;
		DWORD	dwReadyState;

		TraceInfo(_D("CSSSToday::PhoneNotify: Radio State Notification"));

		// If data is null, notification was for completion of a set phone state request
		if ((LPVOID)dwData == NULL)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Set phone state request has completed successfully"));
			UpdateTodayItemData(true);
			UpdateOptionsDialog(0, 0);
			TraceLeave(_D("CSSSToday::PhoneNotify"));
			return;
		}

		// If data is -1, need to request phone state again
		if ((LPVOID)dwData == INVALID_HANDLE_VALUE)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Need to request phone state again"));
			UpdateTodayItemData(true);
			TraceLeave(_D("CSSSToday::PhoneNotify"));
			return;
		}

		LPRILEQUIPMENTSTATE lpril = (LPRILEQUIPMENTSTATE)dwData;
			
		TraceDetail(_D("CSSSToday::PhoneNotify: Radio State: dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_RADIOSUPPORT)
		{
			dwRadioSupport = lpril->dwRadioSupport;

			TraceInfo(_D("CSSSToday::PhoneNotify: Radio Support = <%08X>"), dwRadioSupport);

			switch (dwRadioSupport)
			{
			case RIL_RADIOSUPPORT_UNKNOWN :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Radio Support The Radio Functionality is in an intermediate state"));
					break;	
				}
			case RIL_RADIOSUPPORT_OFF :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Radio Support The Radio Functionality is OFF (DOES NOT Neccessarily mean safe for flight)"));
					break;	
				}
			case RIL_RADIOSUPPORT_ON :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Radio Support The Radio Functionality is ON"));
					break;	
				}
			}
		}
		if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_EQSTATE)
		{
			dwEquipmentState = lpril->dwEqState;

			TraceInfo(_D("CSSSToday::PhoneNotify: Equipment State = <%08X>"), dwEquipmentState);
			switch (dwEquipmentState)
			{
			case RIL_EQSTATE_UNKNOWN :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Equipment state: Unknown"));
					break;	
				}
			case RIL_EQSTATE_MINIMUM :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Equipment state: Minimum power"));
					break;	
				}
			case RIL_EQSTATE_FULL :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Equipment state: Full functionality"));
					break;	
				}
			case RIL_EQSTATE_DISABLETX :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Equipment state: Transmitter disabled"));
					break;	
				}
			case RIL_EQSTATE_DISABLERX :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Equipment state: Receiver disabled"));
					break;	
				}
			case RIL_EQSTATE_DISABLETXANDRX :
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Equipment state: Transmitter & receiver disabled"));
					break;	
				}
			}
		}
		if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_READYSTATE)
		{
			dwReadyState = lpril->dwReadyState;

			TraceInfo(_D("CSSSToday::PhoneNotify: Ready State = <%08X>"), dwReadyState);

			if (dwReadyState == RIL_READYSTATE_NONE)
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Ready state: Nothing is ready yet"));
				TodayFlags.TodayBits.CommsStarted = false;
			}

			if (dwReadyState & RIL_READYSTATE_INITIALIZED)
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Ready state: The Radio has been initialized (but may not be ready)"));
			}

			if (dwReadyState & RIL_READYSTATE_SIM)
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Ready state: The Radio is ready for SIM Access"));
			}

			if (dwReadyState & RIL_READYSTATE_UNLOCKED)
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Ready state: The SIM is unlocked"));

				if (!TodayFlags.TodayBits.CommsStarted)
				{
					TraceDetail(_D("CSSSToday::PhoneNotify: Calling m_pPhone->StartComms"));
					m_pPhone->StartComms();
					TraceDetail(_D("CSSSToday::PhoneNotify: Back from m_pPhone->StartComms"));
					TodayFlags.TodayBits.CommsStarted = true;
				}

				TodayFlags.TodayBits.Initializing = false;
			}

			if (dwReadyState & RIL_READYSTATE_SMS)
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Ready state: The Radio is ready for SMS messages"));

			}

			if (dwReadyState & RIL_READYSTATE_SIMREADY)
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Ready state: The SIM is ready"));
				TodayFlags.TodayBits.SIMReady = true;

				TraceDetail(_D("CSSSToday::PhoneNotify: Calling m_pPhone->GetPhonebookDetails"));
				m_pPhone->GetPhonebookDetails();
				TraceDetail(_D("CSSSToday::PhoneNotify: Back from m_pPhone->GetPhonebookDetails"));
			}
		}

//		DWORD dwParams = lpril->dwParams;

//		DecodeRILParams(dwParams, nType, nDays, m_dwConfig);
//		TraceInfo(_D("CSSSToday::PhoneNotify: Equipment state: dwParams = %08X, License type = %d, Days left = %d"), lpril->dwParams, nType, nDays);

		TraceDetail(_D("CSSSToday::PhoneNotify: Equipment state: dwParams = %08X"), lpril->dwParams);

		TraceDetail(_D("CSSSToday::PhoneNotify: Equipment state, Calling UpdateTodayItemData(true)"));
		UpdateTodayItemData(true);
		UpdateOptionsDialog(0, 0);

	}else if (dwCode == g_dwNotifySIMLock)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: Locked State Notification"));

		if ((LPVOID)dwData == NULL)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Locked State Notification, Phone unlock request successful"));
			m_dwSIMLockedState = RIL_LOCKEDSTATE_READY;
		}
		else
		{
			m_dwSIMLockedState = *(LPDWORD)dwData;
			TraceDetail(_D("CSSSToday::PhoneNotify: Locked State Notification, m_dwSIMLockedState = %08X"), m_dwSIMLockedState);
		}

		TraceDetail(_D("CSSSToday::PhoneNotify: Locked State Notification, Calling UpdateTodayItemData(true)"));
		UpdateTodayItemData(true);

	}
	else if (dwCode == g_dwNotifyRegistration)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: Registration Change Notification"));

		_zclr(m_szRegistration);

		switch (*(LPDWORD)dwData)
		{
		case RIL_REGSTAT_UNKNOWN:
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Registration Change Notification, Registration unknown"));
				_tcsncpy(m_szRegistration, SSS_TEXT_PHONE_REGISTERED_UNKNOWN, SSS_MAX_PROVIDER);
				break;
			}
		case RIL_REGSTAT_UNREGISTERED:
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Registration Change Notification, Unregistered"));
				break;
			}
		case RIL_REGSTAT_HOME:
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Registration Change Notification, Registered on home network"));
				_tcsncpy(m_szRegistration, SSS_TEXT_PHONE_REGISTERED_HOME, SSS_MAX_PROVIDER);
				break;
			}
		case RIL_REGSTAT_ATTEMPTING:
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Registration Change Notification, Attempting to register"));
				break;
			}
		case RIL_REGSTAT_DENIED:
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Registration Change Notification, Registration denied"));
				break;
			}
		case RIL_REGSTAT_ROAMING:
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Registration Change Notification, Registered on roaming network"));
				_tcsncpy(m_szRegistration, SSS_TEXT_PHONE_REGISTERED_ROAMING, SSS_MAX_PROVIDER);
				break;
			}
		default:
			{
				TraceDetail(_D("CSSSToday::PhoneNotify: Registration Change Notification, Other registration state (%08X)"), *(LPDWORD)dwData);
			}
		}

		TraceDetail(_D("CSSSToday::PhoneNotify: Registration Change Notification, Calling UpdateTodayItemData(true)"));
		UpdateTodayItemData(true);

	}else if (dwCode == g_dwNotifyNumber)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: Phone Number Notification"));
		if ((LPVOID)dwData != NULL)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Phone Number Notification, Phone Number = <%s>"), (LPCTSTR)dwData);
		}

		TraceDetail(_D("CSSSToday::PhoneNotify: Phone Number Notification, Calling UpdateTodayItemData(true)"));
		UpdateTodayItemData(true);
		UpdateOptionsDialog(0, 0);

	}else if (dwCode == g_dwNotifyOperator)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: Operator Name Notification"));
		if ((LPVOID)dwData != NULL)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Operator Name Notification, Operator Name = <%s>"), (LPCTSTR)dwData);
		}

		TraceDetail(_D("CSSSToday::PhoneNotify: Operator Name Notification, Calling UpdateTodayItemData(true)"));
		UpdateTodayItemData(true);

	}else if (dwCode == g_dwNotifySerialNumber)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: Serial Number Notification"));
		if ((LPVOID)dwData != NULL)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Serial Number Notification, Serial Number = <%s>"), (LPCTSTR)dwData);
		}

		UpdateTodayItemData(true);
		UpdateOptionsDialog(0, 0);

	}else if (dwCode == g_dwNotifyEquipmentInfo)
	{
		TCHAR	szManufacturer[MAXLENGTH_EQUIPINFO + 1];
		TCHAR	szModel[MAXLENGTH_EQUIPINFO + 1];
		TCHAR	szRevision[MAXLENGTH_EQUIPINFO + 1];
		TCHAR	szSerialNumber[MAXLENGTH_EQUIPINFO + 1];

		// If data is NULL, exit
		if ((LPVOID)dwData == NULL)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Equipment Info, dwData is NULL"));
			TraceLeave(_D("CSSSToday::PhoneNotify"));
			return;
		}

		// If data is -1, need to request phone state again
		if ((LPVOID)dwData == INVALID_HANDLE_VALUE)
		{
			TraceDetail(_D("CSSSToday::PhoneNotify: Equipment Info, Need to request Equipment Info again"));
//			UpdateTodayItemData(true);
			TraceLeave(_D("CSSSToday::PhoneNotify"));
			return;
		}

		LPRILEQUIPMENTINFO	lpril = (LPRILEQUIPMENTINFO)dwData;

		TraceInfo(_D("CSSSToday::PhoneNotify: Equipment Info, dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_EI_MANUFACTURER)
		{
			AtoU(lpril->szManufacturer, szManufacturer, strlen(lpril->szManufacturer));
			TraceDetail(_D("CSSSToday::PhoneNotify: Equipment Info, Manufacturer = <%s>"), szManufacturer);
		}
		if (lpril->dwParams & RIL_PARAM_EI_MODEL)
		{
			AtoU(lpril->szModel, szModel, strlen(lpril->szModel));
			TraceDetail(_D("CSSSToday::PhoneNotify: Equipment Info, Model = <%s>"), szModel);
		}
		if (lpril->dwParams & RIL_PARAM_EI_REVISION)
		{
			AtoU(lpril->szRevision, szRevision, strlen(lpril->szRevision));
			TraceDetail(_D("CSSSToday::PhoneNotify: Equipment Info, Revision = <%s>"), szRevision);
		}
		if (lpril->dwParams & RIL_PARAM_EI_SERIALNUMBER)
		{
			AtoU(lpril->szSerialNumber, szSerialNumber, strlen(lpril->szSerialNumber));
			TraceDetail(_D("CSSSToday::PhoneNotify: Equipment Info, Serial Number = <%s>"), szSerialNumber);

		}

		UpdateOptionsDialog(0, 0);

//		int nType, nDays;
//		DWORD dwParams = lpril->dwParams;
//		DecodeParams(dwParams, nType, nDays, m_dwConfig);

//		TraceInfo(_D("CSSSToday::PhoneNotify: Equipment Info, dwParams = %08X, License type = %d, Days left = %d"), lpril->dwParams, nType, nDays);

	}else if (dwCode == g_dwNotifyPhonebook)
	{
		DWORD				dwPBStoreLocation;
		DWORD				dwPBUsed;
		DWORD				dwPBTotal;

		LPRILPHONEBOOKINFO lpril = (LPRILPHONEBOOKINFO)dwData;
			
		TraceDetail(_D("CSSSToday::PhoneNotify: Phonebook Info, dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_PBI_STORELOCATION)
		{
			dwPBStoreLocation = lpril->dwStoreLocation;
			TraceInfo(_D("PhoneNotify: Phonebook Info, Store Location = <%08X>"), dwPBStoreLocation);

			switch (dwPBStoreLocation)
			{
			case RIL_PBLOC_UNKNOWN :
				{
					TraceDetail(_D("PhoneNotify: Phonebook Info, Store Location: Unknown"));
					break;	
				}
			case RIL_PBLOC_SIMEMERGENCY :
				{
					TraceDetail(_D("PhoneNotify: Phonebook Info, Store Location: Emergency numbers"));
					break;	
				}
			case RIL_PBLOC_SIMFIXDIALING :
				{
					TraceDetail(_D("PhoneNotify: Phonebook Info, Store Location: Fixed dialing"));
					break;	
				}
			case RIL_PBLOC_SIMLASTDIALING :
				{
					TraceDetail(_D("PhoneNotify: Phonebook Info, Store Location: Recent calls list"));
					break;	
				}
			case RIL_PBLOC_OWNNUMBERS :
				{
					TraceDetail(_D("PhoneNotify: Phonebook Info, Store Location: Own numbers"));
					break;	
				}
			case RIL_PBLOC_SIMPHONEBOOK :
				{
					TraceDetail(_D("PhoneNotify: Phonebook Info, Store Location: SIM phonebook"));
					break;	
				}
			}
		}
		if (lpril->dwParams & RIL_PARAM_PBI_USED)
		{
			dwPBUsed = lpril->dwUsed;
			TraceDetail(_D("PhoneNotify: Phone book Info, Used Entries = <%08X>"), dwPBUsed);
		}
		if (lpril->dwParams & RIL_PARAM_PBI_TOTAL)
		{
			dwPBTotal = lpril->dwTotal;
			TraceDetail(_D("PhoneNotify: Phonebook Info, Total Entries = <%08X>"), dwPBTotal);
		}

		UpdateTodayItemData(true);
		UpdateOptionsDialog(0, 0);

//		int nType, nDays;
//		DWORD dwParams = lpril->dwParams;
//		DecodeParams(dwParams, nType, nDays, m_dwConfig);

//		TraceInfo(_D("CSSSToday::PhoneNotify: Phonebook Info: dwParams = %08X, License type = %d, Days left = %d"), lpril->dwParams, nType, nDays);

	}else if (dwCode == g_dwNotifySIMUnlockError)
	{
		TraceError(_D("CSSSToday::PhoneNotify: SIM Unlock Error"));

		TodayFlags.TodayBits.PINError = true;

		TraceDetail(_D("CSSSToday::PhoneNotify: SIM Unlock Error, Calling UpdateTodayItemData(true)"));
		UpdateTodayItemData(true);

	}else if (dwCode == g_dwNotifySIMError)
	{
		TraceError(_D("CSSSToday::PhoneNotify: SIM Error Notification"));

		TraceDetail(_D("CSSSToday::PhoneNotify: SIM Error Notification, Calling UpdateTodayItemData(true)"));
		UpdateTodayItemData(true);

	}else if (dwCode == g_dwNotifyAPIError)
	{
		TraceError(_D("CSSSToday::PhoneNotify: API Error Notification"));

	}
#ifdef SSS_V2_IMP
	else if (dwCode == g_dwNotifySignalQuality)
	{
		TraceInfo(_D("CSSSToday::PhoneNotify: Signal Quality Notification"));

		static int nLastSignalStrength = 0;

		if ((LPVOID)dwData == NULL)
		{
//			TraceInfo(_D("CSSSToday::PhoneNotify: Signal Quality"));
		}
		else
		{
			LPRILSIGNALQUALITY lpril = (LPRILSIGNALQUALITY)dwData;

			TCHAR	szStrength[WIV_MAX_STRING + 1] = _T("Signal strength is ");
			TCHAR	szBars[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

			int			nSignalStrength = lpril->nSignalStrength;
			double		fSignalLoss = (double)nSignalStrength;
			double		fMinSignalLoss = (double)lpril->nMinSignalStrength;
			double		fMaxSignalLoss = (double)lpril->nMaxSignalStrength;
			DWORD		dwBitErrorRate = lpril->dwBitErrorRate;
			double		fLowSignalLoss = (double)lpril->nLowSignalStrength;
			double		fHighSignalLoss = (double)lpril->nHighSignalStrength;
			double		fSignal;
			double		fScale = fMaxSignalLoss-fMinSignalLoss;
			double		fOver100 = 0;
			double		fPercentSignal;
			
//			TraceInfo(_D("fSignalLoss = %f"), fSignalLoss);
//			TraceInfo(_D("fMinSignalLoss = %f"), fMinSignalLoss);
//			TraceInfo(_D("fMaxSignalLoss = %f"), fMaxSignalLoss);

			fSignal = fSignalLoss-fMinSignalLoss;

			if (fSignal >= 0)
			{
				fSignal += fScale;
				fOver100 = 100;
			}
			fSignal = fScale - fSignal;

			fPercentSignal = 100 - (fOver100 + ((fSignal/fScale)*100));

			double	dratio = fSignal/((fScale));
			double	dlog = log10(fPercentSignal/100);
			double	fDecibels = 10*dlog;
			double	fBars = ((double)5 + fDecibels);
			int		nBars;
/*
			for (double dsignal = fSignalLoss-fMaxSignalLoss; dsignal > fMinSignalLoss-fMaxSignalLoss; dsignal--)
			{
				double dratio = (dsignal)/(fMinSignalLoss-fMaxSignalLoss);
				double dlog = log10(dratio);
				double db = 10*dlog;
				double dbase = 10;
				double dantilog = pow(dbase, dlog);
				double dp1 = dantilog*(fMinSignalLoss-fMaxSignalLoss);

				TraceInfo(_D("dsignal = %f, dratio = %f, dlog = %f, db = %f, dantilog = %f, dp1 = %f"), dsignal, dratio, dlog, db, dantilog, dp1);
			}
*/

			if (nSignalStrength != nLastSignalStrength)
			{
				if (nSignalStrength >= -57)
				{
					_tcsncat(szStrength, _T("Excellent"), WIV_MAX_STRING);

				}else if (nSignalStrength < -58 && nSignalStrength >= -67)
				{
					_tcsncat(szStrength, _T("Very Good"), WIV_MAX_STRING);

				}else if (nSignalStrength < -68 && nSignalStrength >= -71)
				{
					_tcsncat(szStrength, _T("Good"), WIV_MAX_STRING);

				}else if (nSignalStrength < -72 && nSignalStrength >= -79)
				{
					_tcsncat(szStrength, _T("Poor"), WIV_MAX_STRING);

				}else if (nSignalStrength < -80 && nSignalStrength >= -90)
				{
					_tcsncat(szStrength, _T("Very Poor"), WIV_MAX_STRING);

				}else if (nSignalStrength <= -91)
				{
					_tcsncpy(szStrength, _T("No Signal"), WIV_MAX_STRING);
				}
				else
				{
					_tcsncat(szStrength, _T("Unknown"), WIV_MAX_STRING);
				}

				if (fBars < 0) {_tcscpy(szBars, _T("No Signal"));nBars = -1;}
				if ((fBars >= 0) && (fBars <= 1.49)) {_tcscpy(szBars, _T("Very Poor"));nBars = 0;}
				if ((fBars >= 1.5) && (fBars <= 2.49)) { _tcscpy(szBars, _T("Poor"));nBars = 1;}
				if ((fBars >= 2.5) && (fBars <= 3.39)) {_tcscpy(szBars, _T("Good"));nBars = 2;}
				if ((fBars >= 3.5) && (fBars <= 4.49)) {_tcscpy(szBars, _T("Very Good"));nBars = 3;}
				if (fBars >= 4.5) {_tcscpy(szBars, _T("Excellent"));nBars = 4;}

				nLastSignalStrength = nSignalStrength;

				TraceInfo(_D("CSSSToday::PhoneNotify: Signal Quality, Bars = %d (%s), fSignalLoss = %f, Min = %f, Max = %f, Percentage = %f%%, %s"), nBars, szBars, fSignalLoss, fMinSignalLoss, fMaxSignalLoss, fPercentSignal, szStrength);
			}
		}

//		m_pPhone->RefreshSignalQuality();


		UpdateTodayItemData(true);

	}
#endif // #ifdef SSS_V2_IMP
	

	TraceLeave(_D("CSSSToday::PhoneNotify"));

	return;
}

//////////////////////////////////////////////////////////////////////
// Main WndProc
//////////////////////////////////////////////////////////////////////
LRESULT CSSSToday::TodayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT	lrResult;

	SSSGlobals = GlobalsLoad(SSSGlobals);

	TraceEnter(_D("CSSSToday::TodayWndProc"));

	TraceDetail(_D("CSSSToday::TodayWndProc: uMsg = 0x%08X, wParam = 0x%08X, lParam = 0x%08X"), uMsg, wParam, lParam);

	if (uMsg == WM_SH_UIMETRIC_CHANGE)
    {
        GetTodayDefaults();
    }
	else switch (uMsg)
	{
	case WM_CREATE :
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			UCHAR	uchR = 0;
			PWIVLANG lpsDefaultLanguage = LangGetDefaultLanguage();
			PWIVLANG lpsCurrentLanguage = LangGetCurrentLanguage();

			m_hwTodayWnd = hWnd;

			if (OnTodayCreate(lpcs) == -1)
			{
				TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)-1);
				return -1;
			}
	
			TraceInfo(_D("CSSSToday::TodayWndProc::WM_CREATE, Calling m_pPhone->InitializeComms"));
			m_pPhone->InitializeComms();
			TraceInfo(_D("CSSSToday::TodayWndProc::WM_CREATE, Back from m_pPhone->InitializeComms"));

			TraceDetail(_D("Calling m_pOptions->GetTodayOptions"));
			m_pOptions->GetTodayOptions(g_dwGetAllOptions | g_dwGetForce);

			if (m_pOptions->SetupHelpFile(lpsCurrentLanguage) != 0)
			{
				TraceDetail(_D("CSSSOptions::TodayOptionsWndProc, Help file set up for language %s"), lpsCurrentLanguage->ID);
			}
			else
			{
				TraceDetail(_D("CSSSOptions::TodayOptionsWndProc, Help file for language %s, not found"), lpsCurrentLanguage->ID);
			}
			break;
		}

	case WM_DESTROY :
		{
			TraceDetail(_D("CSSSToday::TodayWndProc::WM_DESTROY, Calling m_pPhone->StopComms"));
			m_pPhone->StopComms();
			TraceDetail(_D("CSSSToday::TodayWndProc::WM_DESTROY, Back from m_pPhone->StopComms"));

			TraceDetail(_D("CSSSToday::TodayWndProc::WM_DESTROY, Calling OnTodayDestroy"));
			OnTodayDestroy();
			TraceDetail(_D("CSSSToday::TodayWndProc::WM_DESTROY, Back from OnTodayDestroy"));

			TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)0);
			return 0;
		}
	case WM_PAINT :
		{
			HDC hDC;
			PAINTSTRUCT ps;

			hDC = BeginPaint(m_hwTodayWnd, &ps);

			DrawTodayIcon(hDC);

			OnTodayPaint(hDC);

			EndPaint(m_hwTodayWnd, &ps);

			//break;
			TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)0);
			return 0;
		}

	case WM_ERASEBKGND :
		{
			HDC hDC = (HDC)wParam;

			OnTodayEraseBkgnd(hDC);

			TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)TRUE);
			return TRUE;
		}
	case WM_TODAYCUSTOM_CLEARCACHE :
		{
			TODAYLISTITEM *ptli = (TODAYLISTITEM *)wParam;

			TraceDetail(_D("CSSSToday::TodayWndProc: WM_TODAYCUSTOM_CLEARCACHE, wParam = 0x%08X"), wParam);

			if (ptli == NULL)
			{
				TraceError(_D("CSSSToday::TodayWndProc: WM_TODAYCUSTOM_CLEARCACHE, wParam is NULL"));
				TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)FALSE);
				return (LRESULT)FALSE;
			}

			lrResult = (LRESULT)OnTodayCustomClearCache(ptli);
			TraceLeave(_D("CSSSToday::TodayWndProc"), lrResult);
			return lrResult;
		}
	case WM_TODAYCUSTOM_QUERYREFRESHCACHE :
		{

			BOOL	blResult = FALSE;

			TODAYLISTITEM *ptli = (TODAYLISTITEM *)wParam;

			TraceDetail(_D("CSSSToday::TodayWndProc: WM_TODAYCUSTOM_QUERYREFRESHCACHE, wParam = 0x%08X"), wParam);

			if (ptli == NULL)
			{
				TraceError(_D("CSSSToday::TodayWndProc: WM_TODAYCUSTOM_QUERYREFRESHCACHE, wParam is NULL"));
				TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)blResult);
				return (LRESULT)blResult;
			}
/*
			if (ptli->cyp != m_uiHeight)
			{
				ptli->cyp = m_uiHeight;
				blResult = TRUE;
				TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)blResult);
				return (LRESULT)blResult;
			}
*/
			blResult |= OnTodayCustomQueryRefreshCache(ptli);

			TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)blResult);
			return (LRESULT)blResult;
		}
	case WM_LBUTTONDOWN :
		{
			POINT point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);

			OnTodayLButtonDown(wParam, point);

			TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONUP :
		{
			POINT point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);

			// If context menu just been activated, ignore this tap
			if (TodayFlags.TodayBits.IgnoreButtonUp)
			{
				TodayFlags.TodayBits.IgnoreButtonUp = false;
				TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)0);
				return 0;
			}

			OnTodayLButtonUp(wParam, point);

			TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnTodaySettingChange(wParam, (LPCTSTR)lParam);

			TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)0);
			return 0;
		}
	case WM_HELP :
		{
			TraceLeave(_D("CSSSToday::TodayWndProc"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;

			lrResult = OnTodayNotify(wParam, pNMHDR);
			TraceLeave(_D("CSSSToday::TodayWndProc"), lrResult);
			return lrResult;
		}
	default :
		{
			lrResult = OnTodayMessage(uMsg, wParam, lParam);
			TraceLeave(_D("CSSSToday::TodayWndProc"), lrResult);
			return lrResult;
		}
	}

	TraceDetail(_D("CSSSToday::TodayWndProc::Calling DefWindowProc"));
	lrResult = DefWindowProc(m_hwTodayWnd, uMsg, wParam, lParam);
	TraceDetail(_D("CSSSToday::TodayWndProc::Back from DefWindowProc"));
	 
	TraceLeave(_D("CSSSToday::TodayWndProc"), lrResult);
	return lrResult;
}

/////////////////////////////////////////////////
// Message handlers
/////////////////////////////////////////////////

void CSSSToday::OnTodayLButtonDown(const UINT nFlags, const POINT point)
{
	TraceEnter(_D("CSSSToday::OnTodayLButtonDown"));

	bool	blNoAnimate;

	blNoAnimate = (TodayFlags.TodayBits.PleaseRegister) || (TodayFlags.TodayBits.TapToRegister) || (TodayFlags.TodayBits.LicenseInvalid);
	
	if (IsTapAndHold(m_hwTodayWnd, point, blNoAnimate))
	{
		TodayFlags.TodayBits.IgnoreButtonUp = ((true & (!TodayFlags.TodayBits.PleaseRegister)) || (true & (!TodayFlags.TodayBits.TapToRegister)) || (true & (!TodayFlags.TodayBits.LicenseInvalid)));

		this->RefreshWindow(TRUE);

		TraceDetail(_D("CSSSToday::OnTodayLButtonDown: Checking to see if point is on Today item icon"));

		if (this->IsOnTodayIcon(point))
		{
			TraceInfo(_D("CSSSToday::OnTodayLButtonDown: Point is on Today item icon, so do TodayIconTAHAction"));
			this->DoAction(m_dwTodayIconTAHAction, point);
		}
		else
		{
			TraceInfo(_D("CSSSToday::OnTodayLButtonDown: Today item tapped in non-specific area, so do TAHAction"));
			this->DoAction(m_dwTAHAction, point);
		}
	}

	DefWindowProc(m_hwTodayWnd, WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
	TraceLeave(_D("CSSSToday::OnTodayLButtonDown"));

	return;
}

void CSSSToday::OnTodayLButtonUp(const UINT nFlags, const POINT point)
{
	TraceEnter(_D("CSSSToday::OnTodayLButtonUp"));
	TraceInfo(_D("CSSSToday::OnTodayLButtonUp: point.x = <%d>, point.y = <%d>"), point.x, point.y);

	TraceDetail(_D("CSSSToday::OnTodayLButtonUp: Checking to see if point is on Today item icon"));

	if (this->IsOnTodayIcon(point))
	{
		TraceInfo(_D("CSSSToday::OnTodayLButtonUp: Point is on Today item icon, so do TodayIconTapAction"));
		this->DoAction(m_dwTodayIconTapAction, point);
	}
	else
	{
		TraceInfo(_D("CSSSToday::OnTodayLButtonUp: Today item tapped in non-specific area, so do TapAction"));

		if (TodayFlags.TodayBits.PINRequired)
		{
			if (TodayFlags.TodayBits.NoAutoPIN)
			{
				TraceInfo(_D("CSSSToday::OnTodayLButtonUp: PIN required, so show Phone Settings"));
				this->DoAction(g_dwTodayActionPhoneSettings, point);
			}
			else
			{
				this->DoAction(m_dwTapAction, point);
			}
		}
		else
		{
			this->DoAction(m_dwTapAction, point);
		}
	}

	TraceLeave(_D("CSSSToday::OnTodayLButtonUp"));

	return;
}

bool CSSSToday::IsOnTodayIcon(const POINT point)
{
	RECT	rect;
	bool	bResult = false;

	TraceEnter(_D("CSSSToday::IsOnTodayIcon"));

	rect.left = 2;
	rect.top = 1;
	rect.right = rect.left + 16;
	rect.bottom = rect.top + 16;

	if (PtInRect(&rect, point))
	{
		bResult = true;
	}

	TraceLeave(_D("CSSSToday::IsOnTodayIcon"), (DWORD)bResult);

	return bResult;
}

void CSSSToday::DoAction(const DWORD dwAction, const POINT point)
{
	DWORD	dwAct;

	TraceEnter(_D("CSSSToday::DoAction"));

	TraceDetail(_D("CSSSToday::DoAction: dwAction = 0x%08X"), dwAction);

//	if (TodayFlags.TodayBits.RadioIsOn)
	{
		dwAct = (dwAction & ((!TodayFlags.TodayBits.PleaseRegister) * 15));
		TraceDetail(_D("CSSSToday::DoAction: dwAct = 0x%08X"), dwAct);
		dwAct += (TodayFlags.TodayBits.PleaseRegister * g_dwTodayActionOptions);

		if (TodayFlags.TodayBits.TapToRegister)
		{
			if (TodayFlags.TodayBits.LicenseInvalid)
			{
				dwAct = (dwAct & ((!TodayFlags.TodayBits.LicenseInvalid) * 15));
				TraceDetail(_D("CSSSToday::DoAction: dwAct = 0x%08X"), dwAct);
				dwAct += (TodayFlags.TodayBits.LicenseInvalid * g_dwTodayActionOptions);
			}
			else
			{
				dwAct = (dwAct & ((!TodayFlags.TodayBits.TapToRegister) * 15));
				TraceDetail(_D("CSSSToday::DoAction: dwAct = 0x%08X"), dwAct);
				dwAct += (TodayFlags.TodayBits.TapToRegister * g_dwTodayActionOptions);
			}
		}
	}
//	else
//	{
//		dwAct = dwAction;
//	}

	TraceDetail(_D("CSSSToday::DoAction: dwAct = 0x%08X"), dwAct);

	if (dwAct == g_dwTodayActionSwitchSIM)
	{
		this->DoSwitchSIM();

	}else if (dwAct == g_dwTodayActionToggleRadio)
	{
		this->DoToggleRadioState();

	}else if (dwAct == g_dwTodayActionOptions)
	{
		this->DoShowOptions(SSS_FLAG_PLUGIN_ENABLED);

	}else if (dwAct == g_dwTodayActionPhoneSettings)
	{
		this->DoPhoneSettings();

	}else if (dwAct == g_dwTodayActionRefresh)
	{
		this->DoRefresh();

	}else if (dwAct == g_dwTodayActionShowPopup)
	{
		LRESULT	lrResult = -1;

		TodayFlags.TodayBits.IgnoreButtonUp = false;

		lrResult = ShowContextMenu(m_hwTodayWnd, IDM_TODAY_MENU, point);
		TraceInfo(_D("ShowContextMenu returned %d"), lrResult);

		switch(lrResult)
		{
		case 0 :
			{
				TraceInfo(_D("User cancelled today pop-up menu"));
				TodayFlags.TodayBits.IgnoreButtonUp = true;
				break;
			}
		case IDM_TODAY_REFRESH :
			{
				TraceInfo(_D("User chose Refresh menu item"));
				this->DoRefresh();
				break;
			}
		case IDM_TODAY_SWITCH_SIM :
			{
				TraceInfo(_D("User chose Switch SIM menu item"));
				this->RefreshWindow(TRUE);
				this->DoSwitchSIM();
				break;
			}
		case IDM_TODAY_TURN_PHONE_ONOFF :
			{
				TraceInfo(_D("User chose Turn Phone ON/OFF menu item"));
				this->DoToggleRadioState();
				break;
			}
		case IDM_TODAY_OPTIONS :
			{
				TraceInfo(_D("User chose Options menu item"));
				this->DoShowOptions(SSS_FLAG_PLUGIN_ENABLED);
				break;
			}
		case IDM_TODAY_PHONE_SETTINGS :
			{
				TraceInfo(_D("User chose Phone Settings menu item"));
				this->DoPhoneSettings();
				break;
			}
		case IDM_TODAY_ABOUT :
			{
				TraceInfo(_D("User chose About menu item"));
				this->DoShowOptions(SSS_FLAG_PLUGIN_ENABLED | SSS_FLAG_SHOW_ABOUT_ONLY);
				break;
			}
		}
	}

	TraceLeave(_D("CSSSToday::DoAction"));

	return;
}

void CSSSToday::DoShowOptions(DWORD dwFlags)
{
	LRESULT lrResult;

	TraceEnter(_D("CSSSToday::DoShowOptions"));

	lrResult = DialogBoxParam(m_hmInstance, MAKEINTRESOURCE(IDD_TODAY_CUSTOM), m_hwTodayWnd, (DLGPROC)OptionsTodayWndProc, (LPARAM)dwFlags);
	TraceInfo(_D("CSSSToday::DoShowOptions: Back from DialogBoxParam, lrResult = %08X"), lrResult);

	if ((dwFlags & SSS_FLAG_SHOW_ABOUT_ONLY) != SSS_FLAG_SHOW_ABOUT_ONLY)
	{
		m_pOptions->GetTodayOptions(g_dwGetAllOptions | g_dwGetForce);
		this->UpdateTodayItemData(true);
	}

	TraceLeave(_D("CSSSToday::DoShowOptions"));
}

void CSSSToday::DoPhoneSettings()
{
	PROCESS_INFORMATION pi;
	TCHAR	SZProcess[MAX_PATH + 1];
	TCHAR	szCtlPnl[WIV_MAX_NAME + 1];

	TraceEnter(_D("CSSSToday::DoPhoneSettings"));
	_snwprintf(SZProcess, MAX_PATH, _T("%s%s"), GetWindowsPath(), g_szCtlPnl);
	_snwprintf(szCtlPnl, WIV_MAX_NAME, _T("%s,20"), g_szCplMain);

//	SetCursor(LoadCursor(NULL, IDC_WAIT));
	::CreateProcess(SZProcess, szCtlPnl, NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	TraceLeave(_D("CSSSToday::DoPhoneSettings"));
}

void CSSSToday::DoRefresh()
{
	TraceEnter(_D("CSSSToday::DoRefresh"));
	TraceDetail(_D("CSSSToday::DoRefresh: Update display to indicate refresh, and set force refresh flag"));

	_snwprintf(m_szPhonePrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_STATUS_LABEL);
	_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_REFRESHING, SSS_MAX_PHONE_NUMBER);
	_tcsncpy(m_szProvider, SSS_TEXT_PHONE_PROVIDER_LOOKING, SSS_MAX_PROVIDER);

	this->RefreshWindow(TRUE);

	TodayFlags.TodayBits.ForceRefresh = true;
	TraceLeave(_D("CSSSToday::DoRefresh"));
}

void CSSSToday::DoSwitchSIM()
{
	TraceEnter(_D("CSSSToday::DoSwitchSIM"));

	_snwprintf(m_szPhonePrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_STATUS_LABEL);
	_tcsncpy(m_szProvider, SSS_TEXT_PHONE_PROVIDER_LOOKING, SSS_MAX_PROVIDER);

	TodayFlags.TodayBits.Initializing = false;
	SetupPhoneStateFlags();

	this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeNeutral));

	// If phone already off, turn radio on,
	// else just switch radio off, setting flag to switch back on

	if (!TodayFlags.TodayBits.RadioIsOn)
	{
		_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_TURNING_ON, SSS_MAX_PHONE_NUMBER);
		this->RefreshWindow(TRUE);
		m_pPhone->TurnPhoneOn();
	}
	else
	{
		_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_SWITCHING_SIM, SSS_MAX_PHONE_NUMBER);
		this->RefreshWindow(TRUE);
		m_pPhone->SwitchSIM();
		TodayFlags.TodayBits.RadioIsOn = false;
	}

	TraceLeave(_D("CSSSToday::DoSwitchSIM"));
}

void CSSSToday::DoToggleRadioState()
{
	TraceEnter(_D("CSSSToday::DoToggleRadioState"));

	TodayFlags.TodayBits.Initializing = false;
	SetupPhoneStateFlags();

	_snwprintf(m_szPhonePrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_STATUS_LABEL);
	_tcsncpy(m_szProvider, SSS_TEXT_PHONE_PROVIDER_LOOKING, SSS_MAX_PROVIDER);

	// If phone is on, switch it off, else switch it on,
	this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeNeutral));

	if (!TodayFlags.TodayBits.RadioIsOn)
	{
		_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_TURNING_ON, SSS_MAX_PHONE_NUMBER);
		this->RefreshWindow(TRUE);
		m_pPhone->TurnPhoneOn();
	}
	else
	{
		_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_TURNING_OFF, SSS_MAX_PHONE_NUMBER);
		this->RefreshWindow(TRUE);
		m_pPhone->TurnPhoneOff();
		TodayFlags.TodayBits.RadioIsOn = false;
	}

	TraceLeave(_D("CSSSToday::DoToggleRadioState"));
}

void CSSSToday::OnTodayPaint(const HDC hDC)
{
	HFONT hFontOld;
	RECT rect;
	SIZE size;
	
	TraceEnter(_D("CSSSToday::OnTodayPaint"));

	SetTextColor(hDC, m_crTodayText);
	SetBkMode(hDC,TRANSPARENT);

	this->FormatDisplayLines();

	// If only showing one line, concatenate line 2 onto line 1
	if (TodayFlags.TodayBits.SingleLineDisplay)
	{
		_tcsncat(m_szLineOneText, WIV_SPACE_STRING, (MAX_PATH - _tcslen(m_szLineOneText)));
		_tcsncat(m_szLineOneText, m_szLineTwoText, (MAX_PATH - _tcslen(m_szLineOneText)));
		this->SetItemHeight(g_dwTodaySingleLineHeight);
	}
	else
	{
		this->SetItemHeight(g_dwTodayDoubleLineHeight);
	}
	
	// Draw first line of text using specified font
	HFONT	hFont = (TodayFlags.TodayBits.Line1BoldFont ? m_hfBoldTodayFont : m_hfNormalTodayFont);
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextExtentPoint32(hDC, m_szLineOneText, _tcslen(m_szLineOneText), &size);
	rect.left = 30;
	rect.top = 2;
	rect.right = rect.left + size.cx;
	rect.bottom = rect.top + size.cy;

	DrawText(hDC, m_szLineOneText, -1, &rect, DT_LEFT);

	SelectObject(hDC, hFontOld);

	// If enabled, draw second line of text using specified font
	if (!TodayFlags.TodayBits.SingleLineDisplay)
	{
		HFONT	hFont = (TodayFlags.TodayBits.Line2BoldFont ? m_hfBoldTodayFont : m_hfNormalTodayFont);

		hFontOld = (HFONT)SelectObject(hDC, hFont);

		GetTextExtentPoint32(hDC, m_szLineTwoText, _tcslen(m_szLineTwoText), &size);
		rect.left = 30;
		rect.top = 15;
		rect.right = rect.left + size.cx;
		rect.bottom = rect.top + size.cy;

		DrawText(hDC, m_szLineTwoText, -1, &rect, DT_LEFT);

		SelectObject(hDC, hFontOld);
	}

	TraceLeave(_D("CSSSToday::OnTodayPaint"));
}

void CSSSToday::FormatDisplayLines()
{

	TCHAR	szPhonePrompt[SSS_MAX_STATUS_PROMPT + 1];

	TraceEnter(_D("CSSSToday::FormatDisplayLines"));

	_tcsncpy(szPhonePrompt, m_szPhonePrompt, SSS_MAX_STATUS_PROMPT);

	if ((TodayFlags.TodayBits.SingleLineDisplay && TodayFlags.TodayBits.RadioIsOn)
	&& (_tcslen(m_pPhone->GetPhoneNumber()) != 0)
	&& (TodayFlags.TodayBits.ShowPhoneNumber && (TodayFlags.TodayBits.ShowTSP)))
	{
		_tcsncpy(szPhonePrompt, SSS_TEXT_PHONE_STATUS_READY_SL, SSS_MAX_STATUS_PROMPT);
	}

	_tcsncpy(m_szLineOneText, szPhonePrompt, MAX_PATH);

	if ((!TodayFlags.TodayBits.ShowPhoneNumber) && TodayFlags.TodayBits.RadioIsOn)
	{
		_tcsncpy(m_szLineOneText, SSS_TEXT_PHONE_NUMBER_NOT_SHOWN, MAX_PATH);
		_tcsncat(m_szLineOneText, szPhonePrompt, MAX_PATH - _tcslen(m_szLineOneText));
	}

	_tcsncat(m_szLineOneText, m_szPhoneNumber, MAX_PATH - _tcslen(m_szLineOneText));

	if (((_tcsncmp(m_szPhonePrompt, SSS_TEXT_PHONE_STATUS_LABEL, SSS_MAX_STATUS_PROMPT) == 0) && TodayFlags.TodayBits.SingleLineDisplay)
	|| ((!TodayFlags.TodayBits.RadioIsOn) && TodayFlags.TodayBits.SingleLineDisplay)
	|| ((!TodayFlags.TodayBits.ShowTSP) && (_tcsncmp(m_szProvider, SSS_TEXT_PHONE_STATUS_REGISTERING, SSS_MAX_PROVIDER) != 0))
	|| ((TodayFlags.TodayBits.ShowTSP) && TodayFlags.TodayBits.SingleLineDisplay && (_tcsncmp(m_szProvider, SSS_TEXT_PHONE_PROVIDER_LOOKING, SSS_MAX_PROVIDER) == 0)))
	{
		_zclr(m_szLineTwoText);
	}
	else
	{
		if (TodayFlags.TodayBits.SingleLineDisplay)
		{
			_tcsncpy(m_szLineTwoText, SSS_TEXT_PHONE_PROVIDER_LABEL_SL, SSS_MAX_STATUS_PROMPT);
			_tcsncat(m_szLineTwoText, m_szProvider, (MAX_PATH - _tcslen(m_szLineTwoText)));
		}
		else
		{
			_tcsncpy(m_szLineTwoText, m_szProviderPrompt, MAX_PATH);
			_tcsncat(m_szLineTwoText, m_szProvider, MAX_PATH - _tcslen(m_szLineOneText));
		}
	}

	TraceLeave(_D("CSSSToday::FormatDisplayLines"));
	return;
}

int CSSSToday::OnTodayCreate(LPCREATESTRUCT lpCreateStruct)
{
	TraceEnter(_D("CSSSToday::OnTodayCreate"));
/*
	if (!TodayFlags.TodayBits.CommsStarted)
	{
		TraceDetail(_D("CSSSToday::OnTodayCreate: Calling m_pPhone->StartComms"));
		m_pPhone->StartComms();
		TraceDetail(_D("CSSSToday::OnTodayCreate: Back from m_pPhone->StartComms"));
		TodayFlags.TodayBits.CommsStarted = true;
		TraceDetail(_D("CSSSToday::OnTodayCreate: Calling m_pPhone->GetPhonebookDetails"));
		m_pPhone->GetPhonebookDetails();
		TraceDetail(_D("CSSSToday::OnTodayCreate: Back from m_pPhone->GetPhonebookDetails"));
	}
*/
	TraceLeave(_D("CSSSToday::OnTodayCreate"), (DWORD)0);

	return 0;
}

void CSSSToday::OnTodayDestroy()
{
	TraceEnter(_D("CSSSToday::OnTodayDestroy"));
/*	
	TraceDetail(_D("CSSSToday::OnTodayDestroy: Calling DisablePowerNotifications"));
	DisablePowerNotifications();
	TraceDetail(_D("CSSSToday::OnTodayDestroy: Back from DisablePowerNotifications"));
	
	TraceDetail(_D("CSSSToday::OnTodayDestroy: Calling LicenseDeregisterNotification with 0x%08X"), &LicenseTodayNotify);
	LicenseDeregisterNotification(&LicenseTodayNotify);
	TraceDetail(_D("CSSSToday::OnTodayDestroy: Back from LicenseDeregisterNotification"));

	m_pPhone->TodayDestroyed();
	m_pOptions->TodayDestroyed();
*/
	TodayFlags.TodayBits.CommsStarted = false;

	// Force a refresh of data next time window is created
	TodayFlags.TodayBits.ForceRefresh = true;
	DefWindowProc(m_hwTodayWnd, WM_DESTROY, 0, 0);
	
	TraceLeave(_D("CSSSToday::OnTodayDestroy"));
}

void CSSSToday::OnTodayEraseBkgnd(const HDC hDC)
{
	TODAYDRAWWATERMARKINFO dwi;

	TraceEnter(_D("CSSSToday::OnTodayEraseBkgnd"));

	dwi.hdc = hDC;
	GetClientRect(m_hwTodayWnd, &dwi.rc);		
	dwi.hwnd = m_hwTodayWnd;

	SendMessage(m_hwParentHwnd, TODAYM_DRAWWATERMARK, 0,(LPARAM)&dwi);
	TraceLeave(_D("CSSSToday::OnTodayEraseBkgnd"));
}

BOOL CSSSToday::OnTodayCustomQueryRefreshCache(TODAYLISTITEM *pTodayListItem)
{
	BOOL	blResult		= FALSE;
	DWORD	dwHeight		= m_uiHeight;
	static	bool bLicense1	= false;
	static	bool bLicense2	= false;
	static	bool bLicense3	= false;
	
	TraceEnter(_D("CSSSToday::OnTodayCustomQueryRefreshCache"));

	TraceDetail(_D("CSSSToday::OnTodayCustomQueryRefreshCache: ptli->cyp = %d"), pTodayListItem->cyp);

	pTodayListItem->prgbCachedData = (LPBYTE)UpdateSources(pTodayListItem, &dwHeight);
	TraceDetail(_D("CSSSToday::OnTodayCustomQueryRefreshCache: pTodayListItem->prgbCachedData = 0x%08X"), pTodayListItem->prgbCachedData);
	TraceDetail(_D("CSSSToday::OnTodayCustomQueryRefreshCache: pTodayListItem->cbCachedData   = %d"), pTodayListItem->cbCachedData);

	memcpy(&m_tliTLI, pTodayListItem, sizeof(TODAYLISTITEM));

	blResult = (dwHeight != 0);

	TraceDetail(_D("CSSSToday::OnTodayCustomQueryRefreshCache: blResult = %d, dwHeight = %d, ForceRefresh = %d, TapToRegister = %d, PleaseRegister = %d, LicenseInvalid = %d"),
		blResult, dwHeight, TodayFlags.TodayBits.ForceRefresh, TodayFlags.TodayBits.TapToRegister, TodayFlags.TodayBits.PleaseRegister, TodayFlags.TodayBits.LicenseInvalid);

	if ((blResult != 0) || (TodayFlags.TodayBits.ForceRefresh) || (TodayFlags.TodayBits.TapToRegister) || (TodayFlags.TodayBits.PleaseRegister) || (TodayFlags.TodayBits.LicenseInvalid))
	{
		if (blResult != 0)
		{
			TodayFlags.TodayBits.ForceRefresh = true;
		}

		if (TodayFlags.TodayBits.PleaseRegister & !bLicense1)
		{
			TodayFlags.TodayBits.ForceRefresh = true;
			bLicense1 = true;
		}

		if (TodayFlags.TodayBits.TapToRegister & !bLicense2)
		{
			if (TodayFlags.TodayBits.LicenseInvalid & !bLicense3)
			{
				TodayFlags.TodayBits.ForceRefresh = true;
				bLicense3 = true;
			}
			else
			{
				TodayFlags.TodayBits.ForceRefresh = true;
				bLicense2 = true;
			}
		}

		if (TodayFlags.TodayBits.ForceRefresh)
		{
			// Forced refresh set
			TraceDetail(_D("CSSSToday::OnTodayCustomQueryRefreshCache: Forced refresh set, so update display, pTodayListItem->cyp = %d"), pTodayListItem->cyp);

			// Reset force refresh flag
			TodayFlags.TodayBits.ForceRefresh = false;
			blResult = TRUE;

			this->UpdateTodayItemData(true);
		}

#ifdef SSS_V2_IMP
		if (TodayFlags.TodayBits.RadioIsOn)
		{
			m_pPhone->RefreshSignalQuality();
		}
#endif //#ifdef SSS_V2_IMP
	}

	TraceLeave(_D("CSSSToday::OnTodayCustomQueryRefreshCache"), (DWORD)blResult);
	return blResult;
}

BOOL CSSSToday::OnTodayCustomClearCache(TODAYLISTITEM *pTodayListItem)
{
	TraceEnter(_D("CSSSToday::OnTodayCustomClearCache"));

	TraceLeave(_D(" CSSSToday::OnTodayCustomClearCache"), (DWORD)FALSE);

	return FALSE;
}

void CSSSToday::OnTodaySettingChange(const UINT nFlags, LPCTSTR lpszSection)
{
	TraceEnter(_D("CSSSToday::OnTodaySettingChange"));
	TraceLeave(_D("CSSSToday::OnTodaySettingChange"));
}

LRESULT CSSSToday::OnTodayNotify(const UINT nID, NMHDR* pNMHDR)
{
	TraceEnter(_D("CSSSToday::OnTodayNotify"));
	TraceLeave(_D("CSSSToday::OnTodayNotify, returning <0>"));
	return 0;
}

LRESULT CSSSToday::OnTodayMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TraceEnter(_D("CSSSToday::OnTodayMessage"));
	TraceLeave(_D("CSSSToday::OnTodayMessage, return result of DefWindowProc"));
	return DefWindowProc(m_hwTodayWnd, uMsg, wParam, lParam);
}

// Other protected methods
void CSSSToday::DrawTodayIcon(const HDC hDC)
{
	TraceEnter(_D("CSSSToday::DrawTodayIcon"));
	if (m_hiIcon)
	{
		SetBkMode(hDC, TRANSPARENT);
		::DrawIcon(hDC, m_ptIconPos.x, m_ptIconPos.y, m_hiIcon);
	}
	TraceLeave(_D("CSSSToday::DrawTodayIcon"));
}

void CSSSToday::GetTodayDefaults()
{
    LOGFONT lf;
	HFONT	hSysFont;
	DWORD	dwRequired;
    LONG	dwFontSize = 12;

	TraceEnter(_D("CSSSToday::GetTodayDefaults"));

    if (m_hfNormalTodayFont)
    {
        DeleteObject(m_hfNormalTodayFont);
		m_hfNormalTodayFont = NULL;
    }

	if (m_hfBoldTodayFont)
    {
        DeleteObject(m_hfBoldTodayFont);
		m_hfBoldTodayFont = NULL;
    }

	// Default today item font color
	m_crTodayText = SendMessage(m_hwParentHwnd, TODAYM_GETCOLOR, (WPARAM)TODAYCOLOR_TEXT, NULL);

	// Default today fonts
	hSysFont = (HFONT)GetStockObject(SYSTEM_FONT);
	GetObject(hSysFont, sizeof(LOGFONT), &lf);

	if (IsSecondEdition())
	{
	    SHGetUIMetrics(SHUIM_FONTSIZE_PIXEL, &dwFontSize, sizeof(dwFontSize), &dwRequired);
	    lf.lfHeight = -dwFontSize;
	}
	else
	{
		HDC hdc = GetDC(m_hwTodayWnd);

		lf.lfHeight = -1 * (17 * GetDeviceCaps(hdc, LOGPIXELSY) / 72) / 2;
		ReleaseDC(m_hwTodayWnd, hdc);
	}

	lstrcpy(lf.lfFaceName, _T("Tahoma"));

	// Normal text
    lf.lfWeight = FW_NORMAL;
    m_hfNormalTodayFont = CreateFontIndirect(&lf);

	// Bold text
    lf.lfWeight = FW_BOLD;
    m_hfBoldTodayFont = CreateFontIndirect(&lf);

	TraceLeave(_D("CSSSToday::GetTodayDefaults"));
}

void CSSSToday::UpdateOptionsDialog(WPARAM wParam, LPARAM lParam)
{
	HWND hwOptions = m_pOptions->GetWindowHandle();
	if (hwOptions != NULL)
	{
		SendMessage(hwOptions, WM_WIV_REFRESH, wParam, lParam);
	}
}

bool CSSSToday::UpdateTodayItemData(const bool bRefresh)
{
	TCHAR	szPhoneNumber[SSS_MAX_PHONE_NUMBER + 1];
	TCHAR	szProvider[SSS_MAX_PROVIDER + 1];
	bool	blStateHasChanged = false;
	TCHAR	szInfo[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	TraceEnter(_D("CSSSToday::UpdateTodayItemData"));

	blStateHasChanged = this->SetupPhoneStateFlags();							
							
	if (!blStateHasChanged && !bRefresh)
	{
		TraceDetail(_D("CSSSToday::UpdateTodayItemData, blStateHasChanged = %d, bRefresh = %d"), blStateHasChanged, bRefresh);
		TodayFlags.TodayBits.ForceRefresh = false;
		TraceLeave(_D("CSSSToday::UpdateTodayItemData"), (DWORD)blStateHasChanged);
		return blStateHasChanged;
	}

	_snwprintf(m_szProviderPrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_PROVIDER_LABEL);

	TraceDetail(_D("CSSSToday::UpdateTodayItemData: TodayFlags.Options.License          = 0x%08X"), TodayFlags.Options.License);
	TraceDetail(_D("CSSSToday::UpdateTodayItemData: TodayFlags.TodayBits.RadioIsOn      = %d"), TodayFlags.TodayBits.RadioIsOn);
	TraceDetail(_D("CSSSToday::UpdateTodayItemData: TodayFlags.TodayBits.TapToRegister  = %d"), TodayFlags.TodayBits.TapToRegister);
	TraceDetail(_D("CSSSToday::UpdateTodayItemData: TodayFlags.TodayBits.LicenseInvalid = %d"), TodayFlags.TodayBits.LicenseInvalid);
	TraceDetail(_D("CSSSToday::UpdateTodayItemData: TodayFlags.TodayBits.PleaseRegister = %d"), TodayFlags.TodayBits.PleaseRegister);
	TraceDetail(_D("CSSSToday::UpdateTodayItemData: TodayFlags.TodayBits.SIMMissing     = %d"), TodayFlags.TodayBits.SIMMissing);
	TraceDetail(_D("CSSSToday::UpdateTodayItemData: TodayFlags.TodayBits.IncorrectSIM   = %d"), TodayFlags.TodayBits.IncorrectSIM);

	if (!TodayFlags.TodayBits.RadioIsOn)
	{
		TraceDetail(_D("CSSSToday::UpdateTodayItemData: Radio is off"));


		if (TodayFlags.TodayBits.TapToRegister)
		{
			if (TodayFlags.TodayBits.LicenseInvalid)
			{
				TraceInfo(_D("CSSSToday::UpdateTodayItemData: LicenseInvalid is set"));
				this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeNeutral));
				_tcsncpy(m_szPhonePrompt, SSS_TEXT_TODAY_LICENSE_INVALID, SSS_MAX_STATUS_PROMPT);
				_zclr(m_szPhoneNumber);
				_tcsncpy(m_szProviderPrompt, SSS_TEXT_TODAY_LICENSE_TAP_FOR_INFO, SSS_MAX_STATUS_PROMPT);
				_zclr(m_szProvider);
			}
			else
			{
				TraceInfo(_D("CSSSToday::UpdateTodayItemData: TapToRegister is set"));
				this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeNeutral));
				_tcsncpy(m_szPhonePrompt, SSS_TEXT_TODAY_LICENSE_EXPIRED, SSS_MAX_STATUS_PROMPT);
				_zclr(m_szPhoneNumber);
				_tcsncpy(m_szProviderPrompt, SSS_TEXT_TODAY_LICENSE_TAP_FOR_INFO, SSS_MAX_STATUS_PROMPT);
				_zclr(m_szProvider);
			}
		}
		else
		{
			TraceInfo(_D("CSSSToday::UpdateTodayItemData: Phone state is OFF (<%08x>)"), g_dwPhoneStateOff);
			this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeOff));
			_tcsncpy(m_szPhonePrompt, SSS_TEXT_PHONE_STATUS_OFF, SSS_MAX_STATUS_PROMPT);
			_zclr(m_szPhoneNumber);
			_zcpy(m_szProvider, SSS_TEXT_PHONE_PROVIDER_NOT_APPLY);
		}
	}
	else 
	{
		TraceInfo(_D("CSSSToday::UpdateTodayItemData: Phone state is ON (<%08x>)"), g_dwPhoneStateOn);

		TraceInfo(_D("CSSSToday::UpdateTodayItemData: m_dwSIMLockedState = %08X"), m_dwSIMLockedState);

		if (TodayFlags.TodayBits.PleaseRegister)
		{
			TraceDetail(_D("CSSSToday::UpdateTodayItemData: PleaseRegister is set"));
			this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeNeutral));
			_tcsncpy(m_szPhonePrompt, SSS_TEXT_TODAY_LICENSE_EXPIRED, SSS_MAX_STATUS_PROMPT);
			_zclr(m_szPhoneNumber);
			_tcsncpy(m_szProviderPrompt, SSS_TEXT_TODAY_LICENSE_FUNCTION_LTD, SSS_MAX_STATUS_PROMPT);
			_tcsncpy(m_szProvider, SSS_TEXT_TODAY_LICENSE_PLEASE_REG, SSS_MAX_PROVIDER);
		}
		else 
		{
			if ((TodayFlags.TodayBits.SIMMissing) || (TodayFlags.TodayBits.IncorrectSIM))
			{
				TraceDetail(_D("CSSSToday::UpdateTodayItemData: SIM is missing or invalid"));
				this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeOff));
				_tcsncpy(m_szPhonePrompt, SSS_TEXT_PHONE_STATUS_INVALID, SSS_MAX_STATUS_PROMPT);
				_zclr(m_szPhoneNumber);
				_zcpy(m_szProvider, SSS_TEXT_PHONE_PROVIDER_NOT_APPLY);

			} else if (TodayFlags.TodayBits.PINRequired)
			{
				TraceInfo(_D("CSSSToday::UpdateTodayItemData: Waiting for PIN"));

				if (TodayFlags.TodayBits.NoAutoPIN)
				{
					_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_WAIT_PIN, SSS_MAX_PHONE_NUMBER);
					_tcsncpy(m_szProviderPrompt, SSS_TEXT_PHONE_TAP_TO_ENTER_PIN, SSS_MAX_STATUS_PROMPT);
				}
				else
				{
					_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_ENTERING_PIN, SSS_MAX_PHONE_NUMBER);
				}

				_snwprintf(m_szPhonePrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_STATUS_LABEL);
				_tcsncpy(m_szProvider, SSS_TEXT_PHONE_PROVIDER_LOOKING, SSS_MAX_PROVIDER);
			}
			else
			{
				TraceInfo(_D("CSSSToday::UpdateTodayItemData: Phone is unlocked"));
				_tcsncpy(szPhoneNumber, TodayFlags.TodayBits.ShowPhoneNumber ? m_pPhone->GetPhoneNumber() : WIV_EMPTY_STRING, SSS_MAX_PHONE_NUMBER);
				_tcsncpy(szProvider, m_pPhone->GetProvider(), SSS_MAX_PROVIDER);
				TraceInfo(_D("CSSSToday::UpdateTodayItemData: szPhoneNumber = %s"), szPhoneNumber);

				if (_tcslen(m_pPhone->GetPhoneNumber()) == 0)
				{
					this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeNeutral));
					_snwprintf(m_szPhonePrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_STATUS_LABEL);
					_tcsncpy(m_szPhoneNumber, SSS_TEXT_PHONE_STATUS_WAIT_SIGNAL, SSS_MAX_PHONE_NUMBER);
					_tcsncpy(m_szProvider, SSS_TEXT_PHONE_PROVIDER_LOOKING, SSS_MAX_PROVIDER);
				}
				else
				{
					if (TodayFlags.TodayBits.SIMReady)
					{
						this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeFull));
					}
					else
					{
						this->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeOn));
					}

					if (TodayFlags.TodayBits.ShowPhoneNumber)
					{
						_snwprintf(m_szPhonePrompt, SSS_MAX_STATUS_PROMPT, _T("%s: "), SSS_TEXT_PHONE_STATUS_READY);
					}
					else
					{
						_snwprintf(m_szPhonePrompt, SSS_MAX_STATUS_PROMPT, _T("%s"), SSS_TEXT_PHONE_STATUS_READY);
					}

					_tcsncpy(m_szPhoneNumber, szPhoneNumber, SSS_MAX_PHONE_NUMBER);

					if (_tcslen(szProvider) == 0)
					{
						_tcsncpy(m_szProvider, SSS_TEXT_PHONE_STATUS_REGISTERING, SSS_MAX_PROVIDER);
					}
					else
					{
						if (_tcslen(m_szRegistration) != 0)
						{
							_tcsncat(szProvider, _T(" ("), SSS_MAX_PROVIDER - _tcslen(szProvider));
							_tcsncat(szProvider, m_szRegistration, SSS_MAX_PROVIDER - _tcslen(szProvider));
							_tcsncat(szProvider, _T(")"), SSS_MAX_PROVIDER - _tcslen(szProvider));
						}
						_tcsncpy(m_szProvider, szProvider, SSS_MAX_PROVIDER);
					}
				}
			}
		}
	}

	if (bRefresh) this->RefreshWindow(TRUE);

	TodayFlags.TodayBits.ForceRefresh = false;

	TraceLeave(_D("CSSSToday::UpdateTodayItemData"), (DWORD)blStateHasChanged);
	return blStateHasChanged;
}

bool CSSSToday::SetupPhoneStateFlags()
{
	struct {
		unsigned int	StateHasChanged				: 1;
		unsigned int	RadioIsOn					: 1;
		unsigned int	SIMReady					: 1;
		unsigned int	PINRequired					: 1;
		unsigned int	PINPending					: 1;
		unsigned int	PINError					: 1;
		unsigned int	NoAutoPIN					: 1;
		unsigned int	TapToEnterPIN				: 1;
		unsigned int	AllowAutoPINAfterInit		: 1;
		unsigned int	AllowAutoPINAfterRadioON	: 1;
		unsigned int	TapToRegister				: 1;
		unsigned int	PleaseRegister				: 1;
		unsigned int	LicenseInvalid				: 1;
		unsigned int	SIMMissing					: 1;
		unsigned int	IncorrectSIM				: 1;
	} Flags;

	DWORD	dwRadioSupport;
	DWORD	dwEquipmentState;
	DWORD	dwReadyState;
	DWORD	dwPhoneLockedState;
	DWORD	dwLicense;

	TraceEnter(_D("CSSSToday::SetupPhoneStateFlags"));

	memset(&Flags, 0, sizeof(Flags));

	TraceInfo(_D("CSSSToday::SetupPhoneStateFlags: Calling GetPhoneState"));
	m_pPhone->GetPhoneState(dwRadioSupport, dwEquipmentState, dwReadyState, dwPhoneLockedState, dwLicense);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Back from GetPhoneState, dwRadioSupport = <%08X>, dwEquipmentState = <%08X>, dwReadyState = <%08X>, dwPhoneLockedState = <%08X>, dwLicense = <%08X>"),
							dwRadioSupport, dwEquipmentState, dwReadyState, dwPhoneLockedState, dwLicense);

	ProcessLockedState(dwPhoneLockedState);

	Flags.SIMMissing = TodayFlags.TodayBits.SIMMissing;
	Flags.IncorrectSIM = TodayFlags.TodayBits.IncorrectSIM;
	Flags.PINError = TodayFlags.TodayBits.PINError;
	Flags.PINRequired = TodayFlags.TodayBits.PINRequired;
	Flags.PINPending = TodayFlags.TodayBits.PINPending;

	Flags.PleaseRegister = ((dwLicense & 0x0000F000) == 8192);
	Flags.TapToRegister = ((dwLicense & 0x0000F000) == 4096);
	Flags.LicenseInvalid = ((dwLicense & 0x00000400) == 1024);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: (dwLicense & 0x0000F000) = %d"), (dwLicense & 0x0000F000));

	// Determine phone on/off state
	if ((dwRadioSupport == RIL_RADIOSUPPORT_ON) && (dwEquipmentState == RIL_EQSTATE_FULL))
	{
		Flags.RadioIsOn = true;
		m_pOptions->GetTodayOptions(g_dwGetSecurityOptions | g_dwGetForce);
		Flags.AllowAutoPINAfterInit = TodayFlags.TodayBits.AllowAutoPINAfterInit;
		Flags.AllowAutoPINAfterRadioON = TodayFlags.TodayBits.AllowAutoPINAfterRadioON;
	}

	if (Flags.RadioIsOn)
	{
		TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: dwReadyState = 0x%08X"), dwReadyState);
		TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: m_dwSIMLockedState = %08X"), m_dwSIMLockedState);
		if (dwReadyState & RIL_READYSTATE_SIMREADY)
		{
			Flags.SIMReady = true;
		}
	}

	// If PIN is needed
	if (Flags.PINRequired)
	{
		// Determine if auto PIN entry will happen
		if (!(TodayFlags.TodayBits.Initializing ? Flags.AllowAutoPINAfterInit : Flags.AllowAutoPINAfterRadioON))
		{
			Flags.NoAutoPIN = true;
		}
	}

	// If PIN required and no auto entry
	if (Flags.NoAutoPIN)
	{
		Flags.TapToEnterPIN = true;
	}
	else
	{
		Flags.TapToEnterPIN = false;

//		TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Calling UnlockSIM"));
//		m_pPhone->UnlockSIM();
//		TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Back from UnlockSIM"));
	}

	// Set flags
	if ((Flags.RadioIsOn != TodayFlags.TodayBits.RadioIsOn)
		||(Flags.SIMReady != TodayFlags.TodayBits.SIMReady)
		||(Flags.PINRequired != TodayFlags.TodayBits.PINRequired)
		||(Flags.PINPending != TodayFlags.TodayBits.PINPending)
		||(Flags.PINError != TodayFlags.TodayBits.PINError)
		||(Flags.NoAutoPIN != TodayFlags.TodayBits.NoAutoPIN)
		||(Flags.TapToEnterPIN != TodayFlags.TodayBits.TapToEnterPIN)
		||(Flags.AllowAutoPINAfterInit != TodayFlags.TodayBits.AllowAutoPINAfterInit)
		||(Flags.AllowAutoPINAfterRadioON != TodayFlags.TodayBits.AllowAutoPINAfterRadioON)
		||(Flags.PleaseRegister != TodayFlags.TodayBits.PleaseRegister)
		||(Flags.TapToRegister != TodayFlags.TodayBits.TapToRegister)
		||(Flags.LicenseInvalid != TodayFlags.TodayBits.LicenseInvalid)
		||(Flags.SIMMissing != TodayFlags.TodayBits.SIMMissing)
		||(Flags.IncorrectSIM != TodayFlags.TodayBits.IncorrectSIM))
	{
		Flags.StateHasChanged = true;
	}

	if (Flags.RadioIsOn)
	{
		TodayFlags.TodayBits.RadioIsOn = Flags.RadioIsOn & !Flags.TapToRegister;
		TodayFlags.TodayBits.SIMReady = Flags.SIMReady & !Flags.TapToRegister;
		TodayFlags.TodayBits.TapToEnterPIN = Flags.TapToEnterPIN & !Flags.TapToRegister;
		TodayFlags.TodayBits.AllowAutoPINAfterInit = Flags.AllowAutoPINAfterInit & !Flags.TapToRegister;
		TodayFlags.TodayBits.AllowAutoPINAfterRadioON = Flags.AllowAutoPINAfterRadioON & !Flags.TapToRegister;
	}
	else
	{
		TodayFlags.TodayBits.RadioIsOn = Flags.RadioIsOn;
		TodayFlags.TodayBits.SIMReady = Flags.SIMReady;
		TodayFlags.TodayBits.TapToEnterPIN = Flags.TapToEnterPIN;
		TodayFlags.TodayBits.AllowAutoPINAfterInit = Flags.AllowAutoPINAfterInit;
		TodayFlags.TodayBits.AllowAutoPINAfterRadioON = Flags.AllowAutoPINAfterRadioON;
	}

	TodayFlags.TodayBits.PINRequired = Flags.PINRequired;
	TodayFlags.TodayBits.NoAutoPIN = Flags.NoAutoPIN;

	TodayFlags.TodayBits.PleaseRegister = Flags.PleaseRegister;
	TodayFlags.TodayBits.TapToRegister = Flags.TapToRegister;
	TodayFlags.TodayBits.LicenseInvalid = Flags.LicenseInvalid;

	TodayFlags.TodayBits.SIMMissing = Flags.SIMMissing;
	TodayFlags.TodayBits.IncorrectSIM = Flags.IncorrectSIM;
	TodayFlags.TodayBits.PINError = Flags.PINError;
	TodayFlags.TodayBits.PINRequired = Flags.PINRequired;
	TodayFlags.TodayBits.PINPending = Flags.PINPending;

	TraceDetail(WIV_EMPTY_STRING);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Initializing        = %d"), TodayFlags.TodayBits.Initializing);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: AutoPINAfterInit    = %d"), TodayFlags.TodayBits.AllowAutoPINAfterInit);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: AutoPINAfterRadioON = %d"), TodayFlags.TodayBits.AllowAutoPINAfterRadioON);
	TraceDetail(WIV_EMPTY_STRING);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Radio On            = %d"), TodayFlags.TodayBits.RadioIsOn);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: PIN Required        = %d"), TodayFlags.TodayBits.PINRequired);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: No Auto PIN         = %d"), TodayFlags.TodayBits.NoAutoPIN);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Tap to enter PIN    = %d"), TodayFlags.TodayBits.TapToEnterPIN);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Please register     = %d"), TodayFlags.TodayBits.PleaseRegister);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Tap to register     = %d"), TodayFlags.TodayBits.TapToRegister);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: License Invalid     = %d"), TodayFlags.TodayBits.LicenseInvalid);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Missing SIM         = %d"), TodayFlags.TodayBits.SIMMissing);
	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: Incorrect SIM       = %d"), TodayFlags.TodayBits.IncorrectSIM);

	TraceDetail(_D("CSSSToday::SetupPhoneStateFlags: State has changed   = %d"), Flags.StateHasChanged);

	TraceLeave(_D("CSSSToday::SetupPhoneStateFlags"), (DWORD)Flags.StateHasChanged);
	return (bool)(Flags.StateHasChanged != 0);
}	

void CSSSToday::ProcessLockedState(DWORD dwSIMLockedState)
{
	TraceEnter(_D("CSSSToday::ProcessLockedState"));

	TraceInfo(_D("CSSSToday::ProcessLockedState: dwSIMLockedState is %08x"), dwSIMLockedState);

	TodayFlags.TodayBits.SIMMissing = false;
	TodayFlags.TodayBits.IncorrectSIM = false;
	TodayFlags.TodayBits.PINError = false;
	TodayFlags.TodayBits.PINRequired = false;
	TodayFlags.TodayBits.PINPending = false;

	switch (dwSIMLockedState)
	{
	case RIL_E_SIMNOTINSERTED:
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: SIM is missing"));
			TodayFlags.TodayBits.SIMMissing = true;
			break;
		}
	case RIL_E_SIMWRONG:
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Invalid or unrecognised SIM"));
			TodayFlags.TodayBits.IncorrectSIM = true;
			break;
		}
	case RIL_LOCKEDSTATE_UNKNOWN:
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: Unknown"));
			break;
		}
	case RIL_LOCKEDSTATE_READY:
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: SIM is unlocked"));
			break;
		}
	case RIL_LOCKEDSTATE_SIM_PIN :	// PIN required
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: Waiting for PIN"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_SIM_PUK:
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: SIM is blocked, requires PUK"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINError = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_SIM_PIN :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting phone-to-sim password"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_FSIM_PIN :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting phone-to-first-sim password"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_FSIM_PUK :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting phone-to-first-sim PUK"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINError = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_SIM_PIN2 :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting PIN2/CHV2"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_SIM_PUK2 :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting PUK2"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINError = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_NET_PIN :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting network personalization PIN"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_NET_PUK :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting network personalization PUK"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINError = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_NETSUB_PIN :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting network subset personalization PIN"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_NETSUB_PUK :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting network subset personalization PUK"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINError = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_SP_PIN :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting service provider PIN"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_SP_PUK :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting service provider PUK"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINError = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_CORP_PIN :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting corporate personalization PIN"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}
	case RIL_LOCKEDSTATE_PH_CORP_PUK :
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Locked State: ME awaiting corporate personalization PUK"));
			TodayFlags.TodayBits.PINRequired = true;
			TodayFlags.TodayBits.PINError = true;
			TodayFlags.TodayBits.PINPending = true;
			break;
		}

	default:
		{
			TraceDetail(_D("CSSSToday::ProcessLockedState: Other locked state (%08X)"), dwSIMLockedState);
			TodayFlags.TodayBits.PINPending = true;
		}
	}
	
	TraceLeave(_D("CSSSToday::ProcessLockedState"));

	return;
}


//======================================================================
// ShowContextMenu - Show a popup menu.
//======================================================================
LRESULT CSSSToday::ShowContextMenu (HWND hWnd, UINT uiMenuID, POINT ptTap)
{
	LRESULT			lrResult = -1;

	HMENU			hmMenu;
	HMENU			hmMenuMain;
	POINT			ptPoint;
	MENUITEMINFO	miiMII;

	TraceEnter(_D("ShowContextMenu"));

	ptPoint.x = ptTap.x;
	ptPoint.y = ptTap.y;

	MapWindowPoints(hWnd, NULL, &ptPoint, 1);

	hmMenuMain = LoadMenu(m_hmInstance, MAKEINTRESOURCE(uiMenuID));
	hmMenu = GetSubMenu(hmMenuMain, 0);

	memset(&miiMII, 0, sizeof(MENUITEMINFO));
	miiMII.cbSize = sizeof(MENUITEMINFO);
	miiMII.fMask = MIIM_TYPE;

	miiMII.dwTypeData = (LPTSTR)SSS_TEXT_OPTIONS;
	SetMenuItemInfo(hmMenu, 0, true, &miiMII );	
	
	miiMII.dwTypeData = (LPTSTR)SSS_TEXT_REFRESH;
	SetMenuItemInfo(hmMenu, 2, true, &miiMII );	
	
	miiMII.dwTypeData = (LPTSTR)SSS_TEXT_SWITCH_SIM;
	SetMenuItemInfo(hmMenu, 3, true, &miiMII );	

	miiMII.dwTypeData = (LPTSTR)SSS_TEXT_PHONE_SETTINGS;
	SetMenuItemInfo(hmMenu, 6, true, &miiMII );

	miiMII.dwTypeData = (LPTSTR)SSS_TEXT_ABOUT_TITLE;
	SetMenuItemInfo(hmMenu, 8, true, &miiMII );
	
	SetupPhoneStateFlags();

	if (!TodayFlags.TodayBits.RadioIsOn)
	{
		EnableMenuItem(hmMenu, 3, MF_BYPOSITION | MF_GRAYED); 
		miiMII.dwTypeData = (LPTSTR)SSS_TEXT_TURN_PHONE_ON;
		SetMenuItemInfo(hmMenu, 4, true, &miiMII );	
		EnableMenuItem(hmMenu, 6, MF_BYPOSITION | MF_GRAYED); 
	}
	else
	{
		miiMII.dwTypeData = (LPTSTR)SSS_TEXT_TURN_PHONE_OFF;
		SetMenuItemInfo(hmMenu, 4, true, &miiMII );	
		if (!TodayFlags.TodayBits.SIMReady)
		{
			EnableMenuItem(hmMenu, 6, MF_BYPOSITION | MF_GRAYED); 
		}
	}
	
	lrResult = TrackPopupMenuEx(hmMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, ptPoint.x, 
					 ptPoint.y, hWnd, NULL);
	DestroyMenu (hmMenuMain);
	DestroyMenu (hmMenu);
	
	TraceLeave(_D("ShowContextMenu"), lrResult);

    return lrResult; // 0 = menu showed, but no selection, -1 = wasn't a tap and hold, else = ID of menu command
}

bool CALLBACK LicenseTodayNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig)
{
	return m_pThis->TodayLicenseNotify(lpLicenseData, dwLicenseConfig);
}

