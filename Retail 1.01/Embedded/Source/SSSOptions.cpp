//======================================================================
//
// SSSOptions.cpp: implementation of the CSSSOptions class.
//
//======================================================================

extern struct SSS_GLOBALS *SSSGlobals;

#include "SSSCommon.h"
#include "WiVUtils.h"
#include "WiVLicense.h"
#include "WiVLang.h"
#include "WiVReg.h"
#include "SSSOptions.h"
#include "SSSToday.h"
#include "SSSPhone.h"
#include "ril.h"

using namespace WiV;

#ifdef SSS_V2_IMP
//**************************************************************
const int SSS_ICON_SET_INDEX_UNKNOWN			=	0;
const int SSS_ICON_SET_INDEX_EMPTY				=	-1;
const int SSS_ICON_SET_INDEX_COMMENT			=	-2;
const int SSS_ICON_SET_INDEX_HEADER				=	-3;
const int SSS_ICON_SET_INDEX_VERSION			=	-4;
const int SSS_ICON_SET_INDEX_TABLE_BEGIN		=	-5;
const int SSS_ICON_SET_INDEX_TABLE_END			=	-6;
const int SSS_ICON_SET_INDEX_ICON_ON			=	-7;
const int SSS_ICON_SET_INDEX_ICON_OFF			=	-8;
const int SSS_ICON_SET_INDEX_ICON_NEUTRAL		=	-9;

// Identifier strings
const TCHAR SSS_ICON_SET_LABEL_HEADER_ID[]		=	_T("ICONSFILE");
const TCHAR SSS_ICON_SET_LABEL_VERSION[]		=	_T("Version");
const TCHAR SSS_ICON_SET_LABEL_TABLE_BEGIN[]	=	_T("ICONSETSTART");
const TCHAR SSS_ICON_SET_LABEL_TABLE_END[]		=	_T("ICONSETEND");
const TCHAR SSS_ICON_SET_LABEL_ICON_ON[]		=	_T("ON");
const TCHAR SSS_ICON_SET_LABEL_ICON_OFF[]		=	_T("OFF");
const TCHAR SSS_ICON_SET_LABEL_ICON_NEUTRAL[]	=	_T("NEUTRAL");

static	HANDLE		m_hfCustomIconsFile = NULL;
static	TCHAR		m_szCustomIconsFilePath[MAX_PATH + 1] = WIV_EMPTY_STRING;
static	PBYTE		m_pbCustomIconsBuff = 0; 
static	long		m_lCustomIconsThreadId = 0L;
static	DWORD		m_dwCustomIconsFileSize = 0;
static	DWORD		m_dwCustomIconsBufferPointer = 0;

//**************************************************************
#endif //#ifdef SSS_V2_IMP

//======================================================================
//Member variables
//======================================================================

// Flags
union {
	DWORD	dwFlags;
	struct  {
		unsigned int	Flags						: 18;
		unsigned int	License						: 14;
	}Options;

	struct {
		unsigned int	HidePersonalInfo			: 1;
		unsigned int	AllowOptionsCancel			: 1;
		unsigned int	CancelDown					: 1;
		unsigned int	DontPaintLicense			: 1;

		unsigned int	OptionsActive				: 1;
		unsigned int	FullScreen					: 1;
		unsigned int	RegType						: 1;
		unsigned int	PhoneIsOff					: 1;
		unsigned int	SIMUnlocked					: 1;
		unsigned int	SIMReady					: 1;
		unsigned int	SIMMissing					: 1;
		unsigned int	IncorrectSIM				: 1;

		unsigned int	NotApplicable				: 6;

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
	}OptionsBits;
} OptionsFlags;

static	HMODULE			m_hmInstance;
static	HWND			m_hwOptionsWnd;
static	HWND			m_hwPropSheet;
static	TCHAR			m_szTitle[MAX_PATH + 1];
static	SHACTIVATEINFO	m_saiSAI;
static	int				m_nDaysRemaining		= 12;
static	HWND			m_hwTabCtrl				= NULL;
static	HWND			m_hwSpinButton			= NULL;

static	HFONT			m_hfTitleFont;
static	HFONT			m_hfSemiBoldFont;
static	HFONT			m_hfExtraLightFont;
static	HFONT			m_hfBoldFont;
static	HFONT			m_hfNormalFont;

static	DWORD			m_dwCurIconSet;

static	DWORD			m_dwCurTapAction;
static	DWORD			m_dwCurTAHAction;
static	DWORD			m_dwCurTodayIconTapAction;
static	DWORD			m_dwCurTodayIconTAHAction;
static	DWORD			m_dwCurButtonAction;

static	DWORD			m_adwSecuritySequence[3][6];
static	DWORD			m_dwSecuritySeqNumber;
static	DWORD			m_dwSecuritySeqStep;

static	HWND			m_hwInfoWnd;

static	TCHAR			m_szMfg[SSS_MAX_MFG + 1];
static	TCHAR			m_szModel[SSS_MAX_MODEL + 1];
static	TCHAR			m_szRevision[SSS_MAX_REVISION + 1];
static	TCHAR			m_szIMEI[SSS_MAX_IMEI + 1];
static	TCHAR			m_szSubscriber[SSS_MAX_SUBSCRIBER + 1];
static	TCHAR			m_szICCID[SSS_MAX_ICCID + 1];
static	TCHAR			m_szEnteredPIN[SSS_MAX_PIN + 1];
static	TCHAR			m_szDefaultSIM[WIV_MAX_NAME + 1];

static	TCHAR			m_szPBLocation[SSS_MAX_PB_LOCATION_LENGTH + 1];
static	DWORD			m_dwPBTotal;
static	DWORD			m_dwPBUsed;

// Reference to Today plugin and Phone classes
static	CSSSToday		*m_pToday;
static	CSSSPhone		*m_pPhone;
static	CSSSOptions		*m_pThis;

static	HBITMAP			m_hbmpBackgroundBitmap = NULL;
static	HBRUSH			m_ahbrushAbout[SSS_MAX_BRUSH_COUNT];
static	HBRUSH			m_ahbrushDisplay[SSS_MAX_BRUSH_COUNT];
static	HBRUSH			m_ahbrushLanguage[SSS_MAX_BRUSH_COUNT];
static	HBRUSH			m_ahbrushActions[SSS_MAX_BRUSH_COUNT];
static	HBRUSH			m_ahbrushAppearance[SSS_MAX_BRUSH_COUNT];
static	HBRUSH			m_ahbrushSecurity[SSS_MAX_BRUSH_COUNT];
static	HBRUSH			m_ahbrushPINEntry[SSS_MAX_BRUSH_COUNT];
static	HBRUSH			m_ahbrushInformation[SSS_MAX_BRUSH_COUNT];
static	HBRUSH			m_ahbrushRegistration[SSS_MAX_BRUSH_COUNT];

static	TCHAR			m_aszIconSets[SSS_MAX_AVAILABLE_ICON_SETS][SSS_MAX_ICON_SET_NAME_LENGTH + 1];
static	TCHAR			m_aszActions[SSS_MAX_AVAILABLE_ACTIONS][SSS_MAX_ACTION_NAME_LENGTH + 1];

static	PWIVLANG		m_lpsDefaultLanguage = NULL;
static	PWIVLANG		m_lpsCurrentLanguage = NULL;

static	HWND			m_hwInfoDialog = NULL;
static	HWND			m_hwPINEntryDialog = NULL;

static	bool			m_blDefaultLanguageFound = false;

#ifdef WIV_DEBUG

static	bool			m_blTraceActive				= SSS_DEFAULT_TRACE_ACTIVE;
static	TraceLevel		m_tlTraceLevel				= SSS_DEFAULT_TRACE_LEVEL;
static	TCHAR			m_szTracePath[MAX_PATH + 1]	= WIV_EMPTY_STRING;
static	int				m_nNumTraceFiles;
static	HWND			m_hwDebugWnd				= NULL;

static	HBRUSH			m_ahbrushDebug[SSS_MAX_BRUSH_COUNT];

static	TCHAR			m_aszTraceLevels[SSS_MAX_AVAILABLE_TRACE_LEVELS][SSS_MAX_TRACE_LEVEL_NAME_LENGTH + 1];

#endif

//======================================================================
// User configurable options
//======================================================================

static	bool			m_blShowPhoneNumber		= SSS_DEFAULT_SHOW_PHONE_NUMBER;
static	bool			m_blShowTSP				= SSS_DEFAULT_SHOW_TSP;
static	bool			m_blSingleLineDisplay	= SSS_DEFAULT_SINGLE_LINE_DISPLAY;

static	bool			m_blLine1BoldFont		= SSS_DEFAULT_LINE_1_BOLD_FONT;
static	bool			m_blLine2BoldFont		= SSS_DEFAULT_LINE_2_BOLD_FONT;
static	DWORD			m_dwIconSet				= SSS_DEFAULT_TODAY_ICON_SET;

static	DWORD			m_dwTapAction			= SSS_DEFAULT_TAP_ACTION;
static	DWORD			m_dwTAHAction			= SSS_DEFAULT_TAH_ACTION;
static	DWORD			m_dwTodayIconTapAction	= SSS_DEFAULT_TODAY_ICON_TAP_ACTION;
static	DWORD			m_dwTodayIconTAHAction	= SSS_DEFAULT_TODAY_ICON_TAH_ACTION;
static	DWORD			m_dwButtonAction		= SSS_DEFAULT_BUTTON_ACTION;

static	bool			m_blAllowAutoPINAfterInit		= false;
static	bool			m_blAllowAutoPINAfterRadioON	= false;
static	bool			m_blDefaultSIM					= false;
static	TCHAR			m_szPIN[SSS_MAX_PIN + 1]		= SSS_DEFAULT_PIN;
static	BYTE			m_abPIN[WIV_MAX_BINARY + 1]		= SSS_DEFAULT_PIN1;

static CSSSOptions::SSS_SETTINGS	CurrentSettings;
static CSSSOptions::SSS_SETTINGS	NewSettings;

//======================================================================
// Constructors
//======================================================================
CSSSOptions::CSSSOptions()
{
	TraceEnter(_D("CSSSOptions::CSSSOptions (Default Constructor)"));
	TraceLeave(_D("CSSSOptions::CSSSOptions (Default Constructor)"));
}

CSSSOptions::CSSSOptions(HMODULE hmModule, CSSSPhone *pPhone, bool bFullScreen)
{
	TCHAR	szTitle[MAX_PATH + 1] = WIV_EMPTY_STRING;

	TraceEnter(_D("CSSSOptions::CSSSOptions (Constructor)"));

	SSSGlobals = GlobalsLoad(SSSGlobals);

	m_hmInstance = NULL;
	memset(&OptionsFlags, 0, sizeof(OptionsFlags));

	m_pPhone = NULL;

	m_hmInstance = hmModule;

	m_pPhone = pPhone;
	OptionsFlags.OptionsBits.FullScreen = bFullScreen;
	OptionsFlags.OptionsBits.AllowOptionsCancel = true;

	TraceDetail(_D("CSSSOptions::CSSSOptions (Constructor): Calling LicenseRegisterNotification with 0x%08X"), &LicenseOptionsNotify);
	LicenseRegisterNotification(&LicenseOptionsNotify);
	TraceDetail(_D("CSSSOptions::CSSSOptions (Constructor): Back from LicenseRegisterNotification"));
	
	if(_tcslen(g_szNoLicenseLabel) <= 0)
	{
		if (!g_pSSSData)
		{
			TraceError(_D("CSSSOptions::CSSSOptions (Constructor): SSS Data is NULL"));
			_snwprintf(szTitle, MAX_PATH, _T("%s %s"), _T("SIM Status Switcher"), SSS_TEXT_MENU_OPTIONS);
		}
		else
		{
			_snwprintf(szTitle, MAX_PATH, _T("%s %s"), _tcsrev(g_pSSSData->szRevProductName), SSS_TEXT_MENU_OPTIONS);
		}
	}
	else
	{
		_snwprintf(szTitle, MAX_PATH, _T("%s %s"), g_szProductName, SSS_TEXT_MENU_OPTIONS);
	}

	TraceInfo(_D("CSSSOptions::CSSSOptions (Constructor): Calling this->SetTitle"));
	this->SetTitle(szTitle);

	memset(&m_saiSAI, 0, sizeof(SHACTIVATEINFO));
	m_saiSAI.cbSize = sizeof(SHACTIVATEINFO);

	TraceLeave(_D("CSSSOptions::CSSSOptions (Constructor)"));
}

//======================================================================
// Destructor
//======================================================================
CSSSOptions::~CSSSOptions()
{
	TraceEnter(_D("CSSSOptions::~CSSSOptions (Destructor)"));

	TraceDetail(_D("CSSSOptions::~CSSSOptions (Destructor): Calling LicenseDeregisterNotification with 0x%08X"), &LicenseOptionsNotify);
	LicenseDeregisterNotification(&LicenseOptionsNotify);
	TraceDetail(_D("CSSSOptions::}CSSSOptions (Destructor): Back from LicenseDeregisterNotification"));
/*	
	if (m_hfTitleFont)
	{
		DeleteObject(m_hfTitleFont);
		m_hfTitleFont = NULL;
	}

	if (m_hfSemiBoldFont)
	{
		DeleteObject(m_hfSemiBoldFont);
		m_hfSemiBoldFont = NULL;
	}

	if (m_hfExtraLightFont)
	{
		DeleteObject(m_hfExtraLightFont);
		m_hfExtraLightFont = NULL;
	}

	if (m_hfBoldFont)
	{
		DeleteObject(m_hfBoldFont);
		m_hfBoldFont = NULL;
	}

	if (m_hfNormalFont)
	{
		DeleteObject(m_hfNormalFont);
		m_hfNormalFont = NULL;
	}

	if (m_hbmpBackgroundBitmap)
	{
		DeleteObject(m_hbmpBackgroundBitmap);
		m_hbmpBackgroundBitmap = NULL;
	}
*/
	TraceLeave(_D("CSSSOptions::~CSSSOptions (Destructor)"));
}

void CSSSOptions::TodayDestroyed()
{
	TraceEnter(_D("CSSSOptions::TodayDestroyed"));
	TraceDetail(_D("CSSSOptions::TodayDestroyed: Calling LicenseDeregisterNotification with 0x%08X"), &LicenseOptionsNotify);
	LicenseDeregisterNotification(&LicenseOptionsNotify);
	TraceDetail(_D("CSSSOptions::TodayDestroyed: Back from LicenseDeregisterNotification"));
	TraceLeave(_D("CSSSOptions::TodayDestroyed"));
}

//======================================================================
// Save reference to Today class
//======================================================================
void CSSSOptions::SetTodayClass(DWORD pToday)
{
	TraceEnter(_D("CSSSOptions::SetTodayClass"));
	m_pToday = (CSSSToday *)pToday;
	TraceLeave(_D("CSSSOptions::SetTodayClass"));
}

//======================================================================
// Setup data and appearance used by Today plugin
//======================================================================
void CSSSOptions::GetTodayOptions(DWORD dwFlags)
{
	bool	bForce = false;

	TraceEnter(_D("CSSSOptions::GetTodayOptions"));
	TraceInfo(_D("CSSSOptions::GetTodayOptions: m_hmInstance = <%08X>"), m_hmInstance);

	// Check to see if user options required
	if (dwFlags & g_dwGetUserOptions)
	{
		TraceInfo(_D("CSSSOptions::GetTodayOptions: Calling GetUserOptions to get user configurable options"));
		GetUserOptions();
		TraceInfo(_D("CSSSOptions::GetTodayOptions: Back from GetUserOptions"));

		TraceInfo(_D("CSSSOptions::GetTodayOptions: Setting whether phone number and TSP displayed and whether single line"));
		m_pToday->SetShowPhoneNumber(m_blShowPhoneNumber);
		m_pToday->SetShowTSP(m_blShowTSP);
		m_pToday->SetSingleLineDisplay(m_blSingleLineDisplay);

		TraceInfo(_D("CSSSOptions::GetTodayOptions: Setting today item height dependant on 1 or 2 display lines specified"));
		m_pToday->SetItemHeight(m_blSingleLineDisplay ? g_dwTodaySingleLineHeight : g_dwTodayDoubleLineHeight);

		TraceInfo(_D("CSSSOptions::GetTodayOptions: Setting normal/bold fonts for today item display lines 1 and 2"));
		m_pToday->SetLine1BoldFont(m_blLine1BoldFont);
		m_pToday->SetLine2BoldFont(m_blLine2BoldFont);

		TraceInfo(_D("CSSSOptions::GetTodayOptions: Getting icon set from options, and setting appropriate icon on today item"));
		m_pToday->SetIconSet(m_dwIconSet);
		m_pToday->SetIcon(IDFromIconSet(m_dwIconSet, g_dwIconTypeNeutral));

		TraceInfo(_D("CSSSOptions::GetTodayOptions: Getting actions from options, and setting them for today item interaction"));
		m_pToday->SetTapAction(m_dwTapAction);
		m_pToday->SetTAHAction(m_dwTAHAction);
		m_pToday->SetTodayIconTapAction(m_dwTodayIconTapAction);
		m_pToday->SetTodayIconTAHAction(m_dwTodayIconTAHAction);
		m_pToday->SetButtonAction(m_dwButtonAction);
	}

	// Check to see if security options required
	if (dwFlags & g_dwGetSecurityOptions)
	{
		if (dwFlags & g_dwGetForce) bForce = true;

		TraceInfo(_D("CSSSOptions::GetTodayOptions: Calling GetSecurityOptions to get security options"));
		GetSecurityOptions(bForce);
		TraceInfo(_D("CSSSOptions::GetTodayOptions: Back from GetSecurityOptions"));
	}

#ifdef WIV_DEBUG
	// Check to see if debug options required
	if (dwFlags & g_dwGetDebugOptions)
	{
		TraceInfo(_D("CSSSOptions::GetTodayOptions: Calling ReadDebugOptions to get debug options"));
		GetDebugOptions();
		TraceInfo(_D("CSSSOptions::GetTodayOptions: Back from ReadDebugOptions"));
	}
#endif

	if ((dwFlags & g_dwGetAllOptions) && (dwFlags & g_dwGetForce))
	{
		NewSettings = CurrentSettings;
	}

	TraceLeave(_D("CSSSOptions::GetTodayOptions"));

	return;
}

//======================================================================
// Get Device Information
//======================================================================
void CSSSOptions::GetDeviceInfo(CSSSPhone *pPhone, const bool bRead, const bool bProcess, const bool bWay)
{
	BOOL		bResult;
	DWORD		dwSize;
	SSS_BUFFER1	buf1;
	BYTE		out[WIV_MAX_BINARY + 1];

	TraceEnter(_D("CSSSOptions::GetDeviceInfo"));
	
	if (!(bProcess | bRead))
	{
		TraceLeave(_D("CSSSOptions::GetDeviceInfo: Nothing to do"));
		return;
	}

	if (bRead & (pPhone != NULL))
	{
		TraceInfo(_D("CSSSOptions::GetDeviceInfo: Calling pPhone->GetICCID"));
		pPhone->GetICCID(m_szICCID);
		TraceInfo(_D("CSSSOptions::GetDeviceInfo: Back from pPhone->GetICCID, ICCID = <%s>"), m_szICCID);

		if (_tcslen(m_szICCID) == 0)
		{
			TraceInfo(_D("CSSSOptions::GetDeviceInfo: ICCID is empty"));
			TraceLeave(_D("CSSSOptions::GetDeviceInfo: ICCID is empty"));
			return;
		}
	}

	if (!bProcess)
	{
		TraceLeave(_D("CSSSOptions::GetDeviceInfo: information retrieved"));
		return;
	}

	TraceInfo(_D("CSSSOptions::GetDeviceInfo: m_abPIN = <%s"), BtoS(m_abPIN, sizeof(m_abPIN)));
	memcpy(&buf1.dwBufLen, m_abPIN, sizeof(DWORD));

	dwSize = buf1.dwBufLen;

	TraceInfo(_D("CSSSOptions::GetDeviceInfo: dwSize = <%08x>"), dwSize);

	if (dwSize == 0)
	{
		TraceLeave(_D("CSSSOptions::GetDeviceInfo, size is zero"));
		return;
	}

	memcpy(buf1.bBuffer, &m_abPIN[sizeof(DWORD)], dwSize);

	TraceInternal(_D("CSSSOptions::GetDeviceInfo: Before Crypt, PIN = <%s>"), BtoS((BYTE *)&buf1.bBuffer, dwSize));

	bResult = Crypt(m_szICCID, (BYTE *)&buf1.bBuffer, out, &dwSize, bWay);

	TraceInternal(_D("CSSSOptions::GetDeviceInfo: Back from Crypt, bResult = <%08x>, dwSize = %d, out = <%s>"), bResult, dwSize, BtoS(out, dwSize));
	
	if (!bResult)
	{
		TraceError(_D("CSSSOptions::GetDeviceInfo: Information processing failed"));
		memset(buf1.bBuffer, 0, dwSize);
		buf1.dwBufLen = 0;
		memcpy(m_abPIN, &buf1, dwSize + sizeof(DWORD));
		TraceLeave(_D("CSSSOptions::GetDeviceInfo"));
		return;
	}

	memcpy(buf1.bBuffer, out, dwSize);
	buf1.dwBufLen = dwSize;
	memcpy(m_abPIN, &buf1, buf1.dwBufLen + sizeof(DWORD));

	TraceInfo(_D("CSSSOptions::GetDeviceInfo: Information processed"));
	TraceLeave(_D("CSSSOptions::GetDeviceInfo"));

	return;
}

//======================================================================
// Association with option dialog created by system
//======================================================================
void CSSSOptions::AssociateWithOptionsDlg(HWND hWnd)
{
    TraceEnter(_D("CSSSOptions::AssociateWithOptionsDlg"));
    TraceInfo(_D("CSSSOptions::AssociateWithOptionsDlg, m_hwOptionsWnd = <%08X>"), m_hwOptionsWnd);
	if (m_hwOptionsWnd == NULL)
	{
		m_hwOptionsWnd = hWnd;
	}

    TraceLeave(_D("CSSSOptions::AssociateWithOptionsDlg"), (DWORD)m_hwOptionsWnd);

	return;
}

//======================================================================
// Get application instance 
//======================================================================
HMODULE CSSSOptions::GetInstance()
{
    TraceEnter(_D("CSSSOptions::GetInstance"));

    TraceLeave(_D("CSSSOptions::GetInstance"), (DWORD)m_hmInstance);

	return m_hmInstance;
};

//======================================================================
// Get window handle 
//======================================================================
HWND CSSSOptions::GetWindowHandle()
{
    TraceEnter(_D("CSSSOptions::GetWindowHandle"));
	
    TraceLeave(_D("CSSSOptions::GetWindowHandle"), (DWORD)m_hwOptionsWnd);
	
	return m_hwOptionsWnd;
};

//======================================================================
// Get whether full screen or not 
//======================================================================
bool CSSSOptions::GetFullScreen()
{
    TraceEnter(_D("CSSSOptions::GetFullScreen"));

    TraceLeave(_D("CSSSOptions::GetFullScreen"), (DWORD)OptionsFlags.OptionsBits.FullScreen);

	return OptionsFlags.OptionsBits.FullScreen;
};

//======================================================================
// 
//======================================================================
void CSSSOptions::SetInstance(HMODULE HMODULE)
{
    TraceEnter(_D("CSSSOptions::SetInstance"));

    TraceInfo(_D("CSSSOptions::SetInstance, HMODULE = <%08X>"), HMODULE);
	m_hmInstance = HMODULE;

    TraceLeave(_D("CSSSOptions::SetInstance"));

	return;
};

//======================================================================
// 
//======================================================================
void CSSSOptions::SetFullScreen(const bool bFullScreen)
{
    TraceEnter(_D("CSSSOptions::SetFullScreen"));

    TraceInfo(_D("CSSSOptions::SetFullScreen, bFullScreen = %d"), bFullScreen);
	OptionsFlags.OptionsBits.FullScreen = bFullScreen;

    TraceLeave(_D("CSSSOptions::SetFullScreen"));

	return;
};

//======================================================================
// 
//======================================================================
void CSSSOptions::SetTitle(const LPCTSTR lpszTitle, bool bRefresh)
{
    TraceEnter(_D("CSSSOptions::SetTitle"));

	_tcsncpy(m_szTitle, lpszTitle, MAX_PATH);

	if (bRefresh)
		RefreshWindow(m_hwOptionsWnd);

    TraceLeave(_D("CSSSOptions::SetTitle"));

	return;
}

//======================================================================
// 
//======================================================================
void CSSSOptions::RefreshWindow(HWND hwWnd, BOOL blErase)
{
    TraceEnter(_D("CSSSOptions::RefreshWindow"));

	if (hwWnd)
	{
		RECT rect;
		GetClientRect(hwWnd, &rect);
		InvalidateRect(hwWnd, &rect, blErase);
		UpdateWindow(hwWnd);
	}

    TraceLeave(_D("CSSSOptions::RefreshWindow"));

	return;
}

//======================================================================
// Get Methods
//======================================================================
DWORD CSSSOptions::GetOptionsStyle()
{
	DWORD dwStyle;

    TraceEnter(_D("CSSSOptions::GetOptionsStyle"));

	dwStyle =  GetWindowLong(m_hwOptionsWnd, GWL_STYLE);

    TraceLeave(_D("CSSSOptions::GetOptionsStyle"), dwStyle);

	return dwStyle;
}

//======================================================================
// User options from registry
//======================================================================
bool CSSSOptions::GetAllowAutoPINAfterInit()
{
    TraceEnter(_D("CSSSOptions::GetAllowAutoPINAfterInit"));
    TraceLeave(_D("CSSSOptions::GetAllowAutoPINAfterInit"), (DWORD)m_blAllowAutoPINAfterInit);
	return m_blAllowAutoPINAfterInit;
}

bool CSSSOptions::GetAllowAutoPINAfterRadioON()
{
    TraceEnter(_D("CSSSOptions::GetAllowAutoPINAfterRadioON"));
    TraceLeave(_D("CSSSOptions::GetAllowAutoPINAfterRadioON"), (DWORD)m_blAllowAutoPINAfterRadioON);
	return m_blAllowAutoPINAfterRadioON;
}

bool CSSSOptions::GetSingleLineDisplay()
{
    TraceEnter(_D("CSSSOptions::GetSingleLineDisplay"));
    TraceLeave(_D("CSSSOptions::GetSingleLineDisplay"), (DWORD)m_blSingleLineDisplay);
	return m_blSingleLineDisplay;
}

bool CSSSOptions::GetShowPhoneNumber()
{
    TraceEnter(_D("CSSSOptions::GetShowPhoneNumber"));
    TraceLeave(_D("CSSSOptions::GetShowPhoneNumber"), (DWORD)m_blShowPhoneNumber);
	return m_blShowPhoneNumber;
}

bool CSSSOptions::GetShowTSP()
{
    TraceEnter(_D("CSSSOptions::GetShowTSP"));
    TraceLeave(_D("CSSSOptions::GetShowTSP"), (DWORD)m_blShowTSP);
	return m_blShowTSP;
}

bool CSSSOptions::GetLine1Bold()
{
    TraceEnter(_D("CSSSOptions::GetLine1Bold"));
    TraceLeave(_D("CSSSOptions::GetLine1Bold"), (DWORD)m_blLine1BoldFont);
	return m_blLine1BoldFont;
}

bool CSSSOptions::GetLine2Bold()
{
    TraceEnter(_D("CSSSOptions::GetLine2Bold"));
    TraceLeave(_D("CSSSOptions::GetLine2Bold"), (DWORD)m_blLine2BoldFont);
	return m_blLine2BoldFont;
}

DWORD CSSSOptions::GetIconSet()
{
    TraceEnter(_D("CSSSOptions::GetIconSet"));
    TraceLeave(_D("CSSSOptions::GetIconSet"), m_dwIconSet);
	return m_dwIconSet;
}

DWORD CSSSOptions::GetTapAction()
{
    TraceEnter(_D("CSSSOptions::GetTapAction"));
    TraceLeave(_D("CSSSOptions::GetTapAction"), m_dwTapAction);
	return m_dwTapAction;
}

DWORD CSSSOptions::GetTAHAction()
{
    TraceEnter(_D("CSSSOptions::GetTAHAction"));
    TraceLeave(_D("CSSSOptions::GetTAHAction"), m_dwTAHAction);
	return m_dwTAHAction;
}

DWORD CSSSOptions::GetTodayIconTapAction()
{
    TraceEnter(_D("CSSSOptions::GetTodayIconTapAction"));
    TraceLeave(_D("CSSSOptions::GetTodayIconTapAction"), m_dwTodayIconTapAction);
	return m_dwTodayIconTapAction;
}

DWORD CSSSOptions::GetTodayIconTAHAction()
{
    TraceEnter(_D("CSSSOptions::GetTodayIconTAHAction"));
    TraceLeave(_D("CSSSOptions::GetTodayIconTAHAction"), m_dwTodayIconTAHAction);
	return m_dwTodayIconTAHAction;
}

DWORD CSSSOptions::GetButtonAction()
{
    TraceEnter(_D("CSSSOptions::GetButtonAction"));
    TraceLeave(_D("CSSSOptions::GetButtonAction"), m_dwButtonAction);
	return m_dwButtonAction;
}

void CSSSOptions::GetPIN(LPCTSTR szICCID, LPTSTR szPIN)
{
    TraceEnter(_D("CSSSOptions::GetPIN"));
	_tcsncpy(szPIN, m_szPIN, SSS_MAX_PIN);
    TraceLeave(_D("CSSSOptions::GetPIN"));
	return;
}

bool CSSSOptions::GetHidePersonalInfo()
{
    TraceEnter(_D("CSSSOptions::GetHidePersonalInfo"));
    TraceLeave(_D("CSSSOptions::GetHidePersonalInfo"), (DWORD)OptionsFlags.OptionsBits.HidePersonalInfo);
	return OptionsFlags.OptionsBits.HidePersonalInfo;
}

//======================================================================
// Message handlers
//======================================================================

bool CSSSOptions::OnOptionsInitDialog(DWORD dwFlags)
{
	SHINITDLGINFO	shidi;
	LRESULT			lrResult;
    SHMENUBARINFO	mbi;
	TCHAR			szBitmap[WIV_MAX_PATH + 1];

    TraceEnter(_D("CSSSOptions::OnOptionsInitDialog"));
    TraceInfo(_D("CSSSOptions::OnOptionsInitDialog: m_hwOptionsWnd = <%08X>"), m_hwOptionsWnd);

	if(_tcslen(g_szCopyrightLabel) > 0)
	{
		// Load the drop-down lists
		this->LoadIconSetsArray();
		this->LoadActionsArray();
#ifdef WIV_DEBUG
		this->LoadTraceLevelsArray();
#endif

		if( NULL == m_hbmpBackgroundBitmap )
		{
			_tcsncpy(szBitmap, GetInstallPath(), WIV_MAX_PATH);
			_tcsncat(szBitmap, g_szProductShortName, WIV_MAX_PATH - _tcslen(szBitmap));
			_tcsncat(szBitmap, _T(".bmp"), WIV_MAX_PATH - _tcslen(szBitmap));

			m_hbmpBackgroundBitmap = SHLoadDIBitmap(szBitmap);

		}
	}

	shidi.dwMask = SHIDIM_FLAGS;
	shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_EMPTYMENU | SHIDIF_SIZEDLGFULLSCREEN;
	shidi.hDlg = m_hwOptionsWnd;

	SHInitDialog(&shidi);

	m_hfTitleFont		= GetTitleFont(m_hwOptionsWnd);
	GetFont(&m_hfSemiBoldFont, m_hwOptionsWnd, FW_SEMIBOLD);
	GetFont(&m_hfExtraLightFont, m_hwOptionsWnd, FW_EXTRALIGHT, WIV_FONT_MEDIUM);
	GetFont(&m_hfBoldFont, m_hwOptionsWnd, FW_SEMIBOLD);
	GetFont(&m_hfNormalFont, m_hwOptionsWnd, FW_NORMAL);
			
    memset(&mbi, 0, sizeof(SHMENUBARINFO));
    mbi.cbSize = sizeof(SHMENUBARINFO);
    mbi.hwndParent = m_hwOptionsWnd;
    mbi.nToolBarId = NULL;
    mbi.hInstRes = m_hmInstance;
	mbi.dwFlags = SHCMBF_EMPTYBAR;

    SHCreateMenuBar(&mbi);

    TraceInfo(_D("CSSSOptions::OnOptionsInitDialog: Calling  ShowProperties"));
	lrResult = ShowProperties(dwFlags);
	TraceInfo(_D("CSSSOptions::OnOptionsInitDialog: Back from  ShowProperties, lrResult = %08x"), lrResult);

	// Determine if OK or Cancel tapped from options property sheet
	// and exit accordingly
	if(lrResult == IDOK)
	{
		OnOptionsOK();
	}
	else
	{
		OnOptionsCancel();
	}

    TraceLeave(_D("CSSSOptions::OnOptionsInitDialog"), (DWORD)0);

	return 0;
}

void CSSSOptions::OnOptionsOK()
{
    TraceEnter(_D("CSSSOptions::OnOptionsOK"));

    TraceInfo(_D("CSSSOptions::OnOptionsOK: OK tapped by user, saving settings"));

	if(_tcslen(g_szRetailLabel) <= 0)
	{
		TraceLeave(_D("CSSSOptions::OnOptionsOK"));
		EndDialog(m_hwOptionsWnd, IDCANCEL);

		return;
	}

	if (memcmp(&NewSettings, &CurrentSettings, sizeof(SSS_SETTINGS)) != 0)
	{
		TraceInfo(_D("sizeof(SSS_SETTINGS) = %d"), sizeof(SSS_SETTINGS));
		TraceInfo(_D("Original Settings: %s"), BtoS((BYTE *)&CurrentSettings, sizeof(SSS_SETTINGS)));
		TraceInfo(_D("New Settings     : %s"), BtoS((BYTE *)&NewSettings, sizeof(SSS_SETTINGS)));

		SetUserOptions();
		SetSecurityOptions();
#ifdef WIV_DEBUG
		SetDebugOptions();
#endif
		CurrentSettings = NewSettings;
	}

    TraceLeave(_D("CSSSOptions::OnOptionsOK"));
	EndDialog(m_hwOptionsWnd, IDOK);

	return;
}

void CSSSOptions::OnOptionsCancel()
{
    TraceEnter(_D("CSSSOptions::OnOptionsCancel"));

    TraceInfo(_D("CSSSOptions::OnOptionsCancel: Cancel tapped by user, settings will not be saved"));

    TraceLeave(_D("CSSSOptions::OnOptionsCancel"));
	EndDialog(m_hwOptionsWnd, IDCANCEL);

	return;
}

void CSSSOptions::OnOptionsCommand(UINT nID, UINT nNotifyCode, HWND hWndCtrl)
{
    TraceEnter(_D("CSSSOptions::OnOptionsCommand"));

	DefWindowProc(m_hwOptionsWnd, WM_COMMAND, MAKELONG(nID, nNotifyCode), (LPARAM)hWndCtrl);

    TraceLeave(_D("CSSSOptions::OnOptionsCommand"));

	return;
}

void CSSSOptions::OnOptionsSettingChange(UINT nFlags, LPCTSTR lpszSection)
{
    TraceEnter(_D("CSSSOptions::OnOptionsSettingChange"));

	if (!(GetOptionsStyle() & WS_CHILD))
	{
		SHHandleWMSettingChange(m_hwOptionsWnd, (WPARAM)nFlags, (LPARAM)lpszSection, &m_saiSAI);
	}

    TraceLeave(_D("CSSSOptions::OnOptionsSettingChange"));

	return;
}

void CSSSOptions::OnOptionsActivate(UINT nState, HWND hWndPrevious, BOOL bMinimized)
{
    TraceEnter(_D("CSSSOptions::OnOptionsActivate"));

	if (!(GetOptionsStyle() & WS_CHILD))
	{
		SHHandleWMActivate(m_hwOptionsWnd, MAKELONG(nState, (WORD)bMinimized), (LPARAM)hWndPrevious, &m_saiSAI, FALSE);
	    TraceLeave(_D("CSSSOptions::OnOptionsActivate"));
		return;
	}

	DefWindowProc(m_hwOptionsWnd, WM_ACTIVATE, MAKELONG(nState, (WORD)bMinimized), (LPARAM)hWndPrevious);

    TraceLeave(_D("CSSSOptions::OnOptionsActivate"));

	return;
}

LRESULT CSSSOptions::OnOptionsNotify(UINT nID, LPPSHNOTIFY lpSHNotify)
{
    TraceEnter(_D("CSSSOptions::OnOptionsNotify"));

    TraceLeave(_D("CSSSOptions::OnOptionsNotify"), (DWORD)0);

	return 0;
}

LRESULT CSSSOptions::OnOptionsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrResult;

    TraceEnter(_D("CSSSOptions::OnOptionsMessage"));

	lrResult = DefWindowProc(m_hwOptionsWnd, uMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::OnOptionsMessage"), lrResult);

	return lrResult;
}

void CSSSOptions::OnOptionsPaint(HDC hDC)
{
    TraceEnter(_D("CSSSOptions::OnOptionsPaint"));
    TraceLeave(_D("CSSSOptions::OnOptionsPaint"));
	return;
}

void CSSSOptions::OnOptionsDestroy()
{
    TraceEnter(_D("CSSSOptions::OnOptionsDestroy"));

	if (m_hfTitleFont)
	{
		DeleteObject(m_hfTitleFont);
		m_hfTitleFont = NULL;
	}
	
	if (m_hfSemiBoldFont)
	{
		DeleteObject(m_hfSemiBoldFont);
		m_hfSemiBoldFont = NULL;
	}
	
	if (m_hfExtraLightFont)
	{
		DeleteObject(m_hfExtraLightFont);
		m_hfExtraLightFont = NULL;
	}
	
	if (m_hfBoldFont)
	{
		DeleteObject(m_hfBoldFont);
		m_hfBoldFont = NULL;
	}
	
	if (m_hfNormalFont)
	{
		DeleteObject(m_hfNormalFont);
		m_hfNormalFont = NULL;
	}
	
	if (m_hbmpBackgroundBitmap)
	{
		DeleteObject(m_hbmpBackgroundBitmap);
		m_hbmpBackgroundBitmap = NULL;
	}

	OptionsFlags.OptionsBits.OptionsActive = false;
	DefWindowProc(m_hwOptionsWnd, WM_DESTROY, 0, 0);

    TraceLeave(_D("CSSSOptions::OnOptionsDestroy"));

	return;
}

//======================================================================
// Private methods
//======================================================================

void CSSSOptions::LoadSecurityStepsArrays()
{

	TraceEnter(_D("CSSSOptions::LoadSecurityStepsArrays"));

	m_adwSecuritySequence[0][0] = g_dwSecuritySequenceActionNoPIN;
	m_adwSecuritySequence[0][1] = g_dwSecuritySequenceActionNone;
	m_adwSecuritySequence[0][2] = g_dwSecuritySequenceActionNone;
	m_adwSecuritySequence[0][3] = g_dwSecuritySequenceActionNone;
	m_adwSecuritySequence[0][4] = g_dwSecuritySequenceActionNone;
	m_adwSecuritySequence[0][5] = g_dwSecuritySequenceActionNone;
	
	m_adwSecuritySequence[1][0] = g_dwSecuritySequenceActionCreate;
	m_adwSecuritySequence[1][1] = g_dwSecuritySequenceActionEnter;
	m_adwSecuritySequence[1][2] = g_dwSecuritySequenceActionConfirm;
	m_adwSecuritySequence[1][3] = g_dwSecuritySequenceActionNone;
	m_adwSecuritySequence[1][4] = g_dwSecuritySequenceActionNone;
	m_adwSecuritySequence[1][5] = g_dwSecuritySequenceActionNone;
	
	m_adwSecuritySequence[2][0] = g_dwSecuritySequenceActionChange;
	m_adwSecuritySequence[2][1] = g_dwSecuritySequenceActionCurrent;
	m_adwSecuritySequence[2][2] = g_dwSecuritySequenceActionNew;
	m_adwSecuritySequence[2][3] = g_dwSecuritySequenceActionConfirm;
	m_adwSecuritySequence[2][4] = g_dwSecuritySequenceActionNone;
	m_adwSecuritySequence[2][5] = g_dwSecuritySequenceActionNone;
	
	TraceDetail(_D("CSSSOptions::LoadSecurityStepsArrays: m_adwSecuritySequence = <%s>"), BtoS((BYTE *)&m_adwSecuritySequence, sizeof(m_adwSecuritySequence)));

	TraceLeave(_D("CSSSOptions::LoadSecurityStepsArrays"));

	return;
}

void CSSSOptions::LoadActionsArray()
{
	TraceEnter(_D("CSSSOptions::LoadActionsArray"));

	_tcsncpy(m_aszActions[0], SSS_TEXT_ACTION_REFRESH, SSS_MAX_ACTION_NAME_LENGTH);
	_tcsncpy(m_aszActions[1], SSS_TEXT_ACTION_SHOW_POPUP, SSS_MAX_ACTION_NAME_LENGTH);
	_tcsncpy(m_aszActions[2], SSS_TEXT_ACTION_OPTIONS, SSS_MAX_ACTION_NAME_LENGTH);
	_tcsncpy(m_aszActions[3], SSS_TEXT_ACTION_SWITCH_SIM, SSS_MAX_ACTION_NAME_LENGTH);
	_tcsncpy(m_aszActions[4], SSS_TEXT_ACTION_TOGGLE_RADIO, SSS_MAX_ACTION_NAME_LENGTH);
	_tcsncpy(m_aszActions[5], SSS_TEXT_ACTION_PHONE_SETTINGS, SSS_MAX_ACTION_NAME_LENGTH);
	
	TraceDetail(_D("CSSSOptions::LoadActionsArray: m_aszActions = <%s>"), BtoS((BYTE *)&m_aszActions, sizeof(m_aszActions)));

	TraceLeave(_D("CSSSOptions::LoadActionsArray"));

	return;
}

void CSSSOptions::InitialiseIconSetList(HWND hWnd)
{
	int		i;

	TraceEnter(_D("CSSSOptions::InitialiseIconSetList"));

	// List available icon sets into list box.
	SendDlgItemMessage (hWnd, IDC_APPEARANCE_LIST_ICON_SETS, LB_RESETCONTENT, 0, 0);
	
	for (i = 0; i < SSS_MAX_AVAILABLE_ICON_SETS; i++)
	{
		if (_tcsncmp(m_aszIconSets[i], g_szWIVEnd, SSS_MAX_ICON_SET_NAME_LENGTH) == 0)
		{
			break;
		}
		TraceInfo(_D("CSSSOptions::InitialiseIconSetList: Adding <%s> to Icon Sets List"), m_aszIconSets[i]);
		SendDlgItemMessage (hWnd, IDC_APPEARANCE_LIST_ICON_SETS, LB_ADDSTRING,
							0, (LPARAM)m_aszIconSets[i]);
	}

	TraceLeave(_D("CSSSOptions::InitialiseIconSetList"));

	return;
}

void CSSSOptions::LoadIconSetsArray()
{
	bool	blResult = false;
	int		nCurrentIndex = 0;

	TraceEnter(_D("CSSSOptions::LoadIconSetsArray"));

	_tcsncpy(m_aszIconSets[nCurrentIndex++], SSS_TEXT_ICON_SET_STANDARD_PHONE, SSS_MAX_ICON_SET_NAME_LENGTH);
	_tcsncpy(m_aszIconSets[nCurrentIndex++], SSS_TEXT_ICON_SET_IN_OUT_BUTTON, SSS_MAX_ICON_SET_NAME_LENGTH);
	_tcsncpy(m_aszIconSets[nCurrentIndex++], SSS_TEXT_ICON_SET_MOBILE_PHONE, SSS_MAX_ICON_SET_NAME_LENGTH);
	_tcsncpy(m_aszIconSets[nCurrentIndex++], SSS_TEXT_ICON_SET_PDA, SSS_MAX_ICON_SET_NAME_LENGTH);
	_tcsncpy(m_aszIconSets[nCurrentIndex++], SSS_TEXT_ICON_SET_TRAFFIC, SSS_MAX_ICON_SET_NAME_LENGTH);
	_tcsncpy(m_aszIconSets[nCurrentIndex++], SSS_TEXT_ICON_SET_TUBE, SSS_MAX_ICON_SET_NAME_LENGTH);
	_tcsncpy(m_aszIconSets[nCurrentIndex++], g_szWIVEnd, SSS_MAX_ICON_SET_NAME_LENGTH);

#ifdef SSS_V2_IMP
	//	Fetch icons sets from file
	TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Locking"));
	long nThreadId = ::GetCurrentThreadId();

	while(m_lCustomIconsThreadId!=nThreadId)
	{
		// Wait until successfully locked
		::InterlockedCompareExchange(&m_lCustomIconsThreadId, nThreadId, 0);

		if(m_lCustomIconsThreadId==nThreadId) break;

		::Sleep(25);
	}
	TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Locked"));

	TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Opening file"));
	if (OpenCustomIconsFile() != NULL)
	{
		TraceInfo(_D("CSSSOptions::LoadIconSetsArray: File opened"));
		TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Loading file"));
		if (LoadCustomIconsFile() >= 0)
		{
			TraceInfo(_D("CSSSOptions::LoadIconSetsArray: File loaded"));

			TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Validating file"));
			if (ValidateCustomIconsFile(SSS_ICON_SET_LABEL_HEADER_ID, _T("1.0")))
			{
				TraceInfo(_D("CSSSOptions::LoadIconSetsArray: File Validated"));
				TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Loading icon sets"));
				nIndex = LoadCustomIconSets(m_aszIconSets, nCurrentIndex);
				if (nIndex > nCurrentIndex)
				{
					TraceInfo(_D("LoadIconSetsArray: Icon sets loaded"));
				}
				else
				{
					TraceInfo(_D("LoadIconSetsArray: No icon sets defined"));
				}
			}
		}

		TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Closing file"));
		CloseCustomIconsFile();
		TraceInfo(_D("CSSSOptions::LoadIconSetsArray: File closed"));
	}
	else
	{
		TraceInfo(_D("CSSSOptions::LoadIconSetsArray: No custom icon set file found"));
		nIndex = nCurrentIndex;
	}

	TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Unlocking"));
	// Only the thread that set the lock can release it
	::InterlockedCompareExchange(&m_lCustomIconsThreadId, 0, ::GetCurrentThreadId());
	TraceInfo(_D("CSSSOptions::LoadIconSetsArray: Unlocked"));
#endif //#ifdef SSS_V2_IMP


//	for (int i = nIndex; i < SSS_MAX_AVAILABLE_ICON_SETS; i++)
//	{
//		_tcsncpy(m_aszIconSets[i], SSS_TEXT_ICON_SET_CUSTOM, SSS_MAX_ICON_SET_NAME_LENGTH);
//	}

	TraceLeave(_D("CSSSOptions::LoadIconSetsArray"));

	return;
}

#ifdef SSS_V2_IMP
// Open the custom icons file
HANDLE CSSSOptions::OpenCustomIconsFile()
{
	TraceEnter(_D("CSSSOptions::OpenCustomIconsFile"));

	// Construct the language file path
	_snwprintf (m_szCustomIconsFilePath, MAX_PATH, _T("%s%s%s"), GetDLLPath(), g_szProductShortName, _T(".icons"));

	TraceInfo(_D("CSSSOptions::OpenCustomIconsFile: File = %s"), m_szCustomIconsFilePath);

	// Open the Lang file
	m_hfCustomIconsFile = CreateFile
	(
		m_szCustomIconsFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (m_hfCustomIconsFile == INVALID_HANDLE_VALUE)
	{
//		MessageBox (NULL, SSS_TEXT_CANT_OPEN_FILE, SSS_TEXT_ERROR_MESSAGEBOX, MB_OK);
		TraceWarning(_D("CSSSOptions::OpenCustomIconsFile: Cannot open file"));
		TraceLeave(_D("CSSSOptions::OpenCustomIconsFile"), (DWORD)NULL);
		return NULL;
	}

	// Get the size of the file
	m_dwCustomIconsFileSize = (int)GetFileSize (m_hfCustomIconsFile, NULL);

	TraceInfo(_D("CSSSOptions::OpenCustomIconsFile: File size = %d"), m_dwCustomIconsFileSize);

	// See if file > 2Gig
	if (m_dwCustomIconsFileSize < 0)
	{
		TraceError(_D("CSSSOptions::OpenCustomIconsFile: Unmanageable file size"));
		TraceLeave(_D("CSSSOptions::OpenCustomIconsFile"), (DWORD)NULL);
		return NULL;
	}

	// Free any existing file buffer.
	if (m_pbCustomIconsBuff)
	{
		TraceInfo(_D("CSSSOptions::OpenCustomIconsFile: Freeing existing memory buffer"));
		LocalFree (m_pbCustomIconsBuff);
	}

	// Allocate a buffer.
	TraceInfo(_D("CSSSOptions::OpenCustomIconsFile: Allocating %d bytes of memory"), m_dwCustomIconsFileSize + 2);
	m_pbCustomIconsBuff = (PBYTE)LocalAlloc (LPTR, m_dwCustomIconsFileSize + 2);
	if (!m_pbCustomIconsBuff)
	{
//		MessageBox (NULL, SSS_TEXT_ERROR_NOT_ENOUGH_MEMORY, SSS_TEXT_ERROR_MESSAGEBOX, MB_OK);
		TraceError(_D("CSSSOptions::OpenCustomIconsFile: Could not allocate memory"));
		TraceLeave(_D("CSSSOptions::OpenCustomIconsFile"), (DWORD)NULL);
		return NULL;
	}

	// Initialize the buffer.
//	memset(m_pbCustomIconsBuff, 0, m_dwCustomIconsFileSize + 2);

	// Return the file handle
	TraceLeave(_D("CSSSOptions::OpenCustomIconsFile"), (DWORD)m_hfCustomIconsFile);
	return m_hfCustomIconsFile;
}

// Close the custom icons file
void CSSSOptions::CloseCustomIconsFile()
{
	TraceEnter(_D("CSSSOptions::CloseLangFile"));

	// Close any opened icons file.
	if (m_hfCustomIconsFile)
	{
		TraceInfo(_D("CSSSOptions::CloseCustomIconsFile: Closing file"));
		CloseHandle (m_hfCustomIconsFile);
		m_hfCustomIconsFile = NULL;
	}

	// Free any file buffer.
	if (m_pbCustomIconsBuff)
	{
		TraceInfo(_D("CSSSOptions::CloseCustomIconsFile: Freeing memory"));
		LocalFree (m_pbCustomIconsBuff);
		m_pbCustomIconsBuff = NULL;
	}

	TraceLeave(_D("CSSSOptions::CloseCustomIconsFile"));
	return;
}

// Attempt to read the custom icons file into a buffer. 
long CSSSOptions::LoadCustomIconsFile()
{
	DWORD	dwBytesRead;
	DWORD	dwFileSize = m_dwCustomIconsFileSize;

	TraceEnter(_D("CSSSOptions::LoadCustomIconsFile"));

	// Reset file pointer
	m_dwCustomIconsBufferPointer = 0;

	// Check file handle
	if (!m_hfCustomIconsFile)
	{
		TraceError(_D("CSSSOptions::LoadCustomIconsFile: File handle is null"));
		TraceLeave(_D("CSSSOptions::LoadCustomIconsFile"), (DWORD)-1);
		return -1;
	}

	// Check file name specified
	if (_tcslen(m_szCustomIconsFilePath) == 0)
	{
		TraceError(_D("CSSSOptions::LoadCustomIconsFile: File name is blank"));
		TraceLeave(_D("CSSSOptions::LoadCustomIconsFile"), (DWORD)-1);
		return -1;
	}

	// Check file size
	if (dwFileSize == 0)
	{
		TraceError(_D("CSSSOptions::LoadCustomIconsFile: File size is zero"));
		TraceLeave(_D("CSSSOptions::LoadCustomIconsFile"), (DWORD)-1);
		return -1;
	}

	TraceInfo(_D("CSSSOptions::LoadCustomIconsFile: Reading file"));
	if (!ReadFile (m_hfCustomIconsFile, m_pbCustomIconsBuff, dwFileSize, &dwBytesRead, NULL))
	{
		TraceError(_D("CSSSOptions::LoadCustomIconsFile: ReadFile failed"));
		TraceLeave(_D("CSSSOptions::LoadCustomIconsFile"), (DWORD)-1);
		return -1;
	}

	// Check for end of file. 
	if (dwBytesRead == 0) 
	{ 
		// We're at the end of the file 
		TraceInfo(_D("CSSSOptions::LoadCustomIconsFile: End of file"));
		TraceLeave(_D("CSSSOptions::LoadCustomIconsFile"), (DWORD)0);
		return 0;
	} 

	while (dwFileSize > 0)
	{
		dwFileSize -= dwBytesRead;
		if (dwFileSize < 0) dwFileSize = 0;

		TraceInfo(_D("CSSSOptions::LoadCustomIconsFile: Read %d bytes from file, FileSize now %d"), dwBytesRead, dwFileSize);

// TODO: What to do if not all of file read first time

		TraceInfo(_D("CSSSOptions::LoadCustomIconsFile: Reading file"));
		if (!ReadFile (m_hfCustomIconsFile, m_pbCustomIconsBuff, dwFileSize, &dwBytesRead, NULL)) break;
	}

	TraceLeave(_D("CSSSOptions::LoadCustomIconsFile"), (DWORD)0);
	return 0;
}

bool CSSSOptions::ValidateCustomIconsFile(LPCTSTR lpszFileID, LPCTSTR lpszVersion)
{
	TCHAR szLine[WIV_MAX_STRING + 1];

	bool blResult = false;

	TraceEnter(_D("CSSSOptions::ValidateCustomIconsFile"));
	TraceInfo(_D("CSSSOptions::ValidateCustomIconsFile: File ID to check = <%s>, Version to check = <%s>"), lpszFileID, lpszVersion);
	
	// While there a lines left to check, look for header line
	while(m_dwCustomIconsBufferPointer < m_dwCustomIconsFileSize)
	{
		_zclr(szLine);
		if (GetCustomIconsLine(szLine) == SSS_ICON_SET_INDEX_HEADER)
		{
			TraceInfo(_D("CSSSOptions::ValidateCustomIconsFile: Header line found with value <%s>"), szLine);
			if (_tcscmp(szLine, lpszFileID) == 0)
			{
				TraceInfo(_D("CSSSOptions::ValidateCustomIconsFile: Header line confirmed OK"));
				while(m_dwCustomIconsBufferPointer < m_dwCustomIconsFileSize)
				{
					_zclr(szLine);
					if (GetCustomIconsLine(szLine) == SSS_ICON_SET_INDEX_VERSION)
					{
						// Version line found
						TraceInfo(_D("CSSSOptions::ValidateCustomIconsFile: Version line found with value <%s>"), szLine);
						if (_tcscmp(szLine, lpszVersion) == 0)
						{
							TraceInfo(_D("CSSSOptions::ValidateCustomIconsFile: Version confirmed OK"));
							blResult = true;
						}
						else
						{
							TraceInfo(_D("CSSSOptions::ValidateCustomIconsFile: Invalid Version"));
						}

						break;
					}
				}
			}
			else
			{
				TraceInfo(_D("CSSSOptions::ValidateCustomIconsFile: Invalid Header line"));
			}

			break;
		}
	}

	TraceLeave(_D("CSSSOptions::ValidateCustomIconsFile"), (DWORD)blResult);

	return blResult;
}

int CSSSOptions::GetCustomIconsLine(LPTSTR lpszLine)
{
	int		nIndex = 0;
	DWORD	dwLineLength = 0;
	TCHAR	szLine[WIV_MAX_STRING + 1];
	char	*pszCRLF;
	TCHAR	szValue[WIV_MAX_STRING + 1];

	TraceEnter(_D("CSSSOptions::GetCustomIconsLine"));

	// Check file handle
	if (!m_hfCustomIconsFile)
	{
		TraceError(_D("CSSSOptions::GetCustomIconsLine: File handle is null"));
		TraceLeave(_D("CSSSOptions::GetCustomIconsLine"), (DWORD)SSS_ICON_SET_INDEX_UNKNOWN);
		return SSS_ICON_SET_INDEX_UNKNOWN;
	}

	TraceInfo(_D("CWiVLang::GetLine: Buffer pointer = %d"), m_dwCustomIconsBufferPointer);

	// Get the first line
	pszCRLF = strstr( (PSZ)&m_pbCustomIconsBuff[m_dwCustomIconsBufferPointer], WIV_CUSTOM_NEW_LINE );

	if (pszCRLF == NULL)
	{
		TraceInfo(_D("CSSSOptions::GetCustomIconsLine: No end of line found, use end of file instead"));
		dwLineLength = m_dwCustomIconsFileSize - m_dwCustomIconsBufferPointer;
	}
	else
	{
		TraceInfo(_D("CSSSOptions::GetCustomIconsLine: End of line found at <%08p>"), pszCRLF);
		dwLineLength = pszCRLF - (PSZ)&m_pbCustomIconsBuff[m_dwCustomIconsBufferPointer];
	}

	TraceInfo(_D("CSSSOptions::GetCustomIconsLine: Line Length = %d"), dwLineLength);

	_zclr(szLine);
	_zclr(szValue);

	if (dwLineLength > 0)
	{
		if (m_pbCustomIconsBuff[m_dwCustomIconsBufferPointer] == WIV_CUSTOM_COMMENT)
		{
			TraceInfo(_D("CSSSOptions::GetCustomIconsLine: Comment line"));
			// Update buffer pointer
			m_dwCustomIconsBufferPointer += (dwLineLength + 2);
			TraceInfo(_D("CSSSOptions::GetCustomIconsLine: Buffer pointer now = %d"), m_dwCustomIconsBufferPointer);
			TraceLeave(_D("CSSSOptions::GetCustomIconsLine"), (DWORD)SSS_ICON_SET_INDEX_COMMENT);
			return SSS_ICON_SET_INDEX_COMMENT;
		}
		else
		{
			// Convert to a Unicode String

			MultiByteToWideChar(CP_ACP, 0, (PSZ)&m_pbCustomIconsBuff[m_dwCustomIconsBufferPointer], dwLineLength, szLine, WIV_MAX_STRING);
			TraceInfo(_D("CSSSOptions::GetCustomIconsLine: Line = <%s>"), szLine);
			nIndex = ParseCustomIconsLine(szLine, szValue);

			_tcsncpy(lpszLine, szValue, WIV_MAX_STRING);
			// Update buffer pointer
			m_dwCustomIconsBufferPointer += (dwLineLength + 2);
			TraceInfo(_D("CSSSOptions::GetCustomIconsLine: Buffer pointer now = %d"), m_dwCustomIconsBufferPointer);
		}
	}
	else
	{
		TraceInfo(_D("CSSSOptions::GetCustomIconsLine: Empty line"));
		// Update buffer pointer
		m_dwCustomIconsBufferPointer += (dwLineLength + 2);
		TraceInfo(_D("CSSSOptions::GetCustomIconsLine: Buffer pointer now = %d"), m_dwCustomIconsBufferPointer);
		TraceLeave(_D("CSSSOptions::GetCustomIconsLine"), (DWORD)SSS_ICON_SET_INDEX_EMPTY);
		return SSS_ICON_SET_INDEX_EMPTY;
	}

	TraceLeave(_D("CSSSOptions::GetCustomIconsLine"), (DWORD)nIndex);
	return nIndex;
}

int CSSSOptions::ParseCustomIconsLine(LPCTSTR lpszLine, LPTSTR lpszValue)
{
	int		nIndex = 0;
	DWORD	dwLinePointer = 0;
	DWORD	dwValueLength = 0;
	DWORD	dwLineLength = _tcslen(lpszLine);
	TCHAR	szLine[WIV_MAX_STRING + 1];
	TCHAR	szIndex[WIV_MAX_NAME + 1];
	TCHAR	*pszDelim;

	TraceEnter(_D("CSSSOptions::ParseCustomIconsLine"));

	TraceInfo(_D("CSSSOptions::ParseCustomIconsLinee: Line length = %d, line = <%s>"), dwLineLength, lpszLine);


	if (dwLineLength == 0)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Empty line"));
		TraceLeave(_D("CSSSOptions::ParseCustomIconsLine"), SSS_ICON_SET_INDEX_EMPTY);
		return SSS_ICON_SET_INDEX_EMPTY;
	}

	_zcpy(szLine, lpszLine);

	// Find the first delimiter
	pszDelim = _tcsstr( &szLine[dwLinePointer], WIV_CUSTOM_DELIM);

	// If no delimiter
	if (pszDelim == NULL)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: No delimiter"));
		TraceLeave(_D("CSSSOptions::ParseCustomIconsLine"), (DWORD)SSS_ICON_SET_INDEX_EMPTY);
		return SSS_ICON_SET_INDEX_EMPTY;
	}

	TraceInfo(_D("CCSSSOptions::ParseCustomIconsLine: Delimiter found at <%08p>"), pszDelim);

	dwValueLength = pszDelim - &szLine[dwLinePointer];

	TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Value Length = %d"), dwValueLength);

	_zclr(szIndex);
	_tcsncpy(szIndex, &szLine[dwLinePointer], min(dwValueLength, WIV_MAX_NAME));
	TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index = <%s>"), szIndex);

	if (_tcsicmp(szIndex, g_szCustomFileHeader) == 0)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index is the header"));
		nIndex = SSS_ICON_SET_INDEX_HEADER;

	}else if (_tcsicmp(szIndex, SSS_ICON_SET_LABEL_VERSION) == 0)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index is the version"));
		nIndex = SSS_ICON_SET_INDEX_VERSION;

	}else if (_tcsicmp(szIndex, SSS_ICON_SET_LABEL_TABLE_BEGIN) == 0)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index is the table start"));
		nIndex = SSS_ICON_SET_INDEX_TABLE_BEGIN;

	}else if (_tcsicmp(szIndex, SSS_ICON_SET_LABEL_TABLE_END) == 0)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index is the table end"));
		nIndex = SSS_ICON_SET_INDEX_TABLE_END;

	}else if (_tcsicmp(szIndex, SSS_ICON_SET_LABEL_ICON_ON) == 0)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index is the ON icon"));
		nIndex = SSS_ICON_SET_INDEX_ICON_ON;

	}else if (_tcsicmp(szIndex, SSS_ICON_SET_LABEL_ICON_OFF) == 0)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index is the OFF icon"));
		nIndex = SSS_ICON_SET_INDEX_ICON_OFF;

	}else if (_tcsicmp(szIndex, SSS_ICON_SET_LABEL_ICON_NEUTRAL) == 0)
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index is the NEUTRA icon"));
		nIndex = SSS_ICON_SET_INDEX_ICON_NEUTRAL;
	}
	else
	{
		TraceInfo(_D("CSSSOptions::ParseCustomIconsLine: Index is unknown"));
		nIndex = SSS_ICON_SET_INDEX_UNKNOWN;
	}

	// Update line pointer
	dwLinePointer += (dwValueLength + 2);

	TraceInfo(_D("CWiVLang::ParseLine: Line pointer now = %d"), dwLinePointer);

	// Get next value
	TraceInfo(_D("CWiVLang::ParseLine, Value length = %d"), dwLineLength - dwLinePointer);
	_tcsncpy(lpszValue, &szLine[dwLinePointer], dwLineLength - dwLinePointer );
	TraceInfo(_D("CWiVLang::ParseLine, Value <%s>"), lpszValue);

	TraceLeave(_D("CWiVLang::ParseLine"), (DWORD)nIndex);
	return nIndex;
}

int	CSSSOptions::LoadCustomIconSets(TCHAR lpaszIconSets[SSS_MAX_AVAILABLE_ICON_SETS][SSS_MAX_ICON_SET_NAME_LENGTH + 1], int nStartIndex)
{
	TCHAR	szLine[WIV_MAX_STRING + 1];
	int		nIndex = nStartIndex;

	TraceEnter(_D("CSSSOptions::LoadCustomIconSets"));
	TraceInfo(_D("CSSSOptions::LoadCustomIconSets: Start Index = <%d>"), nStartIndex);

	// While there are lines left to check, look for icons table entries
	while(m_dwCustomIconsBufferPointer < m_dwCustomIconsFileSize)
	{
		_zclr(szLine);
		if (GetCustomIconsLine(szLine) == SSS_ICON_SET_INDEX_TABLE_BEGIN)
		{
			// Icon set start line found
			TraceInfo(_D("CSSSOptions::LoadCustomIconSets: Icon set start line found with value <%s>"), szLine);
			TraceInfo(_D("CSSSOptions::LoadCustomIconSets: Adding icon set name <%s> at index %d"), szLine, nIndex);
			_tcsncpy(lpaszIconSets[nIndex++], szLine, SSS_MAX_ICON_SET_NAME_LENGTH);
		}
		continue;
	}

	TraceLeave(_D("CSSSOptions::LoadCustomIconSets"), (DWORD)nIndex);

	return nIndex;
}
#endif //#ifdef SSS_V2_IMP

bool CSSSOptions::InfoPIN(DWORD *lpdwLength, LPCTSTR szPIN, LPCTSTR szICCID, const bool bAutoReset, const bool bAutoPhone)
{
	SSS_BUFFER1	buf;
	SSS_BUFFER2	buf2;
	int			i,j;

	TraceEnter(_D("CSSSOptions::InfoPIN"));

	memset(&buf2, 0, sizeof(buf2));

	// ICCID length
	buf2.dwICCIDLen = _tcslen(szICCID);
	
	if (buf2.dwICCIDLen == 0)
	{
		TraceLeave(_D("CSSSOptions::InfoPIN: ICCID is empty"), (DWORD)false);
		return false;
	}

	// Convert ICCID string to byte nibbles
	for (j= 0, i = 0 ; j <= ((int)buf2.dwICCIDLen)-1 ; )
	{
		unsigned short s1 = 0;
		unsigned short s2 = 0;

		s1 = ((szICCID[j++] - 0x30) << 4);
		TraceInfo(_D("CSSSOptions::InfoPIN: ICCID s1 = <%02X>, j = <%d>"), s1, j-1);

		if (j <= ((int)buf2.dwICCIDLen)-1)
		{
			s2 = szICCID[j++] - 0x30;
			TraceInfo(_D("CSSSOptions::InfoPIN: ICCID s2 = <%02X>, j = <%d>"), s2, j-1);
		}

		buf2.bICCID[i++] = s1 | s2;

		if (j > ((int)buf2.dwICCIDLen)-1)
			break;
	}

	TraceInfo(_D("CSSSOptions::InfoPIN: bICCID = <%s>"), BtoS((BYTE *)&buf2.bICCID, j));

	// Auto PIN flags
	buf2.bAuto[0] = (bAutoReset == true ? 0x10 : 0x00) | (bAutoPhone == true ? 0x01 : 0x00);
	
	// PIN length
	buf2.dwPINLen = _tcslen(szPIN);

	// Convert PIN string to byte nibbles
	for (j= 0, i = 0 ; j <= ((int)buf2.dwPINLen)-1 ; )
	{
		unsigned short s1 = 0;
		unsigned short s2 = 0;

		s1 = szPIN[j++] - 0x30;
		TraceInfo(_D("CSSSOptions::InfoPIN: PIN s1 = <%02X>, j = <%d>"), s1, j-1);

		if (j <= ((int)buf2.dwPINLen)-1)
		{
			s2 = szPIN[j++] - 0x30;
			TraceInfo(_D("CSSSOptions::InfoPIN: PIN s2 = <%02X>, j = <%d>"), s2, j-1);
		}

		buf2.bPIN[i++] = (s1 << 4) | s2;

		if (j > ((int)buf2.dwPINLen)-1)
			break;
	}
	
	TraceInfo(_D("CSSSOptions::InfoPIN: bPIN = <%s>"), BtoS((BYTE *)&buf2.bPIN, j));

	buf.dwBufLen = sizeof(buf2);
	memcpy(buf.bBuffer, &buf2, buf.dwBufLen);

	memcpy(m_abPIN, &buf, buf.dwBufLen+sizeof(DWORD));

	// Encrypt data
	this->GetDeviceInfo(m_pPhone, false, true, true);
	
	TraceInfo(_D("CSSSOptions::InfoPIN: PIN1 = <%s>"), BtoS(m_abPIN, WIV_MAX_BINARY));
	
	TraceLeave(_D("CSSSOptions::InfoPIN"), (DWORD)true);

	return true;
}

bool CSSSOptions::PINInfo(const DWORD dwLength, LPTSTR lpszPIN, LPTSTR lpszICCID, bool *lpbAutoReset, bool *lpbAutoPhone)
{
	SSS_BUFFER1	buf;
	SSS_BUFFER2	buf2;

	TraceEnter(_D("CSSSOptions::PINInfo"));

	if (dwLength == 0)
	{
		TraceLeave(_D("CSSSOptions::PINInfo: Buffer is zero"), (DWORD)false);
		return false;
	}

	memset(&buf, 0, sizeof(buf));
	memset(&buf2, 0, sizeof(buf2));

	buf.dwBufLen = dwLength;
	memcpy(buf.bBuffer, m_abPIN, dwLength);
	TraceInfo(_D("CSSSOptions::PINInfo: After memcpy, buf.dwBufLen = %d, buf.bBuffer = <%s>"), buf.dwBufLen, BtoS(buf.bBuffer, buf.dwBufLen));

	memcpy(m_abPIN, (BYTE *)&buf, buf.dwBufLen+sizeof(DWORD));
	TraceInfo(_D("CSSSOptions::PINInfo: After memcpy, length = %d, m_abPIN = <%s>"), buf.dwBufLen+sizeof(DWORD), BtoS(m_abPIN, buf.dwBufLen+sizeof(DWORD)));

	// Decrypt data
	this->GetDeviceInfo(m_pPhone, false, true);
	TraceDetail(_D("CSSSOptions::PINInfo: Back from GetDeviceInfo, m_abPIN = <%s>"), BtoS(m_abPIN, buf.dwBufLen+sizeof(DWORD)));
	
	memcpy(&buf.dwBufLen, m_abPIN, sizeof(DWORD));
	memcpy(buf.bBuffer, &m_abPIN[sizeof(DWORD)], buf.dwBufLen);

	TraceDetail(_D("CSSSOptions::PINInfo: After memcpy to buf, length = %d, buf.bBuffer = <%s>"), buf.dwBufLen, BtoS(buf.bBuffer, buf.dwBufLen));

	memcpy((VOID *)&buf2, buf.bBuffer, buf.dwBufLen);

	TraceInfo(_D("CSSSOptions::PINInfo: buf2.dwICCIDLen = <%08x>"), buf2.dwICCIDLen);

	TraceInfo(_D("CSSSOptions::PINInfo: buf2.bICCID = <%s>"), BtoS((BYTE *)&buf2.bICCID[0], sizeof(buf2.bICCID)));

	TraceInfo(_D("CSSSOptions::PINInfo: buf2.bAuto = <%s>"), BtoS((BYTE *)&buf2.bAuto[0], sizeof(buf2.bAuto)));
	
	TraceInfo(_D("CSSSOptions::PINInfo: buf2.dwPINLen = <%08x>"), buf2.dwPINLen);

	TraceInfo(_D("CSSSOptions::PINInfo: buf2.bPIN = <%s>"), BtoS((BYTE *)&buf2.bPIN[0], sizeof(buf2.bPIN)));

	TraceInfo(_D("CSSSOptions::PINInfo: buf2 = <%s>"), BtoS((BYTE *)&buf2, sizeof(SSS_BUFFER2)));

	// ICCID
	int		i,j;

	for (j= 0, i = 0 ; j <= ((int)buf2.dwICCIDLen - 1) ; )
	{
		char c1 = ((buf2.bICCID[i] & 0xf0) >> 4) + 0x30;
		char c2 = (buf2.bICCID[i++] & 0x0f) + 0x30;

		TraceDetail(_D("CSSSOptions::PINInfo: c1 = <%c>, j = <%d>"), c1, j);
		lpszICCID[j++] = (TCHAR)c1;

		if (j > ((int)buf2.dwICCIDLen - 1))
			break;

		TraceDetail(_D("CSSSOptions::PINInfo: c2 = <%c>, j = <%d>"), c2, j);
		lpszICCID[j++] = (TCHAR)c2;
	}

	lpszICCID[j] = WIV_NULL_CHAR;

	TraceInfo(_D("CSSSOptions::PINInfo: ICCID = <%s>"), lpszICCID);

	// PIN
	lpszPIN[0] = WIV_NULL_CHAR;

	for (j= 0, i = 0 ; j <= ((int)buf2.dwPINLen - 1) ; )
	{
		char c1 = ((buf2.bPIN[i] & 0xf0) >> 4) + 0x30;
		char c2 = (buf2.bPIN[i++] & 0x0f) + 0x30;

		lpszPIN[j++] = (TCHAR)c1;

		if (j > ((int)buf2.dwPINLen - 1))
			break;

		lpszPIN[j++] = (TCHAR)c2;
	}

	lpszPIN[j] = WIV_NULL_CHAR;

	TraceInfo(_D("CSSSOptions::PINInfo: PIN = <%s>"), lpszPIN);

	char c1 = ((buf2.bAuto[0] & 0xf0) >> 4);
	char c2 = (buf2.bAuto[0] & 0x0f);

	TraceDetail(_D("CSSSOptions::PINInfo: auto c1 = <%02X>, auto c2 = <%02X>"), c1, c2);

	*lpbAutoReset = (c1 == '\0' ? false : true);
	*lpbAutoPhone = (c2 == '\0' ? false : true);

	TraceLeave(_D("CSSSOptions::PINInfo"), (DWORD)true);
	return true;
}

void CSSSOptions::DoName(LPCTSTR szICCID, LPTSTR lpszName)
{

	TCHAR		szWork[SSS_MAX_ICCID + 1];
	BYTE		bBuf[WIV_MAX_BINARY + 1];
	int			i,j;
	DWORD		dwLength;
	SSS_BUFFER3	buf3;
	TCHAR		*pszMap;
	UCHAR		s1;
	UCHAR		s2;

	TraceEnter(_D("CSSSOptions::DoName"));

	if (SSSGlobals)
	{
		pszMap = (TCHAR *)SSSGlobals->bNameData;

		if (pszMap)
		{
			TraceInfo(_D("DoName: pszMap = <%s>"), BtoS((LPBYTE)pszMap, SSSGlobals->dwNameSize));
		}
		else
		{
			TraceError(_D("DoName: pszMap is NULL"));
		}

	}
	else
	{
		TraceError(_D("DoName: SSSGlobals is NULL"));
	}

	memset(szWork, 0, sizeof(szWork));
	memset(bBuf, 0, sizeof(bBuf));
	memset(&buf3, 0, sizeof(buf3));

	// ICCID length
	dwLength = _tcslen(szICCID);

	if (dwLength == 0)
	{
		_tcsncpy(lpszName, pszMap, 8);
		lpszName[8] = WIV_NULL_CHAR;
		TraceLeave(_D("CSSSOptions::DoName"));
		return;
	}

	dwLength -= 3;

	_tcsncpy(szWork, &szICCID[3], dwLength);
	
	// Convert ICCID string to byte nibbles
	for (j= 0, i = 0 ; j <= ((int)dwLength)-1 ; )
	{
		s1 = 0;
		s2 = 0;

		s1 = ((szWork[j++] - 0x30) << 4);
		s2 = szWork[j++] - 0x30;
		TraceInfo(_D("CSSSOptions::DoName: szWork s1 = <%02X>, szWork s2 = <%02X>, j = <%d>"), s1, s2, j-2);

		bBuf[i++] = (s1 | s2);

		if (j > ((int)dwLength)-1)
			break;
	}

	TraceInfo(_D("CSSSOptions::DoName: bBuf = <%s>"), BtoS(bBuf, 2*sizeof(DWORD)));

	memcpy(&buf3, &bBuf, 2*sizeof(DWORD));

	buf3.dw3 = buf3.dw1 + buf3.dw2;
	TraceInfo(_D("CSSSOptions::DoName: buf3.dw1 = %08X"), buf3.dw1);
	TraceInfo(_D("CSSSOptions::DoName: buf3.dw2 = %08X"), buf3.dw2);
	TraceInfo(_D("CSSSOptions::DoName: buf3.dw3 = %08X"), buf3.dw3);

	for (i =0 ; i < 2*sizeof(DWORD) ; )
	{
		s1 = (UCHAR)(buf3.dw3 & 0x0000000f);
		s2 = ((UCHAR)(buf3.dw3 & 0x000000f0)) >> 4;

		lpszName[i++] = pszMap[s1];
		lpszName[i++] = pszMap[s2];
		buf3.dw3 >>= 8;

		TraceInfo(_D("CSSSOptions::DoName: buf3.dw3 s1 = <%02X>, buf3.dw3 s2 = <%02X>, i = <%d>"), s1, s2, i-2);
	}

	lpszName[2 * sizeof(DWORD)] = WIV_NULL_CHAR;

	TraceLeave(_D("CSSSOptions::DoName"));

	return;
}

//======================================================================
// Read user options from the registry
//======================================================================
DWORD CSSSOptions::GetUserOptions()
{
	DWORD		dwResult;
	HKEY		hKey;

	TraceEnter(_D("CSSSOptions::GetUserOptions"));

	// Get user settings from registry
	TraceInfo(_D("CSSSOptions::GetUserOptions: Getting user options from registry"));

	// Open the Options Key [HKEY_LOCAL_MACHINE\SOFTWARE\WiViT\SIM Status Switcher\Options]
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegKeyOpen for Options key"));
	dwResult = RegKeyOpen(g_szRegOptionsKey, &hKey);
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegKeyOpen, dwResult = <%08x>, hKey = <%08x>"), dwResult, hKey);

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("CSSSOptions::GetUserOptions: Error opening Options key, dwResult = <%08x>"), dwResult);
		TraceLeave(_D("CSSSOptions::GetUserOptions"), dwResult);
		return dwResult;
	}

	memset(&CurrentSettings, 0, sizeof(SSS_SETTINGS));
	memset(&NewSettings, 0, sizeof(SSS_SETTINGS));

	// Show telephone number flag with default of TRUE "ShowNumber"=dword:00000001
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for ShowNumber"));
	m_blShowPhoneNumber = RegGetValue(hKey, g_szRegShowPhoneNumber, SSS_DEFAULT_SHOW_PHONE_NUMBER);
	CurrentSettings.blShowPhoneNumber = m_blShowPhoneNumber;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, ShowNumber = <%X>"), m_blShowPhoneNumber);

	// Show service provider flag with default of TRUE "ShowTSP"=dword:00000001
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for ShowTSP"));
	m_blShowTSP = RegGetValue(hKey, g_szRegShowTSP, SSS_DEFAULT_SHOW_TSP);
	CurrentSettings.blShowTSP = m_blShowTSP;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, ShowTSP = <%X>"), m_blShowTSP);

	// Single line display flag with default of FALSE "SingleLineDisplay"=dword:00000000
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for SingleLineDisplay"));
	m_blSingleLineDisplay = RegGetValue(hKey, g_szRegSingleLineDisplay, SSS_DEFAULT_SINGLE_LINE_DISPLAY);
	CurrentSettings.blSingleLineDisplay = m_blSingleLineDisplay;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, SingleLineDisplay = <%X>"), m_blSingleLineDisplay);

	// Use bold font for line 1 flag with default of TRUE "Line1BoldFont"=dword:00000001
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for Line1BoldFont"));
	m_blLine1BoldFont = RegGetValue(hKey, g_szRegLine1BoldFont, SSS_DEFAULT_LINE_1_BOLD_FONT);
	CurrentSettings.blLine1BoldFont = m_blLine1BoldFont;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, Line1BoldFont = <%X>"), m_blLine1BoldFont);

	// Use bold font for line 2 flag with default of FALSE "Line2BoldFont"=dword:00000000
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for Line2BoldFont"));
	m_blLine2BoldFont = RegGetValue(hKey, g_szRegLine2BoldFont, SSS_DEFAULT_LINE_2_BOLD_FONT);
	CurrentSettings.blLine2BoldFont = m_blLine2BoldFont;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, Line2BoldFont = <%X>"), m_blLine2BoldFont);

	// Icon set value with default of 0 (Standard Phone) "IconSet"=dword:00000000
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for IconSet"));
	m_dwIconSet = RegGetValue(hKey, g_szRegTodayIconSet, SSS_DEFAULT_TODAY_ICON_SET);
	CurrentSettings.dwIconSet = m_dwIconSet;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, IconSet = <%X>"), m_dwIconSet);

	// Tap action value with default of 0 (  ) "TapAction"=dword:00000000
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for TapAction"));
	m_dwTapAction = RegGetValue(hKey, g_szRegTapAction, SSS_DEFAULT_TAP_ACTION);
	CurrentSettings.dwTapAction = m_dwTapAction;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, TapAction = <%X>"), m_dwTapAction);

	// Tap and Hold action value with default of 0 (  ) "TAHAction"=dword:00000001
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for TAHAction"));
	m_dwTAHAction = RegGetValue(hKey, g_szRegTAHAction, SSS_DEFAULT_TAH_ACTION);
	CurrentSettings.dwTAHAction = m_dwTAHAction;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, TAHAction = <%X>"), m_dwTAHAction);

	// Today icon tap action value with default of 0 (  ) "TodayIconTapAction"=dword:00000000
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for TodayIconTapAction"));
	m_dwTodayIconTapAction = RegGetValue(hKey, g_szRegTodayIconTapAction, SSS_DEFAULT_TODAY_ICON_TAP_ACTION);
	CurrentSettings.dwTodayIconTapAction = m_dwTodayIconTapAction;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, TodayIconTapAction = <%X>"), m_dwTodayIconTapAction);

	// Today icon tap and hold action value with default of 0 (  ) "TodayIconTAHAction"=dword:00000000
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for TodayIconTAHAction"));
	m_dwTodayIconTAHAction = RegGetValue(hKey, g_szRegTodayIconTAHAction, SSS_DEFAULT_TODAY_ICON_TAH_ACTION);
	CurrentSettings.dwTodayIconTAHAction = m_dwTodayIconTAHAction;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, TodayIconTAHAction = <%X>"), m_dwTodayIconTAHAction);

	// Button action value with default of 0 (  ) "ButtonAction"=dword:00000000
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for ButtonAction"));
	m_dwButtonAction = RegGetValue(hKey, g_szRegButtonAction, SSS_DEFAULT_BUTTON_ACTION);
	CurrentSettings.dwButtonAction = m_dwButtonAction;
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, ButtonAction = <%X>"), m_dwButtonAction);
	
	// Hide personal information flag value with default of FALSE "HidePersonalInfo"=dword:00000000
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegGetValue for HidePersonalInfo"));
	OptionsFlags.OptionsBits.HidePersonalInfo = RegGetValue(hKey, g_szRegOptionsHidePersonal, false);
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegGetValue, HidePersonalInfo = <%d>"), OptionsFlags.OptionsBits.HidePersonalInfo);
	m_pPhone->SetHidePersonalInfo(OptionsFlags.OptionsBits.HidePersonalInfo);

	TraceInfo(_D("CSSSOptions::GetUserOptions: User options read from registry"));

	TraceDetail(_D("CSSSOptions::GetUserOptions: ShowNumber = <%X>"), m_blShowPhoneNumber);
	TraceDetail(_D("CSSSOptions::GetUserOptions: ShowTSP = <%X>"), m_blShowTSP);
	TraceDetail(_D("CSSSOptions::GetUserOptions: SingleLineDisplay = <%X>"), m_blSingleLineDisplay);
	TraceDetail(_D("CSSSOptions::GetUserOptions: Line1BoldFont = <%X>"), m_blLine1BoldFont);
	TraceDetail(_D("CSSSOptions::GetUserOptions: Line2BoldFont = <%X>"), m_blLine2BoldFont);
	TraceDetail(_D("CSSSOptions::GetUserOptions: IconSet = <%X>"), m_dwIconSet);
	TraceDetail(_D("CSSSOptions::GetUserOptions: TapAction = <%X>"), m_dwTapAction);
	TraceDetail(_D("CSSSOptions::GetUserOptions: TAHAction = <%X>"), m_dwTAHAction);
	TraceDetail(_D("CSSSOptions::GetUserOptions: TodayIconTapAction = <%X>"), m_dwTodayIconTapAction);
	TraceDetail(_D("CSSSOptions::GetUserOptions: TodayIconTAHAction = <%X>"), m_dwTodayIconTAHAction);
	TraceDetail(_D("CSSSOptions::GetUserOptions: ButtonAction = <%X>"), m_dwButtonAction);
	TraceDetail(_D("CSSSOptions::GetUserOptions: HidePersonalInfo = <%d>"), OptionsFlags.OptionsBits.HidePersonalInfo);

	// Close the Options Key
	TraceDetail(_D("CSSSOptions::GetUserOptions: Calling RegKeyClose"));
	dwResult = RegKeyClose(hKey);
	TraceDetail(_D("CSSSOptions::GetUserOptions: Back from RegKeyClose, dwResult = <%08x>"), dwResult);

	TraceLeave(_D("CSSSOptions::GetUserOptions"), dwResult);
	return dwResult;
}

//======================================================================
// Write user options to the registry
//======================================================================
DWORD CSSSOptions::SetUserOptions()
{
	DWORD		dwResult;
	HKEY		hKey;

	TraceEnter(_D("CSSSOptions::SetUserOptions"));

	// Write user settings to registry
	TraceInfo(_D("Writing user options to registry"));

	TraceDetail(_D("CSSSOptions::SetUserOptions: ShowNumber = <%X>"), m_blShowPhoneNumber);
	TraceDetail(_D("CSSSOptions::SetUserOptions: ShowTSP = <%X>"), m_blShowTSP);
	TraceDetail(_D("CSSSOptions::SetUserOptions: SingleLineDisplay = <%X>"), m_blSingleLineDisplay);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Line1BoldFont = <%X>"), m_blLine1BoldFont);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Line2BoldFont = <%X>"), m_blLine2BoldFont);
	TraceDetail(_D("CSSSOptions::SetUserOptions: IconSet = <%X>"), m_dwIconSet);
	TraceDetail(_D("CSSSOptions::SetUserOptions: TapAction = <%X>"), m_dwTapAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: TAHAction = <%X>"), m_dwTAHAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: TodayIconTapAction = <%X>"), m_dwTodayIconTapAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: TodayIconTAHAction = <%X>"), m_dwTodayIconTAHAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: ButtonAction = <%X>"), m_dwButtonAction);

	// Open the Options Key [HKEY_LOCAL_MACHINE\SOFTWARE\WiViT\SIM Status Switcher\Options]
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegKeyOpen for Options key"));
	dwResult = RegKeyOpen(g_szRegOptionsKey, &hKey);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegKeyOpen, dwResult = <%08x>, hKey = <%08x>"), dwResult, hKey);

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("CSSSOptions::SetUserOptions: Error opening Options key, dwResult = <%08x>"), dwResult);
		TraceLeave(_D("CSSSOptions::SetUserOptions"), dwResult);
		return dwResult;
	}

	// Show telephone number flag
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for ShowNumber = <%X>"), m_blShowPhoneNumber);
	dwResult = RegSetValue(hKey, g_szRegShowPhoneNumber, m_blShowPhoneNumber);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Show service provider flag
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for ShowTSP = <%08x>"), m_blShowTSP);
	dwResult = RegSetValue(hKey, g_szRegShowTSP, m_blShowTSP);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%X>"), dwResult);

	// Single line display flag
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for SingleLineDisplay = <%X>"), m_blSingleLineDisplay);
	dwResult = RegSetValue(hKey, g_szRegSingleLineDisplay, m_blSingleLineDisplay);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Use bold font for line 1 flag
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for Line1BoldFont = <%X>"), m_blLine1BoldFont);
	dwResult = RegSetValue(hKey, g_szRegLine1BoldFont, m_blLine1BoldFont);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Use bold font for line 2 flag
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for Line2BoldFont = <%X>"), m_blLine2BoldFont);
	dwResult = RegSetValue(hKey, g_szRegLine2BoldFont, m_blLine2BoldFont);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Icon set value
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for IconSet = <%X>"), m_dwIconSet);
	dwResult = RegSetValue(hKey, g_szRegTodayIconSet, m_dwIconSet);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Tap action value
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for TapAction = <%X>"), m_dwTapAction);
	dwResult = RegSetValue(hKey, g_szRegTapAction, m_dwTapAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Tap and Hold action value
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for TAHAction = <%X>"), m_dwTAHAction);
	dwResult = RegSetValue(hKey, g_szRegTAHAction, m_dwTAHAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Today icon tap action value
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for TodayIconTapAction = <%X>"), m_dwTodayIconTapAction);
	dwResult = RegSetValue(hKey, g_szRegTodayIconTapAction, m_dwTodayIconTapAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Today icon tap and hold action value
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for TodayIconTAHAction = <%X>"), m_dwTodayIconTAHAction);
	dwResult = RegSetValue(hKey, g_szRegTodayIconTAHAction, m_dwTodayIconTAHAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Button action value
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegSetValue for ButtonAction = <%X>"), m_dwButtonAction);
	dwResult = RegSetValue(hKey, g_szRegButtonAction, m_dwButtonAction);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);
	
	// Close the Options Key
	TraceDetail(_D("CSSSOptions::SetUserOptions: Calling RegKeyClose"));
	dwResult = RegKeyClose(hKey);
	TraceDetail(_D("CSSSOptions::SetUserOptions: Back from RegKeyClose, dwResult = <%08x>"), dwResult);

	TraceLeave(_D("CSSSOptions::SetUserOptions"), dwResult);
	return dwResult;
}

DWORD CSSSOptions::GetSecurityOptions(bool bForce)
{
	BYTE		bPIN1[WIV_MAX_BINARY + 1];
	TCHAR		szICCID[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
	HKEY		hKey = NULL;
	DWORD		dwResult = S_OK;
	TCHAR		szName[SSS_MAX_PIN + 1];

	TraceEnter(_D("CSSSOptions::GetSecurityOptions"));

	// Unless force read set, if ICCID is specified, already have security options, so return
	if (!bForce)
	{
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Checking ICCID"));
		if (_tcslen(m_szICCID) != 0)
		{
			TraceInfo(_D("CSSSOptions::GetSecurityOptions: Already have security options, so exit"));
			TraceLeave(_D("CSSSOptions::GetSecurityOptions"), dwResult);
			return dwResult;
		}
	}

	TraceDetail(_D("CSSSOptions::GetSecurityOptions: ICCID empty, so get device information"));
	this->GetDeviceInfo(m_pPhone, true);
	TraceInfo(_D("CSSSOptions::GetSecurityOptions: Back from GetDeviceInfo, ICCID = <%s>"), m_szICCID);

	// If ICCID is still empty, return
	if (_tcslen(m_szICCID) == 0)
	{
		TraceWarning(_D("CSSSOptions::GetSecurityOptions, ICCID not available"));
		TraceLeave(_D("CSSSOptions::GetSecurityOptions"), dwResult);
		return dwResult;
	}

	// ICCID present for first time, so get registry settings

	TraceInfo(_D("CSSSOptions::GetSecurityOptions: Got ICCID = <%s>"), m_szICCID);

	memset(bPIN1, 0, sizeof(bPIN1));
	memset (szName, 0, sizeof(szName));

	TraceInfo(_D("CSSSOptions::GetSecurityOptions: Calling this->DoName, m_szICCID = <%s>"), m_szICCID);
	this->DoName(m_szICCID, szName);
	TraceInfo(_D("CSSSOptions::GetSecurityOptions: Back from this->DoName, m_szICCID = <%s>, szName = <%s>"), m_szICCID, szName);

	// If name was generated OK, get setting from registry
	if (_tcslen(szName) != 0)
	{
		// Get security settings from registry
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: Getting security options from registry"));

		// Open the Options Key [HKEY_LOCAL_MACHINE\SOFTWARE\WiViT\SIM Status Switcher\Options]
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Calling RegKeyOpen for Options key"));
		dwResult = RegKeyOpen(g_szRegOptionsKey, &hKey);
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Back from RegKeyOpen, dwResult = <%08x>, hKey = <%08x>"), dwResult, hKey);

		if (dwResult != ERROR_SUCCESS)
		{
			TraceError(_D("CSSSOptions::GetSecurityOptions: Error opening Options key, dwResult = <%08x>"), dwResult);
			TraceLeave(_D("CSSSOptions::GetSecurityOptions"), dwResult);
			return dwResult;
		}

		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Calling RegGetValue for <%s>"), g_szRegDefaultSIMID);
		dwResult = RegGetValue(hKey, g_szRegDefaultSIMID, m_szDefaultSIM, WIV_MAX_NAME, WIV_EMPTY_STRING);
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Back from RegGetValue, dwResult = 0x%08X, m_szDefaultSIM = <%s>"), dwResult, m_szDefaultSIM);
		
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Calling RegGetValue for <%s>"), szName);
		dwResult = RegGetValue(hKey, szName, m_abPIN, WIV_MAX_BINARY);
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Back from RegGetValue, dwResult = 0x%08X, m_abPIN = <%s>"), dwResult, BtoS(m_abPIN, dwResult));
		
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: Security options read from registry"));

		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Calling this->PINInfo"));
		bool bResult = this->PINInfo(dwResult, m_szPIN, szICCID, &m_blAllowAutoPINAfterInit, &m_blAllowAutoPINAfterRadioON );
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Back from this->PINInfo, bResult = %d"), bResult);
	}
	else
	{
		TraceError(_D("CSSSOptions::GetSecurityOptions: PIN Key name is blank"));
		m_blAllowAutoPINAfterInit = false;
		m_blAllowAutoPINAfterRadioON = false;
		_zclr(m_szPIN);
		_zclr(m_szDefaultSIM);
	}

	if ((_tcslen(szName) != 0) && (_tcsncmp(szName, m_szDefaultSIM, SSS_MAX_PIN) == 0))
	{
		m_blDefaultSIM = true;
	}
	else
	{
		m_blDefaultSIM = false;
	}

	CurrentSettings.blDefaultSIM = m_blDefaultSIM;
	CurrentSettings.blAllowAutoPINAfterInit = m_blAllowAutoPINAfterInit;
	CurrentSettings.blAllowAutoPINAfterRadioON = m_blAllowAutoPINAfterRadioON;
	_tcsncpy(CurrentSettings.szPIN, m_szPIN, SSS_MAX_PIN);
	_tcsncpy(CurrentSettings.szDefaultSIM, m_szDefaultSIM, SSS_MAX_PIN);

	// See whether PIN is automatically entered
	TraceDetail(_D("CSSSOptions::GetSecurityOptions: Checking if AutoPIN after reset is enabled in options"));

	if (m_blAllowAutoPINAfterInit)
	{
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: AutoPIN after reset is enabled"));
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: Calling m_pToday->SetAutoPINAfterInit(true)"));
		m_pToday->SetAutoPINAfterInit(true);
	}
	else
	{
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: AutoPIN after reset is disabled"));
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: Calling m_pToday->SetAutoPINAfterInit(false)"));
		m_pToday->SetAutoPINAfterInit(false);
	}

	if (m_blAllowAutoPINAfterRadioON)
	{
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: AutoPIN after radio on is enabled"));
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: Calling m_pToday->SetAutoPINAfterRadioON(true)"));
		m_pToday->SetAutoPINAfterRadioON(true);
	}
	else
	{
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: AutoPIN after radio on is disabled"));
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: Calling m_pToday->SetAutoPINAfterRadioON(false)"));
		m_pToday->SetAutoPINAfterRadioON(false);
	}

	if (m_blAllowAutoPINAfterRadioON || m_blAllowAutoPINAfterInit)
	{
		TraceInfo(_D("CSSSOptions::GetSecurityOptions: Calling m_pPhone->SetPIN(\"%s\")"), m_szPIN);
		m_pPhone->SetPIN(m_szPIN);
	}

	TraceInfo(_D("CSSSOptions::GetSecurityOptions: Name = <%s>"), szName);
	TraceInfo(_D("CSSSOptions::GetSecurityOptions: ICCID = <%s>"), szICCID);
	TraceInfo(_D("CSSSOptions::GetSecurityOptions: AutoPINAfterInit = <%X>"), m_blAllowAutoPINAfterInit);
	TraceInfo(_D("CSSSOptions::GetSecurityOptions: AutoPINAfterRadioON = <%X>"), m_blAllowAutoPINAfterRadioON);
	TraceInfo(_D("CSSSOptions::GetSecurityOptions: DefaultSIM = <%X>"), m_blDefaultSIM);
	TraceInfo(_D("CSSSOptions::GetSecurityOptions: PIN = <%s>"), m_szPIN);
	TraceInfo(_D("CSSSOptions::GetSecurityOptions: DefaultSIMID = <%s>"), m_szDefaultSIM);

	// Close the Options Key
	if (hKey)
	{
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Calling RegKeyClose"));
		dwResult = RegKeyClose(hKey);
		TraceDetail(_D("CSSSOptions::GetSecurityOptions: Back from RegKeyClose, dwResult = <%08x>"), dwResult);
	}

	TraceLeave(_D("CSSSOptions::GetSecurityOptions"), dwResult);
	return dwResult;
}

DWORD CSSSOptions::SetSecurityOptions()
{
	SSS_BUFFER1	buf;
	HKEY		hKey;
	DWORD		dwResult;
	DWORD		dwLength = 56;
	TCHAR		szName[17];

	TraceEnter(_D("CSSSOptions::SetSecurityOptions"));

	// Don't bother if no ICCID
	if (_tcslen(m_szICCID) == 0)
	{
		TraceWarning(_D("CSSSOptions::SetSecurityOptions, ICCID is blank, settings not saved"));
		TraceLeave(_D("CSSSOptions::SetSecurityOptions"), (DWORD)-1);
		return -1;
	}

	// Open the Options Key [HKEY_LOCAL_MACHINE\SOFTWARE\WiViT\SIM Status Switcher\Options]
	TraceDetail(_D("CSSSOptions::SetSecurityOptions: Calling RegKeyOpen for Options key"));
	dwResult = RegKeyOpen(g_szRegOptionsKey, &hKey);
	TraceDetail(_D("CSSSOptions::SetSecurityOptions: Back from RegKeyOpen, dwResult = <%08x>, hKey = <%08x>"), dwResult, hKey);

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("CSSSOptions::SetSecurityOptions: Error opening Options key, dwResult = <%08x>"), dwResult);
		TraceLeave(_D("CSSSOptions::SetSecurityOptions"), dwResult);
		return dwResult;
	}
	
	bool  bResult = this->InfoPIN(&dwLength, m_szPIN, m_szICCID, m_blAllowAutoPINAfterInit, m_blAllowAutoPINAfterRadioON);
	TraceDetail(_D("CSSSOptions::SetSecurityOptions: Back from InfoPIN, dwLength = <%08x>"), dwLength);

	memcpy(&buf.dwBufLen, m_abPIN, sizeof(DWORD));
	memcpy(buf.bBuffer, &m_abPIN[sizeof(DWORD)], buf.dwBufLen);

	memcpy(m_abPIN, &buf.bBuffer, buf.dwBufLen);

	TraceInfo(_D("CSSSOptions::SetSecurityOptions: Calling this->DoName, m_szICCID = <%s>"), m_szICCID);
	this->DoName(m_szICCID, szName);
	TraceInfo(_D("CSSSOptions::SetSecurityOptions: Back from this->DoName, m_szICCID = <%s>, szName = <%s>"), m_szICCID, szName);

	// Don't bother if key name is blank
	if (_tcslen(szName) == 0)
	{
		TraceWarning(_D("CSSSOptions::GetSecurityOptions: PIN Key name is blank, settings not saved"));
	}
	else
	{
		// Write user settings to registry
		TraceInfo(_D("CSSSOptions::SetSecurityOptions: Writing security options to registry"));

		TraceDetail(_D("CSSSOptions::SetSecurityOptions: Calling RegSetValue for <%s>"), szName);
		dwResult = RegSetValue(hKey, szName, m_abPIN, dwLength);
		TraceDetail(_D("CSSSOptions::SetSecurityOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

		if (m_blDefaultSIM)
		{
			TraceAlways(_D("CSSSOptions::SetSecurityOptions: Calling RegSetValue for <%s>"), g_szRegDefaultSIMID);
			dwResult = RegSetValue(hKey, g_szRegDefaultSIMID, szName);
			TraceDetail(_D("CSSSOptions::SetSecurityOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);
		}
		else
		{
			if (_tcsncmp(m_szDefaultSIM, szName, SSS_MAX_PIN) == 0)
			{
				TraceAlways(_D("CSSSOptions::SetSecurityOptions: Clearing value for <%s>"), g_szRegDefaultSIMID);
				dwResult = RegSetValue(hKey, g_szRegDefaultSIMID, WIV_EMPTY_STRING);
				TraceDetail(_D("CSSSOptions::SetSecurityOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);
			}
		}
	}

	// Close the Options Key
	TraceDetail(_D("CSSSOptions::SetSecurityOptions: Calling RegKeyClose"));
	dwResult = RegKeyClose(hKey);
	TraceDetail(_D("CSSSOptions::SetSecurityOptions: Back from RegKeyClose, dwResult = <%08x>"), dwResult);

	TraceLeave(_D("CSSSOptions::SetSecurityOptions"), dwResult);
	return dwResult;
}

//======================================================================
// Show property sheet.
//======================================================================
LPARAM CSSSOptions::ShowProperties(DWORD dwFlags)
{
	LPPROPSHEETPAGE	ppsp;
	PROPSHEETHEADER psh;
	INT i;
	int nPages = 0;
	LPARAM lpResult;

    TraceEnter(_D("CSSSOptions::ShowProperties"));
    TraceInfo(_D("CSSSOptions::ShowProperties: m_hwOptionsWnd = <%08X>"), m_hwOptionsWnd);

	nPages = ((dwFlags & SSS_FLAG_SHOW_ABOUT_ONLY) == SSS_FLAG_SHOW_ABOUT_ONLY) ? 1 : SSS_MAX_PROP_SHEET_PAGES;

	// Allocate zero initialised memory for all the property page structures.
	ppsp = (LPPROPSHEETPAGE)LocalAlloc (LPTR, (sizeof (PROPSHEETPAGE)*nPages));

//	memset (ppsp, 0, (sizeof (PROPSHEETPAGE)*nPages));

//	memset (ppsp, 0, sizeof (psp));

	// Fill in default values in property page structures.
	for (i = 0; i < nPages; i++) {
		ppsp[i].dwSize = sizeof (PROPSHEETPAGE);
		ppsp[i].dwFlags = PSP_DEFAULT | PSP_HASHELP | PSP_USETITLE;
		ppsp[i].hInstance = m_hmInstance;
		ppsp[i].lParam = (LPARAM)m_hwOptionsWnd;
	}

	
	// Set the tab text for each page.
	ppsp[nPages - 1].pszTitle = SSS_TEXT_ABOUT_TITLE;

	if (nPages > 1)
	{
		ppsp[0].pszTitle = SSS_TEXT_DISPLAY_TITLE;
		ppsp[1].pszTitle = SSS_TEXT_APPEARANCE_TITLE;
		ppsp[2].pszTitle = SSS_TEXT_ACTIONS_TITLE;
		ppsp[3].pszTitle = SSS_TEXT_SECURITY_TITLE;
		ppsp[4].pszTitle = SSS_TEXT_LANGUAGE_TITLE;
#ifdef WIV_DEBUG
		ppsp[5].pszTitle = SSS_TEXT_DEBUG_TITLE;
#endif
	}

	// Set the dialog box templates for each page.
	ppsp[nPages - 1].pszTemplate = MAKEINTRESOURCE(IDD_ABOUT);

	if (nPages > 1)
	{
		ppsp[0].pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_DISPLAY);
		ppsp[1].pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_APPEARANCE);
		ppsp[2].pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_ACTIONS);
		ppsp[3].pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_SECURITY);
		ppsp[4].pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_LANGUAGE);
#ifdef WIV_DEBUG
		ppsp[5].pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_DEBUG);
#endif
	}

	// Set the dialog box procedures for each page.
	ppsp[nPages - 1].pfnDlgProc = OptionsDlgProcAbout;

	if (nPages > 1)
	{
		ppsp[0].pfnDlgProc = OptionsDlgProcDisplay;
		ppsp[1].pfnDlgProc = OptionsDlgProcAppearance;
		ppsp[2].pfnDlgProc = OptionsDlgProcActions;
		ppsp[3].pfnDlgProc = OptionsDlgProcSecurity;
		ppsp[4].pfnDlgProc = OptionsDlgProcLanguage;
#ifdef WIV_DEBUG
		ppsp[5].pfnDlgProc = OptionsDlgProcDebug;
#endif
	}

	// Initialize property sheet structure.
	psh.dwSize = sizeof (PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USECALLBACK | PSH_MAXIMIZE | PSH_HASHELP;
	psh.hwndParent = m_hwOptionsWnd;
	psh.hInstance = m_hmInstance;
	psh.pszCaption = SSS_TEXT_MENU_OPTIONS;
	psh.nPages = nPages;
	psh.nStartPage = 0;
	psh.ppsp = ppsp;
	psh.pfnCallback = OptionsPropSheetProc;

	if(_tcslen(g_szReservedLabel) <= 0)
	{
		psh.nStartPage = nPages - 1;
	}

	// Create and display property sheet.
    TraceInfo(_D("CSSSOptions::ShowProperties: Calling PropertySheet"));
	lpResult = PropertySheet (&psh);
    TraceInfo(_D("CSSSOptions::ShowProperties: Back from PropertySheet, lpResult = %08X"), lpResult);

	LocalFree(ppsp);
	
    TraceLeave(_D("CSSSOptions::ShowProperties"), (DWORD)lpResult);

	return lpResult;
}

BOOL CSSSOptions::OnInitDialog(HWND hWnd, HBRUSH *haBrush)
{
	TraceEnter(_D("CSSSOptions::OnInitDialog"));

	if( NULL == m_hbmpBackgroundBitmap )
	{
	    TraceLeave(_D("CSSSOptions::OnInitDialog"), (DWORD)0);
		return 0;
	}

	if (haBrush == NULL)
	{
	    TraceLeave(_D("CSSSOptions::OnInitDialog"), (DWORD)0);
		return 0;
	}

	if (haBrush == m_ahbrushAbout)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_COPYRIGHT, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_WIVIT, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_PRODUCT, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_VERSION, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_URL, m_hbmpBackgroundBitmap );
		haBrush[5] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_SUPPORT, m_hbmpBackgroundBitmap );
		haBrush[6] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_LICENCE_TYPE, m_hbmpBackgroundBitmap );
		haBrush[7] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_SUPPORT2, m_hbmpBackgroundBitmap );
		haBrush[8] = GetBkBrush( hWnd, IDC_ABOUT_STATIC_LOGO, m_hbmpBackgroundBitmap );
		haBrush[9] = GetBkBrush( hWnd, IDC_ABOUT_BUTTON_SIM_INFORMATION, m_hbmpBackgroundBitmap );
		haBrush[10] = GetBkBrush( hWnd, IDC_ABOUT_BUTTON_SIM_REGISTRATION, m_hbmpBackgroundBitmap );
	}else if(haBrush == m_ahbrushDisplay)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_DISPLAY_CHECK_SHOW_TSP, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY, m_hbmpBackgroundBitmap );
	}else if(haBrush == m_ahbrushLanguage)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_LANGUAGE_STATIC_LANGUAGES_LIST, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_LANGUAGE_BUTTON_LOAD, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_LANGUAGE_BUTTON_SET_DEFAULT, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_LANGUAGE_STATIC_DEFAULT_LANGUAGE, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_LANGUAGE_STATIC_CURRENT_LANGUAGE, m_hbmpBackgroundBitmap );
	}else if(haBrush == m_ahbrushActions)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_ACTIONS_STATIC_TAP_ACTIONS_LIST, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_ACTIONS_COMBO_TAP_ACTION, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_ACTIONS_STATIC_TAH_ACTIONS_LIST, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_ACTIONS_COMBO_TAH_ACTION, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_ACTIONS_STATIC_TODAY_ICON_TAP_ACTIONS_LIST, m_hbmpBackgroundBitmap );
		haBrush[5] = GetBkBrush( hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION, m_hbmpBackgroundBitmap );
		haBrush[6] = GetBkBrush( hWnd, IDC_ACTIONS_STATIC_TODAY_ICON_TAH_ACTIONS_LIST, m_hbmpBackgroundBitmap );
		haBrush[7] = GetBkBrush( hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION, m_hbmpBackgroundBitmap );
		haBrush[8] = GetBkBrush( hWnd, IDC_ACTIONS_STATIC_BUTTON_ACTIONS_LIST, m_hbmpBackgroundBitmap );
		haBrush[9] = GetBkBrush( hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, m_hbmpBackgroundBitmap );
	}else if (haBrush == m_ahbrushAppearance)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_APPEARANCE_CHECK_LINE_1_BOLD, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_APPEARANCE_CHECK_LINE_2_BOLD, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_APPEARANCE_STATIC_ICON_SETS_LIST, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_APPEARANCE_LIST_ICON_SETS, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_APPEARANCE_STATIC_ICONS, m_hbmpBackgroundBitmap );
	}else if (haBrush == m_ahbrushSecurity)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_SECURITY_STATIC_ACTIVE_SIM, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_SECURITY_STATIC_PHONE, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_SECURITY_STATIC_DEFAULT_SIM, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_SECURITY_STATIC_AUTO_PIN, m_hbmpBackgroundBitmap );
		haBrush[5] = GetBkBrush( hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, m_hbmpBackgroundBitmap );
		haBrush[6] = GetBkBrush( hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, m_hbmpBackgroundBitmap );
		haBrush[7] = GetBkBrush( hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, m_hbmpBackgroundBitmap );
	}else if (haBrush == m_ahbrushPINEntry)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_PINENTRY_STATIC_ACTIVE_SIM, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_PINENTRY_STATIC_PHONE, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_PINENTRY_STATIC_ERROR, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_PINENTRY_STATIC_ENTER_PIN, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, m_hbmpBackgroundBitmap );
		haBrush[5] = GetBkBrush( hWnd, IDC_PINENTRY_BUTTON_CANCEL, m_hbmpBackgroundBitmap );
		haBrush[6] = GetBkBrush( hWnd, IDC_PINENTRY_BUTTON_OK, m_hbmpBackgroundBitmap );

	}else if (haBrush == m_ahbrushInformation)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_INFORMATION_STATIC_HEADER1, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_INFORMATION_STATIC_DEVICE_LABELS, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_INFORMATION_STATIC_DEVICE_DATA, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_INFORMATION_STATIC_HEADER2, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_INFORMATION_STATIC_SIM_LABELS, m_hbmpBackgroundBitmap );
		haBrush[5] = GetBkBrush( hWnd, IDC_INFORMATION_STATIC_SIM_DATA, m_hbmpBackgroundBitmap );
		haBrush[6] = GetBkBrush( hWnd, IDC_INFORMATION_BUTTON_CANCEL, m_hbmpBackgroundBitmap );
	}else if (haBrush == m_ahbrushRegistration)
	{
		haBrush[0] = GetBkBrush( hWnd, IDC_REGISTRATION_BUTTON_CANCEL, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_REGISTRATION_BUTTON_OK, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_REGISTRATION_STATIC_HEADER, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_REGISTRATION_EDIT_LIC1, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_REGISTRATION_EDIT_LIC2, m_hbmpBackgroundBitmap );
		haBrush[5] = GetBkBrush( hWnd, IDC_REGISTRATION_EDIT_LIC3, m_hbmpBackgroundBitmap );
		haBrush[6] = GetBkBrush( hWnd, IDC_REGISTRATION_EDIT_LIC4, m_hbmpBackgroundBitmap );
#ifndef WIV_DEBUG
	}
#else
	}else if (haBrush == m_ahbrushDebug)
	{
		TraceInfo(_D("Getting Debug Brushes"));
		haBrush[0] = GetBkBrush( hWnd, IDC_DEBUG_CHECK_TRACE_ACTIVE, m_hbmpBackgroundBitmap );
		haBrush[1] = GetBkBrush( hWnd, IDC_DEBUG_STATIC_TRACE_PATH, m_hbmpBackgroundBitmap );
		haBrush[2] = GetBkBrush( hWnd, IDC_DEBUG_EDIT_TRACE_PATH, m_hbmpBackgroundBitmap );
		haBrush[3] = GetBkBrush( hWnd, IDC_DEBUG_BUTTON_BROWSE, m_hbmpBackgroundBitmap );
		haBrush[4] = GetBkBrush( hWnd, IDC_DEBUG_STATIC_TRACE_FILES_LIST, m_hbmpBackgroundBitmap );
		haBrush[5] = GetBkBrush( hWnd, IDC_DEBUG_LIST_TRACE_FILES, m_hbmpBackgroundBitmap );
		haBrush[6] = GetBkBrush( hWnd, IDC_DEBUG_STATIC_TRACE_LEVEL, m_hbmpBackgroundBitmap );
		haBrush[7] = GetBkBrush( hWnd, IDC_DEBUG_COMBO_TRACE_LEVELS, m_hbmpBackgroundBitmap );

	}
#endif // WIV_DEBUG

    TraceLeave(_D("CSSSOptions::OnInitDialog"), (DWORD)0);

	return 0;
}

BOOL CSSSOptions::OnEraseBackground(HWND hWnd, HDC hDC) 
{
	TraceEnter(_D("CSSSOptions::OnEraseBackground"));
	TraceLeave(_D("CSSSOptions::OnEraseBackground"), (DWORD)FALSE);
	return FALSE;
}

void CSSSOptions::OnSettingChange(HWND hWnd, UINT nFlags, LPCTSTR lpszSection)
{
	TraceEnter(_D("CSSSOptions::OnSettingChange"));

	if (!(GetWindowStyle(hWnd) & WS_CHILD))
	{
		::SHHandleWMSettingChange(hWnd, (WPARAM)nFlags, (LPARAM)lpszSection, &m_saiSAI);
		TraceLeave(_D("CSSSOptions::OnSettingChange"));
		return;
	}

	DefWindowProc(hWnd, WM_SETTINGCHANGE, (WPARAM)nFlags, (LPARAM)lpszSection);
	TraceLeave(_D("CSSSOptions::OnSettingChange"));

	return;
}

void CSSSOptions::OnActivate(HWND hWnd, UINT nState, HWND hWndPrevious, BOOL bMinimized)
{
	TraceEnter(_D("CSSSOptions::OnActivate"));

	if (!(GetWindowStyle(hWnd) & WS_CHILD))
	{
		::SHHandleWMActivate(hWnd, MAKELONG(nState, (WORD)bMinimized), (LPARAM)hWndPrevious, &m_saiSAI, FALSE);
		TraceLeave(_D("CSSSOptions::OnActivate"));
		return;
	}

	DefWindowProc(hWnd, WM_ACTIVATE, MAKELONG(nState, (WORD)bMinimized), (LPARAM)hWndPrevious);

	TraceLeave(_D("CSSSOptions::OnActivate"));

	return;
}

void CSSSOptions::OnMouseMove(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	POINT point;

	TraceEnter(_D("CSSSOptions::OnMouseMove"));

	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);

	//ClientToScreen(hDlg, &point);

	TraceInfo(_D("CSSSOptions::OnMouseMove: point.x = <%d>, point.y = <%d>"), point.x, point.y);


	if (OptionsFlags.OptionsBits.AllowOptionsCancel) 
	{
		TraceDetail(_D("CSSSOptions::OnMouseMove: Checking to see if point is on Cancel icon"));
		if (this->IsOnCancelIcon(point))
		{
			if (OptionsFlags.OptionsBits.CancelDown)
			{
				TraceInfo(_D("CSSSOptions::OnMouseMove: Point is on Cancel icon, so draw the Cancel icon in depressed state"));
				HDC	hDC = GetDC(hDlg);
				DrawCancelIcon(m_hmInstance, hDC, true);
				ReleaseDC(hDlg, hDC);
			}
		}
		else
		{
			TraceInfo(_D("CSSSOptions::OnMouseMove: Point is not on Cancel icon, so draw the Cancel icon in un-depressed state"));
			HDC	hDC = GetDC(hDlg);
			DrawCancelIcon(m_hmInstance, hDC, false);
			ReleaseDC(hDlg, hDC);
		}
	}

	TraceLeave(_D("CSSSOptions::OnMouseMove"));

	return;
}

void CSSSOptions::OnLButtonDown(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	POINT point;
	
	TraceEnter(_D("CSSSOptions::OnLButtonDown"));

	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);

	//ClientToScreen(hDlg, &point);

	TraceInfo(_D("CSSSOptions::OnLButtonDown: point.x = <%d>, point.y = <%d>"), point.x, point.y);

	// If cancel allowed, draw the Cancel icon in depressed state
	if ((OptionsFlags.OptionsBits.AllowOptionsCancel) && (this->IsOnCancelIcon(point)))
	{
		HDC	hDC = GetDC(hDlg);
		DrawCancelIcon(m_hmInstance, hDC, true);
		ReleaseDC(hDlg, hDC);
		OptionsFlags.OptionsBits.CancelDown = true;
	}

	DefWindowProc(m_hwOptionsWnd, WM_LBUTTONDOWN, wParam, lParam);
	TraceLeave(_D("CSSSOptions::OnLButtonDown"));

	return;
}

void CSSSOptions::OnLButtonUp(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	POINT point;

	TraceEnter(_D("CSSSOptions::OnLButtonUp"));

	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);

	//ClientToScreen(hDlg, &point);

	TraceInfo(_D("CSSSOptions::OnLButtonUp: point.x = <%d>, point.y = <%d>"), point.x, point.y);


	if (OptionsFlags.OptionsBits.AllowOptionsCancel) 
	{
		OptionsFlags.OptionsBits.CancelDown = false;

		// Draw the Cancel icon in un-depressed state
		HDC	hDC = GetDC(hDlg);
		DrawCancelIcon(m_hmInstance, hDC, false);
		ReleaseDC(hDlg, hDC);

		TraceDetail(_D("CSSSOptions::OnLButtonUp: Checking to see if point is on Cancel icon"));
		if (this->IsOnCancelIcon(point))
		{
			TraceInfo(_D("CSSSOptions::OnLButtonUp: Point is on Cancel icon, so process cancel request"));

			CancelPropSheet();
		}
		else
		{
			TraceInfo(_D("CSSSOptions::OnLButtonUp: Not on Cancel icon"));
		}
	}

	TraceLeave(_D("CSSSOptions::OnLButtonUp"));

	return;
}

bool CSSSOptions::IsOnCancelIcon(POINT point)
{
	RECT	rect;
	bool	bResult = false;

	TraceEnter(_D("CSSSOptions::IsOnCancelIcon"));

	if (InWideMode())
	{
		rect.left = 298;
	}
	else
	{
		rect.left = 218;
	}

	rect.top = 3;
	rect.right = rect.left + 18;
	rect.bottom = rect.top + 18;

	TraceInfo(_D("CSSSOptions::IsOnCancelIcon: point.x = <%d>, point.y = <%d>"), point.x, point.y);

	if (PtInRect(&rect, point))
	{
		TraceInfo(_D("CSSSOptions::IsOnCancelIcon: On Cancel icon"));
		bResult = true;
	}

	TraceLeave(_D("CCSSSOptions::IsOnCancelIcon"), (DWORD)bResult);

	return bResult;
}

void CSSSOptions::CancelPropSheet()
{
	bool	bCancel = true;
	int		nResult = 0;

	TraceEnter(_D("CSSSOptions::CancelPropSheet"));

	TraceInfo(_D("CCSSSOptions::CancelPropSheet: m_hwPropSheet = %08X"), m_hwPropSheet);

	if(_tcslen(g_szRegisteredLabel) <= 0)
	{
		SendMessage(m_hwPropSheet, WM_COMMAND, IDCANCEL, 0);
		TraceLeave(_D("CSSSOptions::CancelPropSheet"));
		return;
	}

	SendMessage(m_hwPropSheet, PSM_QUERYSIBLINGS, 0, 0);

	TraceInfo(_D("CCSSSOptions::CancelPropSheet: PSM_QUERYSIBLINGS message sent"));

	if (memcmp(&NewSettings, &CurrentSettings, sizeof(SSS_SETTINGS)) != 0)
	{
		TraceInfo(_D("CCSSSOptions::CancelPropSheet: settings have changed"));
		bCancel = false;

		TCHAR	szMsg[WIV_MAX_STRING +1];
		_snwprintf(szMsg, WIV_MAX_STRING, _T("%s.\r\n%s"), SSS_TEXT_MESSAGE_CHANGES_MADE, SSS_TEXT_MESSAGE_SURE_TO_CANCEL);

		nResult = MessageBox (NULL, szMsg, g_szProductName, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_SETFOREGROUND);

		if (nResult == IDYES) bCancel = true;
	}

	if (bCancel)
	{
//		NewSettings = CurrentSettings;
		TraceInfo(_D("CCSSSOptions::CancelPropSheet: Sending IDCANCEL"));
		SendMessage(m_hwPropSheet, WM_COMMAND, IDCANCEL, 0);
	}

	TraceLeave(_D("CSSSOptions::CancelPropSheet"));
	return;
}

void CSSSOptions::OnAboutPaint(HWND hWnd, HDC hDC) 
{
	TCHAR	szInfo[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
	RECT	rectDraw;
	int		nLicType;
	TCHAR	szRemaining[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	TraceEnter(_D("CSSSOptions::OnAboutPaint"));

    SendDlgItemMessage (hWnd, IDC_ABOUT_BUTTON_SIM_INFORMATION, WM_SETTEXT,
                          0, (LPARAM)SSS_TEXT_ABOUT_SIM_INFORMATION);

    SendDlgItemMessage (hWnd, IDC_ABOUT_BUTTON_SIM_REGISTRATION, WM_SETTEXT,
                          0, (LPARAM)SSS_TEXT_ABOUT_REGISTRATION);

	if(_tcslen(g_szCompanyIdentity) <= 0)
	{
		if (!g_pSSSData)
		{
			TraceError(_D("CSSSOptions::OnAboutPaint: SSS Data is NULL"));
			_snwprintf(szInfo, WIV_MAX_STRING, _T("\r\n%s %s."), SSS_TEXT_ABOUT_FILE_ERROR, _T("WiViT Support"));
		}
		else
		{
			_snwprintf(szInfo, WIV_MAX_STRING, _T("\r\n%s %s."), SSS_TEXT_ABOUT_FILE_ERROR, _tcsrev(g_pSSSData->szRevSupport));
		}

		DrawStaticText(hWnd, IDC_ABOUT_STATIC_FILE_ERROR, szInfo, DT_CENTER | DT_WORDBREAK, FW_BOLD, WIV_FONT_LARGE, 0x000012C4);

		EnableWindow(ItemHandleFromID(hWnd, IDC_ABOUT_BUTTON_SIM_REGISTRATION), false); 
		EnableWindow(ItemHandleFromID(hWnd, IDC_ABOUT_BUTTON_SIM_INFORMATION), false); 

		TraceError(_D("CSSSOptions::OnAboutPaint: File is corrupt"));
		TraceLeave(_D("CSSSOptions::OnAboutPaint"));
		return;
	}

	_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: %s (%s) %s: %s"), g_szVersionLabel, g_szProductVersion,
									g_szProductType, g_szBuildLabel, g_szProductBuild);

	DrawStaticText(hWnd, IDC_ABOUT_STATIC_VERSION, szInfo, DT_CENTER, FW_MEDIUM);

	_snwprintf(szInfo, WIV_MAX_STRING, _T("%s %s."), g_szDevelopedByLabel, g_szCompanyIdentity);

	DrawStaticText(hWnd, IDC_ABOUT_STATIC_WIVIT, szInfo, DT_CENTER | DT_WORDBREAK);

	DrawStaticText(hWnd, IDC_ABOUT_STATIC_URL, g_szCompanyURL, DT_CENTER, FW_MEDIUM, WIV_FONT_UNDERLINE, g_crBlueLink);

	_snwprintf(szInfo, WIV_MAX_STRING, _T("%s %s%s %s. %s."), g_szCopyrightLabel, WIV_COPYRIGHT_SIGN, g_szFormat2005, g_szCompanyName,
									g_szReservedLabel);

	DrawStaticText(hWnd, IDC_ABOUT_STATIC_COPYRIGHT, szInfo, DT_CENTER);

	_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_ABOUT_SUPPORT, g_szCompanySupport);

	DrawStaticText(hWnd, IDC_ABOUT_STATIC_SUPPORT_LABEL, szInfo, DT_RIGHT);
	DrawStaticText(hWnd, IDC_ABOUT_STATIC_SUPPORT, g_szCompanySupport, DT_LEFT, FW_MEDIUM, WIV_FONT_UNDERLINE, g_crBlueLink);

	nLicType = GetLicenseType();
	TraceDetail(_D("CSSSOptions::OnAboutPaint: License type = %d"), nLicType);

	TraceDetail(_D("CSSSOptions::OnAboutPaint: OptionsFlags.OptionsBits.DontPaintLicense = %d"), OptionsFlags.OptionsBits.DontPaintLicense);

	if (OptionsFlags.OptionsBits.DontPaintLicense)
	{
		if(nLicType < WIV_LICTYPE_FULL)
		{
			TraceDetail(_D("CSSSOptions::OnAboutPaint: Invalid License"));
		}

		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: %s"), SSS_TEXT_ABOUT_LICENCE_TYPE, SSS_TEXT_ABOUT_PENDING_VERIFICATION);

		GetDrawRect(hWnd, IDC_ABOUT_STATIC_LICENCE_TYPE, szInfo, &rectDraw);

		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: "), SSS_TEXT_ABOUT_LICENCE_TYPE);
		rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM);

		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s"), SSS_TEXT_ABOUT_PENDING_VERIFICATION);
		rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crAmberWarning);

		TraceLeave(_D("CSSSOptions::OnAboutPaint"));
		return;
	}

	if(nLicType == WIV_LICTYPE_FULL)
	{
		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: %s"), SSS_TEXT_ABOUT_LICENCE_TYPE, g_szRegisteredLabel);
		GetDrawRect(hWnd, IDC_ABOUT_STATIC_LICENCE_TYPE, szInfo, &rectDraw);

		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: "), SSS_TEXT_ABOUT_LICENCE_TYPE);
		rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM);

		rectDraw = AppendStaticText(hWnd, rectDraw, g_szRegisteredLabel, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crGreenOK);
		EnableWindow(ItemHandleFromID(hWnd, IDC_ABOUT_BUTTON_SIM_REGISTRATION), false); 

	}else if(nLicType == WIV_LICTYPE_SPECIAL)
	{

		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: %s"), SSS_TEXT_ABOUT_LICENCE_TYPE, g_szSpecialLabel);
		GetDrawRect(hWnd, IDC_ABOUT_STATIC_LICENCE_TYPE, szInfo, &rectDraw);

		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: "), SSS_TEXT_ABOUT_LICENCE_TYPE);
		rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM);

		rectDraw = AppendStaticText(hWnd, rectDraw, g_szSpecialLabel, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crBlueInfo);

	}else if(nLicType == WIV_LICTYPE_NONE)
	{
		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s"), g_szNoLicenseLabel);
		GetDrawRect(hWnd, IDC_ABOUT_STATIC_LICENCE_TYPE, szInfo, &rectDraw);

		rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crRedError);

	} else if((nLicType != WIV_LICTYPE_TRIAL) && (nLicType != WIV_LICTYPE_BETA))
	{
		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s"), g_szInvalidLabel);
		GetDrawRect(hWnd, IDC_ABOUT_STATIC_LICENCE_TYPE, szInfo, &rectDraw);

		rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crRedError);
	}
	else
	{
		TraceDetail(_D("CSSSOptions::OnAboutPaint: Calling CalcDays"));
		m_nDaysRemaining = CalcDays(CalcRemainingTime(nLicType));

		if (m_nDaysRemaining == 1)
		{
			_snwprintf(szRemaining, WIV_MAX_STRING, _T("%d %s"), m_nDaysRemaining, SSS_TEXT_ABOUT_TRIAL_DAY_REMAINING);
		}
		else
		{
			if (m_nDaysRemaining > 1)
			{
				_snwprintf(szRemaining, WIV_MAX_STRING, _T("%d %s"), m_nDaysRemaining, SSS_TEXT_ABOUT_TRIAL_DAYS_REMAINING);
			}
			else
			{
				_snwprintf(szRemaining, WIV_MAX_STRING, _T("%s"), SSS_TEXT_ABOUT_TRIAL_EXPIRED);
			}
		}

		TraceDetail(_D("CSSSOptions::OnAboutPaint: m_nDaysRemaining = %d, szRemaining = <%s>"), m_nDaysRemaining, szRemaining);

		if (nLicType == WIV_LICTYPE_TRIAL)
		{

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: %s (%s)"), SSS_TEXT_ABOUT_LICENCE_TYPE, g_szTrialLabel, szRemaining);
			GetDrawRect(hWnd, IDC_ABOUT_STATIC_LICENCE_TYPE, szInfo, &rectDraw);

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: %s ("), SSS_TEXT_ABOUT_LICENCE_TYPE, g_szTrialLabel);
			rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM);

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s"), szRemaining);
			if (m_nDaysRemaining > WIV_DAYS_REMAINING_GREEN)
			{
				rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crGreen);
			}
			else if (m_nDaysRemaining > WIV_DAYS_REMAINING_AMBER)
			{
				rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crAmberWarning);
			}
			else
			{
				rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crRedError);
			}

			rectDraw = AppendStaticText(hWnd, rectDraw, _T(")"), DT_LEFT, FW_MEDIUM);

		}else if(nLicType == WIV_LICTYPE_BETA)
		{
			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: %s (%s)"), SSS_TEXT_ABOUT_LICENCE_TYPE, g_szBetaLabel, szRemaining);
			GetDrawRect(hWnd, IDC_ABOUT_STATIC_LICENCE_TYPE, szInfo, &rectDraw);

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s: %s ("), SSS_TEXT_ABOUT_LICENCE_TYPE, g_szBetaLabel);
			rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM);

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s"), szRemaining);
			if (m_nDaysRemaining > WIV_DAYS_REMAINING_GREEN)
			{
				rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crGreen);
			}
			else if (m_nDaysRemaining > WIV_DAYS_REMAINING_AMBER)
			{
				rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crAmberWarning);
			}
			else
			{
				rectDraw = AppendStaticText(hWnd, rectDraw, szInfo, DT_LEFT, FW_MEDIUM, WIV_FONT_NORMAL, g_crRedError);
			}

			rectDraw = AppendStaticText(hWnd, rectDraw, _T(")"), DT_LEFT, FW_MEDIUM);
		}
	}

	TraceLeave(_D("CSSSOptions::OnAboutPaint"));
	return;
}

void CSSSOptions::OnPaint(HWND hWnd, HDC hDC, bool blNoCancel) 
{
	int		nOldBkMode;

	TraceEnter(_D("CSSSOptions::OnPaint"));

	DrawTitle(m_hmInstance, hWnd, hDC, m_hfTitleFont, m_szTitle, blNoCancel ? false : OptionsFlags.OptionsBits.AllowOptionsCancel);

	if(NULL == m_hbmpBackgroundBitmap)
	{
		TraceLeave(_D("CSSSOptions::OnPaint"));
		return;
	}

	HDC		dc = CreateCompatibleDC(hDC);

	nOldBkMode = GetBkMode(hDC);

	BITMAP *pOldBitmap = (BITMAP *)SelectObject(dc, m_hbmpBackgroundBitmap);
	BITMAP bitmap;

	GetObject(m_hbmpBackgroundBitmap, sizeof(BITMAP), &bitmap);

	// Draw the bitmap
	BitBlt(hDC, 0, WIV_OPTIONS_TITLE_BAR_HEIGHT, bitmap.bmWidth, bitmap.bmHeight, dc, 0, 0, SRCCOPY);

	SelectObject(dc, pOldBitmap);
	SetBkMode(hDC, nOldBkMode);

	TraceLeave(_D("CSSSOptions::OnPaint"));
	return;
}

void CSSSOptions::OnDestroy(HWND hWnd, HBRUSH *haBrush) 
{
	TraceEnter(_D("CSSSOptions::OnDestroy"));

	// Clean Up.. 
	for (int i = 0 ; i < SSS_MAX_BRUSH_COUNT ; i++)
	{
		if (haBrush[i])
		{
			DeleteObject(haBrush[i]);
			haBrush[i] = NULL;
		}
	}

	OptionsFlags.OptionsBits.CancelDown = false;

	DefWindowProc(hWnd, WM_DESTROY, 0, 0);

	TraceLeave(_D("CSSSOptions::OnDestroy"));

	return;
}

LRESULT CSSSOptions::OnCtlColor(HBRUSH *haBrush, HWND hWndCtrl)
{
	LRESULT	lrResult = 0;

	TraceEnter(_D("CSSSOptions::OnCtlColor"));

	UINT nID = ::GetDlgCtrlID( hWndCtrl );
	
	if (haBrush == m_ahbrushAbout)
	{
		switch( nID )
		{
		case IDC_ABOUT_STATIC_COPYRIGHT:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_ABOUT_STATIC_WIVIT:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_ABOUT_STATIC_PRODUCT:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		case IDC_ABOUT_STATIC_VERSION:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_ABOUT_STATIC_URL:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		case IDC_ABOUT_STATIC_SUPPORT:
			{
				lrResult = (LRESULT)haBrush[5];
				break;
			}
		case IDC_ABOUT_STATIC_LICENCE_TYPE:
			{
				lrResult = (LRESULT)haBrush[6];
				break;
			}
		case IDC_ABOUT_STATIC_SUPPORT2:
			{
				lrResult = (LRESULT)haBrush[7];
				break;
			}
		case IDC_ABOUT_STATIC_LOGO:
			{
				lrResult = (LRESULT)haBrush[8];
				break;
			}
		case IDC_ABOUT_BUTTON_SIM_INFORMATION:
			{
				lrResult = (LRESULT)haBrush[9];
				break;
			}
		case IDC_ABOUT_BUTTON_SIM_REGISTRATION:
			{
				lrResult = (LRESULT)haBrush[10];
				break;
			}
		}
	}else if (haBrush == m_ahbrushDisplay)
	{
		switch( nID )
		{
		case IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_DISPLAY_CHECK_SHOW_TSP:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		}
	}else if (haBrush == m_ahbrushLanguage)
	{
		switch( nID )
		{
		case IDC_LANGUAGE_STATIC_LANGUAGES_LIST:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_LANGUAGE_BUTTON_LOAD:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_LANGUAGE_BUTTON_SET_DEFAULT:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		case IDC_LANGUAGE_STATIC_DEFAULT_LANGUAGE:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_LANGUAGE_STATIC_CURRENT_LANGUAGE:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		}
	}else if (haBrush == m_ahbrushActions)
	{
		switch( nID )
		{
		case IDC_ACTIONS_STATIC_TAP_ACTIONS_LIST:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_ACTIONS_COMBO_TAP_ACTION:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_ACTIONS_STATIC_TAH_ACTIONS_LIST:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		case IDC_ACTIONS_COMBO_TAH_ACTION:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_ACTIONS_STATIC_TODAY_ICON_TAP_ACTIONS_LIST:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		case IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION:
			{
				lrResult = (LRESULT)haBrush[5];
				break;
			}
		case IDC_ACTIONS_STATIC_TODAY_ICON_TAH_ACTIONS_LIST:
			{
				lrResult = (LRESULT)haBrush[6];
				break;
			}
		case IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION:
			{
				lrResult = (LRESULT)haBrush[7];
				break;
			}
		case IDC_ACTIONS_STATIC_BUTTON_ACTIONS_LIST:
			{
				lrResult = (LRESULT)haBrush[8];
				break;
			}
		case IDC_ACTIONS_COMBO_BUTTON_ACTION:
			{
				lrResult = (LRESULT)haBrush[9];
				break;
			}
		}
	}else if (haBrush == m_ahbrushAppearance)
	{
		switch( nID )
		{
		case IDC_APPEARANCE_CHECK_LINE_1_BOLD:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_APPEARANCE_CHECK_LINE_2_BOLD:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_APPEARANCE_STATIC_ICON_SETS_LIST:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		case IDC_APPEARANCE_LIST_ICON_SETS:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_APPEARANCE_STATIC_ICONS:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		}
	}else if (haBrush == m_ahbrushSecurity)
	{
		switch( nID )
		{
		case IDC_SECURITY_STATIC_ACTIVE_SIM:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_SECURITY_STATIC_PHONE:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_SECURITY_STATIC_DEFAULT_SIM:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		case IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_SECURITY_STATIC_AUTO_PIN:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		case IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT:
			{
				lrResult = (LRESULT)haBrush[5];
				break;
			}
		case IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON:
			{
				lrResult = (LRESULT)haBrush[6];
				break;
			}
		case IDC_SECURITY_BUTTON_CHANGE_PIN:
			{
				lrResult = (LRESULT)haBrush[7];
				break;
			}
		}
	}else if (haBrush == m_ahbrushPINEntry)
	{
		switch( nID )
		{
		case IDC_PINENTRY_STATIC_ACTIVE_SIM:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_PINENTRY_STATIC_PHONE:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_PINENTRY_STATIC_ERROR:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		case IDC_PINENTRY_STATIC_ENTER_PIN:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_PINENTRY_EDIT_ENTER_PIN:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		case IDC_PINENTRY_BUTTON_CANCEL:
			{
				lrResult = (LRESULT)haBrush[5];
				break;
			}
		case IDC_PINENTRY_BUTTON_OK:
			{
				lrResult = (LRESULT)haBrush[6];
				break;
			}
			//TODO:
		}
	}else if (haBrush == m_ahbrushInformation)
	{
		switch( nID )
		{
		case IDC_INFORMATION_STATIC_HEADER1:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_INFORMATION_STATIC_DEVICE_LABELS:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_INFORMATION_STATIC_DEVICE_DATA:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		case IDC_INFORMATION_STATIC_HEADER2:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_INFORMATION_STATIC_SIM_LABELS:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		case IDC_INFORMATION_STATIC_SIM_DATA:
			{
				lrResult = (LRESULT)haBrush[5];
				break;
			}
		case IDC_INFORMATION_BUTTON_CANCEL:
			{
				lrResult = (LRESULT)haBrush[6];
				break;
			}
		}
	}else if (haBrush == m_ahbrushRegistration)
	{
		switch( nID )
		{
		case IDC_REGISTRATION_BUTTON_CANCEL:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_REGISTRATION_BUTTON_OK:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_REGISTRATION_STATIC_HEADER:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}

		case IDC_REGISTRATION_EDIT_LIC1:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_REGISTRATION_EDIT_LIC2:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		case IDC_REGISTRATION_EDIT_LIC3:
			{
				lrResult = (LRESULT)haBrush[5];
				break;
			}
		case IDC_REGISTRATION_EDIT_LIC4:
			{
				lrResult = (LRESULT)haBrush[6];
				break;
			}
		}
#ifndef WIV_DEBUG
	}
#else
	}else if (haBrush == m_ahbrushDebug)
	{
		switch( nID )
		{
		case IDC_DEBUG_CHECK_TRACE_ACTIVE:
			{
				lrResult = (LRESULT)haBrush[0];
				break;
			}
		case IDC_DEBUG_STATIC_TRACE_PATH:
			{
				lrResult = (LRESULT)haBrush[1];
				break;
			}
		case IDC_DEBUG_EDIT_TRACE_PATH:
			{
				lrResult = (LRESULT)haBrush[2];
				break;
			}
		case IDC_DEBUG_BUTTON_BROWSE:
			{
				lrResult = (LRESULT)haBrush[3];
				break;
			}
		case IDC_DEBUG_STATIC_TRACE_FILES_LIST:
			{
				lrResult = (LRESULT)haBrush[4];
				break;
			}
		case IDC_DEBUG_LIST_TRACE_FILES:
			{
				lrResult = (LRESULT)haBrush[5];
				break;
			}
		case IDC_DEBUG_STATIC_TRACE_LEVEL:
			{
				lrResult = (LRESULT)haBrush[6];
				break;
			}
		case IDC_DEBUG_COMBO_TRACE_LEVELS:
			{
				lrResult = (LRESULT)haBrush[7];
				break;
			}
		}
	}
#endif // WIV_DEBUG
	
	TraceLeave(_D("CSSSOptions::OnCtlColor"), lrResult);
	return lrResult;
}

//======================================================================
// General utility functions
//======================================================================

void CSSSOptions::DrawSecurityHeader(HWND hWnd, UINT uID)
{
	TCHAR		szNumber[SSS_MAX_PHONE_NUMBER + 1];
	COLORREF	cr = g_crBlueInfo;
	TCHAR		szTemp[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
	RECT		rectDraw;

	TraceEnter(_D("CSSSOptions::DrawSecurityHeader"));

	_tcscpy(szNumber, m_pPhone->GetPhoneNumber());

	TraceInfo(_D("CSSSOptions::DrawSecurityHeader: Phone Number = <%s>"), szNumber);

	if (_tcslen(szNumber) == 0)
	{
		_tcscpy(szNumber, SSS_TEXT_ERROR_UNAVAILABLE);
		cr = g_crRedError;
	}

	_snwprintf(szTemp, WIV_MAX_STRING, _T("%s: %s"), SSS_TEXT_TELEPHONE_NO, szNumber);
	GetDrawRect(hWnd, uID, szTemp, &rectDraw, DT_LEFT);
	_snwprintf(szTemp, WIV_MAX_STRING, _T("%s: "), SSS_TEXT_TELEPHONE_NO);

	if (IsSecondEdition() && InWideMode())
	{
		rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_NORMAL, WIV_FONT_MEDIUM);
		_snwprintf(szTemp, WIV_MAX_STRING, _T("%s"), szNumber);
		rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_SEMIBOLD, WIV_FONT_MEDIUM, cr);
	}
	else
	{
		rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_MEDIUM);
		_snwprintf(szTemp, WIV_MAX_STRING, _T("%s"), szNumber);
		rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_SEMIBOLD, WIV_FONT_NONE, cr);
	}

	TraceLeave(_D("CSSSOptions::DrawSecurityHeader"));

	return;
}

void CSSSOptions::DrawSecurityError(HWND hWnd, LPCTSTR lpszError)
{
	TraceEnter(_D("CSSSOptions::DrawSecurityError"));

	if (IsSecondEdition() && InWideMode())
	{
		DrawStaticText(hWnd, IDC_PINENTRY_STATIC_ERROR, lpszError, DT_VCENTER | DT_CENTER, FW_NORMAL, WIV_FONT_MEDIUM, g_crRedError);
	}
	else
	{
		DrawStaticText(hWnd, IDC_PINENTRY_STATIC_ERROR, lpszError, DT_VCENTER | DT_CENTER, FW_NORMAL, WIV_FONT_NONE, g_crRedError);
	}

	::MessageBeep(MB_ICONEXCLAMATION);
	TraceLeave(_D("CSSSOptions::DrawSecurityError"));
	return;
}

void CSSSOptions::ClearSecurityError(HWND hWnd)
{
	TraceEnter(_D("CSSSOptions::ClearSecurityError"));
	ClearStaticText(hWnd, IDC_PINENTRY_STATIC_ERROR, (NULL == m_hbmpBackgroundBitmap ? GetSysColorBrush(COLOR_WINDOW) : m_ahbrushPINEntry[6]));
	TraceLeave(_D("CSSSOptions::ClearSecurityError"));
	return;
}

DWORD CSSSOptions::GetCurrentSeqStep()
{
	DWORD dwSeqStep;

	TraceEnter(_D("CSSSOptions::GetCurrentSeqStep"));

	dwSeqStep = m_adwSecuritySequence[m_dwSecuritySeqNumber][m_dwSecuritySeqStep];

	TraceDetail(_D("CSSSOptions::GetCurrentSeqStep, dwSeqStep = %d"), dwSeqStep);

	TraceLeave(_D("CSSSOptions::GetCurrentSeqStep"), dwSeqStep);

	return dwSeqStep;
}

LPCTSTR CSSSOptions::SetupPINEntryPage(HWND hWnd, bool blFromPaint)
{
	LPCTSTR	lpszText	= NULL;
	bool	bAuto		= false;
	bool	bCurrentPIN	= false;
	bool	bNewPIN		= false;
	bool	bConfirmPIN	= false;
	bool	bEnterPIN	= false;
	bool	bPINExists	= (_tcslen(m_szPIN) == 0) ? false : true;
	TCHAR	szInfo[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	TraceEnter(_D("CSSSOptions::SetupPINEntryPage"));

	DWORD dwCurStep = GetCurrentSeqStep();

	if (dwCurStep == g_dwSecuritySequenceActionNoPIN)
	{
		bAuto = false;

	}else if (dwCurStep == g_dwSecuritySequenceActionCreate)
	{
		bAuto = true;

	}else if (dwCurStep == g_dwSecuritySequenceActionChange)
	{
		bAuto = true;

	}else if (dwCurStep == g_dwSecuritySequenceActionCurrent)
	{
		bAuto = true;
		bCurrentPIN = true;
		bEnterPIN = true;

	}else if (dwCurStep == g_dwSecuritySequenceActionNew)
	{
		bAuto = true;
		bNewPIN = true;
		bEnterPIN = true;

	}else if (dwCurStep == g_dwSecuritySequenceActionEnter)
	{
		bAuto = true;
		bEnterPIN = true;

	}else if (dwCurStep == g_dwSecuritySequenceActionConfirm)
	{
		bAuto = true;
		bConfirmPIN = true;
		bEnterPIN = true;
	}
	else
	{
		m_dwSecuritySeqStep = g_dwSecuritySequenceStepStart;
	}
	
//	ShowWindow(ItemHandleFromID(hWnd, IDC_PINENTRY_EDIT_ENTER_PIN), bAuto & bEnterPIN); 
//	ShowWindow(ItemHandleFromID(hWnd, IDC_PINENTRY_BUTTON_OK), bAuto & bEnterPIN); 
//	ShowWindow(ItemHandleFromID(hWnd, IDC_PINENTRY_BUTTON_CANCEL), bAuto & bEnterPIN);

	if (bAuto & !bEnterPIN)
	{
		TraceDetail(_D("CSSSOptions::SetupPINEntryPage: Clearing IDC_PINENTRY_STATIC_ENTER_PIN"));
		ClearStaticText(hWnd, IDC_PINENTRY_STATIC_ENTER_PIN, (NULL == m_hbmpBackgroundBitmap ? GetSysColorBrush(COLOR_WINDOW) : m_ahbrushPINEntry[7]));
	}

//	EnableWindow(ItemHandleFromID(hWnd, IDC_PINENTRY_BUTTON_OK), bAuto & bEnterPIN); 
//	EnableWindow(ItemHandleFromID(hWnd, IDC_PINENTRY_BUTTON_CANCEL), bAuto); 

	if (bAuto & bEnterPIN & !bPINExists & !bCurrentPIN & !bNewPIN & !bConfirmPIN)
	{
		TraceDetail(_D("CSSSOptions::SetupPINEntryPage: Drawing SSS_TEXT_PINENTRY_STATIC_ENTER_PIN"));
		lpszText = SSS_TEXT_PINENTRY_STATIC_ENTER_PIN;
	}
	if (bAuto & bEnterPIN & bCurrentPIN)
	{
		TraceDetail(_D("CSSSOptions::SetupPINEntryPage: Drawing SSS_TEXT_PINENTRY_STATIC_CURRENT"));
		lpszText = SSS_TEXT_PINENTRY_STATIC_CURRENT;
	}
	if (bAuto & bEnterPIN & bNewPIN)
	{
		TraceDetail(_D("CSSSOptions::SetupPINEntryPage: Drawing SSS_TEXT_PINENTRY_STATIC_NEW"));
		lpszText = SSS_TEXT_PINENTRY_STATIC_NEW;
	}
	if (bAuto & bEnterPIN & bConfirmPIN)
	{
		TraceDetail(_D("CSSSOptions::SetupPINEntryPage: Drawing SSS_TEXT_PINENTRY_STATIC_CONFIRM"));
		lpszText = SSS_TEXT_PINENTRY_STATIC_CONFIRM;
	}

	TraceDetail(_D("CSSSOptions::SetupPINEntryPage: Clearing IDC_PINENTRY_STATIC_ENTER_PIN"));
	ClearStaticText(hWnd, IDC_PINENTRY_STATIC_ENTER_PIN, (NULL == m_hbmpBackgroundBitmap ? GetSysColorBrush(COLOR_WINDOW) : m_ahbrushPINEntry[7]));

	if (!blFromPaint && (lpszText != NULL))
	{
		TraceDetail(_D("CSSSOptions::SetupPINEntryPage: Drawing text"));
		_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), lpszText);

		if (IsSecondEdition() && InWideMode())
		{
			DrawStaticText(hWnd, IDC_PINENTRY_STATIC_ENTER_PIN, szInfo, DT_RIGHT, FW_NORMAL, WIV_FONT_MEDIUM);
		}
		else
		{
			DrawStaticText(hWnd, IDC_PINENTRY_STATIC_ENTER_PIN, szInfo, DT_RIGHT);
		}
	}

	if (bEnterPIN) SetFocus(ItemHandleFromID(hWnd, IDC_PINENTRY_EDIT_ENTER_PIN));

	TraceDetail(_D("CSSSOptions::SetupPINEntryPage: dwCurStep   = %d"), dwCurStep);
	TraceDetail(_D("CSSSOptions::SetupPINEntryPage: bAuto       = %d"), bAuto);
	TraceDetail(_D("CSSSOptions::SetupPINEntryPage: bCurrentPIN = %d"), bCurrentPIN);
	TraceDetail(_D("CSSSOptions::SetupPINEntryPage: bNewPIN     = %d"), bNewPIN);
	TraceDetail(_D("CSSSOptions::SetupPINEntryPage: bConfirmPIN = %d"), bConfirmPIN);
	TraceDetail(_D("CSSSOptions::SetupPINEntryPage: bEnterPIN   = %d"), bEnterPIN);
	TraceDetail(_D("CSSSOptions::SetupPINEntryPage: bPINExists  = %d"), bPINExists);

	TraceLeave(_D("CSSSOptions::SetupPINEntryPage"), (DWORD)lpszText);

	return lpszText;
}

void CSSSOptions::InitializeActionLists( HWND hWnd)
{
	int		i;

	TraceEnter(_D("CSSSOptions::InitializeActionLists"));

	// List single tap actions into combo box.
	SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAP_ACTION, CB_RESETCONTENT,
						0, 0);

	for (i = 0; i < SSS_MAX_AVAILABLE_ACTIONS; i++)
	{
		TraceInfo(_D("CSSSOptions::InitializeActionLists: Adding <%s> to Actions:IDC_ACTIONS_COMBO_TAP_ACTION"), m_aszActions[i]);
		SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAP_ACTION, CB_ADDSTRING,
							0, (LPARAM)m_aszActions[i]);
	}

	// List tap and hold actions into combo box.
	SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAH_ACTION, CB_RESETCONTENT,
						0, 0);

	for (i = 0; i < SSS_MAX_AVAILABLE_ACTIONS; i++)
	{
		TraceInfo(_D("CSSSOptions::InitializeActionLists: Adding <%s> to Actions:IDC_ACTIONS_COMBO_TAH_ACTION"), m_aszActions[i]);
		SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAH_ACTION, CB_ADDSTRING,
							0, (LPARAM)m_aszActions[i]);
	}

	// List today icon tap actions into combo box.
	SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION, CB_RESETCONTENT,
						0, 0);

	for (i = 0; i < SSS_MAX_AVAILABLE_ACTIONS; i++)
	{
		TraceInfo(_D("CSSSOptions::InitializeActionLists: Adding <%s> to Actions:IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION"), m_aszActions[i]);
		SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION, CB_ADDSTRING,
							0, (LPARAM)m_aszActions[i]);
	}

	// List today icon tap and hold actions into combo box.
	SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION, CB_RESETCONTENT,
						0, 0);

	for (i = 0; i < SSS_MAX_AVAILABLE_ACTIONS; i++)
	{
		TraceInfo(_D("CSSSOptions::InitializeActionLists: Adding <%s> to Actions:IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION"), m_aszActions[i]);
		SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION, CB_ADDSTRING,
							0, (LPARAM)m_aszActions[i]);

	}

	// List hardware button actions into combo box.
	SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, CB_RESETCONTENT, 0, 0);
	
	TraceInfo(_D("CSSSOptions::InitializeActionLists: Adding <%s> to Actions:IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION"), m_aszActions[3]);
	SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, CB_ADDSTRING,
		0, (LPARAM)m_aszActions[3]);
	
	TraceInfo(_D("CSSSOptions::InitializeActionLists: Adding <%s> to Actions:IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION"), m_aszActions[4]);
	SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, CB_ADDSTRING,
		0, (LPARAM)m_aszActions[4]);
		
	TraceLeave(_D("CSSSOptions::InitializeActionLists"));

	return;
}

bool CSSSOptions::InitializeLanguagesList(HWND hWnd)
{
	int			nLanguageCount;
	PWIVLANG	psLanguages;
	LVITEM		lvItem;
	LVCOLUMN	lvColumn;
	LVFINDINFO	lvFindInfo;
	int			nIndex = 0;
	bool		blRetVal = false;

	TraceEnter(_D("CSSSOptions::InitializeLanguagesList"));

	SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)LVS_EX_FULLROWSELECT, (LPARAM)LVS_EX_FULLROWSELECT);

	SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_DELETEALLITEMS, 0, 0);
	SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_DELETECOLUMN, 1, 0);
	SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_DELETECOLUMN, 0, 0);

	nLanguageCount = LangGetLanguagesList(&psLanguages);

	// Insert the Columns
    memset(&lvColumn, 0, sizeof(lvColumn));
	lvColumn.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT|LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 0x25; 
	lvColumn.pszText = (LPTSTR)SSS_TEXT_LANGUAGE_ID;
	lvColumn.cchTextMax = _tcslen(lvColumn.pszText);
	lvColumn.iSubItem = 0;

	SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_INSERTCOLUMN, 0, (LPARAM)&lvColumn);

	lvColumn.iSubItem = 1;
 	lvColumn.cx = 0x75;
	lvColumn.pszText = (LPTSTR)SSS_TEXT_LANGUAGE_NAME;
	lvColumn.cchTextMax = _tcslen(lvColumn.pszText);

    SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_INSERTCOLUMN, 1, (LPARAM)&lvColumn);

	// List available languages.
    memset(&lvItem, 0, sizeof(lvItem));
	
	for (int i = 0; i < nLanguageCount; i++)
	{
		TraceInfo(_D("CSSSOptions::InitializeLanguagesList: Adding <%s> <%s> to Display:IDC_DISPLAY_LIST_LANGUAGES"),
			psLanguages[i].ID, psLanguages[i].Name);
		
		lvItem.mask = LVIF_TEXT;   // Text Style
		lvItem.pszText = psLanguages[i].ID; // Text to display (can be from a char variable) (Items)
		lvItem.cchTextMax = _tcslen(lvItem.pszText); // Max size of text
		lvItem.iItem = i;          // choose item  
		lvItem.iSubItem = 0;       // Put in first column
    
		TraceDetail(_D("CSSSOptions::InitializeLanguagesList: Inserting <%s> as item %d, subitem %d"), lvItem.pszText, lvItem.iItem, lvItem.iSubItem);
		SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

		lvItem.pszText = psLanguages[i].Name;
		lvItem.cchTextMax = _tcslen(lvItem.pszText); // Max size of text
		lvItem.iSubItem = 1;

		TraceDetail(_D("CSSSOptions::InitializeLanguagesList: Adding <%s> to item %d, subitem %d"), lvItem.pszText, lvItem.iItem, lvItem.iSubItem);
		SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_SETITEM, 0, (LPARAM)&lvItem);
	}

	m_lpsDefaultLanguage = LangGetDefaultLanguage();
	m_lpsCurrentLanguage = LangGetCurrentLanguage();

	memset(&lvFindInfo, 0, sizeof(LVFINDINFO));

	lvFindInfo.flags = LVFI_STRING;

	if (m_lpsDefaultLanguage != NULL)
	{
		TraceDetail(_D("CSSSOptions::InitializeLanguagesList: Default Language ID = <%s>"), m_lpsDefaultLanguage->ID);
		lvFindInfo.psz = m_lpsDefaultLanguage->ID;
	}
	else
	{
		lvFindInfo.psz = WIV_EMPTY_STRING;
	}

	nIndex = SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_FINDITEM, -1, (LPARAM)&lvFindInfo);
	TraceDetail(_D("CSSSOptions::InitializeLanguagesList: nIndex = %d"), nIndex);

	if (nIndex >= 0)
	{
		TraceDetail(_D("CSSSOptions::InitializeLanguagesList: Default Language ID <%s> found at %d"), m_lpsDefaultLanguage->ID, nIndex);
		lvItem.mask = LVIF_STATE; 
		lvItem.iItem = nIndex;
		lvItem.stateMask = LVIS_SELECTED;
		lvItem.state = 0;
		SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_SETITEMSTATE, -1, (LPARAM)&lvItem);
		
		lvItem.state = LVIS_SELECTED;
		SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_SETITEMSTATE, nIndex, (LPARAM)&lvItem);

		lvItem.stateMask = LVIS_FOCUSED;
		lvItem.state = LVIS_FOCUSED;
		SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_SETITEMSTATE, nIndex, (LPARAM)&lvItem);

		SendDlgItemMessage(hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_ENSUREVISIBLE, nIndex, (LPARAM)FALSE);

		blRetVal = true;
	}
	else
	{
		TraceDetail(_D("CSSSOptions::InitializeLanguagesList: Default Language ID <%s> not found"), m_lpsDefaultLanguage->ID);
	}

	//SetFocus(ItemHandleFromID(hWnd, IDC_LANGUAGE_LIST_LANGUAGES));

	TraceLeave(_D("CSSSOptions::InitializeLanguagesList"), (DWORD)blRetVal);

	return blRetVal;
}

//========================================================================
// Called when a property sheet page receives a PSN_SETACTIVE notification
//========================================================================
void CSSSOptions::AutoScroll(HWND hWndMain, HWND hTabCtrl)
{
	int iItem;
	RECT rItemRect,rWindow;

	TraceEnter(_D("CSSSOptions::AutoScroll"));

	GetWindowRect(hWndMain,&rWindow);

	iItem = TabCtrl_GetCurSel(hTabCtrl);
	TabCtrl_GetItemRect(hTabCtrl,iItem,&rItemRect);

	if(rItemRect.left < 10)
	{
	 PostMessage(m_hwSpinButton,WM_LBUTTONDOWN,1,0x9000A);
	 PostMessage(m_hwSpinButton,WM_LBUTTONUP,1,0xD000D);
	}

	if(rItemRect.right > rWindow.right-40)
	{
	 PostMessage(m_hwSpinButton,WM_LBUTTONDOWN,1,0x70016);
	 PostMessage(m_hwSpinButton,WM_LBUTTONUP,1,0x6001B);
	}

	TraceLeave(_D("CSSSOptions::AutoScroll"));

	return;
}

void CSSSOptions::UpdateTabsText()
{
	TCITEM	tcItem;

	TraceEnter(_D("CSSSOptions::UpdateTabsText"));

	memset(&tcItem, 0, sizeof(TCITEM));
	tcItem.mask = TCIF_TEXT;
	
	tcItem.pszText = (LPTSTR)SSS_TEXT_DISPLAY_TITLE;
	TabCtrl_SetItem(m_hwTabCtrl, 0, &tcItem);
	tcItem.pszText = (LPTSTR)SSS_TEXT_APPEARANCE_TITLE;
	TabCtrl_SetItem(m_hwTabCtrl, 1, &tcItem);
	tcItem.pszText = (LPTSTR)SSS_TEXT_ACTIONS_TITLE;
	TabCtrl_SetItem(m_hwTabCtrl, 2, &tcItem);
	tcItem.pszText = (LPTSTR)SSS_TEXT_SECURITY_TITLE;
	TabCtrl_SetItem(m_hwTabCtrl, 3, &tcItem);
	tcItem.pszText = (LPTSTR)SSS_TEXT_LANGUAGE_TITLE;
	TabCtrl_SetItem(m_hwTabCtrl, 4, &tcItem);
#ifdef WIV_DEBUG
	tcItem.pszText = (LPTSTR)SSS_TEXT_DEBUG_TITLE;
	TabCtrl_SetItem(m_hwTabCtrl, 5, &tcItem);
#endif
	tcItem.pszText = (LPTSTR)SSS_TEXT_ABOUT_TITLE;
	TabCtrl_SetItem(m_hwTabCtrl, SSS_MAX_PROP_SHEET_PAGES - 1, &tcItem);

	TraceLeave(_D("CSSSOptions::UpdateTabsText"));
	return;
}

//======================================================================
// DrawIconSet - Draws an icon set.
//======================================================================
void CSSSOptions::DrawIconSet(HWND hWnd, HDC hDC, HWND hIcons)
{
	RECT	rect;

	TraceEnter(_D("CSSSOptions::DrawIconSet"));

	GetWindowRect(hIcons, &rect);

	rect.top -= WIV_OPTIONS_TITLE_BAR_HEIGHT;
	rect.bottom -= WIV_OPTIONS_TITLE_BAR_HEIGHT;

	// If device context supplied, load and draw the icons
	if (hDC)
	{
		HICON	hIcon = NULL;

		SetBkMode(hDC, TRANSPARENT);

		UINT	uID = IDFromIconSet(m_dwCurIconSet, g_dwIconTypeNeutral);
		hIcon = LoadIcon(m_hmInstance, MAKEINTRESOURCE(uID + g_dwIconTypeOff));
		DrawIcon(hDC, _X(rect), _Y(rect), hIcon);
		hIcon = LoadIcon(m_hmInstance, MAKEINTRESOURCE(uID + g_dwIconTypeOn));
		DrawIcon(hDC, _X(rect) + 42, _Y(rect), hIcon);
		hIcon = LoadIcon(m_hmInstance, MAKEINTRESOURCE(uID + g_dwIconTypeFull));
		DrawIcon(hDC, _X(rect), _Y(rect) + 42, hIcon);
		hIcon = LoadIcon(m_hmInstance, MAKEINTRESOURCE(uID + g_dwIconTypeNeutral));
		DrawIcon(hDC, _X(rect) + 42, _Y(rect) + 42, hIcon);
	}

	// If window handle supplied, invalidate the icons' area to force painting
	if (hWnd)
	{
		InvalidateRect(hWnd, &rect, TRUE);
		UpdateWindow(hWnd);
	}

	TraceLeave(_D("CSSSOptions::DrawIconSet"));

	return;
}

BOOL CSSSOptions::SetupHelpFile(PWIVLANG lpHelpLanguage)
{
	BOOL	blRetVal = FALSE;
	DWORD	dwError = 0;
	TCHAR	szSourceFileName[WIV_MAX_PATH + 1] = WIV_EMPTY_STRING;
	TCHAR	szTargetFileName[WIV_MAX_PATH + 1] = WIV_EMPTY_STRING;
	HANDLE	hfFile = NULL;

	TraceEnter(_D("CSSSOptions::SetupHelpFile"));
/*
	
	// Open/Create the trace file
	hfFile = CreateFile
		(
		m_szTraceFilePath,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		NULL
		);
	
*/	
	
	_snwprintf(szSourceFileName, WIV_MAX_PATH, _T("%s%s_%s.lnk"), GetInstallPath(), g_szProductShortName, lpHelpLanguage->ID);
	_snwprintf(szTargetFileName, WIV_MAX_PATH, _T("%sHelp\\%s.lnk"), GetWindowsPath(), g_szProductName);

	TraceDetail(_D("CSSSOptions::SetupHelpFile: szSourceFileName = <%s>, szTargetFileName = <%s>"), szSourceFileName, szTargetFileName);

	blRetVal = CopyFile(szSourceFileName, szTargetFileName, FALSE);
	if (!blRetVal) dwError = GetLastError();

	TraceDetail(_D("CSSSOptions::SetupHelpFile: blRetVal = 0x%08X, dwError = 0x%08X"), blRetVal, dwError);

	TraceLeave(_D("CSSSOptions::SetupHelpFile"), (DWORD)blRetVal);
	return blRetVal;
}

//======================================================================
// DoShowHelp - Show help for specified section.
//======================================================================
LRESULT CSSSOptions::DoShowHelp(LPCTSTR lpszSection)
{
	BOOL	bResult;
	LRESULT	lrResult;

	TCHAR	szHelp[MAX_PATH + 1] = WIV_EMPTY_STRING;

	TraceEnter(_D("CSSSOptions::DoShowHelp"));

	m_lpsDefaultLanguage = LangGetDefaultLanguage();
	m_lpsCurrentLanguage = LangGetCurrentLanguage();
			
	_snwprintf(szHelp, MAX_PATH, _T("%s%s%s_%s.htm#%s"), g_szHelpPrefix, GetWindowsPath(), g_szHelpFile, m_lpsCurrentLanguage->ID, lpszSection);

	TraceDetail(_D("CSSSOptions::DoShowHelp: szHelp = <%s>"), szHelp);

	bResult = CreateProcess(g_szPegHelpExe, szHelp, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);

	if (bResult == FALSE)
	{
		 // CreateProcess failed.
		lrResult = MessageBox (NULL, SSS_TEXT_ERROR_PEGHELP, SSS_TEXT_ERROR_MESSAGEBOX, MB_OK);

		if (lrResult == 0)
		{
			// Not enough memory to create MessageBox.
			TraceLeave(_D("CSSSOptions::DoShowHelp"), (DWORD)E_OUTOFMEMORY);
			return E_OUTOFMEMORY;
		}

		TraceLeave(_D("CSSSOptions::DoShowHelp"), (DWORD)E_FAIL);
		return E_FAIL;
	}
	else
	{
		// CreateProcess succeeded.
		TraceLeave(_D("CSSSOptions::DoShowHelp"), (DWORD)S_OK);
		return S_OK;
	}
}

bool CSSSOptions::OptionsLicenseNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig)
{
	TraceEnter(_D("CSSSOptions::OptionsLicenseNotify"));
	
	if (lpLicenseData == NULL)
	{
		OptionsFlags.Options.License = dwLicenseConfig;
		TraceDetail(_D("CSSSOptions::OptionsLicenseNotify: OptionsFlags.Options.License = 0x%08X"), OptionsFlags.Options.License);
	}

	TraceLeave(_D("CSSSOptions::OptionsLicenseNotify"));
	return true;
}

void CSSSOptions::SetupPhoneState()
{
	DWORD	dwRadioSupport;
	DWORD	dwEquipmentState;
	DWORD	dwReadyState;
	DWORD	dwSIMLockedState;
	DWORD	dwLicense;

	TraceEnter(_D("CSSSOptions::SetupPhoneState"));

	m_pPhone->GetPhoneState(dwRadioSupport, dwEquipmentState, dwReadyState, dwSIMLockedState, dwLicense);
	OptionsFlags.OptionsBits.PhoneIsOff = (!(dwRadioSupport & RIL_RADIOSUPPORT_ON))
						|| (!(dwEquipmentState & RIL_EQSTATE_FULL))
						|| (!(dwReadyState & RIL_READYSTATE_SIM));

	OptionsFlags.OptionsBits.SIMReady = ((dwReadyState & RIL_READYSTATE_SIMREADY) == RIL_READYSTATE_SIMREADY);
	
	OptionsFlags.OptionsBits.SIMUnlocked = (dwSIMLockedState & RIL_LOCKEDSTATE_READY);

	OptionsFlags.OptionsBits.SIMMissing = (dwSIMLockedState == RIL_E_SIMNOTINSERTED);
	OptionsFlags.OptionsBits.IncorrectSIM = (dwSIMLockedState == RIL_E_SIMWRONG);

	TraceLeave(_D("CSSSOptions::SetupPhoneState"));
	return;
}

//======================================================================
// Main WndProc
//======================================================================
LRESULT	WINAPI OptionsTodayWndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->TodayOptionsWndProc(hDlg, uMsg, wParam, lParam);
}

LRESULT CSSSOptions::TodayOptionsWndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT	lrResult;

    TraceEnter(_D("CSSSOptions::TodayOptionsWndProc"));

	switch (uMsg)
	{
	case WM_INITDIALOG :
		{
			DWORD dwFlags = (DWORD)lParam;

		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_INITDIALOG, dwFlags = <%08X>"), dwFlags);

			// If options already showing, don't bother doing anything
			if ((OptionsFlags.OptionsBits.OptionsActive) && (m_hwOptionsWnd != hDlg))
			{
				TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: hDlg = <%08X>"), hDlg);
				TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: m_hwPropSheet = <%08X>"), m_hwPropSheet);

				if (m_hwPropSheet == NULL) m_hwPropSheet = m_hwOptionsWnd;
				TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: m_hwPropSheet = %08X"), m_hwPropSheet);

				SetForegroundWindow(m_hwPropSheet);

				TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: Ending options dialog because options already showing."));
				TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
				EndDialog(hDlg, 0);

				return 0;
			}

			if ((_tcslen(g_szBuildLabel) <= 0) || (OptionsFlags.OptionsBits.TapToRegister) || (OptionsFlags.OptionsBits.LicenseInvalid))
			{
				dwFlags |= SSS_FLAG_SHOW_ABOUT_ONLY;
			}

			m_hwOptionsWnd = hDlg;
			OptionsFlags.OptionsBits.OptionsActive = true;

			INITCOMMONCONTROLSEX iccControls;

			iccControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
			iccControls.dwICC = ICC_LISTVIEW_CLASSES;
			
			InitCommonControlsEx(&iccControls);

			m_lpsDefaultLanguage = LangGetDefaultLanguage();
			m_lpsCurrentLanguage = LangGetCurrentLanguage();
			
			if (SetupHelpFile(m_lpsCurrentLanguage) != 0)
			{
			    TraceDetail(_D("CSSSOptions::TodayOptionsWndProc, Help file set up for language %s"), m_lpsCurrentLanguage->ID);
			}
			else
			{
			    TraceDetail(_D("CSSSOptions::TodayOptionsWndProc, Help file for language %s, not found"), m_lpsCurrentLanguage->ID);
			}

			lrResult = (LRESULT)OnOptionsInitDialog(dwFlags);

		    TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), lrResult);
			return lrResult;
		}
	case WM_DESTROY :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_DESTROY"));
			OnOptionsDestroy();

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_CLOSE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_CLOSE"));
			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			EndDialog(hDlg, IDCANCEL);
			return 0;
		}
	case WM_QUIT :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_QUIT"));
			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			EndDialog(hDlg, IDCANCEL);
			return 0;
		}
	case WM_ERASEBKGND:
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_ERASEBKGND"));
			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)FALSE);
			return 0;
		}
	case WM_SYSCOMMAND :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_SYSCOMMAND"));
			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case 0x0086 : //WM_NCACTIVATE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_NCACTIVATE: wParam = %08X"), wParam);
			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)!wParam);
			return !wParam;
		}
	case WM_COMMAND :
		{
			UINT nID = LOWORD(wParam);

		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_COMMAND: ID = %08X"), nID);

			if (nID == IDOK)
			{
			    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_COMMAND: IDOK"));
				TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
				EndDialog(hDlg, IDOK);
				OnOptionsOK();
				return 0;
			}
			else if (nID == IDCANCEL)
			{
			    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_COMMAND: IDCANCEL"));
				TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
				EndDialog(hDlg, IDCANCEL);
				OnOptionsCancel();
				return 0;
			}

			OnOptionsCommand(nID, HIWORD(wParam), (HWND)lParam);
			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_SETTINGCHANGE"));
			OnOptionsSettingChange(wParam, (LPCTSTR)lParam);

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_CANCELMODE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_CANCELMODE"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_CTLCOLORDLG :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_CTLCOLORDLG"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_GETDLGCODE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_GETDLGCODE"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_ENABLE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_ENABLE"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_NULL :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_NULL"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_MOVE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_MOVE"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_STYLECHANGED :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_STYLECHANGED"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_QUERYNEWPALETTE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_QUERYNEWPALETTE"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_KILLFOCUS :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_KILLFOCUS"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_SETFOCUS :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_SETFOCUS"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_SETFONT :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_SETFONT"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_WINDOWPOSCHANGED :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_WINDOWPOSCHANGED"));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_NOTIFY, code = %08X, hwndFrom = %08X, idFrom = %08X, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);

			lrResult = OnOptionsNotify(uiFrom, lpSHNotify);
		    TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), lrResult);
			return lrResult;
		}
	case WM_PAINT :
		{
			PAINTSTRUCT ps;
			HDC hDC;

		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_PAINT"));
			hDC = BeginPaint(m_hwOptionsWnd, &ps);

			DrawTitle(m_hmInstance, m_hwOptionsWnd, hDC, m_hfTitleFont, m_szTitle, OptionsFlags.OptionsBits.AllowOptionsCancel);

			OnOptionsPaint(hDC);

			EndPaint(m_hwOptionsWnd, &ps);
			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_ACTIVATE :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_ACTIVATE"));
			OnOptionsActivate(LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_SIZE:
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: WM_SIZE"));
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hDlg, InWideMode() ? 
					MAKEINTRESOURCE(IDD_TODAY_CUSTOM_WIDE) :
					MAKEINTRESOURCE(IDD_TODAY_CUSTOM));

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::TodayOptionsWndProc: WM_WIV_REFRESH"));

			HWND hwCurrentPage = (HWND)SendMessage(m_hwPropSheet, PSM_GETCURRENTPAGEHWND, 0, 0);
			if (hwCurrentPage != NULL)
			{
				SendMessage(hwCurrentPage, uMsg, wParam, lParam);
			}

			TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), (DWORD)0);
			return 0;
		}
	default :
		{
		    TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: Message = <%08X>"), uMsg);
			lrResult = OnOptionsMessage(uMsg, wParam, lParam);
		    TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), lrResult);
			return lrResult;
		}
	}

	TraceInfo(_D("CSSSOptions::TodayOptionsWndProc: Calling DefWindowProc"));
	lrResult = DefWindowProc(hDlg, uMsg, wParam, lParam);
	TraceLeave(_D("CSSSOptions::TodayOptionsWndProc"), lrResult);
	return lrResult;
}

//======================================================================
// PropSheetProc - Function called when Property sheet created
//======================================================================
int CSSSOptions::PropSheetProc(HWND hDlg, UINT uMsg, LPARAM lParam)
{

	TraceEnter(_D("PropSheetProc"));
	TraceInfo(_D("PropSheetProc: uMsg = %08x"), uMsg);

	m_hwPropSheet = hDlg;

	switch (uMsg)
	{

	case PSCB_INITIALIZED:
		{
			TraceInfo(_D("PropSheetProc: uMsg = PSCB_INITIALIZED"));
			TCHAR classname[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

			// Get tab control.
			m_hwTabCtrl = GetDlgItem (hDlg, 0x3020);

			DWORD dwStyle = GetWindowLong (m_hwTabCtrl, GWL_STYLE);
			SetWindowLong (m_hwTabCtrl, GWL_STYLE, dwStyle | TCS_BOTTOM);

			// Find the hWnd for the spin control on the tab control
			HWND hWndFind = GetWindow(m_hwTabCtrl,GW_HWNDFIRST);

			while(hWndFind)
			{
				 GetClassName(hWndFind,classname,WIV_MAX_STRING);

				 if(_tcscmp(_T("SysTabControl32"),classname) == 0)
				 {
					hWndFind = GetWindow(hWndFind,GW_CHILD);
				 }
				 else if(lstrcmp(_T("msctls_updown32"),classname) == 0)
				 {
					m_hwSpinButton = hWndFind;
					break;
				 }
				 else
					hWndFind = GetWindow(hWndFind,GW_HWNDNEXT);
			};
			TraceLeave(_D("PropSheetProc"), (DWORD)0);
			return 0;
		}
	case PSCB_GETVERSION:
		{
			TraceInfo(_D("PropSheetProc: uMsg = PSCB_GETVERSION"));
			TraceLeave(_D("PropSheetProc"), (DWORD)COMCTL32_VERSION);
			return COMCTL32_VERSION;
		}
	case PSCB_GETLINKTEXT:
		{
			//	View or change <file:ctlpnl cplmain,20{Phone settings}>.
			TraceInfo(_D("PropSheetProc: uMsg = PSCB_GETLINKTEXT"));
			if(_tcslen(g_szCustomFileHeader) <= 0)
			{
				TraceLeave(_D("PropSheetProc"), (DWORD)0);
				return 0;
			}

			_snwprintf((LPTSTR)lParam, WIV_MAX_STRING, _T("%s <%s,20{%s}>."), SSS_TEXT_OPTIONS_VIEWORCHANGE, g_szLinkText, SSS_TEXT_MENU_PHONE_SETTINGS);
			TraceLeave(_D("PropSheetProc"), (DWORD)0);
			return 0;
		}
	case PSCB_PRECREATE :
		{
			TraceInfo(_D("PropSheetProc: uMsg = PSCB_PRECREATE"));
			break;
		}
	case PSCB_GETTITLE  :
		{
			TraceInfo(_D("PropSheetProc: uMsg = PSCB_GETTITLE"));
			break;
		}
	case WM_ERASEBKGND:
		{
			TraceInfo(_D("PropSheetProc: uMsg = WM_ERASEBKGND"));
//			HWND hOwner = GetWindow(hDlg, GW_OWNER);
//			if(IsWindow(hOwner))
//			{
//				EnableWindow(hOwner, TRUE); // enable main window
//				TraceLeave(_D("PropSheetProc"), (DWORD)FALSE);
//				return FALSE;
//			}
			TraceLeave(_D("PropSheetProc"), (DWORD)FALSE);
			return FALSE;
		}
	case WM_PAINT:
		{
			TraceInfo(_D("PropSheetProc: uMsg = WM_PAINT"));
			break;
		}
	case WM_LBUTTONDOWN:
		{
			TraceInfo(_D("PropSheetProc: uMsg = WM_LBUTTONDOWN"));
			break;
		}
	case WM_LBUTTONUP:
		{
			TraceInfo(_D("PropSheetProc: uMsg = WM_LBUTTONUP"));
			break;
		}
	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("PropSheetProc: uMsg = WM_WIV_REFRESH"));
			break;
		}
	}

	TraceLeave(_D("PropSheetProc"), (DWORD)1);
	return 1;
}

//======================================================================
//Property page dialog box procedures
//======================================================================

//======================================================================
// DlgProcAbout - About page dialog box procedure
//======================================================================
BOOL CSSSOptions::DlgProcAbout (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwndMain;
	LRESULT	lrResult;
	BOOL	bResult;
	TCHAR	szIMEI[MAXLENGTH_EQUIPINFO + 1] = WIV_DEFAULT_IMEI;

    TraceEnter(_D("CSSSOptions::DlgProcAbout"));

	switch (wMsg)
	{
	case WM_INITDIALOG :
		{
			// The generic parameter contains the
			// top-level window handle.
			hwndMain = (HWND)((LPPROPSHEETPAGE)lParam)->lParam;

			// Save the window handle in the window structure.
			SetWindowLong (hWnd, DWL_USER, (LONG)hwndMain);

			// Force SIP down
			SHSipPreference (hWnd, SIP_FORCEDOWN);

			SendDlgItemMessage (hWnd, IDC_ABOUT_BUTTON_SIM_INFORMATION, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_ABOUT_BUTTON_SIM_REGISTRATION, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);

			EnableWindow(ItemHandleFromID(hWnd, IDC_ABOUT_BUTTON_SIM_INFORMATION),
			((!OptionsFlags.OptionsBits.PhoneIsOff) & (!OptionsFlags.OptionsBits.SIMMissing) & (!OptionsFlags.OptionsBits.IncorrectSIM))); 

			bResult = OnInitDialog(hWnd, m_ahbrushAbout);

#ifdef WIV_ENCRYPT_BLD
			WiVGenEncryptedData();
#endif

			TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)bResult);

			return bResult;
		}
	case WM_DESTROY :
		{
			OnDestroy(hWnd, m_ahbrushAbout);
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}
	case WM_SYSCOMMAND :
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_SYSCOMMAND received for About, type = %d"), wParam);
			break;
		}
	case WM_COMMAND :
		{
			UINT uID = LOWORD (wParam);
			UINT uCommand = HIWORD (wParam);

			switch (uID)
			{

			case IDC_ABOUT_STATIC_URL:
				{
					switch (uCommand)
					{
					case STN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcAbout: STN_CLICKED received for About:IDC_ABOUT_STATIC_URL"));
							SHELLEXECUTEINFO	sei;
							memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
							sei.cbSize =  sizeof(SHELLEXECUTEINFO);
							sei.lpFile = g_szCompanyURL;
							ShellExecuteEx(&sei);
						}
					}
				    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
					return 0;
				}
			case IDC_ABOUT_BUTTON_SIM_INFORMATION:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							LRESULT lrResult;

							TraceInfo(_D("CSSSOptions::DlgProcAbout: BN_CLICKED received for About:IDC_ABOUT_BUTTON_SIM_INFORMATION"));
								
							TraceInfo(_D("CSSSOptions::DlgProcAbout: Calling m_pPhone->GetEquipmentInfo"));
							m_pPhone->GetEquipmentInfo(m_szIMEI, m_szMfg, m_szModel, m_szRevision);
							m_pPhone->GetSIMInfo(m_szSubscriber, m_szPBLocation, &m_dwPBTotal, &m_dwPBUsed);
							TraceInfo(_D("CSSSOptions::DlgProcAbout: Back from m_pPhone->GetEquipmentInfo"));

							TraceDetail(_D("CSSSOptions::DlgProcAbout: Manufacturer = <%s>"), m_szMfg);
							TraceDetail(_D("CSSSOptions::DlgProcAbout: Model = <%s>"), m_szModel);
							TraceDetail(_D("CSSSOptions::DlgProcAbout: Revision = <%s>"), m_szRevision);
							TraceDetail(_D("CSSSOptions::DlgProcAbout: IMEI = <%s>"), m_szIMEI);
							TraceDetail(_D("CSSSOptions::DlgProcAbout: Subscriber = <%s>"), m_szSubscriber);

							lrResult = (LPARAM)DialogBox(m_hmInstance, MAKEINTRESOURCE(IDD_INFORMATION_DIALOG), hWnd, (DLGPROC)OptionsDlgProcInformation);
												
							TraceInfo(_D("CSSSOptions::DlgProcAbout: DialogBox:InfoWindowDlgProc returned <%08x>"), lrResult);
						    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
							return 0;
						}
					}
				    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
					return 0;
				}
			case IDC_ABOUT_BUTTON_SIM_REGISTRATION:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							LRESULT lrResult;

							TraceDetail(_D("CSSSOptions::DlgProcAbout: BN_CLICKED received for About:IDC_ABOUT_BUTTON_SIM_REGISTRATION"));

							lrResult = (LPARAM)DialogBox(m_hmInstance, MAKEINTRESOURCE(IDD_REGISTRATION_DIALOG), hWnd, (DLGPROC)OptionsDlgProcRegistration);
							TraceDetail(_D("CSSSOptions::DlgProcAbout: DialogBox:RegWindowDlgProc returned <%08x>"), lrResult);
							if (lrResult == IDYES)
							{
								OptionsFlags.OptionsBits.DontPaintLicense = true;
								RefreshWindow(hWnd);
							}else
							{
								OptionsFlags.OptionsBits.DontPaintLicense = false;
							}
							
						    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
							return 0;
						}
					}
				    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
					return 0;
				}
				case IDC_INFORMATION_STATIC_DEVICE_DATA:
				{
					TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_COMMAND %d received for About:IDC_INFORMATION_STATIC_DEVICE_DATA"), uCommand);
					switch (uCommand)
					{
					case WM_LBUTTONDOWN:
						{
							TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_LBUTTONDOWN received for About:IDC_INFORMATION_STATIC_DEVICE_DATA"));
							break;
						}
					}
				    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
					return 0;
				}
			}
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}
	case WM_ACTIVATE :
		{
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}
	case PSM_QUERYSIBLINGS :
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: PSM_QUERYSIBLINGS received for About"));
			TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_NOTIFY, code = %08X, hwndFrom = %08X, idFrom = %08X, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);

			if (uiCode == PSN_SETACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAbout: PSN_SETACTIVE received for About"));

//				if (!m_blAboutOnly)
//				{
//					AutoScroll(m_hwOptionsWnd, m_hwTabCtrl);
//				}

				TraceLeave(_D("CSSSOptions::DlgProcAbout"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, 0);
				return TRUE;
			}
			else if (uiCode == PSN_KILLACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAbout: PSN_KILLACTIVE received for About"));

				TraceLeave(_D("CSSSOptions::DlgProcAbout"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_APPLY)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAbout: PSN_APPLY received for About"));
			    TraceLeave(_D("CSSSOptions::DlgProcAbout"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
				return TRUE;
			}
			else if (uiCode == PSN_RESET)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAbout: PSN_RESET received for About"));
				TraceLeave(_D("CSSSOptions::DlgProcAbout"), TRUE);
				return TRUE;
			}
			else if (uiCode == PSN_QUERYCANCEL)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAbout: PSN_QUERYCANCEL received for About"));
				TraceLeave(_D("CSSSOptions::DlgProcAbout"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_HELP)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAbout: PSN_HELP received for About"));

				DoShowHelp(g_szHelpTagAboutSettings);

				break; 
			}

			TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)FALSE);
			return FALSE;  // Return false to force default processing.
		}
	case WM_PAINT :
		{
			PAINTSTRUCT	ps;
			HDC			hDC;
			hDC = BeginPaint(hWnd, &ps);

			OnPaint(hWnd, hDC);

			OnAboutPaint(hWnd, hDC);

			EndPaint(hWnd, &ps);

		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}

	case WM_ERASEBKGND:
		{
			bResult = OnEraseBackground(hWnd, (HDC)wParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)bResult);

			return bResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_CTLCOLORDLG received for About"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAbout, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_CTLCOLORBTN received for About"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAbout, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_CTLCOLORSTATIC received for About"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAbout, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)bResult);
			return bResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_CTLCOLOREDIT received for About"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAbout, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)bResult);
			return bResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_CTLCOLORLISTBOX received for About"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAbout, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)bResult);
			return bResult;
        }

	case WM_MOUSEMOVE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_MOUSEMOVE received for About"));

			OnMouseMove(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_LBUTTONDOWN received for About"));

			OnLButtonDown(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONUP:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_LBUTTONUP received for About"));

			if(_tcslen(g_szCompanySupport) <= 0)
			{
				OnLButtonUp(hWnd, wParam, lParam);
				break;
			}

			if (PointInControl(hWnd, IDC_ABOUT_STATIC_URL, lParam))
			{
				SHELLEXECUTEINFO	sei;

				TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_LBUTTONUP is in URL Static, launching IE for WiViT URL"));

				memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
				sei.cbSize =  sizeof(SHELLEXECUTEINFO);
				sei.lpFile = g_szCompanyURL;
				ShellExecuteEx(&sei);

				TraceInfo(_D("Back from ShellExecuteEx for mail program for IE for WiViT URL"));

			}else if(PointInControl(hWnd, IDC_ABOUT_STATIC_SUPPORT, lParam))
			{
				SHELLEXECUTEINFO	sei;
				TCHAR				szSupport[WIV_MAX_NAME + 1] = WIV_EMPTY_STRING;
				
				TraceInfo(_D("CSSSOptions::DlgProcAbout: WM_LBUTTONUP is in Support Static, launching mail program for support"));

				_tcsncpy(szSupport, g_szMailTo, WIV_MAX_NAME);
				_tcsncat(szSupport, g_szCompanySupport, WIV_MAX_NAME - _tcslen(szSupport));

				memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
				sei.cbSize =  sizeof(SHELLEXECUTEINFO);
				sei.lpFile = szSupport;
				ShellExecuteEx(&sei);

				TraceInfo(_D("CSSSOptions::DlgProcAbout: Back from ShellExecuteEx for mail program for support"));
			}else
			{
				OnLButtonUp(hWnd, wParam, lParam);
			}

			break;
		}
	case WM_SIZE:
		{
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_ABOUT_WIDE) :
					MAKEINTRESOURCE(IDD_ABOUT));
			TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}

	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcAbout: WM_WIV_REFRESH received for About"));
			if (m_hwInfoDialog != NULL)
			{
				SendMessage(m_hwInfoDialog, wMsg, wParam, lParam);
			}
			TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
			return 0;
		}
//TODO: check if this should be here
	    TraceLeave(_D("CSSSOptions::DlgProcAbout"), (DWORD)0);
		return 0;
	}

	lrResult = DefWindowProc(hWnd, wMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcAbout"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// DlgProcActions - Actions options page dialog box procedure
//======================================================================
BOOL CSSSOptions::DlgProcActions (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwndMain;
	LRESULT	lrResult;
	BOOL	bResult;

	TraceEnter(_D("CSSSOptions::DlgProcActions"));

	switch (wMsg)
	{
	case WM_INITDIALOG :
		{
			// The generic parameter contains the
			// top-level window handle.
			hwndMain = (HWND)((LPPROPSHEETPAGE)lParam)->lParam;

			// Save the window handle in the window structure.
			SetWindowLong (hWnd, DWL_USER, (LONG)hwndMain);

			// Set up the action lists
			InitializeActionLists(hWnd);
			
			// Set current selection for each action list.
			SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAP_ACTION, CB_SETCURSEL, m_dwTapAction, 0);
			SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAH_ACTION, CB_SETCURSEL, m_dwTAHAction, 0);
			SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION, CB_SETCURSEL, m_dwTodayIconTapAction, 0);
			SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION, CB_SETCURSEL, m_dwTodayIconTAHAction, 0);
			if (m_dwButtonAction == g_dwTodayActionToggleRadio)
			{
				SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, CB_SETCURSEL, 1, 0);
			}
			else
			{
				SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, CB_SETCURSEL, 0, 0);
			}

			m_dwCurTapAction = m_dwTapAction;
			m_dwCurTAHAction = m_dwTAHAction;
			m_dwCurTodayIconTapAction = m_dwTodayIconTapAction;
			m_dwCurTodayIconTAHAction = m_dwTodayIconTAHAction;
			m_dwCurButtonAction = m_dwButtonAction;

			bResult = OnInitDialog(hWnd, m_ahbrushActions);

		    TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)bResult);

			return bResult;
		}
	case WM_DESTROY :
		{
			OnDestroy(hWnd, m_ahbrushActions);
			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			UINT uID = LOWORD (wParam);
			UINT uCommand = HIWORD (wParam);

			switch (uID)
			{
			case IDC_ACTIONS_COMBO_TAP_ACTION:
				{
					switch (uCommand)
					{
					case CBN_SELCHANGE:
						{
							TraceInfo(_D("CSSSOptions::DlgProcActions: CBN_SELCHANGE received for Actions:IDC_ACTIONS_COMBO_TAP_ACTION"));
							m_dwCurTapAction = SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAP_ACTION, CB_GETCURSEL, 0, 0);
							
							if (m_dwCurTapAction == CB_ERR)
								m_dwCurTapAction = 0;
						}
					}
					break;
				}
			case IDC_ACTIONS_COMBO_TAH_ACTION:
				{
					switch (uCommand)
					{
					case CBN_SELCHANGE:
						{
							TraceInfo(_D("CSSSOptions::DlgProcActions: CBN_SELCHANGE received for Actions:IDC_ACTIONS_COMBO_TAH_ACTION"));
							m_dwCurTAHAction = SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAH_ACTION, CB_GETCURSEL, 0, 0);
							
							if (m_dwCurTAHAction == CB_ERR)
								m_dwCurTAHAction = 0;
						}
					}
					break;
				}
			case IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION:
				{
					switch (uCommand)
					{
					case CBN_SELCHANGE:
						{
							TraceInfo(_D("CSSSOptions::DlgProcActions: CBN_SELCHANGE received for Actions:IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION"));
							m_dwCurTodayIconTapAction = SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION, CB_GETCURSEL, 0, 0);
							
							if (m_dwCurTodayIconTapAction == CB_ERR)
								m_dwCurTodayIconTapAction = 0;
						}
					}
					break;
				}
			case IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION:
				{
					switch (uCommand)
					{
					case CBN_SELCHANGE:
						{
							TraceInfo(_D("CSSSOptions::DlgProcActions: CBN_SELCHANGE received for Actions:IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION"));
							m_dwCurTodayIconTAHAction = SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION, CB_GETCURSEL, 0, 0);
							
							if (m_dwCurTodayIconTAHAction == CB_ERR)
								m_dwCurTodayIconTAHAction = 0;
						}
					}
					break;
				}
			case IDC_ACTIONS_COMBO_BUTTON_ACTION:
				{
					switch (uCommand)
					{
					case CBN_SELCHANGE:
						{
							TraceInfo(_D("CSSSOptions::DlgProcActions: CBN_SELCHANGE received for Actions:IDC_ACTIONS_COMBO_BUTTON_ACTION"));
							m_dwCurButtonAction = SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, CB_GETCURSEL, 0, 0);
							
							if (m_dwCurButtonAction == 1)
							{
								m_dwCurButtonAction = g_dwTodayActionToggleRadio;
							}
							else
							{
								m_dwCurButtonAction = g_dwTodayActionSwitchSIM;
							}
						}
					}
					break;
				}
			}
			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)TRUE);
			return TRUE;
		}
	case WM_ACTIVATE :
		{
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
		}
	case WM_MOUSEMOVE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_MOUSEMOVE received for Actions"));

			OnMouseMove(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_LBUTTONDOWN received for Actions"));

			OnLButtonDown(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONUP:
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_LBUTTONUP received for Actions"));

			OnLButtonUp(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
		}
	case PSM_QUERYSIBLINGS :
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: PSM_QUERYSIBLINGS received for Actions"));
			NewSettings.dwTapAction = m_dwCurTapAction;
			NewSettings.dwTAHAction = m_dwCurTAHAction;
			NewSettings.dwTodayIconTapAction = m_dwCurTodayIconTapAction;
			NewSettings.dwTodayIconTAHAction = m_dwCurTodayIconTAHAction;
			NewSettings.dwButtonAction = m_dwCurButtonAction;
			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_NOTIFY, code = %08X, hwndFrom = %08X, idFrom = %08X, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);

			if (uiCode == PSN_SETACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcActions: PSN_SETACTIVE received for Actions"));
				TraceDetail(_D("CSSSOptions::DlgProcActions: OptionsFlags.OptionsBits.TapToRegister = %d"), OptionsFlags.OptionsBits.TapToRegister);

				if ((_tcslen(g_szBuildLabel) <= 0) || (OptionsFlags.OptionsBits.TapToRegister) || (OptionsFlags.OptionsBits.LicenseInvalid))
				{
					TraceLeave(_D("CSSSOptions::DlgProcActions"), TRUE);
					SetWindowLong(hWnd, DWL_MSGRESULT, InWideMode() ? IDD_ABOUT_WIDE : IDD_ABOUT);
					return TRUE;
				}

				// Set up the action lists
				InitializeActionLists(hWnd);
				
				// Set current selection for each action list.
				SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAP_ACTION, CB_SETCURSEL, m_dwCurTapAction, 0);
				SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TAH_ACTION, CB_SETCURSEL, m_dwCurTAHAction, 0);
				SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAP_ACTION, CB_SETCURSEL, m_dwCurTodayIconTapAction, 0);
				SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_TODAY_ICON_TAH_ACTION, CB_SETCURSEL, m_dwCurTodayIconTAHAction, 0);
				if (m_dwCurButtonAction == g_dwTodayActionToggleRadio)
				{
					SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, CB_SETCURSEL, 1, 0);
				}
				else
				{
					SendDlgItemMessage (hWnd, IDC_ACTIONS_COMBO_BUTTON_ACTION, CB_SETCURSEL, 0, 0);
				}
				
				AutoScroll(m_hwOptionsWnd, m_hwTabCtrl);

				TraceLeave(_D("CSSSOptions::DlgProcActions"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, 0);
				return TRUE;
			}
			else if (uiCode == PSN_KILLACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcActions: PSN_KILLACTIVE received for Actions"));
				TraceLeave(_D("CSSSOptions::DlgProcActions"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_APPLY)
			{
				TraceInfo(_D("CSSSOptions::DlgProcActions: PSN_APPLY received for Actions"));

				NewSettings.dwTapAction = m_dwCurTapAction;
				NewSettings.dwTAHAction = m_dwCurTAHAction;
				NewSettings.dwTodayIconTapAction = m_dwCurTodayIconTapAction;
				NewSettings.dwTodayIconTAHAction = m_dwCurTodayIconTAHAction;
				NewSettings.dwButtonAction = m_dwCurButtonAction;

				// Save current selection for each action list.
				m_dwTapAction = m_dwCurTapAction;
				m_dwTAHAction = m_dwCurTAHAction;
				m_dwTodayIconTapAction = m_dwCurTodayIconTapAction;
				m_dwTodayIconTAHAction = m_dwCurTodayIconTAHAction;
				m_dwButtonAction = m_dwCurButtonAction;

				TraceLeave(_D("CSSSOptions::DlgProcActions"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
				return TRUE;
			}
			else if (uiCode == PSN_RESET)
			{
				TraceInfo(_D("CSSSOptions::DlgProcActions: PSN_RESET received for Actions"));
				TraceLeave(_D("CSSSOptions::DlgProcActions"), TRUE);
				return TRUE;
			}
			else if (uiCode == PSN_QUERYCANCEL)
			{
				TraceInfo(_D("CSSSOptions::DlgProcActions: PSN_QUERYCANCEL received for Actions"));
				TraceLeave(_D("CSSSOptions::DlgProcActions"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_HELP)
			{
				TraceInfo(_D("CSSSOptions::DlgProcActions: PSN_HELP received for Actions"));

				DoShowHelp(g_szHelpTagActionsSettings);

				break;
			}

			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)FALSE);
			return FALSE;  // Return false to force default processing.
		}
	case WM_PAINT :
		{
			PAINTSTRUCT ps;
			HDC hDC;

			hDC = BeginPaint(hWnd, &ps);

			OnPaint(hWnd, hDC);

			if (IsSecondEdition() && InWideMode())
			{
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_TAP_ACTIONS_LIST, SSS_TEXT_ACTIONS_TAP, DT_LEFT, FW_NORMAL, WIV_FONT_MEDIUM);
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_TAH_ACTIONS_LIST, SSS_TEXT_ACTIONS_TAH, DT_LEFT, FW_NORMAL, WIV_FONT_MEDIUM);
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_TODAY_ICON_TAP_ACTIONS_LIST, SSS_TEXT_ACTIONS_TODAY_ICON_TAP, DT_LEFT, FW_NORMAL, WIV_FONT_MEDIUM);
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_TODAY_ICON_TAH_ACTIONS_LIST, SSS_TEXT_ACTIONS_TODAY_ICON_TAH, DT_LEFT, FW_NORMAL, WIV_FONT_MEDIUM);
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_BUTTON_ACTIONS_LIST, SSS_TEXT_ACTIONS_BUTTON, DT_LEFT, FW_NORMAL, WIV_FONT_MEDIUM);
			}
			else
			{
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_TAP_ACTIONS_LIST, SSS_TEXT_ACTIONS_TAP, DT_LEFT);
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_TAH_ACTIONS_LIST, SSS_TEXT_ACTIONS_TAH, DT_LEFT);
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_TODAY_ICON_TAP_ACTIONS_LIST, SSS_TEXT_ACTIONS_TODAY_ICON_TAP, DT_LEFT);
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_TODAY_ICON_TAH_ACTIONS_LIST, SSS_TEXT_ACTIONS_TODAY_ICON_TAH, DT_LEFT);
				DrawStaticText(hWnd, IDC_ACTIONS_STATIC_BUTTON_ACTIONS_LIST, SSS_TEXT_ACTIONS_BUTTON, DT_LEFT);
			}

			EndPaint(hWnd, &ps);
			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
			//break;
		}

	case WM_ERASEBKGND:
		{
			bResult = OnEraseBackground(hWnd, (HDC)wParam);
		    TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_CTLCOLORDLG received for Actions"));
			bResult = (BOOL)OnCtlColor(m_ahbrushActions, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_CTLCOLORBTN received for Actions"));
			bResult = (BOOL)OnCtlColor(m_ahbrushActions, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_CTLCOLORSTATIC received for Actions"));
			bResult = (BOOL)OnCtlColor(m_ahbrushActions, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)bResult);
			return bResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_CTLCOLOREDIT received for Actions"));
			bResult = (BOOL)OnCtlColor(m_ahbrushActions, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)bResult);
			return bResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcActions: WM_CTLCOLORLISTBOX received for Actions"));
			bResult = (BOOL)OnCtlColor(m_ahbrushActions, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)bResult);
			return bResult;
        }

	case WM_SIZE:
		{
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_OPTIONS_ACTIONS_WIDE) :
					MAKEINTRESOURCE(IDD_OPTIONS_ACTIONS));
		}
	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcActions: WM_WIV_REFRESH received for Actions"));
			TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
			return 0;
		}
		
		TraceLeave(_D("CSSSOptions::DlgProcActions"), (DWORD)0);
		return 0;
	}

	lrResult = DefWindowProc(hWnd, wMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcActions"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// DlgProcAppearance - Appearance options page dialog box procedure
//======================================================================
BOOL CSSSOptions::DlgProcAppearance (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwndMain;
	LRESULT	lrResult;
	BOOL	bResult;

    TraceEnter(_D("CSSSOptions::DlgProcAppearance"));

	switch (wMsg)
	{
	case WM_INITDIALOG :
		{
			// The generic parameter contains the
			// top-level window handle.
			hwndMain = (HWND)((LPPROPSHEETPAGE)lParam)->lParam;
			// Save the window handle in the window structure.
			SetWindowLong (hWnd, DWL_USER, (LONG)hwndMain);

			// Force SIP down
			SHSipPreference (hWnd, SIP_FORCEDOWN);

			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_INITDIALOG in Appearance, m_blLine1BoldFont=%08x,m_blLine2BoldFont=%08x"),
								m_blLine1BoldFont, m_blLine2BoldFont);
			
			SetCheckButton(hWnd, IDC_APPEARANCE_CHECK_LINE_1_BOLD, m_blLine1BoldFont);
			SetCheckButton(hWnd, IDC_APPEARANCE_CHECK_LINE_2_BOLD, m_blLine2BoldFont);

			// Setup the icon sets list
			InitialiseIconSetList(hWnd);
			
			// Set current selection for icon set list.
			SendDlgItemMessage (hWnd, IDC_APPEARANCE_LIST_ICON_SETS, LB_SETCURSEL, m_dwIconSet, 0);
			
			m_dwCurIconSet = m_dwIconSet;
			
			bResult = OnInitDialog(hWnd, m_ahbrushAppearance);

		    TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)bResult);

			return bResult;
		}
	case WM_DESTROY :
		{
			OnDestroy(hWnd, m_ahbrushAppearance);
			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			UINT uID = LOWORD (wParam);
			UINT uCommand = HIWORD (wParam);

			switch (uID)
			{
			case IDC_APPEARANCE_CHECK_LINE_1_BOLD:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcAppearance: BN_CLICKED received for Appearance:IDC_APPEARANCE_CHECK_LINE_1_BOLD"));
						}
					}
					break;
				}
			case IDC_APPEARANCE_CHECK_LINE_2_BOLD:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcAppearance: BN_CLICKED received for Appearance:IDC_APPEARANCE_CHECK_LINE_2_BOLD"));
						}
					}
					break;
				}
			case IDC_APPEARANCE_LIST_ICON_SETS:
				{
					switch (uCommand)
					{
					case LBN_SELCHANGE:
						{
							HWND	hIcons;

							m_dwCurIconSet = SendDlgItemMessage (hWnd, IDC_APPEARANCE_LIST_ICON_SETS, LB_GETCURSEL, 0, 0);
							TraceInfo(_D("CSSSOptions::DlgProcAppearance: LBN_SELCHANGE received for Appearance:IDC_APPEARANCE_LIST_ICON_SETS, Sel = %d"), m_dwCurIconSet);
							
							if (m_dwCurIconSet == LB_ERR)
								m_dwCurIconSet = 0;

							hIcons = ItemHandleFromID(hWnd, IDC_APPEARANCE_STATIC_ICONS);
							DrawIconSet(hWnd, NULL, hIcons);
						
						}
					}
					break;
				}
			}

			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)TRUE);
			return TRUE;
		}
	case WM_ACTIVATE :
		{
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}
	case WM_MOUSEMOVE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_MOUSEMOVE received for Appearance"));

			OnMouseMove(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_LBUTTONDOWN received for Appearance"));

			OnLButtonDown(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONUP:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_LBUTTONUP received for Appearance"));

			OnLButtonUp(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}
	case PSM_QUERYSIBLINGS :
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: PSM_QUERYSIBLINGS received for Appearance"));
			NewSettings.blLine1BoldFont = IsButtonChecked(hWnd, IDC_APPEARANCE_CHECK_LINE_1_BOLD);
			NewSettings.blLine2BoldFont = IsButtonChecked(hWnd, IDC_APPEARANCE_CHECK_LINE_2_BOLD);
			NewSettings.dwIconSet = m_dwCurIconSet;
			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_NOTIFY, code = %08X, hwndFrom = %08X, idFrom = %08X, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);

			if (uiCode == PSN_SETACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAppearance: PSN_SETACTIVE received for Appearance"));

				if ((_tcslen(g_szCompanyIdentity) <= 0) || (OptionsFlags.OptionsBits.TapToRegister) || (OptionsFlags.OptionsBits.LicenseInvalid))
				{
					TraceLeave(_D("CSSSOptions::DlgProcAppearance"), TRUE);
					SetWindowLong(hWnd, DWL_MSGRESULT, InWideMode() ? IDD_ABOUT_WIDE : IDD_ABOUT);
					return TRUE;
				}

				// Setup the icon sets list
				InitialiseIconSetList(hWnd);
				
				// Set current selection for icon set list.
				SendDlgItemMessage (hWnd, IDC_APPEARANCE_LIST_ICON_SETS, LB_SETCURSEL, m_dwCurIconSet, 0);
				
				AutoScroll(m_hwOptionsWnd, m_hwTabCtrl);

				TraceLeave(_D("CSSSOptions::DlgProcAppearance"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, 0);
				return TRUE;
			}
			else if (uiCode == PSN_KILLACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAppearance: PSN_KILLACTIVE received for Appearance"));
				TraceLeave(_D("CSSSOptions::DlgProcAppearance"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_APPLY)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAppearance: PSN_APPLY received for Appearance"));

				NewSettings.blLine1BoldFont = IsButtonChecked(hWnd, IDC_APPEARANCE_CHECK_LINE_1_BOLD);
				NewSettings.blLine2BoldFont = IsButtonChecked(hWnd, IDC_APPEARANCE_CHECK_LINE_2_BOLD);
				NewSettings.dwIconSet = m_dwCurIconSet;

				m_blLine1BoldFont = IsButtonChecked(hWnd, IDC_APPEARANCE_CHECK_LINE_1_BOLD);
				m_blLine2BoldFont = IsButtonChecked(hWnd, IDC_APPEARANCE_CHECK_LINE_2_BOLD);

				m_dwIconSet = m_dwCurIconSet;

				TraceInfo(_D("CSSSOptions::DlgProcAppearance: PSN_APPLY in Appearance, m_blLine1BoldFont=%08x,m_blLine2BoldFont=%08x"),
								m_blLine1BoldFont, m_blLine2BoldFont);

				TraceLeave(_D("CSSSOptions::DlgProcAppearance"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
				return TRUE;
			}
			else if (uiCode == PSN_RESET)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAppearance: PSN_RESET received for Appearance"));
				TraceLeave(_D("CSSSOptions::DlgProcAppearance"), TRUE);
				return TRUE;
			}
			else if (uiCode == PSN_QUERYCANCEL)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAppearance: PSN_QUERYCANCEL received for Appearance"));
				TraceLeave(_D("CSSSOptions::DlgProcAppearance"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_HELP)
			{
				TraceInfo(_D("CSSSOptions::DlgProcAppearance: PSN_HELP received for Appearance"));

				DoShowHelp(g_szHelpTagAppearanceSettings);

				break;
			}

			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)FALSE);
			return FALSE;  // Return false to force default processing.
		}
	case WM_PAINT :
		{
			PAINTSTRUCT ps;
			HDC			hDC;
			HWND		hIcons;

			hDC = BeginPaint(hWnd, &ps);

			OnPaint(hWnd, hDC);

            SendDlgItemMessage (hWnd, IDC_APPEARANCE_CHECK_LINE_1_BOLD, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_APPEARANCE_LINE_1_BOLD);

            SendDlgItemMessage (hWnd, IDC_APPEARANCE_CHECK_LINE_2_BOLD, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_APPEARANCE_LINE_2_BOLD);

			DrawStaticText(hWnd, IDC_APPEARANCE_STATIC_ICON_SETS_LIST, SSS_TEXT_APPEARANCE_ICON_SETS, DT_LEFT);

			hIcons = ItemHandleFromID(hWnd, IDC_APPEARANCE_STATIC_ICONS);
			DrawIconSet(NULL, hDC, hIcons);

			EndPaint(hWnd, &ps);
			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}

	case WM_ERASEBKGND:
		{
			bResult = OnEraseBackground(hWnd, (HDC)wParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)bResult);

			return bResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_CTLCOLORDLG received for Appearance"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAppearance, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_CTLCOLORBTN received for Appearance"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAppearance, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_CTLCOLORSTATIC received for Appearance"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAppearance, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)bResult);
			return bResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_CTLCOLOREDIT received for Appearance"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAppearance, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)bResult);
			return bResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcAppearance: WM_CTLCOLORLISTBOX received for Appearance"));
			bResult = (BOOL)OnCtlColor(m_ahbrushAppearance, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)bResult);
			return bResult;
        }

	case WM_SIZE:
		{
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_OPTIONS_APPEARANCE_WIDE) :
					MAKEINTRESOURCE(IDD_OPTIONS_APPEARANCE));
		}

	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcAppearance: WM_WIV_REFRESH received for Appearance"));
			TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
			return 0;
		}

		TraceLeave(_D("CSSSOptions::DlgProcAppearance"), (DWORD)0);
		return 0;
	}

	lrResult = DefWindowProc(hWnd, wMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcAppearance"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// DlgProcDisplay - Display options page dialog box procedure
//======================================================================
BOOL CSSSOptions::DlgProcDisplay (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwndMain;
	LRESULT	lrResult;
	BOOL	bResult;

	TraceEnter(_D("CSSSOptions::DlgProcDisplay"));

	TraceInfo(_D("CSSSOptions::DlgProcDisplay: wMsg = <%08X>, wParam = <%08X>, lParam = <%08X>"),
						wMsg, wParam, lParam);

	switch (wMsg)
	{
	case WM_INITDIALOG :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_INITDIALOG received for Display"));

			// The generic parameter contains the
			// top-level window handle.
			hwndMain = (HWND)((LPPROPSHEETPAGE)lParam)->lParam;
			// Save the window handle in the window structure.
			SetWindowLong (hWnd, DWL_USER, (LONG)hwndMain);

			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_INITDIALOG, m_blShowPhoneNumber=%08x, m_blShowTSP=%08x,m_blSingleLineDisplay=%08x"),
								m_blShowPhoneNumber, m_blShowTSP, m_blSingleLineDisplay);
			SetCheckButton(hWnd, IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER, m_blShowPhoneNumber);
			SetCheckButton(hWnd, IDC_DISPLAY_CHECK_SHOW_TSP, m_blShowTSP);
			if (!m_blShowTSP)
			{
				SetCheckButton(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY, true);
				EnableWindow(ItemHandleFromID(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY), false ); 
			}
			else
			{
				SetCheckButton(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY, m_blSingleLineDisplay);
				EnableWindow(ItemHandleFromID(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY), true ); 
			}

			bResult = OnInitDialog(hWnd, m_ahbrushDisplay);

			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)bResult);

			return bResult;
		}
	case WM_DESTROY :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_DESTROY received for Display"));
			OnDestroy(hWnd, m_ahbrushDisplay);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_COMMAND received for Display"));
			UINT uID = LOWORD (wParam);
			UINT uCommand = HIWORD (wParam);

			switch (uID)
			{
			case IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcDisplay: BN_CLICKED received for Display:IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER"));
						}
					}
					break;
				}
			case IDC_DISPLAY_CHECK_SHOW_TSP:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcDisplay: BN_CLICKED received for Display:IDC_DISPLAY_CHECK_SHOW_TSP"));
							if (!IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SHOW_TSP))
							{
								SetCheckButton(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY, true);
								EnableWindow(ItemHandleFromID(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY), false ); 
							}
							else
							{
								SetCheckButton(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY, m_blSingleLineDisplay);
								EnableWindow(ItemHandleFromID(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY), true ); 
							}
						}
					}
					break;
				}
			case IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcDisplay: BN_CLICKED received for Display:IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY"));
						}
					}
					break;
				}
			}

			break;
		}
	case WM_MOUSEMOVE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_MOUSEMOVE received for Display"));

			OnMouseMove(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_LBUTTONDOWN received for Display"));

			OnLButtonDown(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONUP:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_LBUTTONUP received for Display"));

			OnLButtonUp(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case WM_ACTIVATE :
		{
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case PSM_QUERYSIBLINGS :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSM_QUERYSIBLINGS received for Display"));
			NewSettings.blShowPhoneNumber = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER);
			NewSettings.blShowTSP = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SHOW_TSP);
			NewSettings.blSingleLineDisplay = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_NOTIFY received for Display"));

			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_NOTIFY, code = %08X, hwndFrom = %08X, idFrom = %08X, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);

			if (uiCode == PSN_SETACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_SETACTIVE received for Display"));

				if ((_tcslen(g_szDevelopedByLabel) <= 0) || (OptionsFlags.OptionsBits.TapToRegister) || (OptionsFlags.OptionsBits.LicenseInvalid))
				{
					TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
					SetWindowLong(hWnd, DWL_MSGRESULT, InWideMode() ? IDD_ABOUT_WIDE : IDD_ABOUT);
					return TRUE;
				}

				AutoScroll(m_hwOptionsWnd, m_hwTabCtrl);

				TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, 0);
				return TRUE;
			}
			else if (uiCode == PSN_KILLACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_KILLACTIVE received for Display"));
				TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_APPLY)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_APPLY received for Display"));

				TraceInfo(_D("CSSSOptions::DlgProcDisplay: About to save settings to struct, &NewSettings = %08X"), &NewSettings);
				NewSettings.blShowPhoneNumber = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER);
				NewSettings.blShowTSP = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SHOW_TSP);
				NewSettings.blSingleLineDisplay = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY);

				TraceInfo(_D("CSSSOptions::DlgProcDisplay: Settings saved to struct"));

				m_blShowPhoneNumber = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER);
				m_blShowTSP = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SHOW_TSP);
				m_blSingleLineDisplay = IsButtonChecked(hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY);

				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_APPLY in Display, m_blShowPhoneNumber=%08x, m_blShowTSP=%08x,m_blSingleLineDisplay=%08x"),
								m_blShowPhoneNumber, m_blShowTSP, m_blSingleLineDisplay);

				TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
				return TRUE;
			}
			else if (uiCode == PSN_RESET)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_RESET received for Display"));
				TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
				return TRUE;
			}
			else if (uiCode == PSN_QUERYCANCEL)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_QUERYCANCEL received for Display"));
				TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_HELP)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_HELP received for Display"));

				DoShowHelp(g_szHelpTagDisplaySettings);

				break;
			}

			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)FALSE);
			return FALSE;  // Return false to force default processing.
		}
	case WM_ERASEBKGND:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_ERASEBKGND received for Display"));
			lrResult = OnEraseBackground(hWnd, (HDC)wParam);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), lrResult);
			return lrResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_CTLCOLORDLG received for Display"));
			lrResult = OnCtlColor(m_ahbrushDisplay, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), lrResult);
			return (BOOL)lrResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_CTLCOLORBTN received for Display"));
			lrResult = OnCtlColor(m_ahbrushDisplay, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), lrResult);
			return (BOOL)lrResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_CTLCOLORSTATIC received for Display"));
			lrResult = OnCtlColor(m_ahbrushDisplay, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), lrResult);
			return (BOOL)lrResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_CTLCOLOREDIT received for Display"));
			lrResult = OnCtlColor(m_ahbrushDisplay, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), lrResult);
			return (BOOL)lrResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_CTLCOLORLISTBOX received for Display"));
			lrResult = OnCtlColor(m_ahbrushDisplay, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), lrResult);
			return (BOOL)lrResult;
        }

	case WM_PAINT :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_PAINT received for Display"));
			PAINTSTRUCT ps;
			HDC hDC;

			hDC = BeginPaint(hWnd, &ps);

			OnPaint(hWnd, hDC);

			EndPaint(hWnd, &ps);

            SendDlgItemMessage (hWnd, IDC_DISPLAY_CHECK_SHOW_PHONE_NUMBER, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_DISPLAY_SHOW_PHONE_NUMBER);
            SendDlgItemMessage (hWnd, IDC_DISPLAY_CHECK_SHOW_TSP, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_DISPLAY_SHOW_TSP);
            SendDlgItemMessage (hWnd, IDC_DISPLAY_CHECK_SINGLE_LINE_DISPLAY, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_DISPLAY_SINGLE_LINE);

			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case WM_SIZE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDisplay: WM_SIZE received for Display"));
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_OPTIONS_DISPLAY_WIDE) :
					MAKEINTRESOURCE(IDD_OPTIONS_DISPLAY));
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}

	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcDisplay: WM_WIV_REFRESH received for Display"));
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)0);
			return 0;
		}
	case WM_CLOSE :
		{
			TraceDetail(_D("CSSSOptions::DlgProcDisplay: WM_CLOSE received for Display"));
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)1);
			return 1;
		}
	case WM_QUIT :
		{
			TraceDetail(_D("CSSSOptions::DlgProcDisplay: WM_QUIT received for Display"));
			TraceLeave(_D("CSSSOptions::DlgProcDisplay"), (DWORD)1);
			return 1;
		}
		break;
	}

	lrResult = DefWindowProc(hWnd, wMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcDisplay"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// DlgProcLanguage - Language options page dialog box procedure
//======================================================================
BOOL CSSSOptions::DlgProcLanguage (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwndMain;
	LRESULT	lrResult;
	BOOL	bResult;

	TraceEnter(_D("CSSSOptions::DlgProcLanguage"));

	TraceInfo(_D("CSSSOptions::DlgProcLanguage: wMsg = <%08X>, wParam = <%08X>, lParam = <%08X>"),
						wMsg, wParam, lParam);

	switch (wMsg)
	{
	case WM_INITDIALOG :
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_INITDIALOG received for Language"));
			// The generic parameter contains the
			// top-level window handle.
			hwndMain = (HWND)((LPPROPSHEETPAGE)lParam)->lParam;
			// Save the window handle in the window structure.
			SetWindowLong (hWnd, DWL_USER, (LONG)hwndMain);

			// Set up the languages list
			m_blDefaultLanguageFound = InitializeLanguagesList(hWnd);

			SendDlgItemMessage (hWnd, IDC_LANGUAGE_BUTTON_SET_DEFAULT, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_LANGUAGE_BUTTON_LOAD, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);

			bResult = OnInitDialog(hWnd, m_ahbrushLanguage);

			SetFocus(ItemHandleFromID(hWnd, IDC_LANGUAGE_LIST_LANGUAGES));

			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)bResult);

			return FALSE;
		}
	case WM_DESTROY :
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_DESTROY received for Language"));
			OnDestroy(hWnd, m_ahbrushLanguage);
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_COMMAND received for Language"));
			UINT uID = LOWORD (wParam);
			UINT uCommand = HIWORD (wParam);

			switch (uID)
			{
			case IDC_LANGUAGE_BUTTON_SET_DEFAULT:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							int nIndex;
							TCHAR	szID[WIV_MAX_LANGUAGE_ID + 1] = WIV_EMPTY_STRING;
							TCHAR	szName[WIV_MAX_NAME + 1] = WIV_EMPTY_STRING;
							LVITEM	lvItem;
							WIVLANG	sLang;
						
							TraceInfo(_D("CSSSOptions::DlgProcLanguage: BN_CLICKED received for Language:IDC_LANGUAGE_BUTTON_SET_DEFAULT"));

							nIndex = SendDlgItemMessage (hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_GETSELECTIONMARK, 0, 0);
							TraceDetail(_D("CSSSOptions::DlgProcLanguage: BN_CLICKED nIndex = %d"), nIndex);
							if (nIndex >= 0)
							{
								memset (&lvItem, 0, sizeof(LVITEM));
								lvItem.mask = LVIF_TEXT;
								lvItem.iItem = nIndex;
								lvItem.pszText = szID;
								lvItem.cchTextMax = WIV_MAX_LANGUAGE_ID;

								SendDlgItemMessage (hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_GETITEM, 0, (LPARAM)&lvItem);

								lvItem.pszText = szName;
								lvItem.cchTextMax = WIV_MAX_NAME;
								lvItem.iSubItem = 1;

								SendDlgItemMessage (hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_GETITEM, 0, (LPARAM)&lvItem);
								
								_tcsncpy(sLang.ID, szID, WIV_MAX_LANGUAGE_ID);
								_tcsncpy(sLang.Name, szName, WIV_MAX_NAME);
								
								LangSetDefaultLanguage(sLang);
								m_lpsDefaultLanguage = LangGetDefaultLanguage();

								if (m_lpsDefaultLanguage == NULL)
								{
									m_blDefaultLanguageFound = false;
								}
								else
								{
									m_blDefaultLanguageFound = true;
								}

								RefreshWindow(hWnd);
							}
						}
					}
					break;
				}
			case IDC_LANGUAGE_BUTTON_LOAD:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							int nIndex;
							TCHAR	szID[WIV_MAX_LANGUAGE_ID + 1] = WIV_EMPTY_STRING;
							LVITEM	lvItem;
							TCHAR	szName[WIV_MAX_NAME + 1] = WIV_EMPTY_STRING;
							bool	blResult;
							TCHAR	szTitle[MAX_PATH + 1];
							WIVLANG	sLang;
						
							TraceInfo(_D("CSSSOptions::DlgProcLanguage: BN_CLICKED received for Language:IDC_LANGUAGE_BUTTON_LOAD"));

							nIndex = SendDlgItemMessage (hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_GETSELECTIONMARK, 0, 0);
							TraceDetail(_D("CSSSOptions::DlgProcLanguage: BN_CLICKED nIndex = %d"), nIndex);

							if (nIndex >= 0)
							{
								memset (&lvItem, 0, sizeof(LVITEM));
								lvItem.mask = LVIF_TEXT;
								lvItem.iItem = nIndex;
								lvItem.pszText = szID;
								lvItem.cchTextMax = WIV_MAX_LANGUAGE_ID;

								SendDlgItemMessage (hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_GETITEM, 0, (LPARAM)&lvItem);

								lvItem.pszText = szName;
								lvItem.cchTextMax = WIV_MAX_NAME;
								lvItem.iSubItem = 1;

								SendDlgItemMessage (hWnd, IDC_LANGUAGE_LIST_LANGUAGES, LVM_GETITEM, 0, (LPARAM)&lvItem);
								
								TraceDetail(_D("CSSSOptions:DlgProcLanguage: Calling LangLoadTable, ")
										_D("Product = %s and Language = %s"), g_szProductShortName, szID);

								blResult = LangLoadTable(g_szProductShortName, szID);
								TraceDetail(_D("CSSSOptions:DlgProcLanguage: Back from LangLoadTable, Result = %d"), blResult);

								_tcsncpy(sLang.ID, szID, WIV_MAX_LANGUAGE_ID);
								_tcsncpy(sLang.Name, szName, WIV_MAX_NAME);
								
								LangSetCurrentLanguage(sLang);
								m_lpsCurrentLanguage = LangGetCurrentLanguage();
								SetupHelpFile(m_lpsCurrentLanguage);
								
								if(_tcslen(g_szNoLicenseLabel) <= 0)
								{
									if (!g_pSSSData)
									{
										TraceError(_D("CSSSOptions::DlgProcLanguage: SSS Data is NULL"));
										_snwprintf(szTitle, MAX_PATH, _T("%s %s"), _T("WiVSSS"), SSS_TEXT_MENU_OPTIONS);
									}
									else
									{
										_snwprintf(szTitle, MAX_PATH, _T("%s %s"), _tcsrev(g_pSSSData->szRevProductName), SSS_TEXT_MENU_OPTIONS);
									}
								}
								else
								{
									_snwprintf(szTitle, MAX_PATH, _T("%s %s"), g_szProductName, SSS_TEXT_MENU_OPTIONS);
								}

								TraceInfo(_D("CSSSOptions::DlgProcLanguage: Drawing new title"));

								SetTitle(szTitle);
								RefreshWindow(hWnd);

								UpdateTabsText();
								
								// Load the drop-down lists
								this->LoadIconSetsArray();
								this->LoadActionsArray();

								// Set up the languages list
								m_blDefaultLanguageFound = InitializeLanguagesList(hWnd);
							}
						}
					}
					break;
				}
			}

			break;
		}
	case WM_MOUSEMOVE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_MOUSEMOVE received for Language"));

			OnMouseMove(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_LBUTTONDOWN received for Language"));

			OnLButtonDown(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONUP:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_LBUTTONUP received for Language"));

			OnLButtonUp(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case WM_ACTIVATE :
		{
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case PSM_QUERYSIBLINGS :
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: PSM_QUERYSIBLINGS received for Language"));
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_NOTIFY received for Language"));

			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceDetail(_D("CSSSOptions::DlgProcLanguage: WM_NOTIFY, code = %d, hwndFrom = %08X, idFrom = %d, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);

			if (uiFrom == IDC_LANGUAGE_LIST_LANGUAGES)
			{
				if (uiCode == NM_RECOGNIZEGESTURE)
				{
					TraceInfo(_D("CSSSOptions::DlgProcLanguage: NM_RECOGNIZEGESTURE received for Language:IDC_LANGUAGE_LIST_LANGUAGES"));
					SetWindowLong(hWnd, DWL_MSGRESULT, TRUE);
					TraceLeave(_D("CSSSOptions::DlgProcLanguage"), TRUE);
					return TRUE;
				}
			}

			if (uiCode == PSN_SETACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcLanguage: PSN_SETACTIVE received for Language"));

				if ((_tcslen(g_szDevelopedByLabel) <= 0) || (OptionsFlags.OptionsBits.TapToRegister) || (OptionsFlags.OptionsBits.LicenseInvalid))
				{
					TraceLeave(_D("CSSSOptions::DlgProcLanguage"), TRUE);
					SetWindowLong(hWnd, DWL_MSGRESULT, InWideMode() ? IDD_ABOUT_WIDE : IDD_ABOUT);
					return TRUE;
				}

				SetFocus(ItemHandleFromID(hWnd, IDC_LANGUAGE_LIST_LANGUAGES));

				AutoScroll(m_hwOptionsWnd, m_hwTabCtrl);

				TraceLeave(_D("CSSSOptions::DlgProcLanguage"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, 0);
				return TRUE;
			}
			else if (uiCode == PSN_KILLACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcLanguage: PSN_KILLACTIVE received for Language"));
				TraceLeave(_D("CSSSOptions::DlgProcLanguage"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_APPLY)
			{
				TraceInfo(_D("CSSSOptions::DlgProcLanguage: PSN_APPLY received for Language"));
				TraceLeave(_D("CSSSOptions::DlgProcLanguage"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
				return TRUE;
			}
			else if (uiCode == PSN_RESET)
			{
				TraceInfo(_D("CSSSOptions::DlgProcLanguage: PSN_RESET received for Language"));
				TraceLeave(_D("CSSSOptions::DlgProcLanguage"), TRUE);
				return TRUE;
			}
			else if (uiCode == PSN_QUERYCANCEL)
			{
				TraceInfo(_D("CSSSOptions::DlgProcLanguage: PSN_QUERYCANCEL received for Language"));
				TraceLeave(_D("CSSSOptions::DlgProcLanguage"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_HELP)
			{
				TraceInfo(_D("CSSSOptions::DlgProcLanguage: PSN_HELP received for Language"));

				DoShowHelp(g_szHelpTagLanguageSettings);

				break;
			}

			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)FALSE);
			return FALSE;  // Return false to force default processing.
		}
	case WM_ERASEBKGND:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_ERASEBKGND received for Language"));
			lrResult = OnEraseBackground(hWnd, (HDC)wParam);
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), lrResult);
			return lrResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_CTLCOLORDLG received for Language"));
			lrResult = OnCtlColor(m_ahbrushLanguage, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), lrResult);
			return (BOOL)lrResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_CTLCOLORBTN received for Language"));
			lrResult = OnCtlColor(m_ahbrushLanguage, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), lrResult);
			return (BOOL)lrResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_CTLCOLORSTATIC received for Language"));
			lrResult = OnCtlColor(m_ahbrushLanguage, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), lrResult);
			return (BOOL)lrResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_CTLCOLOREDIT received for Language"));
			lrResult = OnCtlColor(m_ahbrushLanguage, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), lrResult);
			return (BOOL)lrResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_CTLCOLORLISTBOX received for Language"));
			lrResult = OnCtlColor(m_ahbrushLanguage, (HWND)lParam);
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), lrResult);
			return (BOOL)lrResult;
        }

	case WM_PAINT :
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_PAINT received for Language"));
			PAINTSTRUCT ps;
			HDC hDC;
			TCHAR	szTemp[WIV_MAX_STRING + 1];
			RECT	rectDraw;

			hDC = BeginPaint(hWnd, &ps);

			OnPaint(hWnd, hDC);

			EndPaint(hWnd, &ps);

			_snwprintf(szTemp, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_LANGUAGE_AVAILABLE);
			DrawStaticText(hWnd, IDC_LANGUAGE_STATIC_LANGUAGES_LIST, szTemp, DT_LEFT);

			if (m_lpsDefaultLanguage != NULL)
			{
				if (_tcslen(m_lpsDefaultLanguage->ID) == 0)
				{
					DrawStaticText(hWnd, IDC_LANGUAGE_STATIC_DEFAULT_LANGUAGE, SSS_TEXT_LANGUAGE_DEFAULT_NOT_DEFINED, DT_CENTER, FW_NORMAL, WIV_FONT_NONE, g_crRedError);

				} else if (!m_blDefaultLanguageFound)
				{
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s: %s (%s)"), SSS_TEXT_LANGUAGE_DEFAULT, m_lpsDefaultLanguage->ID, SSS_TEXT_LANGUAGE_DEFAULT_NOT_FOUND);
					GetDrawRect(hWnd, IDC_LANGUAGE_STATIC_DEFAULT_LANGUAGE, szTemp, &rectDraw);
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s: "), SSS_TEXT_LANGUAGE_DEFAULT);
					rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_MEDIUM);
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s (%s)"), m_lpsDefaultLanguage->ID, SSS_TEXT_LANGUAGE_DEFAULT_NOT_FOUND);
					rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_NORMAL, WIV_FONT_NONE, g_crRedError);
				}
				else
				{
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s: %s (%s)"), SSS_TEXT_LANGUAGE_DEFAULT, m_lpsDefaultLanguage->ID, m_lpsDefaultLanguage->Name);
					GetDrawRect(hWnd, IDC_LANGUAGE_STATIC_DEFAULT_LANGUAGE, szTemp, &rectDraw);
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s: "), SSS_TEXT_LANGUAGE_DEFAULT);
					rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_MEDIUM);
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s (%s)"), m_lpsDefaultLanguage->ID, m_lpsDefaultLanguage->Name);
					rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_SEMIBOLD, WIV_FONT_NONE, g_crBlueInfo);
				}
			}
			else
			{
				DrawStaticText(hWnd, IDC_LANGUAGE_STATIC_DEFAULT_LANGUAGE, SSS_TEXT_LANGUAGE_DEFAULT_NOT_DEFINED, DT_CENTER, FW_NORMAL, WIV_FONT_NONE, g_crRedError);
			}

			if (m_lpsCurrentLanguage != NULL)
			{
				if ((_tcslen(m_lpsCurrentLanguage->ID) == 0) || (!LangGetTableStatus()))
				{
					DrawStaticText(hWnd, IDC_LANGUAGE_STATIC_CURRENT_LANGUAGE, SSS_TEXT_LANGUAGE_CURRENT_BUILT_IN, DT_CENTER, FW_NORMAL);
				}
				else
				{
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s: %s (%s)"), SSS_TEXT_LANGUAGE_CURRENT, m_lpsCurrentLanguage->ID, m_lpsCurrentLanguage->Name);
					GetDrawRect(hWnd, IDC_LANGUAGE_STATIC_CURRENT_LANGUAGE, szTemp, &rectDraw);
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s: "), SSS_TEXT_LANGUAGE_CURRENT);
					rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_MEDIUM);
					_snwprintf(szTemp, WIV_MAX_STRING, _T("%s (%s)"), m_lpsCurrentLanguage->ID, m_lpsCurrentLanguage->Name);
					rectDraw = AppendStaticText(hWnd, rectDraw, szTemp, DT_LEFT, FW_NORMAL, WIV_FONT_NONE, g_crBlueInfo);
				}
			}
			else
			{
				DrawStaticText(hWnd, IDC_LANGUAGE_STATIC_CURRENT_LANGUAGE, SSS_TEXT_LANGUAGE_CURRENT_BUILT_IN, DT_CENTER, FW_NORMAL);
			}

            SendDlgItemMessage (hWnd, IDC_LANGUAGE_BUTTON_SET_DEFAULT, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_LANGUAGE_SET_DEFAULT);
            SendDlgItemMessage (hWnd, IDC_LANGUAGE_BUTTON_LOAD, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_LANGUAGE_LOAD);

			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case WM_SIZE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcLanguage: WM_SIZE received for Language"));
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_OPTIONS_LANGUAGE_WIDE) :
					MAKEINTRESOURCE(IDD_OPTIONS_LANGUAGE));
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}

	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcLanguage: WM_WIV_REFRESH received for Language"));
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)0);
			return 0;
		}
	case WM_CLOSE :
		{
			TraceDetail(_D("CSSSOptions::DlgProcLanguage: WM_CLOSE received for Language"));
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)1);
			return 1;
		}
	case WM_QUIT :
		{
			TraceDetail(_D("CSSSOptions::DlgProcLanguage: WM_QUIT received for Language"));
			TraceLeave(_D("CSSSOptions::DlgProcLanguage"), (DWORD)1);
			return 1;
		}
		break;
	}

	lrResult = DefWindowProc(hWnd, wMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcLanguage"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// DlgProcSecurity - Security options page dialog box procedure
//======================================================================
BOOL CSSSOptions::DlgProcSecurity (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwndMain;
	LRESULT	lrResult;
	BOOL	bResult;

    TraceEnter(_D("CSSSOptions::DlgProcSecurity"));

	switch (wMsg)
	{
	case WM_INITDIALOG :
		{
			// The generic parameter contains the
			// top-level window handle.
			hwndMain = (HWND)((LPPROPSHEETPAGE)lParam)->lParam;

			// Save the window handle in the window structure.
			SetWindowLong (hWnd, DWL_USER, (LONG)hwndMain);

			LoadSecurityStepsArrays();

			SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);

			if ((OptionsFlags.OptionsBits.PhoneIsOff) || (OptionsFlags.OptionsBits.SIMMissing) || (OptionsFlags.OptionsBits.IncorrectSIM))
			{
				TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_INITDIALOG in Security, Phone is OFF"));
				m_blAllowAutoPINAfterInit = false;
				m_blAllowAutoPINAfterRadioON = false;
				m_blDefaultSIM = false;
			}
			else
			{
				TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_INITDIALOG in Security, Phone is ON"));
			}

			// Check/Uncheck default SIM
			TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_INITDIALOG in Security, m_blDefaultSIM=%08x"),	m_blDefaultSIM);
			SetCheckButton(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, m_blAllowAutoPINAfterInit);

			// Check/Uncheck allow auto-pin
			TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_INITDIALOG in Security, m_blAllowAutoPINAfterInit=%08x"),	m_blAllowAutoPINAfterInit);
			SetCheckButton(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, m_blAllowAutoPINAfterInit);
			TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_INITDIALOG in Security, m_blAllowAutoPINAfterRadioON=%08x"),	m_blAllowAutoPINAfterRadioON);
			SetCheckButton(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, m_blAllowAutoPINAfterRadioON);

			if (_tcslen(m_szPIN) != 0)
			{
				SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
					0, (LPARAM)SSS_TEXT_SECURITY_CHANGE_PIN);
			}
			else
			{
				SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
					0, (LPARAM)SSS_TEXT_SECURITY_CREATE_PIN);
			}
			
			if (IsSecondEdition() && InWideMode())
			{
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);

			}
			else
			{
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);

			}

			// Enable/Disable other controls
			if ((OptionsFlags.OptionsBits.PhoneIsOff) || (OptionsFlags.OptionsBits.SIMMissing) || (OptionsFlags.OptionsBits.IncorrectSIM))
			{
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT), false); 
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON), false); 
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM), false); 
			}
			else
			{
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT), true); 
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON), true); 
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM), true); 
			}
			
			// Setup up appropriate sequence for PIN entry
			if (m_blAllowAutoPINAfterInit | m_blAllowAutoPINAfterRadioON)
			{
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN), true); 
				m_dwSecuritySeqNumber = g_dwSecuritySequencePIN;
			}
			else
			{
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN), false); 
				m_dwSecuritySeqNumber = g_dwSecuritySequenceNoAuto;
			}
			
			m_dwSecuritySeqStep  = g_dwSecuritySequenceStepStart;
			
			DrawSecurityHeader(hWnd, IDC_SECURITY_STATIC_PHONE);
			bResult = OnInitDialog(hWnd, m_ahbrushSecurity);

		    TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)bResult);

			return bResult;
		}
	case WM_DESTROY :
		{
			OnDestroy(hWnd, m_ahbrushSecurity);
			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			UINT uID = LOWORD (wParam);
			UINT uCommand = HIWORD (wParam);
			TraceDetail(_D("CSSSOptions::DlgProcSecurity: uID = %d, uCommand = 0x%08X"), uID, uCommand);

			switch (uID)
			{
			case IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcSecurity: BN_CLICKED received for Security:IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM"));

							// Get checked state of Make Default check box
							m_blDefaultSIM = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM);
						}
					}
					break;
				}
			case IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcSecurity: BN_CLICKED received for Security:IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT"));
							
							// Get checked state of Allow Auto PIN check box
							m_blAllowAutoPINAfterInit = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT);
							
							if (m_blAllowAutoPINAfterInit | m_blAllowAutoPINAfterRadioON)
							{
								if (_tcslen(m_szPIN) != 0)
								{
									m_dwSecuritySeqNumber = g_dwSecuritySequencePIN;
									SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
										0, (LPARAM)SSS_TEXT_SECURITY_CHANGE_PIN);
								}
								else
								{
									m_dwSecuritySeqNumber = g_dwSecuritySequenceNoPIN;
									SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
										0, (LPARAM)SSS_TEXT_SECURITY_CREATE_PIN);
								}
								
								EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN), true); 
							}
							else
							{
								m_dwSecuritySeqNumber = g_dwSecuritySequenceNoAuto;
								SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
									0, (LPARAM)SSS_TEXT_SECURITY_CREATE_PIN);

								EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN), false); 
							}

							m_dwSecuritySeqStep  = g_dwSecuritySequenceStepStart;
						}
					}
					break;
				}
			case IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcSecurity: BN_CLICKED received for Security:IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON"));

							// Get checked state of Allow Auto PIN check box
							m_blAllowAutoPINAfterRadioON = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON);

							if (m_blAllowAutoPINAfterInit | m_blAllowAutoPINAfterRadioON)
							{
								if (_tcslen(m_szPIN) != 0)
								{
									m_dwSecuritySeqNumber = g_dwSecuritySequencePIN;
									SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
										0, (LPARAM)SSS_TEXT_SECURITY_CHANGE_PIN);
								}
								else
								{
									m_dwSecuritySeqNumber = g_dwSecuritySequenceNoPIN;
									SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
										0, (LPARAM)SSS_TEXT_SECURITY_CREATE_PIN);
								}

								EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN), true); 
							}
							else
							{
								m_dwSecuritySeqNumber = g_dwSecuritySequenceNoAuto;
								SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
									0, (LPARAM)SSS_TEXT_SECURITY_CREATE_PIN);

								EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN), false); 
							}

							m_dwSecuritySeqStep  = g_dwSecuritySequenceStepStart;
						}
					}
					break;
				}
			case IDC_SECURITY_BUTTON_CHANGE_PIN:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							LRESULT lrResult;
							
							TraceInfo(_D("CSSSOptions::DlgProcSecurity: BN_CLICKED received for Security: IDC_SECURITY_BUTTON_CHANGE_PIN"));
							
							m_dwSecuritySeqStep++; 
							lrResult = (LPARAM)DialogBox(m_hmInstance, MAKEINTRESOURCE(IDD_PINENTRY_DIALOG), hWnd, (DLGPROC)OptionsDlgProcPINEntry);
							
							TraceInfo(_D("CSSSOptions::DlgProcSecurity: DialogBox:PINEntryDlgProc returned <%08x>"), lrResult);
							TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
							return 0;
/*
							SendMessage(hWnd, DM_SETDEFID, IDC_PINENTRY_BUTTON_OK, 0);
							m_dwSecuritySeqStep++; 

							// Show/Hide PIN entry controls
							SetupSecurityPage(hWnd);
*/
						}
					}
					break;
				}
			}
			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)TRUE);
			return TRUE;
		}
	case WM_ACTIVATE :
		{
			TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_ACTIVATE"));
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));


			if ((OptionsFlags.OptionsBits.PhoneIsOff) || (OptionsFlags.OptionsBits.SIMMissing) || (OptionsFlags.OptionsBits.IncorrectSIM))
			{
				m_dwSecuritySeqNumber = g_dwSecuritySequenceNoAuto;
				m_dwSecuritySeqStep = g_dwSecuritySequenceStepStart;
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT), false); 
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON), false); 
				EnableWindow(ItemHandleFromID(hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM), false); 
			}

			DrawSecurityHeader(hWnd, IDC_SECURITY_STATIC_PHONE);
			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_SETTINGCHANGE"));
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);

			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}
	case WM_MOUSEMOVE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_MOUSEMOVE received for Security"));

			OnMouseMove(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_LBUTTONDOWN received for Security"));

			OnLButtonDown(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONUP:
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_LBUTTONUP received for Security"));

			OnLButtonUp(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}
	case PSM_QUERYSIBLINGS :
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: PSM_QUERYSIBLINGS received for Security"));

			if ((!OptionsFlags.OptionsBits.PhoneIsOff) && (!OptionsFlags.OptionsBits.SIMMissing) && (!OptionsFlags.OptionsBits.IncorrectSIM))
			{
				NewSettings.blDefaultSIM = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM);
				NewSettings.blAllowAutoPINAfterInit = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT);
				NewSettings.blAllowAutoPINAfterRadioON = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON);
				_tcsncpy(NewSettings.szPIN, m_szPIN, SSS_MAX_PIN);
			}

			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_NOTIFY, code = %08X, hwndFrom = %08X, idFrom = %08X, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);

			if (uiCode == PSN_SETACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcSecurity: PSN_SETACTIVE received for Security"));

				if ((_tcslen(g_szVersionLabel) <= 0) || (OptionsFlags.OptionsBits.TapToRegister) || (OptionsFlags.OptionsBits.LicenseInvalid))
				{
					TraceLeave(_D("CSSSOptions::DlgProcSecurity"), TRUE);
					SetWindowLong(hWnd, DWL_MSGRESULT, InWideMode() ? IDD_ABOUT_WIDE : IDD_ABOUT);
					return TRUE;
				}

				AutoScroll(m_hwOptionsWnd, m_hwTabCtrl);
				
				// Check/Uncheck default SIM
				TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_INITDIALOG in Security, m_blDefaultSIM=%08x"),	m_blDefaultSIM);
				SetCheckButton(hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, m_blDefaultSIM);

				// Check/Uncheck allow auto-pin
				TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_INITDIALOG in Security, m_blAllowAutoPINAfterInit=%08x"),	m_blAllowAutoPINAfterInit);
				SetCheckButton(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, m_blAllowAutoPINAfterInit);
				TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_INITDIALOG in Security, m_blAllowAutoPINAfterRadioON=%08x"),	m_blAllowAutoPINAfterRadioON);
				SetCheckButton(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, m_blAllowAutoPINAfterRadioON);

				if (IsSecondEdition() && InWideMode())
				{
					SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
					SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
					SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);

				}
				else
				{
					SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);
					SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);
					SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);

				}

				if (_tcslen(m_szPIN) != 0)
				{
					SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
						0, (LPARAM)SSS_TEXT_SECURITY_CHANGE_PIN);
				}
				else
				{
					SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
						0, (LPARAM)SSS_TEXT_SECURITY_CREATE_PIN);
				}
				
				TraceLeave(_D("CSSSOptions::DlgProcSecurity"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, 0);
				return TRUE;
			}
			else if (uiCode == PSN_KILLACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_KILLACTIVE received for Display"));
				TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_APPLY)
			{
				TraceInfo(_D("CSSSOptions::DlgProcSecurity: PSN_APPLY received for Security"));

				if ((OptionsFlags.OptionsBits.PhoneIsOff) || (OptionsFlags.OptionsBits.SIMMissing) || (OptionsFlags.OptionsBits.IncorrectSIM))
				{
					TraceLeave(_D("CSSSOptions::DlgProcSecurity"), TRUE);
					SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
					return TRUE;
				}

				m_blDefaultSIM = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM);
				m_blAllowAutoPINAfterInit = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT);
				m_blAllowAutoPINAfterRadioON = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON);

				TraceInfo(_D("CSSSOptions::DlgProcSecurity: PSN_APPLY in Security, m_blDefaultSIM=%08x"), m_blDefaultSIM);
				TraceInfo(_D("CSSSOptions::DlgProcSecurity: PSN_APPLY in Security, m_blAllowAutoPINAfterInit=%08x"), m_blAllowAutoPINAfterInit);
				TraceInfo(_D("CSSSOptions::DlgProcSecurity: PSN_APPLY in Security, m_blAllowAutoPINAfterRadioON=%08x"), m_blAllowAutoPINAfterRadioON);

				if (!(m_blAllowAutoPINAfterInit | m_blAllowAutoPINAfterRadioON))
				{
					_zclr(m_szPIN);
				}
				else
				{
					if (_tcslen(m_szPIN) == 0)
					{
						m_blAllowAutoPINAfterInit = false;
						m_blAllowAutoPINAfterRadioON = false;
					}
				}

				NewSettings.blDefaultSIM = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM);
				NewSettings.blAllowAutoPINAfterInit = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT);
				NewSettings.blAllowAutoPINAfterRadioON = IsButtonChecked(hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON);
				_tcsncpy(NewSettings.szPIN, m_szPIN, SSS_MAX_PIN);

				TraceLeave(_D("CSSSOptions::DlgProcSecurity"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
				return TRUE;
			}
			else if (uiCode == PSN_RESET)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_RESET received for Security"));
				TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
				return TRUE;
			}
			else if (uiCode == PSN_QUERYCANCEL)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDisplay: PSN_QUERYCANCEL received for Security"));
				TraceLeave(_D("CSSSOptions::DlgProcDisplay"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_HELP)
			{
				TraceInfo(_D("CSSSOptions::DlgProcSecurity: PSN_HELP received for Security"));

				DoShowHelp(g_szHelpTagSecuritySettings);

				break;
			}

			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)FALSE);
			return FALSE;  // Return false to force default processing.
		}
	case WM_PAINT :
		{
			PAINTSTRUCT ps;
			HDC			hDC;
			TCHAR		szInfo[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

			TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_PAINT"));

			hDC = BeginPaint(hWnd, &ps);

			DrawTitle(m_hmInstance, hWnd, hDC, m_hfTitleFont, m_szTitle, OptionsFlags.OptionsBits.AllowOptionsCancel);
			
			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_ACTIVE_SIM);
			if (IsSecondEdition() && InWideMode())
			{
				DrawStaticText(hWnd, IDC_SECURITY_STATIC_ACTIVE_SIM, szInfo, DT_LEFT, FW_SEMIBOLD, WIV_FONT_MEDIUM);
			}
			else
			{
				DrawStaticText(hWnd, IDC_SECURITY_STATIC_ACTIVE_SIM, szInfo, DT_LEFT, FW_SEMIBOLD);
			}

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_SECURITY_STATIC_DEFAULT);
			if (IsSecondEdition() && InWideMode())
			{
				DrawStaticText(hWnd, IDC_SECURITY_STATIC_DEFAULT_SIM, szInfo, DT_LEFT, FW_SEMIBOLD, WIV_FONT_MEDIUM);
			}
			else
			{
				DrawStaticText(hWnd, IDC_SECURITY_STATIC_DEFAULT_SIM, szInfo, DT_LEFT, FW_SEMIBOLD);
			}
			
			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_SECURITY_STATIC_AUTO_PIN);
			if (IsSecondEdition() && InWideMode())
			{
				DrawStaticText(hWnd, IDC_SECURITY_STATIC_AUTO_PIN, szInfo, DT_LEFT, FW_SEMIBOLD, WIV_FONT_MEDIUM);
			}
			else
			{
				DrawStaticText(hWnd, IDC_SECURITY_STATIC_AUTO_PIN, szInfo, DT_LEFT, FW_SEMIBOLD);
			}
			
            SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, WM_SETTEXT,
				0, (LPARAM)SSS_TEXT_SECURITY_MAKE_DEFAULT);
            SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_SECURITY_AUTO_PIN_INIT);
            SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_SECURITY_AUTO_PIN_RADIOON);

			if (_tcslen(m_szPIN) != 0)
			{
				SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
									  0, (LPARAM)SSS_TEXT_SECURITY_CHANGE_PIN);
			}
			else
			{
				SendDlgItemMessage (hWnd, IDC_SECURITY_BUTTON_CHANGE_PIN, WM_SETTEXT,
									  0, (LPARAM)SSS_TEXT_SECURITY_CREATE_PIN);
			}

			if (IsSecondEdition() && InWideMode())
			{
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);

			}
			else
			{
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_MAKE_DEFAULT_SIM, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_INIT, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);
				SendDlgItemMessage (hWnd, IDC_SECURITY_CHECK_ALLOW_AUTO_PIN_RADIOON, WM_SETFONT, (WPARAM)m_hfNormalFont, (LPARAM)TRUE);

			}
			
			EndPaint(hWnd, &ps);

			DrawSecurityHeader(hWnd, IDC_SECURITY_STATIC_PHONE);

			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}

	case WM_ERASEBKGND:
		{
			bResult = OnEraseBackground(hWnd, (HDC)wParam);
		    TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)bResult);

			return bResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_CTLCOLORDLG received for Security"));
			bResult = (BOOL)OnCtlColor(m_ahbrushSecurity, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_CTLCOLORBTN received for Security"));
			bResult = (BOOL)OnCtlColor(m_ahbrushSecurity, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_CTLCOLORSTATIC received for Security"));
			bResult = (BOOL)OnCtlColor(m_ahbrushSecurity, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)bResult);
			return bResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_CTLCOLOREDIT received for Security"));
			bResult = (BOOL)OnCtlColor(m_ahbrushSecurity, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)bResult);
			return bResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcSecurity: WM_CTLCOLORLISTBOX received for Security"));
			bResult = (BOOL)OnCtlColor(m_ahbrushSecurity, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)bResult);
			return bResult;
        }

	case WM_SIZE:
		{
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_OPTIONS_SECURITY_WIDE) :
					MAKEINTRESOURCE(IDD_OPTIONS_SECURITY));
		}
	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcSecurity: WM_WIV_REFRESH received for Security"));
			TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
			return 0;
		}
		TraceLeave(_D("CSSSOptions::DlgProcSecurity"), (DWORD)0);
		return 0;
	}

	lrResult = DefWindowProc(hWnd, wMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcSecurity"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// DlgProcPINEntry - PIN entry dialog box procedure
//======================================================================
LRESULT CSSSOptions::DlgProcPINEntry(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT	lrResult;
	BOOL	bResult;

    TraceEnter(_D("CSSSOptions::DlgProcPINEntry"));

	switch (uMsg)
	{
	case WM_INITDIALOG :
		{
			SHINITDLGINFO	shidi;
			
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_EMPTYMENU | SHIDIF_SIZEDLGFULLSCREEN;
			
			shidi.hDlg = hWnd;
			SHInitDialog(&shidi);
			
			SHMENUBARINFO mbi;
			memset(&mbi, 0, sizeof(SHMENUBARINFO));
			mbi.cbSize = sizeof(SHMENUBARINFO);
			mbi.hwndParent = hWnd;
			mbi.nToolBarId = NULL;
			mbi.hInstRes = m_hmInstance;
			mbi.dwFlags = SHCMBF_EMPTYBAR;
			SHCreateMenuBar(&mbi);
			
			LoadSecurityStepsArrays();
			
			SendDlgItemMessage (hWnd, IDC_PINENTRY_BUTTON_OK, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_PINENTRY_BUTTON_CANCEL, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);
			
			if ((OptionsFlags.OptionsBits.PhoneIsOff) || (OptionsFlags.OptionsBits.SIMMissing) || (OptionsFlags.OptionsBits.IncorrectSIM))
			{
				TraceDetail(_D("CSSSOptions::DlgProcPINEntry: WM_INITDIALOG in PINEntry, Phone is OFF"));
				//TODO:
			}
			else
			{
				TraceDetail(_D("CSSSOptions::DlgProcPINEntry: WM_INITDIALOG in PINEntry, Phone is ON"));
			}
			
			// Set up maximum length for PIN
			SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, EM_LIMITTEXT, SSS_MAX_PIN, 0);

			// Show/Hide PIN entry controls
			SetupPINEntryPage(hWnd);
			
			DrawSecurityHeader(hWnd, IDC_PINENTRY_STATIC_PHONE);
			bResult = OnInitDialog(hWnd, m_ahbrushPINEntry);
			
			TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)bResult);

			return bResult;
		}
	case WM_DESTROY :
		{
			OnDestroy(hWnd, m_ahbrushPINEntry);

			m_hwPINEntryDialog = NULL;
			
			TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			UINT nID = LOWORD(wParam);
			UINT nCommand = HIWORD (wParam);

			switch (nID)
			{
			case IDOK:
				{
					TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
					EndDialog(hWnd,0);
					return 0;
				}
			case IDCANCEL:
				{
					TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
					EndDialog(hWnd,0);
					return 0;
				}
			case IDC_PINENTRY_BUTTON_OK:
				{
					switch (nCommand)
					{
					case BN_CLICKED:
						{
							TCHAR	szPIN[SSS_MAX_PIN + 1];
							
							TraceDetail(_D("CSSSOptions::DlgProcPINEntry: BN_CLICKED received for Security: IDC_PINENTRY_BUTTON_OK"));
							
							ClearSecurityError(hWnd);
							
							SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, WM_GETTEXT,
								sizeof(szPIN), (LPARAM)szPIN);
							
							if (GetCurrentSeqStep() == g_dwSecuritySequenceActionConfirm)
							{
								TraceDetail(_D("CSSSOptions::DlgProcPINEntry: Confirm PIN"));
								if (_tcsncmp(m_szEnteredPIN, szPIN, SSS_MAX_PIN) == 0)
								{
									_tcsncpy(m_szPIN, m_szEnteredPIN, SSS_MAX_PIN);
									_zclr(m_szEnteredPIN);
									
									SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, WM_SETTEXT,
										SSS_MAX_PIN, (LPARAM)WIV_EMPTY_STRING);
									
									m_dwSecuritySeqNumber = g_dwSecuritySequencePIN;
									m_dwSecuritySeqStep = g_dwSecuritySequenceStepStart;
									TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
									EndDialog(hWnd,0);
									return 0;
								}
								else
								{
									DrawSecurityError(hWnd, SSS_TEXT_ERROR_PIN_MISMATCH);
									SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, EM_SETSEL, 0, -1);
								}
							}
							else if (GetCurrentSeqStep() == g_dwSecuritySequenceActionCurrent)
							{
								TraceDetail(_D("CSSSOptions::DlgProcPINEntry: Current PIN"));
								if (_tcsncmp(m_szPIN, szPIN, SSS_MAX_PIN) == 0)
								{
									_tcsncpy(m_szEnteredPIN, szPIN, SSS_MAX_PIN);
									SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, WM_SETTEXT,
										SSS_MAX_PIN, (LPARAM)WIV_EMPTY_STRING);
									m_dwSecuritySeqStep++;
								}
								else
								{
									DrawSecurityError(hWnd, SSS_TEXT_ERROR_INCORRECT_PIN);
									SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, EM_SETSEL, 0, -1);
								}
							} else
							{
								TraceDetail(_D("CSSSOptions::DlgProcPINEntry: Enter PIN"));
								_tcsncpy(m_szEnteredPIN, szPIN, SSS_MAX_PIN);
								SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, WM_SETTEXT,
									SSS_MAX_PIN, (LPARAM)WIV_EMPTY_STRING);
								m_dwSecuritySeqStep++;
							}
							
							// Show/Hide PIN entry controls
							SetupPINEntryPage(hWnd);
						}
					}
					break;
				}
			case IDC_PINENTRY_BUTTON_CANCEL:
				{
					switch (nCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcPINEntry: BN_CLICKED received for Security: IDC_PINENTRY_BUTTON_CANCEL"));
							
							ClearSecurityError(hWnd);
							
							if (GetCurrentSeqStep() == g_dwSecuritySequenceActionConfirm)
							{
								SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, WM_SETTEXT,
									SSS_MAX_PIN, (LPARAM)m_szEnteredPIN);
								
								m_dwSecuritySeqStep--;
							}
							else
							{
								SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, WM_SETTEXT,
									SSS_MAX_PIN, (LPARAM)WIV_EMPTY_STRING);
								
								m_dwSecuritySeqStep = g_dwSecuritySequenceStepStart;
								TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
								EndDialog(hWnd,0);
								return 0;
							}
							
							// Show/Hide PIN entry controls
							SetupPINEntryPage(hWnd);
						}
					}
					break;
				}
			case IDC_PINENTRY_EDIT_ENTER_PIN:
				{
					switch (nCommand)
					{
					case EN_CHANGE:
						{
							TCHAR szPIN[SSS_MAX_PIN + 1];
							
							TraceInfo(_D("CSSSOptions::DlgProcPINEntry: EN_CHANGE received for IDC_PINENTRY_EDIT_ENTER_PIN"));
							
							SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, WM_GETTEXT,
								sizeof(szPIN), (LPARAM)szPIN);
							
							EnableWindow(ItemHandleFromID(hWnd, IDC_PINENTRY_BUTTON_OK),
								((_tcslen(szPIN) < 4) ? false : true));
							
							TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
							return 0;
						}
					case EN_SETFOCUS:
						{
							TCHAR szPIN[SSS_MAX_PIN + 1];
							
							TraceInfo(_D("CSSSOptions::DlgProcPINEntry: EN_SETFOCUS received for IDC_PINENTRY_EDIT_ENTER_PIN"));
							
							// SIP up
							SHSipPreference (hWnd, SIP_UP);
							
							SendDlgItemMessage (hWnd, IDC_PINENTRY_EDIT_ENTER_PIN, WM_GETTEXT,
								sizeof(szPIN), (LPARAM)szPIN);
							
							EnableWindow(ItemHandleFromID(hWnd, IDC_PINENTRY_BUTTON_OK),
								((_tcslen(szPIN) < 4) ? false : true));
							
							TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
							return 0;
						}
					case EN_KILLFOCUS:
						{
							TraceInfo(_D("CSSSOptions::DlgProcPINEntry: EN_KILLFOCUS received for IDC_PINENTRY_EDIT_ENTER_PIN"));
							
							// SIP down
							SHSipPreference (hWnd, SIP_DOWN);
							
							TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
							return 0;
						}
					}
					break;
				}
			}
			TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
			return 0;
		}
	case WM_ACTIVATE :
		{
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
			TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{

			TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
			return 0;//OnNotify(wParam, pNMHDR);
		}
	case WM_PAINT :
		{
			PAINTSTRUCT ps;
			HDC			hDC;
			TCHAR		szInfo[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
			
			TraceDetail(_D("CSSSOptions::DlgProcPINEntry: WM_PAINT, current step = %d"), GetCurrentSeqStep());
			
			hDC = BeginPaint(hWnd, &ps);
			
			OnPaint(hWnd, hDC, true);
			
            SendDlgItemMessage (hWnd, IDC_PINENTRY_BUTTON_OK, WM_SETTEXT,
				0, (LPARAM)SSS_TEXT_OK);
            SendDlgItemMessage (hWnd, IDC_PINENTRY_BUTTON_CANCEL, WM_SETTEXT,
				0, (LPARAM)SSS_TEXT_CANCEL);
			
			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_PINENTRY_SPECIFY_PIN);
			if (IsSecondEdition() && InWideMode())
			{
				DrawStaticText(hWnd, IDC_PINENTRY_STATIC_SPECIFY_PIN, szInfo, DT_LEFT, FW_SEMIBOLD, WIV_FONT_MEDIUM);
			}
			else
			{
				DrawStaticText(hWnd, IDC_PINENTRY_STATIC_SPECIFY_PIN, szInfo, DT_LEFT, FW_SEMIBOLD);
			}
			
			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_ACTIVE_SIM);
			if (IsSecondEdition() && InWideMode())
			{
				DrawStaticText(hWnd, IDC_PINENTRY_STATIC_ACTIVE_SIM, szInfo, DT_LEFT, FW_SEMIBOLD, WIV_FONT_MEDIUM);
			}
			else
			{
				DrawStaticText(hWnd, IDC_PINENTRY_STATIC_ACTIVE_SIM, szInfo, DT_LEFT, FW_SEMIBOLD);
			}
			
			if (SetupPINEntryPage(hWnd, true) != NULL)
			{
				TraceDetail(_D("CSSSOptions::DlgProcPINEntry: Drawing IDC_PINENTRY_STATIC_ENTER_PIN"));
				
				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SetupPINEntryPage(hWnd, true));
				if (IsSecondEdition() && InWideMode())
				{
					DrawStaticText(hWnd, IDC_PINENTRY_STATIC_ENTER_PIN, szInfo, DT_RIGHT, FW_NORMAL, WIV_FONT_MEDIUM);
				}
				else
				{
					DrawStaticText(hWnd, IDC_PINENTRY_STATIC_ENTER_PIN, szInfo, DT_RIGHT);
				}
			}
			else
			{
				TraceDetail(_D("CSSSOptions::DlgProcPINEntry: Clearing IDC_PINENTRY_STATIC_ENTER_PIN"));
				ClearStaticText(hWnd, IDC_PINENTRY_STATIC_ENTER_PIN, (NULL == m_hbmpBackgroundBitmap ? GetSysColorBrush(COLOR_WINDOW) : m_ahbrushSecurity[7]));
			}
			
			EndPaint(hWnd, &ps);
			
			// Display phone number and SIM ID.
			DrawSecurityHeader(hWnd, IDC_PINENTRY_STATIC_PHONE);
			
			TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
			return 0;
		}
		
	case WM_ERASEBKGND:
		{
			bResult = OnEraseBackground(hWnd, (HDC)wParam);
		    TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)bResult);

			return bResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcPINEntry: WM_CTLCOLORDLG received for PINEntry"));
			bResult = (BOOL)OnCtlColor(m_ahbrushPINEntry, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcPINEntry: WM_CTLCOLORBTN received for PINEntry"));
			bResult = (BOOL)OnCtlColor(m_ahbrushPINEntry, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcPINEntry: WM_CTLCOLORSTATIC received for PINEntry"));
			bResult = (BOOL)OnCtlColor(m_ahbrushPINEntry, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)bResult);
			return bResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcPINEntry: WM_CTLCOLOREDIT received for PINEntry"));
			bResult = (BOOL)OnCtlColor(m_ahbrushPINEntry, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)bResult);
			return bResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcPINEntry: WM_CTLCOLORLISTBOX received for PINEntry"));
			bResult = (BOOL)OnCtlColor(m_ahbrushPINEntry, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)bResult);
			return bResult;
        }

	case WM_SIZE:
		{
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_PINENTRY_DIALOG_WIDE) :
					MAKEINTRESOURCE(IDD_PINENTRY_DIALOG));
			break;
		}

	case WM_HELP:
		{
			TraceDetail(_D("CSSSOptions::DlgProcPINEntry: WM_HELP received for PINEntry"));

//TODO:			DoShowHelp(g_szHelpTagPINEntryDialog);

			break;
		}

	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcPINEntry: WM_WIV_REFRESH received for PINEntry"));
			
			RefreshWindow(hWnd);
			
			TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
			return 0;
		}

	default :
		TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), (DWORD)0);
		return 0;//OnMessage(uMsg, wParam, lParam);
	}

	lrResult = DefWindowProc(hWnd, uMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcPINEntry"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// DlgProcRegistration - Registration details dialog box procedure
//======================================================================
LRESULT CSSSOptions::DlgProcRegistration(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT	lrResult;
	BOOL	bResult;

    TraceEnter(_D("CSSSOptions::DlgProcRegistration"));

	switch (uMsg)
	{
	case WM_INITDIALOG :
		{
			SHINITDLGINFO	shidi;

			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_EMPTYMENU | SHIDIF_SIZEDLGFULLSCREEN;

			shidi.hDlg = hWnd;
			SHInitDialog(&shidi);

			SHMENUBARINFO mbi;
			memset(&mbi, 0, sizeof(SHMENUBARINFO));
			mbi.cbSize = sizeof(SHMENUBARINFO);
			mbi.hwndParent = hWnd;
			mbi.nToolBarId = NULL;
			mbi.hInstRes = m_hmInstance;
			mbi.dwFlags = SHCMBF_EMPTYBAR;
			SHCreateMenuBar(&mbi);

			SendDlgItemMessage (hWnd, IDC_REGISTRATION_BUTTON_OK, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_BUTTON_CANCEL, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);

			SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC1, EM_LIMITTEXT, WIV_MAX_LICENSE_SECTION, 0);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC2, EM_LIMITTEXT, WIV_MAX_LICENSE_SECTION, 0);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC3, EM_LIMITTEXT, WIV_MAX_LICENSE_SECTION, 0);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC4, EM_LIMITTEXT, WIV_MAX_LICENSE_SECTION, 0);

			SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC1, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC2, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC3, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC4, WM_SETFONT, (WPARAM)m_hfExtraLightFont, (LPARAM)TRUE);

			SendDlgItemMessage (hWnd, IDC_REGISTRATION_STATIC_DASH1, WM_SETFONT, (WPARAM)m_hfBoldFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_STATIC_DASH2, WM_SETFONT, (WPARAM)m_hfBoldFont, (LPARAM)TRUE);
			SendDlgItemMessage (hWnd, IDC_REGISTRATION_STATIC_DASH3, WM_SETFONT, (WPARAM)m_hfBoldFont, (LPARAM)TRUE);

			EnableWindow(ItemHandleFromID(hWnd, IDC_REGISTRATION_BUTTON_OK), false);
//			SetFocus(ItemHandleFromID(hWnd, IDC_REGISTRATION_EDIT_LIC1));

			bResult = OnInitDialog(hWnd, m_ahbrushRegistration);

		    TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)bResult);

			return bResult;
		}
	case WM_CUT :
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_CUT received"));
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}
	case WM_COPY :
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_COPY received"));
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}
	case WM_PASTE :
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_PASTE received"));
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}
	case WM_DESTROY :
		{
			OnDestroy(hWnd, m_ahbrushRegistration);
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			UINT nID = LOWORD(wParam);
			UINT nCommand = HIWORD (wParam);

			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_COMMAND, ID = %08X, Command = %08X"), nID, nCommand);

			switch (nID)
			{
			case IDOK:
				{
					TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
					EndDialog(hWnd,IDOK);
					return 0;
				}
			case IDCANCEL:
				{
					TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
					EndDialog(hWnd,IDCANCEL);
					return 0;
				}
			case IDC_REGISTRATION_BUTTON_OK:
				{
					switch (nCommand)
					{
					case BN_CLICKED:
						{

							TCHAR	szLicSection[WIV_MAX_LICENSE_SECTION + 1];
							BYTE	bLicOld[sizeof(WIVLIC)];
							BYTE	bLicNew[sizeof(WIVLIC)];
							TCHAR	szLicNew[(WIV_MAX_LICENSE_SECTION * 4) + 1] = WIV_EMPTY_STRING;
							char	szLic[(WIV_MAX_LICENSE_SECTION * 4) + 1] = "";
							DWORD	dwSize;
							DWORD	dwRetID = IDOK;

							_zclr(szLicNew);
							_zclr(szLicSection);

							memcpy(bLicOld, WiV::GetLicense(), sizeof(WIVLIC));

							SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC1, WM_GETTEXT,
												  sizeof(szLicSection), (LPARAM)szLicSection);
							_tcsncpy(szLicNew, szLicSection, WIV_MAX_LICENSE_SECTION);
							TraceDetail(_D("CSSSOptions::DlgProcRegistration: IDC_REGISTRATION_BUTTON_OK: szLicSection1 = <%s>, szLicNew = <%s>"), szLicSection, szLicNew);

							SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC2, WM_GETTEXT,
												  sizeof(szLicSection), (LPARAM)szLicSection);
							_tcsncat(szLicNew, szLicSection, WIV_MAX_LICENSE_SECTION);
							TraceDetail(_D("CSSSOptions::DlgProcRegistration: IDC_REGISTRATION_BUTTON_OK: szLicSection2 = <%s>, szLicNew = <%s>"), szLicSection, szLicNew);

							SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC3, WM_GETTEXT,
												  sizeof(szLicSection), (LPARAM)szLicSection);
							_tcsncat(szLicNew, szLicSection, WIV_MAX_LICENSE_SECTION);
							TraceDetail(_D("CSSSOptions::DlgProcRegistration: IDC_REGISTRATION_BUTTON_OK: szLicSection3 = <%s>, szLicNew = <%s>"), szLicSection, szLicNew);

							SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC4, WM_GETTEXT,
												  sizeof(szLicSection), (LPARAM)szLicSection);
							_tcsncat(szLicNew, szLicSection, WIV_MAX_LICENSE_SECTION);
							TraceDetail(_D("CSSSOptions::DlgProcRegistration: IDC_REGISTRATION_BUTTON_OK: szLicSection4 = <%s>, szLicNew = <%s>"), szLicSection, szLicNew);
							
							UtoA(szLicNew, szLic, _tcslen(szLicNew));
							dwSize = strlen(szLic);

							// Encrypt license
							Crypt(NULL, (LPBYTE)szLic, bLicNew, &dwSize, true);
							TraceDetail(_D("CSSSOptions::DlgProcRegistration: Encrypted license = <%s>"), BtoS(bLicNew, sizeof(WIVLIC)));

							m_pPhone->GetEquipmentInfo(m_szIMEI);
							TraceDetail(_D("CSSSOptions::DlgProcRegistration: m_szIMEI = <%s>"), m_szIMEI);

							char	szIMEI[SSS_MAX_IMEI + 1];
							UtoA(m_szIMEI, szIMEI, SSS_MAX_IMEI);
							szIMEI[15] = 0x35;
							szIMEI[16] = 0x00;

							TraceDetail(_D("CSSSOptions::DlgProcRegistration: szIMEI = <%S>"), szIMEI);
							
							// Convert IMEI string to byte nibbles
							for (int j = 0, i = WIV_MAX_ENCRYPTED_LICENSE_LENGTH; j <= (int)strlen(szIMEI) - 1;)
							{
								unsigned short s1 = 0;
								unsigned short s2 = 0;

								s1 = ((szIMEI[j++] - 0x30) << 4);

								if (j <= (int)strlen(szIMEI) - 1)
								{
									s2 = szIMEI[j++] - 0x30;
								}

								bLicNew[i] = (s1 | s2) ^ bLicNew[i - WIV_MAX_ENCRYPTED_LICENSE_LENGTH];
								i++;

								if (j > (int)strlen(szIMEI) - 1)
									break;
							}

							TraceDetail(_D("CSSSOptions::DlgProcRegistration: Encrypted license = <%s>"), BtoS(bLicNew, sizeof(WIVLIC)));

							if (memcmp(bLicOld, bLicNew, WIV_MAX_LICENSE_LENGTH) != 0)
							{
								TraceDetail(_D("CSSSOptions::DlgProcRegistration: IDC_REGISTRATION_BUTTON_OK: Entered license differs, so process it"));
								WiV::SetLicense((LPWIVLIC)bLicNew, true);
								WiV::WriteLicense();
								dwRetID = IDYES;
							}
							else
							{
								TraceDetail(_D("CSSSOptions::DlgProcRegistration: IDC_REGISTRATION_BUTTON_OK: Entered license the same, so do nothing"));
							}

							// Force SIP down
							SHSipPreference (hWnd, SIP_FORCEDOWN);

							TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
							SetCursor(LoadCursor(NULL, IDC_WAIT));
							Sleep(2000);
							SetCursor(LoadCursor(NULL, IDC_ARROW));
							EndDialog(hWnd,dwRetID);
							return 0;
						}
					}
				}
			case IDC_REGISTRATION_BUTTON_CANCEL:
				{
					switch (nCommand)
					{
					case BN_CLICKED:
						{
							// Force SIP down
							SHSipPreference (hWnd, SIP_FORCEDOWN);

							TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
							EndDialog(hWnd,IDCANCEL);
							return 0;
						}
					}
				}
			case IDC_REGISTRATION_EDIT_LIC1:
			case IDC_REGISTRATION_EDIT_LIC2:
			case IDC_REGISTRATION_EDIT_LIC3:
			case IDC_REGISTRATION_EDIT_LIC4:
				{
					switch (nCommand)
					{
					case EN_CHANGE:
						{
							TCHAR szLic[WIV_MAX_LICENSE_SECTION + 1];
							TCHAR szLic1[WIV_MAX_LICENSE_SECTION + 1];
							TCHAR szLic2[WIV_MAX_LICENSE_SECTION + 1];
							TCHAR szLic3[WIV_MAX_LICENSE_SECTION + 1];
							TCHAR szLic4[WIV_MAX_LICENSE_SECTION + 1];
							int	  nLen;

							TraceInfo(_D("CSSSOptions::DlgProcRegistration: EN_CHANGE received for Registration:IDC_REGISTRATION_EDIT_LICx"));

							SendDlgItemMessage (hWnd, nID, WM_GETTEXT,
												  sizeof(szLic), (LPARAM)szLic);

							if (_tcslen(szLic) >= WIV_MAX_LICENSE_SECTION)
							{
								UINT	unToID = IDC_REGISTRATION_EDIT_LIC1;

								if (nID == IDC_REGISTRATION_EDIT_LIC1)
								{
									unToID = IDC_REGISTRATION_EDIT_LIC2;

								}else if (nID == IDC_REGISTRATION_EDIT_LIC2)
								{
									unToID = IDC_REGISTRATION_EDIT_LIC3;

								}else if (nID == IDC_REGISTRATION_EDIT_LIC3)
								{
									unToID = IDC_REGISTRATION_EDIT_LIC4;

								}else if (nID == IDC_REGISTRATION_EDIT_LIC4)
								{
									unToID = IDC_REGISTRATION_EDIT_LIC1;
								}

								SetFocus(ItemHandleFromID(hWnd, unToID));
							}

							SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC1, WM_GETTEXT,
												  sizeof(szLic1), (LPARAM)szLic1);
							SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC2, WM_GETTEXT,
												  sizeof(szLic2), (LPARAM)szLic2);
							SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC3, WM_GETTEXT,
												  sizeof(szLic3), (LPARAM)szLic3);
							SendDlgItemMessage (hWnd, IDC_REGISTRATION_EDIT_LIC4, WM_GETTEXT,
												  sizeof(szLic4), (LPARAM)szLic4);
						
							nLen = _tcslen(szLic1) + _tcslen(szLic2) + _tcslen(szLic3) + _tcslen(szLic4);

							EnableWindow(ItemHandleFromID(hWnd, IDC_REGISTRATION_BUTTON_OK),
								(nLen < WIV_MAX_LIC_STRING_LENGTH) ? false : true);
							
							TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
							return 0;
						}
					case EN_SETFOCUS:
						{
							TraceInfo(_D("CSSSOptions::DlgProcRegistration: EN_SETFOCUS received for Registration:IDC_REGISTRATION_EDIT_LICx"));

							SendDlgItemMessage (hWnd, nID, EM_SETSEL, 0, -1);

							// SIP up
							SHSipPreference (hWnd, SIP_UP);

							TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
							return 0;
						}
					case EN_KILLFOCUS:
						{
							TraceInfo(_D("CSSSOptions::DlgProcRegistration: EN_KILLFOCUS received for Registration:IDC_REGISTRATION_EDIT_LICx"));

							SendDlgItemMessage (hWnd, nID, EM_SETSEL, -1, 0);

							// SIP down
							SHSipPreference (hWnd, SIP_DOWN);

							TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
							return 0;
						}
					}
					break;
				}
			}
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}
	case WM_ACTIVATE :
		{
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_NOTIFY, code = %08X, hwndFrom = %08X, idFrom = %08X, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;//OnNotify(wParam, pNMHDR);
		}
	case WM_PAINT :
		{
			PAINTSTRUCT ps;
			HDC			hDC;
			TCHAR		szInfo[WIV_MAX_STRING*2 + 1] = WIV_EMPTY_STRING;

			hDC = BeginPaint(hWnd, &ps);

			OnPaint(hWnd, hDC, true);

			EndPaint(hWnd, &ps);

			if (IsSecondEdition() && InWideMode())
			{
				_snwprintf(szInfo, WIV_MAX_STRING*2, _T("%s %s."), SSS_TEXT_REGISTRATION_CAREFUL, g_szCompanyName);
				DrawStaticText(hWnd, IDC_REGISTRATION_STATIC_HEADER, szInfo, DT_CENTER | DT_EXPANDTABS | DT_WORDBREAK, FW_SEMIBOLD, WIV_FONT_MEDIUM);

				_snwprintf(szInfo, WIV_MAX_STRING*2, _T("%s."), SSS_TEXT_REGISTRATION_OK_TO_RETURN);
				DrawStaticText(hWnd, IDC_REGISTRATION_STATIC_DETAILS, szInfo, DT_CENTER| DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL);

				_snwprintf(szInfo, WIV_MAX_STRING*2, _T("%s %s %s."), SSS_TEXT_REGISTRATION_TAKE_EFFECT,
					g_szProductName, SSS_TEXT_REGISTRATION_TODAY_PLUGIN);

				DrawStaticText(hWnd, IDC_REGISTRATION_STATIC_DETAILS2, szInfo, DT_CENTER| DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL, WIV_FONT_MEDIUM);
//				_snwprintf(szInfo, WIV_MAX_STRING*2, _T("%s %s."), SSS_TEXT_REGISTRATION_CAREFUL, g_szCompanyName);
//				DrawStaticText(hWnd, IDC_REGISTRATION_STATIC_HEADER, szInfo, DT_CENTER | DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL, WIV_FONT_MEDIUM);

//				_snwprintf(szInfo, WIV_MAX_STRING*2, _T("%s. %s %s %s."),
//					SSS_TEXT_REGISTRATION_OK_TO_RETURN, SSS_TEXT_REGISTRATION_TAKE_EFFECT, g_szProductName, SSS_TEXT_REGISTRATION_TODAY_PLUGIN);

//				DrawStaticText(hWnd, IDC_REGISTRATION_STATIC_DETAILS, szInfo, DT_CENTER| DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL, WIV_FONT_MEDIUM);
			}
			else
			{
				_snwprintf(szInfo, WIV_MAX_STRING*2, _T("%s %s."), SSS_TEXT_REGISTRATION_CAREFUL, g_szCompanyName);
				DrawStaticText(hWnd, IDC_REGISTRATION_STATIC_HEADER, szInfo, DT_CENTER | DT_EXPANDTABS | DT_WORDBREAK, FW_SEMIBOLD);

				_snwprintf(szInfo, WIV_MAX_STRING*2, _T("%s."), SSS_TEXT_REGISTRATION_OK_TO_RETURN);
				DrawStaticText(hWnd, IDC_REGISTRATION_STATIC_DETAILS, szInfo, DT_CENTER| DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL);

				_snwprintf(szInfo, WIV_MAX_STRING*2, _T("%s %s %s."), SSS_TEXT_REGISTRATION_TAKE_EFFECT,
					g_szProductName, SSS_TEXT_REGISTRATION_TODAY_PLUGIN);

				DrawStaticText(hWnd, IDC_REGISTRATION_STATIC_DETAILS2, szInfo, DT_CENTER| DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL);
			}

            SendDlgItemMessage (hWnd, IDC_REGISTRATION_BUTTON_OK, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_OK);
            SendDlgItemMessage (hWnd, IDC_REGISTRATION_BUTTON_CANCEL, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_CANCEL);

			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}

	case WM_ERASEBKGND:
		{
			bResult = OnEraseBackground(hWnd, (HDC)wParam);
		    TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)bResult);

			return bResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_CTLCOLORDLG received for Registration"));
			bResult = (BOOL)OnCtlColor(m_ahbrushRegistration, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_CTLCOLORBTN received for Registration"));
			bResult = (BOOL)OnCtlColor(m_ahbrushRegistration, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_CTLCOLORSTATIC received for Registration"));
			bResult = (BOOL)OnCtlColor(m_ahbrushRegistration, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)bResult);
			return bResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_CTLCOLOREDIT received for Registration"));
			bResult = (BOOL)OnCtlColor(m_ahbrushRegistration, (HWND)lParam);
			SetTextColor((HDC)wParam,(!OptionsFlags.OptionsBits.RegType ? g_crGreenOK : g_crRedError));
			if (!bResult) bResult = 1;

		    TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)bResult);
			return bResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_CTLCOLORLISTBOX received for Registration"));
			bResult = (BOOL)OnCtlColor(m_ahbrushRegistration, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)bResult);
			return bResult;
        }

	case WM_SIZE:
		{
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_REGISTRATION_DIALOG_WIDE) :
					MAKEINTRESOURCE(IDD_REGISTRATION_DIALOG));

			break;
		}

	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcRegistration: WM_WIV_REFRESH received for Registration"));
			TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
			return 0;
		}
	case WM_HELP:
		{
			TraceInfo(_D("CSSSOptions::DlgProcRegistration: WM_HELP received for Registration"));

			DoShowHelp(g_szHelpTagRegistrationDialog);

			break;
		}

	default :
		TraceLeave(_D("CSSSOptions::DlgProcRegistration"), (DWORD)0);
		return 0;//OnMessage(uMsg, wParam, lParam);
	}

	lrResult = DefWindowProc(hWnd, uMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcRegistration"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// DlgProcInformation - Device/SIM Information dialog box procedure
//======================================================================
LRESULT CSSSOptions::DlgProcInformation(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT	lrResult;
	BOOL	bResult;

    TraceEnter(_D("CSSSOptions::DlgProcInformation"));

	switch (uMsg)
	{
	case WM_INITDIALOG :
		{
			SHINITDLGINFO	shidi;

			m_hwInfoDialog = hWnd;
			
			// Force SIP down
			SHSipPreference (hWnd, SIP_FORCEDOWN);

			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_EMPTYMENU | SHIDIF_SIZEDLGFULLSCREEN;

			shidi.hDlg = hWnd;
			SHInitDialog(&shidi);

			SHMENUBARINFO mbi;
			memset(&mbi, 0, sizeof(SHMENUBARINFO));
			mbi.cbSize = sizeof(SHMENUBARINFO);
			mbi.hwndParent = hWnd;
			mbi.nToolBarId = NULL;
			mbi.hInstRes = m_hmInstance;
			mbi.dwFlags = SHCMBF_EMPTYBAR;
			SHCreateMenuBar(&mbi);
			
			SendDlgItemMessage (hWnd, IDC_INFORMATION_BUTTON_CANCEL, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);

			bResult = OnInitDialog(hWnd, m_ahbrushInformation);

			TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)bResult);

			return bResult;
		}
	case WM_DESTROY :
		{
			OnDestroy(hWnd, m_ahbrushInformation);

			m_hwInfoDialog = NULL;
			
			TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			UINT nID = LOWORD(wParam);
			UINT nCommand = HIWORD (wParam);

			switch (nID)
			{
			case IDOK:
				{
					TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
					EndDialog(hWnd,0);
					return 0;
				}
			case IDCANCEL:
				{
					TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
					EndDialog(hWnd,0);
					return 0;
				}
			case IDC_INFORMATION_BUTTON_CANCEL:
				{
					switch (nCommand)
					{
					case BN_CLICKED:
						{
							TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
							EndDialog(hWnd,0);
							return 0;
						}
					}
				}
			}
			TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
			return 0;
		}
	case WM_ACTIVATE :
		{
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
			TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{

			TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
			return 0;//OnNotify(wParam, pNMHDR);
		}
	case WM_PAINT :
		{
			PAINTSTRUCT ps;
			HDC			hDC;
			TCHAR		szInfo[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
			COLORREF	crPhonebook;

			hDC = BeginPaint(hWnd, &ps);

			OnPaint(hWnd, hDC, true);

			EndPaint(hWnd, &ps);

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_DEVICE_INFORMATION);
			DrawStaticText(hWnd, IDC_INFORMATION_STATIC_HEADER1, szInfo, DT_LEFT, FW_BOLD);

			if (IsSecondEdition() && InWideMode())
			{
				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:\r\n%s:\r\n%s:\r\n%s:"),
					SSS_TEXT_MANUFACTURER, SSS_TEXT_MODEL, SSS_TEXT_REVISION, SSS_TEXT_IMEI_NUMBER);

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_DEVICE_LABELS, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL, WIV_FONT_MEDIUM);

				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s\r\n%s\r\n%s\r\n%15.15s"), m_szMfg, m_szModel, m_szRevision, m_szIMEI);

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_DEVICE_DATA, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_SEMIBOLD, WIV_FONT_MEDIUM, g_crBlueInfo);
			}
			else
			{
				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:\r\n%s:\r\n%s:\r\n%s:"),
					SSS_TEXT_MANUFACTURER, SSS_TEXT_MODEL, SSS_TEXT_REVISION, SSS_TEXT_IMEI_NUMBER);

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_DEVICE_LABELS, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK);

				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s\r\n%s\r\n%s\r\n%15.15s"), m_szMfg, m_szModel, m_szRevision, m_szIMEI);

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_DEVICE_DATA, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_SEMIBOLD, WIV_FONT_NONE, g_crBlueInfo);
			}

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_SIM_INFORMATION);
			DrawStaticText(hWnd, IDC_INFORMATION_STATIC_HEADER2, szInfo, DT_LEFT, FW_BOLD);

			if (IsSecondEdition() && InWideMode())
			{
				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:\r\n%s:\r\n%s:"),
					SSS_TEXT_TELEPHONE_NO, SSS_TEXT_IMSI_NUMBER, SSS_TEXT_ICCID_NUMBER);

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_SIM_LABELS, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL, WIV_FONT_MEDIUM);

				if (OptionsFlags.OptionsBits.HidePersonalInfo)
				{
					_snwprintf(szInfo, WIV_MAX_STRING, _T("%s\r\n%s\r\n%13.17s"),
						m_pPhone->GetPhoneNumber(), g_szFakeUserID, g_szFakeICCID);
				}
				else
				{
					_snwprintf(szInfo, WIV_MAX_STRING, _T("%s\r\n%s\r\n%13.17s"),
						m_pPhone->GetPhoneNumber(), m_szSubscriber, &m_szICCID[2]);
				}

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_SIM_DATA, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_SEMIBOLD, WIV_FONT_MEDIUM, g_crBlueInfo);
			}
			else
			{
				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:\r\n%s:\r\n%s:"),
					SSS_TEXT_TELEPHONE_NO, SSS_TEXT_IMSI_NUMBER, SSS_TEXT_ICCID_NUMBER);

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_SIM_LABELS, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK);

				if (OptionsFlags.OptionsBits.HidePersonalInfo)
				{
					_snwprintf(szInfo, WIV_MAX_STRING, _T("%s\r\n%s\r\n%13.17s"),
						m_pPhone->GetPhoneNumber(), g_szFakeUserID, g_szFakeICCID);
				}
				else
				{
					_snwprintf(szInfo, WIV_MAX_STRING, _T("%s\r\n%s\r\n%13.17s"),
						m_pPhone->GetPhoneNumber(), m_szSubscriber, &m_szICCID[2]);
				}

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_SIM_DATA, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_SEMIBOLD, WIV_FONT_NONE, g_crBlueInfo);
			}

			_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:"), SSS_TEXT_ABOUT_PHONEBOOK);
			DrawStaticText(hWnd, IDC_INFORMATION_STATIC_HEADER3, szInfo, DT_LEFT, FW_BOLD);

			if (IsSecondEdition() && InWideMode())
			{

				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:\r\n%s:\r\n%s:"), SSS_TEXT_ABOUT_LOCATION, SSS_TEXT_ABOUT_TOTAL_ENTRIES, SSS_TEXT_ABOUT_USED_ENTRIES);
				
				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_PB_LABELS, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_NORMAL, WIV_FONT_MEDIUM);

				if (OptionsFlags.OptionsBits.SIMReady)
				{
					_snwprintf(szInfo, WIV_MAX_STRING, _T("%s\r\n%d\r\n%d"), m_szPBLocation, m_dwPBTotal, m_dwPBUsed);
					crPhonebook = g_crBlueInfo;
				}
				else
				{
					_snwprintf(szInfo, WIV_MAX_STRING, SSS_TEXT_PHONEBOOK_UNAVAILABLE);
					crPhonebook = g_crAmberWarning;
				}

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_PB_DATA, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_SEMIBOLD, WIV_FONT_MEDIUM, crPhonebook);
			}
			else
			{

				_snwprintf(szInfo, WIV_MAX_STRING, _T("%s:\r\n%s:\r\n%s:"), SSS_TEXT_ABOUT_LOCATION, SSS_TEXT_ABOUT_TOTAL_ENTRIES, SSS_TEXT_ABOUT_USED_ENTRIES);
				
				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_PB_LABELS, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK);

				if (OptionsFlags.OptionsBits.SIMReady)
				{
					_snwprintf(szInfo, WIV_MAX_STRING, _T("%s\r\n%d\r\n%d"), m_szPBLocation, m_dwPBTotal, m_dwPBUsed);
					crPhonebook = g_crBlueInfo;
				}
				else
				{
					_snwprintf(szInfo, WIV_MAX_STRING, SSS_TEXT_PHONEBOOK_UNAVAILABLE);
					crPhonebook = g_crAmberWarning;
				}

				DrawStaticText(hWnd, IDC_INFORMATION_STATIC_PB_DATA, szInfo,
									DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK, FW_SEMIBOLD, WIV_FONT_NONE, crPhonebook);
			}

            SendDlgItemMessage (hWnd, IDC_INFORMATION_BUTTON_CANCEL, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_CANCEL);

			TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
			return 0;
		}

	case WM_ERASEBKGND:
		{
			bResult = OnEraseBackground(hWnd, (HDC)wParam);
		    TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)bResult);

			return bResult;
		}

    case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcInformation: WM_CTLCOLORDLG received for Information"));
			bResult = (BOOL)OnCtlColor(m_ahbrushInformation, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcInformation: WM_CTLCOLORBTN received for Information"));
			bResult = (BOOL)OnCtlColor(m_ahbrushInformation, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcInformation: WM_CTLCOLORSTATIC received for Information"));
			bResult = (BOOL)OnCtlColor(m_ahbrushInformation, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)bResult);
			return bResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcInformation: WM_CTLCOLOREDIT received for Information"));
			bResult = (BOOL)OnCtlColor(m_ahbrushInformation, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)bResult);
			return bResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcInformation: WM_CTLCOLORLISTBOX received for Information"));
			bResult = (BOOL)OnCtlColor(m_ahbrushInformation, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)bResult);
			return bResult;
        }

	case WM_SIZE:
		{
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_INFORMATION_DIALOG_WIDE) :
					MAKEINTRESOURCE(IDD_INFORMATION_DIALOG));
			break;
		}

	case WM_HELP:
		{
			TraceDetail(_D("CSSSOptions::DlgProcInformation: WM_HELP received for Information"));

			DoShowHelp(g_szHelpTagInformationDialog);

			break;
		}

	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcInformation: WM_WIV_REFRESH received for Information"));
			
			m_pPhone->GetEquipmentInfo(m_szIMEI, m_szMfg, m_szModel, m_szRevision);
			m_pPhone->GetSIMInfo(m_szSubscriber, m_szPBLocation, &m_dwPBTotal, &m_dwPBUsed);
			RefreshWindow(hWnd);
			
			TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
			return 0;
		}

	default :
		TraceLeave(_D("CSSSOptions::DlgProcInformation"), (DWORD)0);
		return 0;//OnMessage(uMsg, wParam, lParam);
	}

	lrResult = DefWindowProc(hWnd, uMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcInformation"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// Non-member functions
//======================================================================

//======================================================================
// OptionsPropSheetProc - Function called when Property sheet created
//======================================================================
int CALLBACK OptionsPropSheetProc (HWND hDlg, UINT uMsg, LPARAM lParam)
{
	return m_pThis->PropSheetProc(hDlg, uMsg, lParam);
}

//======================================================================
// OptionsDlgProcAbout - About page dialog box procedure
//======================================================================
BOOL CALLBACK OptionsDlgProcAbout (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();

	return m_pThis->DlgProcAbout(hWnd, wMsg, wParam, lParam);
}

//======================================================================
// OptionsDlgProcActions - Actions options page dialog box procedure
//======================================================================
BOOL CALLBACK OptionsDlgProcActions (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();

	return m_pThis->DlgProcActions(hWnd, wMsg, wParam, lParam);
}

//======================================================================
// OptionsDlgProcAppearance - Appearance options page dialog box procedure
//======================================================================
BOOL CALLBACK OptionsDlgProcAppearance (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();

	return m_pThis->DlgProcAppearance(hWnd, wMsg, wParam, lParam);
}

//======================================================================
// OptionsDlgProcDisplay - Display options page dialog box procedure
//======================================================================
BOOL CALLBACK OptionsDlgProcDisplay (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();

	return m_pThis->DlgProcDisplay(hWnd, wMsg, wParam, lParam);
}

//======================================================================
// OptionsDlgProcLanguage - Language options page dialog box procedure
//======================================================================
BOOL CALLBACK OptionsDlgProcLanguage (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();

	return m_pThis->DlgProcLanguage(hWnd, wMsg, wParam, lParam);
}

//======================================================================
// OptionsDlgProcSecurity - Security options page dialog box procedure
//======================================================================
BOOL CALLBACK OptionsDlgProcSecurity (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();

	return m_pThis->DlgProcSecurity(hWnd, wMsg, wParam, lParam);
}

//======================================================================
// OptionsDlgProcPINEntry - PIN entry dialog box procedure
//======================================================================
LRESULT CALLBACK OptionsDlgProcPINEntry(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();
	
	return m_pThis->DlgProcPINEntry(hWnd, uMsg, wParam, lParam);
}

//======================================================================
// OptionsDlgProcRegistration - Registration details dialog box procedure
//======================================================================
LRESULT CALLBACK OptionsDlgProcRegistration(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();

	return m_pThis->DlgProcRegistration(hWnd, uMsg, wParam, lParam);
}

//======================================================================
// OptionsDlgProcInformation - Information dialog box procedure
//======================================================================
LRESULT CALLBACK OptionsDlgProcInformation(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_pThis->SetupPhoneState();

	return m_pThis->DlgProcInformation(hWnd, uMsg, wParam, lParam);
}

bool CALLBACK LicenseOptionsNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig)
{
	return m_pThis->OptionsLicenseNotify(lpLicenseData, dwLicenseConfig);
}

//*******************************************************************************/
// Debug Functions
//*******************************************************************************/

#ifdef WIV_DEBUG

//======================================================================
// GetDebugOptions - Read debug options from the registry
//======================================================================
DWORD CSSSOptions::GetDebugOptions()
{
	DWORD dwResult;
	HKEY hKey;

	TraceEnter(_D("GetDebugOptions"));

	// Get debug settings from registry
	TraceInfo(_D("GetDebugOptions: Getting debug options from registry"));

	// Open the Internal Key [HKEY_LOCAL_MACHINE\SOFTWARE\WiViT\SIM Status Switcher\Internal]
	TraceDetail(_D("GetDebugOptions: Calling RegKeyOpen for Internal key"));
	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);
	TraceDetail(_D("GetDebugOptions: Back from RegKeyOpen, dwResult = <%08x>, hKey = <%08x>"), dwResult, hKey);

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("GetDebugOptions: Error opening Internal key, dwResult = <%08x>"), dwResult);
		TraceLeave(_D("GetDebugOptions"), dwResult);
		return dwResult;
	}

	// Trace active flag with default of FALSE "TraceActive"=dword:00000000
	TraceDetail(_D("GetDebugOptions: Calling RegGetValue for TraceActive"));
	m_blTraceActive = RegGetValue(hKey, WIV_REG_TRACE_ACTIVE, SSS_DEFAULT_TRACE_ACTIVE);
	CurrentSettings.blTraceActive = m_blTraceActive;
	TraceDetail(_D("GetDebugOptions: Back from RegGetValue, TraceActive = <%X>"), m_blTraceActive);

	// Debug trace level value with default of 0 (TraceNone) "TraceLevel"=dword:00000000
	TraceDetail(_D("GetDebugOptions: Calling RegGetValue for TraceLevel"));
	m_tlTraceLevel = (TraceLevel)RegGetValue(hKey, WIV_REG_TRACE_LEVEL, (DWORD)SSS_DEFAULT_TRACE_LEVEL);
	CurrentSettings.tlTraceLevel = m_tlTraceLevel;
	TraceDetail(_D("GetDebugOptions: Back from RegGetValue, TraceLevel = <%X>"), m_tlTraceLevel);

	// Trace path with default of "\\" "TracePath"="\\"
	TraceDetail(_D("GetDebugOptions: Calling RegGetValue for TracePath"));
	TCHAR	szPath[MAX_PATH +1] = WIV_EMPTY_STRING;
	_snwprintf(szPath, MAX_PATH, _D("%s\\Trace"), SSS_DEFAULT_TRACE_PATH);
	dwResult = RegGetValue(hKey, WIV_REG_TRACE_PATH, m_szTracePath, WIV_MAX_PATH, szPath);
	_tcsncpy(CurrentSettings.szTracePath, m_szTracePath, WIV_MAX_PATH);
	TraceDetail(_D("GetDebugOptions: Back from RegGetValue, TracePath = <%s>"), m_szTracePath);

	TraceInfo(_D("GetDebugOptions: Debug options read from registry"));

	TraceDetail(_D("GetDebugOptions: TraceActive = <%X>"), m_blTraceActive);
	TraceDetail(_D("GetDebugOptions: TraceLevel = <%X>"), m_tlTraceLevel);
	TraceDetail(_D("GetDebugOptions: TracePath = <%s>"), m_szTracePath);

	// Close the Internal Key
	TraceDetail(_D("GetDebugOptions: Calling RegKeyClose"));
	dwResult = RegKeyClose(hKey);
	TraceDetail(_D("GetDebugOptions: Back from RegKeyClose, dwResult = <%08x>"), dwResult);

	TraceLeave(_D("GetDebugOptions"), dwResult);
	return dwResult;
}

//======================================================================
// SetDebugOptions - Write debug options to the registry
//======================================================================
DWORD CSSSOptions::SetDebugOptions()
{
	DWORD	dwResult;
	HKEY	hKey;

	TraceEnter(_D("SetDebugOptions"));

	// Write debug settings to registry
	TraceInfo(_D("SetDebugOptions: Writing debug options to registry"));

	TraceDetail(_D("SetDebugOptions: TraceActive = <%X>"), m_blTraceActive);
	TraceDetail(_D("SetDebugOptions: TraceLevel = <%X>"), m_tlTraceLevel);
	TraceDetail(_D("SetDebugOptions: TracePath = <%s>"), m_szTracePath);


	// Open the Internal Key [HKEY_LOCAL_MACHINE\SOFTWARE\WiViT\SIM Status Switcher\Internal]
	TraceDetail(_D("SetDebugOptions: Calling RegKeyOpen for Internal key"));
	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);
	TraceDetail(_D("SetDebugOptions: Back from RegKeyOpen, dwResult = <%08x>, hKey = <%08x>"), dwResult, hKey);

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("SetDebugOptions: Error opening Internal key, dwResult = <%08x>"), dwResult);
		TraceLeave(_D("SetDebugOptions"), dwResult);
		return dwResult;
	}

	// Trace active flag
	TraceDetail(_D("SetDebugOptions: Calling RegSetValue for TraceActive = <%X>"), m_blTraceActive);
	dwResult = RegSetValue(hKey, WIV_REG_TRACE_ACTIVE, m_blTraceActive);
	TraceDetail(_D("SetDebugOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Debug trace level value
	TraceDetail(_D("SetDebugOptions: Calling RegSetValue for TraceLevel = <%X>"), m_tlTraceLevel);
	dwResult = RegSetValue(hKey, WIV_REG_TRACE_LEVEL, (DWORD)m_tlTraceLevel);
	TraceDetail(_D("SetDebugOptions: Back from RegSetValue, dwResult = <%08x"), dwResult);

	// Trace path value
	TraceDetail(_D("SetDebugOptions: Calling RegSetValue for TracePath = <%s>"), m_szTracePath);
	dwResult = RegSetValue(hKey, WIV_REG_TRACE_PATH, m_szTracePath);
	TraceDetail(_D("SetDebugOptions: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	
	// Close the Internal Key
	TraceDetail(_D("SetDebugOptions: Calling RegKeyClose"));
	dwResult = RegKeyClose(hKey);
	TraceDetail(_D("SetDebugOptions: Back from RegKeyClose, dwResult = <%08x>"), dwResult);

	TraceLeave(_D("SetDebugOptions"), dwResult);
	return dwResult;
}

//======================================================================
// BrowseTracePath - Process browse for trace path
//======================================================================
LPARAM CSSSOptions::BrowseTracePath (HWND hWnd, LPTSTR lpszPath)
{
	TraceEnter(_D("CSSSOptions::BrowseTracePath"));
	TraceInfo(_D("CSSSOptions::BrowseTracePath: Initial path = <%s>"), lpszPath);

    OPENFILENAME	ofOpenFile;
    TCHAR			szFileName [MAX_PATH + 1]			= {0};
    TCHAR			szFolderName [MAX_PATH + 1]			= {0};
    TCHAR			szInitialFolderName [MAX_PATH + 1];
    TCHAR			szOpenFilter[MAX_PATH + 1];
    int				nResult;
	TCHAR			szPath[MAX_PATH +1] = WIV_EMPTY_STRING;

	_snwprintf(szPath, MAX_PATH, _D("%s\\Trace"), SSS_DEFAULT_TRACE_PATH);
	
	
	_tcsncpy(szInitialFolderName, szPath, MAX_PATH);

	_snwprintf(szOpenFilter, MAX_PATH, _T(" %s (*.*)\\0*.*\\0\\0"), SSS_TEXT_FILTER_ALL_FILES);

    memset (&ofOpenFile, 0, sizeof (ofOpenFile));   // Initialize File Open structure.

    ofOpenFile.lStructSize		= sizeof (ofOpenFile);
    ofOpenFile.hwndOwner		= hWnd;
    ofOpenFile.lpstrFile		= szFileName;
    ofOpenFile.nMaxFile			= MAX_PATH;
    ofOpenFile.lpstrFilter		= szOpenFilter;
	ofOpenFile.lpstrInitialDir	= szInitialFolderName;
    ofOpenFile.Flags			= OFN_PROJECT;
	ofOpenFile.lpstrFileTitle	= szFolderName;

    nResult = GetOpenFileName (&ofOpenFile);
	
	if ((_tcslen(szFolderName) == 0) || (nResult == 0))
	{
		TraceLeave(_D("CSSSOptions::BrowseTracePath: Nothing selected"), (DWORD)1);
		return 1;
	}

	_tcsncpy(lpszPath, szFileName, MAX_PATH);

	TraceInfo(_D("CSSSOptions::BrowseTracePath: GetOpenFileName returned: %x, folder = <%s>, file = <%s>"),
              nResult, szFolderName, szFileName);

	TraceLeave(_D("CSSSOptions::BrowseTracePath"), (DWORD)0);

    return 0;
}

//======================================================================
// ListTraceFiles - List all trace files within a folder
//======================================================================
int CSSSOptions::ListTraceFiles (HWND hWnd, UINT uID, LPTSTR pszDir)
{
    WIN32_FIND_DATA fdFindData;
    TCHAR			szFilePrefix[MAX_PATH + 1];
    int				nResult;
    HANDLE			hFind;
	int				nFiles = 0;

	TraceEnter(_D("CSSSOptions::ListTraceFiles"));

	_tcsncpy(szFilePrefix, pszDir, MAX_PATH);
	_tcsncat(szFilePrefix, SSS_TRACE_PREFIX, MAX_PATH - _tcslen(szFilePrefix));
	_tcsncat(szFilePrefix, _T("_*.*"), MAX_PATH - _tcslen(szFilePrefix));

	// Empty list box
	SendDlgItemMessage (hWnd, uID, LB_RESETCONTENT, 0, 0);

    // Find matching files.
    hFind = FindFirstFile (szFilePrefix, &fdFindData);
    if (hFind != INVALID_HANDLE_VALUE)
	{

        do {
            // Report all matching files.
            if (!(fdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				TraceInfo(_D("Adding <%s> to trace file list"), fdFindData.cFileName);
                SendDlgItemMessage (hWnd, uID, LB_ADDSTRING, 0, (LPARAM)fdFindData.cFileName);
                nFiles++;
            }
            nResult = FindNextFile (hFind, &fdFindData);
        } while (nResult);

        FindClose (hFind);
    }
	else
	{
        nResult = GetLastError();
        if ((nResult != ERROR_FILE_NOT_FOUND)  && 
            (nResult != ERROR_NO_MORE_FILES))
		{
//            SendDlgItemMessage (hWnd, uID, LB_ADDSTRING, 0, (LPARAM)_T("Invalid Path"));
			TraceLeave(_D("CSSSOptions::ListTraceFiles"), -1);
            return -1;
        }
    }

	TraceLeave(_D("CSSSOptions::ListTraceFiles"), nFiles);

    return nFiles;
}

//======================================================================
// LoadTraceLevelsArray - Load array with defined trace levels
//======================================================================
void CSSSOptions::LoadTraceLevelsArray()
{
	_tcsncpy(m_aszTraceLevels[0], SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_NONE, SSS_MAX_TRACE_LEVEL_NAME_LENGTH);
	_tcsncpy(m_aszTraceLevels[1], SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_FATAL, SSS_MAX_TRACE_LEVEL_NAME_LENGTH);
	_tcsncpy(m_aszTraceLevels[2], SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_ERROR, SSS_MAX_TRACE_LEVEL_NAME_LENGTH);
	_tcsncpy(m_aszTraceLevels[3], SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_WARNING, SSS_MAX_TRACE_LEVEL_NAME_LENGTH);
	_tcsncpy(m_aszTraceLevels[4], SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_INFO, SSS_MAX_TRACE_LEVEL_NAME_LENGTH);
	_tcsncpy(m_aszTraceLevels[5], SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_DEBUG, SSS_MAX_TRACE_LEVEL_NAME_LENGTH);
	_tcsncpy(m_aszTraceLevels[6], SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_DETAIL, SSS_MAX_TRACE_LEVEL_NAME_LENGTH);

	return;
}



//======================================================================
// InitialiseTraceLevelsList - Load list box with stored trace levels
//======================================================================
void CSSSOptions::InitialiseTraceLevelsList(HWND hWnd)
{
	int		i;

	TraceEnter(_D("CSSSOptions::InitialiseTraceLevelsList"));

	// List available trace levels into combo box.
	for (i = 0; i < SSS_MAX_AVAILABLE_TRACE_LEVELS; i++)
	{
		TraceInfo(_D("CSSSOptions::InitialiseTraceLevelsList: Adding <%s> to Debug:IDC_DEBUG_COMBO_TRACE_LEVELS"), m_aszTraceLevels[i]);
		SendDlgItemMessage (hWnd, IDC_DEBUG_COMBO_TRACE_LEVELS, CB_ADDSTRING,
							0, (LPARAM)m_aszTraceLevels[i]);
	}

	TraceLeave(_D("CSSSOptions::InitialiseTraceLevelsList"));

	return;
}

//======================================================================
// Property page window procedures
//======================================================================
//======================================================================
// DlgProcDebug - Debug options page dialog box procedure
//======================================================================
BOOL CSSSOptions::DlgProcDebug (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwndMain;
	LRESULT	lrResult;
	BOOL	bResult;

    TraceEnter(_D("CSSSOptions::DlgProcDebug"));

	switch (wMsg)
	{
	case WM_INITDIALOG :
		{
			m_hwDebugWnd = hWnd;

			// The generic parameter contains the
			// top-level window handle.
			hwndMain = (HWND)((LPPROPSHEETPAGE)lParam)->lParam;

			// Save the window handle in the window structure.
			SetWindowLong (hWnd, DWL_USER, (LONG)hwndMain);

			SendDlgItemMessage (hWnd, IDC_DEBUG_BUTTON_BROWSE, WM_SETFONT, (WPARAM)m_hfSemiBoldFont, (LPARAM)TRUE);

            SendDlgItemMessage (hWnd, IDC_DEBUG_EDIT_TRACE_PATH, WM_SETTEXT,
                                  0, (LPARAM)m_szTracePath);

			// List existing trace files in list box.
			m_nNumTraceFiles = ListTraceFiles(hWnd, IDC_DEBUG_LIST_TRACE_FILES, m_szTracePath);

			if (m_nNumTraceFiles == 0)
			{
	            SendDlgItemMessage (hWnd, IDC_DEBUG_LIST_TRACE_FILES, LB_ADDSTRING, 0, (LPARAM)SSS_TEXT_NO_TRACE_FILES);
			}
			else if (m_nNumTraceFiles < 0)
			{
	            SendDlgItemMessage (hWnd, IDC_DEBUG_LIST_TRACE_FILES, LB_ADDSTRING, 0, (LPARAM)SSS_TEXT_INVALID_PATH);
				m_nNumTraceFiles = 0;
			}

			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_INITDIALOG in Debug, m_blTraceActive=%08x"),	m_blTraceActive);
			
			SetCheckButton(hWnd, IDC_DEBUG_CHECK_TRACE_ACTIVE, m_blTraceActive);

			// Enable/Disable trace file list
			EnableWindow(ItemHandleFromID(hWnd, IDC_DEBUG_LIST_TRACE_FILES), (m_nNumTraceFiles == 0)?false:true); 

			// Set up the trace levels lists
			InitialiseTraceLevelsList(hWnd);

			// Set current selection for trace levels list.
			SendDlgItemMessage (hWnd, IDC_DEBUG_COMBO_TRACE_LEVELS, CB_SETCURSEL, m_tlTraceLevel, 0);

			bResult = OnInitDialog(hWnd, m_ahbrushDebug);

		    TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)bResult);

			return bResult;

		}
	case WM_DESTROY :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_DESTROY received for Debug"));
			OnDestroy(hWnd, m_ahbrushDebug);
			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
	case WM_COMMAND :
		{
			UINT uID = LOWORD (wParam);
			UINT uCommand = HIWORD (wParam);

			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_COMMAND received for Debug, ID = %08X (%d), Command = %08X (%d)"),
				uID, uID, uCommand, uCommand);

			switch (uID)
			{
			case IDC_DEBUG_CHECK_TRACE_ACTIVE:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcDebug: BN_CLICKED received for Debug:IDC_DEBUG_CHECK_TRACE_ACTIVE"));

							// Get checked state of Trace Active check box
							m_blTraceActive = IsButtonChecked(hWnd, IDC_DEBUG_CHECK_TRACE_ACTIVE);

							TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
							return 0;
						}
					}
					break;
				}
			case IDC_DEBUG_EDIT_TRACE_PATH:
				{
					switch (uCommand)
					{
					case EN_SETFOCUS:
						{
							TraceInfo(_D("CSSSOptions::DlgProcDebug: EN_SETFOCUS received for Debug:IDC_DEBUG_EDIT_TRACE_PATH"));

							// SIP up
							SHSipPreference (hWnd, SIP_UP);

							// Save current contents of edit field
							SendDlgItemMessage (hWnd, IDC_DEBUG_EDIT_TRACE_PATH, WM_GETTEXT,
												  MAX_PATH, (LPARAM)m_szTracePath);

							TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
							return 0;
						}
					case EN_KILLFOCUS:
						{
							TCHAR	szTracePath[MAX_PATH + 1];

							TraceInfo(_D("CSSSOptions::DlgProcDebug: EN_KILLFOCUS received for Debug:IDC_DEBUG_EDIT_TRACE_PATH"));

							// SIP down
							SHSipPreference (hWnd, SIP_DOWN);

							// If trace path different than when field entered, re-list trace files
							SendDlgItemMessage (hWnd, IDC_DEBUG_EDIT_TRACE_PATH, WM_GETTEXT,
												  MAX_PATH, (LPARAM)szTracePath);

							if (_tcscmp(m_szTracePath, szTracePath) != 0)
							{
								// List existing trace files in list box.
								m_nNumTraceFiles = ListTraceFiles(hWnd, IDC_DEBUG_LIST_TRACE_FILES, szTracePath);

								if (m_nNumTraceFiles == 0)
								{
									SendDlgItemMessage (hWnd, IDC_DEBUG_LIST_TRACE_FILES, LB_ADDSTRING, 0, (LPARAM)SSS_TEXT_NO_TRACE_FILES);
								}
								else if (m_nNumTraceFiles < 0)
								{
									SendDlgItemMessage (hWnd, IDC_DEBUG_LIST_TRACE_FILES, LB_ADDSTRING, 0, (LPARAM)SSS_TEXT_INVALID_PATH);
									m_nNumTraceFiles = 0;
								}

								// Enable/Disable trace file list
								EnableWindow(ItemHandleFromID(hWnd, IDC_DEBUG_LIST_TRACE_FILES), (m_nNumTraceFiles == 0)?false:true); 

								_tcsncpy(m_szTracePath, szTracePath, MAX_PATH);
							}

							TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
							return 0;
						}
					}
					break;
				}
			case IDC_DEBUG_COMBO_TRACE_LEVELS:
				{
					switch (uCommand)
					{
					case CBN_SELCHANGE:
						{
							TraceInfo(_D("CSSSOptions::DlgProcDebug: CBN_SELCHANGE received for Debug:IDC_DEBUG_COMBO_TRACE_LEVELS"));
						}
					}
					break;
				}
			case IDC_DEBUG_BUTTON_BROWSE:
				{
					switch (uCommand)
					{
					case BN_CLICKED:
						{
							TraceInfo(_D("CSSSOptions::DlgProcDebug: BN_CLICKED received for Debug:IDC_DEBUG_BUTTON_BROWSE"));
							TraceInfo(_D("CSSSOptions::DlgProcDebug: Calling BrowseTracePath, m_szTracePath = <%s>"), m_szTracePath);
							BrowseTracePath(hWnd, m_szTracePath);
							TraceInfo(_D("CSSSOptions::DlgProcDebug: Back from BrowseTracePath, m_szTracePath = <%s>"), m_szTracePath);

							// Force SIP down
							SHSipPreference (hWnd, SIP_FORCEDOWN);

							SendDlgItemMessage (hWnd, IDC_DEBUG_EDIT_TRACE_PATH, WM_SETTEXT,
											      0, (LPARAM)m_szTracePath);

							// List existing trace files in list box.
							m_nNumTraceFiles = ListTraceFiles(hWnd, IDC_DEBUG_LIST_TRACE_FILES, m_szTracePath);

							if (m_nNumTraceFiles == 0)
							{
								SendDlgItemMessage (hWnd, IDC_DEBUG_LIST_TRACE_FILES, LB_ADDSTRING, 0, (LPARAM)SSS_TEXT_NO_TRACE_FILES);
							}
							else if (m_nNumTraceFiles < 0)
							{
								SendDlgItemMessage (hWnd, IDC_DEBUG_LIST_TRACE_FILES, LB_ADDSTRING, 0, (LPARAM)SSS_TEXT_INVALID_PATH);
								m_nNumTraceFiles = 0;
							}

							// Enable/Disable trace file list
							EnableWindow(ItemHandleFromID(hWnd, IDC_DEBUG_LIST_TRACE_FILES), (m_nNumTraceFiles == 0)?false:true); 

							TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
							return 0;
						}
					}
					break;
				}
			case IDC_DEBUG_LIST_TRACE_FILES:
				{
					switch (uCommand)
					{
					case LBN_SELCHANGE:
						{
							TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
							return 0;
						}
					case LBN_DBLCLK:
						{
							SHELLEXECUTEINFO	sei;
							TCHAR	szPath[MAX_PATH + 1];
							TCHAR	szFileName[MAX_PATH + 1];
							DWORD	dwTraceFileIndex;

							_tcscpy(szPath, g_szOpenDoc);

							SendDlgItemMessage (hWnd, IDC_DEBUG_EDIT_TRACE_PATH, WM_GETTEXT,
												  MAX_PATH, (LPARAM)(LPCTSTR) &szPath[_tcslen(szPath)]);

							dwTraceFileIndex = SendDlgItemMessage (m_hwDebugWnd, IDC_DEBUG_LIST_TRACE_FILES, LB_GETCURSEL, 0, 0);
							
							if (dwTraceFileIndex == LB_ERR)
								dwTraceFileIndex = 0;

							SendDlgItemMessage (m_hwDebugWnd, IDC_DEBUG_LIST_TRACE_FILES, LB_GETTEXT,
								(WPARAM) dwTraceFileIndex, (LPARAM)(LPCTSTR) szFileName);

							_tcscat(szPath, g_szFormatBackSlash);
							_tcscat(szPath, szFileName);

							memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
							sei.cbSize =  sizeof(SHELLEXECUTEINFO);
							sei.lpVerb = g_szOpen;
							sei.lpFile = g_szPWordExe;
							sei.lpDirectory = NULL;
							sei.lpParameters = szPath;

							TraceInfo(_D("CSSSOptions::DlgProcDebug: FileName = %s"), sei.lpFile);
							TraceInfo(_D("CSSSOptions::DlgProcDebug: Directory = %s"), sei.lpDirectory);
							TraceInfo(_D("CSSSOptions::DlgProcDebug: Parameters = %s"), sei.lpParameters);
							TraceInfo(_D("CSSSOptions::DlgProcDebug: Verb = %s"), sei.lpVerb);
							ShellExecuteEx(&sei);

							TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
							return 0;
						}
					}
					break;
				}
			}

			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)TRUE);
			return TRUE;
		}
	case WM_ACTIVATE :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_ACTIVATE received for Debug"));
			OnActivate(hWnd, LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
	case WM_SETTINGCHANGE :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_SETTINGCHANGE received for Debug"));
			OnSettingChange(hWnd, wParam, (LPCTSTR)lParam);
			RefreshWindow(hWnd, FALSE);
			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
	case PSM_QUERYSIBLINGS :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: PSM_QUERYSIBLINGS received for Debug"));
			NewSettings.tlTraceLevel = (TraceLevel)SendDlgItemMessage (hWnd, IDC_DEBUG_COMBO_TRACE_LEVELS, CB_GETCURSEL, 0, 0);
			NewSettings.blTraceActive = IsButtonChecked(hWnd, IDC_DEBUG_CHECK_TRACE_ACTIVE);
			_tcsncpy(NewSettings.szTracePath, m_szTracePath, MAX_PATH);
			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
	case WM_NOTIFY :
		{
			LPPSHNOTIFY lpSHNotify	= (LPPSHNOTIFY)lParam;
			UINT		uiCode		= lpSHNotify->hdr.code;
			HWND		hwWndFrom	= lpSHNotify->hdr.hwndFrom;
			UINT		uiFrom		= lpSHNotify->hdr.idFrom;
			LPARAM		lpParam		= lpSHNotify->lParam;

			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_NOTIFY, code = %08X, hwndFrom = %08X, idFrom = %08X, lParam = %08X"), uiCode, hwWndFrom, uiFrom, lpParam);

			if (uiCode == PSN_SETACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_SETACTIVE received for Debug"));

				if ((_tcslen(g_szCompanyURL) <= 0) || (OptionsFlags.OptionsBits.TapToRegister) || (OptionsFlags.OptionsBits.LicenseInvalid))
				{
					TraceLeave(_D("CSSSOptions::DlgProcDebug"), TRUE);
					SetWindowLong(hWnd, DWL_MSGRESULT, InWideMode() ? IDD_ABOUT_WIDE : IDD_ABOUT);
					return TRUE;
				}

				AutoScroll(m_hwOptionsWnd, m_hwTabCtrl);

				TraceLeave(_D("CSSSOptions::DlgProcDebug"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, 0);
				return TRUE;
			}
			else if (uiCode == PSN_KILLACTIVE)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_KILLACTIVE received for Debug"));
				TraceLeave(_D("CSSSOptions::DlgProcDebug"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_APPLY)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_APPLY received for Debug"));

				NewSettings.tlTraceLevel = (TraceLevel)SendDlgItemMessage (hWnd, IDC_DEBUG_COMBO_TRACE_LEVELS, CB_GETCURSEL, 0, 0);
				NewSettings.blTraceActive = IsButtonChecked(hWnd, IDC_DEBUG_CHECK_TRACE_ACTIVE);
				_tcsncpy(NewSettings.szTracePath, m_szTracePath, MAX_PATH);
				
				// Get current selection for trace levels list.
				m_tlTraceLevel = (TraceLevel)SendDlgItemMessage (hWnd, IDC_DEBUG_COMBO_TRACE_LEVELS, CB_GETCURSEL, 0, 0);
				if (m_tlTraceLevel == (TraceLevel)CB_ERR)
					m_tlTraceLevel = tlNone;
				TraceSetLevel(m_tlTraceLevel);
				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_APPLY in Debug, m_tlTraceLevel = %d"), m_tlTraceLevel);

				SendDlgItemMessage (hWnd, IDC_DEBUG_EDIT_TRACE_PATH, WM_GETTEXT,
									  MAX_PATH, (LPARAM)m_szTracePath);
				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_APPLY in Debug, m_szTracePath = <%s>"), m_szTracePath);

				m_blTraceActive = IsButtonChecked(hWnd, IDC_DEBUG_CHECK_TRACE_ACTIVE);

				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_APPLY in Debug, m_blTraceActive=%08x"), m_blTraceActive);

				TraceLeave(_D("CSSSOptions::DlgProcDebug"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
				return TRUE;
			}
			else if (uiCode == PSN_RESET)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_RESET received for Debug"));
				TraceLeave(_D("CSSSOptions::DlgProcDebug"), TRUE);
				return TRUE;
			}
			else if (uiCode == PSN_QUERYCANCEL)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_QUERYCANCEL received for Debug"));
				TraceLeave(_D("CSSSOptions::DlgProcDebug"), TRUE);
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
				return TRUE;
			}
			else if (uiCode == PSN_HELP)
			{
				TraceInfo(_D("CSSSOptions::DlgProcDebug: PSN_HELP received for Debug"));

				DoShowHelp(SSS_HELP_TAG_DEBUG_SETTINGS);

				break;
			}

			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)FALSE);
			return FALSE;  // Return false to force default processing.
		}
	case WM_PAINT :
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_PAINT received for Debug"));
			PAINTSTRUCT ps;
			HDC hDC;

			hDC = BeginPaint(hWnd, &ps);

			OnPaint(hWnd, hDC);

			DrawStaticText(hWnd, IDC_DEBUG_STATIC_TRACE_PATH, SSS_TEXT_DEBUG_STATIC_TRACE_PATH, DT_LEFT);
			DrawStaticText(hWnd, IDC_DEBUG_STATIC_TRACE_FILES_LIST, SSS_TEXT_DEBUG_STATIC_TRACE_FILES, DT_LEFT);
			DrawStaticText(hWnd, IDC_DEBUG_STATIC_TRACE_LEVEL, SSS_TEXT_DEBUG_STATIC_TRACE_LEVEL, DT_LEFT);

            SendDlgItemMessage (hWnd, IDC_DEBUG_CHECK_TRACE_ACTIVE, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_DEBUG_CHECK_TRACE_ACTIVE);

            SendDlgItemMessage (hWnd, IDC_DEBUG_BUTTON_BROWSE, WM_SETTEXT,
                                  0, (LPARAM)SSS_TEXT_DEBUG_BUTTON_ELLIPSES);

			EndPaint(hWnd, &ps);

			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}

	case WM_ERASEBKGND:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_ERASEBKGND received for Debug"));
			bResult = OnEraseBackground(hWnd, (HDC)wParam);
		    TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)bResult);

			return bResult;
		}

	case WM_CTLCOLORDLG:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_CTLCOLORDLG received for Debug"));
			bResult = (BOOL)OnCtlColor(m_ahbrushDebug, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORBTN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_CTLCOLORBTN received for Debug"));
			bResult = (BOOL)OnCtlColor(m_ahbrushDebug, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)bResult);
			return bResult;
		}

    case WM_CTLCOLORSTATIC:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_CTLCOLORSTATIC received for Debug"));
			bResult = (BOOL)OnCtlColor(m_ahbrushDebug, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)bResult);
			return bResult;
		}

	case WM_CTLCOLOREDIT:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_CTLCOLOREDIT received for Debug"));
			bResult = (BOOL)OnCtlColor(m_ahbrushDebug, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)bResult);
			return bResult;
        }

	case WM_CTLCOLORLISTBOX:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_CTLCOLORLISTBOX received for Debug"));
			bResult = (BOOL)OnCtlColor(m_ahbrushDebug, (HWND)lParam);
		    TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)bResult);
			return bResult;
        }

	case WM_MOUSEMOVE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_MOUSEMOVE received for Debug"));

			OnMouseMove(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_LBUTTONDOWN received for Debug"));

			OnLButtonDown(hWnd, wParam, lParam);

			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
	case WM_LBUTTONUP:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_LBUTTONUP received for Debug"));

			if (PointInControl(hWnd, IDC_DEBUG_LIST_TRACE_FILES, lParam))
			{
				TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_LBUTTONUP is in Trace files list"));
			}
			else
			{
				OnLButtonUp(hWnd, wParam, lParam);
			}

			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
	case WM_SIZE:
		{
			TraceInfo(_D("CSSSOptions::DlgProcDebug: WM_SIZE received for Debug"));
			if (IsSecondEdition())
				TransposeDlg(m_hmInstance, hWnd, InWideMode() ?
					MAKEINTRESOURCE(IDD_OPTIONS_DEBUG_WIDE) :
					MAKEINTRESOURCE(IDD_OPTIONS_DEBUG));
			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
	case WM_WIV_REFRESH:
		{
			TraceDetail(_D("CSSSOptions::DlgProcDebug: WM_WIV_REFRESH received for Debug"));
			TraceLeave(_D("CSSSOptions::DlgProcDebug"), (DWORD)0);
			return 0;
		}
		break;
	}

	lrResult = DefWindowProc(hWnd, wMsg, wParam, lParam);

    TraceLeave(_D("CSSSOptions::DlgProcDebug"), lrResult);

	return (BOOL)lrResult;
}

//======================================================================
// Global non-member debug functions
//======================================================================
//======================================================================
// OptionsDlgProcDebug - Debug options page dialog box procedure
//======================================================================
BOOL CALLBACK OptionsDlgProcDebug (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->DlgProcDebug(hWnd, wMsg, wParam, lParam);
}

#endif // WIV_DEBUG
