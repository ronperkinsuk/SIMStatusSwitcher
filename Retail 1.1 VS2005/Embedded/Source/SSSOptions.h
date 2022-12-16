//////////////////////////////////////////////////////////////////////
//
// SSSOptions.h: Interface for the CSSSOptions class.
//
//////////////////////////////////////////////////////////////////////

#ifndef INC_SSS_OPTIONS_H
#define INC_SSS_OPTIONS_H

using namespace WiV;

class CSSSPhone;

class CSSSOptions  
{

// Structures
typedef struct 
{
	DWORD	dwBufLen;					//data length	30000000
	BYTE	bBuffer[WIV_MAX_BINARY];	//data			0F000000 23410694002313300000000000000000000000 11 04000000 0127000000000000000000000000000000000000

} SSS_BUFFER1, *PSSS_BUFFER1; // 104

typedef struct 
{
	DWORD	dwICCIDLen;	//ICCID length
	BYTE	bICCID[19];	//ICCID          
	BYTE	bAuto[1];	//auto
	DWORD	dwPINLen;	//pin length
	BYTE	bPIN[20];	//pin          
} SSS_BUFFER2, *PSSS_BUFFER2; // 48

typedef struct 
{
	DWORD	dw1;
	DWORD	dw2;
	DWORD	dw3;
} SSS_BUFFER3, *PSSS_BUFFER3;

//static	SSS_SETTINGS	CurrentSettings;
//static	SSS_SETTINGS	NewSettings;

// Friend functions
friend	LRESULT	WINAPI		CustomItemOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
friend	int		CALLBACK	OptionsPropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
friend	LRESULT	WINAPI		OptionsTodayWndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

friend	BOOL	CALLBACK	OptionsDlgProcAbout(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
friend	BOOL	CALLBACK	OptionsDlgProcDisplay(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
friend	BOOL	CALLBACK	OptionsDlgProcAppearance(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
friend	BOOL	CALLBACK	OptionsDlgProcActions(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
friend	BOOL	CALLBACK	OptionsDlgProcLanguage(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
friend	BOOL	CALLBACK	OptionsDlgProcSecurity(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
friend	LRESULT	CALLBACK	OptionsDlgProcInformation(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
friend	LRESULT	CALLBACK	OptionsDlgProcRegistration(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

friend	bool	CALLBACK	LicenseOptionsNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig);

#ifdef WIV_DEBUG
friend	BOOL	CALLBACK    OptionsDlgProcDebug(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
#endif

private:

//static SSS_SETTINGS	CurrentSettings;
//static SSS_SETTINGS	NewSettings;

CSSSOptions();

bool OptionsLicenseNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig);
void SetupPhoneState();

// Dialog window procedures
LRESULT	TodayOptionsWndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int		PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);

BOOL 	DlgProcAbout(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL	DlgProcDisplay(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL 	DlgProcAppearance(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL	DlgProcActions(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL	DlgProcLanguage(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL 	DlgProcSecurity(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
LRESULT	DlgProcInformation(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT	DlgProcRegistration(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef WIV_DEBUG
BOOL	DlgProcDebug(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
#endif

#ifdef SSS_V2_IMP
HANDLE	OpenCustomIconsFile();
void	CloseCustomIconsFile();
long	LoadCustomIconsFile();
bool	ValidateCustomIconsFile(LPCTSTR lpszFileID, LPCTSTR lpszVersion);
int		LoadCustomIconSets(TCHAR lpaszIconSets[SSS_MAX_AVAILABLE_ICON_SETS][SSS_MAX_ICON_SET_NAME_LENGTH + 1], int nIndex);
//int		LoadCustomIconSets(TCHAR lpaszIconSets[][], int nStartIndex);
int		GetCustomIconsLine(LPTSTR lpszLine);
int		ParseCustomIconsLine(LPCTSTR lpszLine, LPTSTR lpszValue);
#endif //#ifdef SSS_V2_IMP

void	LoadIconSetsArray();
void	InitialiseIconSetList(HWND hWnd);

void	LoadActionsArray();
void	InitializeActionLists(HWND hWnd);

bool	InitializeLanguagesList(HWND hWnd);

void	LoadSecurityStepsArrays();

#ifdef WIV_DEBUG
void	LoadTraceLevelsArray();
void	InitialiseTraceLevelsList(HWND hWnd);
#endif

void	DrawIconSet(HWND hWnd, HDC hDC, HWND hIcons);
void	DrawSecurityHeader(HWND hWnd);
void	DrawSecurityError(HWND hWnd, LPCTSTR lpszError);
void	ClearSecurityError(HWND hWnd);

void	AutoScroll(HWND hWndMain, HWND hTabCtrl);
void	UpdateTabsText();

LRESULT	DoShowHelp(LPCTSTR lpszSection);

LPARAM	ShowProperties(DWORD dwFlags);
LPCTSTR	SetupSecurityPage(HWND hWnd, bool blFromPaint = false);
DWORD	GetCurrentSeqStep();

bool	OnOptionsInitDialog(DWORD dwFlags);
void	OnOptionsDestroy();
void	OnOptionsOK();
void	OnOptionsCancel();
void	OnOptionsPaint(HDC hDC);
void	OnOptionsActivate(UINT nState, HWND hWndPrevious, BOOL bMinimized);
void	OnOptionsCommand(UINT nID, UINT nNotifyCode, HWND hWndCtrl);
void	OnOptionsSettingChange(UINT nFlags, LPCTSTR lpszSection);
LRESULT	OnOptionsNotify(UINT nID, LPPSHNOTIFY lpSHNotify);
LRESULT	OnOptionsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT	OnCtlColor(HBRUSH *haBrush, HWND hWndCtrl);
void	OnPaint(HWND hWnd, HDC hDC, bool blNoCancel = false); 
void	OnAboutPaint(HWND hWnd, HDC hDC);
void	OnLButtonUp(HWND hDlg, WPARAM wParam, LPARAM lParam);
void	OnLButtonDown(HWND hDlg, WPARAM wParam, LPARAM lParam);
void	OnMouseMove(HWND hDlg, WPARAM wParam, LPARAM lParam);
bool	IsOnCancelIcon(POINT point);
void	CancelPropSheet();
void	OnSettingChange(HWND hWnd, UINT nFlags, LPCTSTR lpszSection);
void	OnActivate(HWND hWnd, UINT nState, HWND hWndPrevious, BOOL bMinimized);
void	OnDestroy(HWND hWnd, HBRUSH *haBrush);
BOOL	OnInitDialog(HWND hWnd, HBRUSH *haBrush);
BOOL	OnEraseBackground(HWND hWnd, HDC hDC);

DWORD	GetUserOptions();
DWORD	SetUserOptions();
DWORD	GetSecurityOptions(bool bForce = false);
DWORD	SetSecurityOptions();
#ifdef WIV_DEBUG
DWORD	SetDebugOptions();
DWORD	GetDebugOptions();
#endif

void	GetDeviceInfo(CSSSPhone *pPhone, const bool bRead = true, const bool bProcess = false, const bool bWay = false);
bool	PINInfo(const DWORD dwLength, LPTSTR lpszPIN, LPTSTR lpszICCID, bool *lpbAutoReset, bool *lpbAutoPhone);
bool	InfoPIN(DWORD *lpdwLength, LPCTSTR szPIN, LPCTSTR szICCID, const bool bAutoReset, const bool bAutoPhone);
void	DoName(LPCTSTR szICCID, LPTSTR lpszName);
void	GetPIN(LPCTSTR szICCID, LPTSTR szPIN);

#ifdef WIV_DEBUG
int		ListTraceFiles (HWND hWnd, UINT uID, LPTSTR pszDir);
LPARAM	ViewTraceFile (HWND hWnd, LPCTSTR szFileName);
LPARAM	BrowseTracePath (HWND hWnd, LPTSTR lpszPath);
#endif

public:

struct SSS_SETTINGS
{
	bool		blShowPhoneNumber;
	bool		blShowTSP;
	bool		blSingleLineDisplay;

	bool		blLine1BoldFont;
	bool		blLine2BoldFont;
	DWORD		dwIconSet;

	DWORD		dwTapAction;
	DWORD		dwTAHAction;
	DWORD		dwTodayIconTapAction;
	DWORD		dwTodayIconTAHAction;

	bool		blAllowAutoPINAfterInit;
	bool		blAllowAutoPINAfterRadioON;
	TCHAR		szPIN[SSS_MAX_PIN + 1];

//	TCHAR		szLic[WIV_MAX_SIGNAL_STATE_LENGTH + 1];
	int			nLicType;
	int			nDaysRemaining;

#ifdef WIV_DEBUG
	bool		blTraceActive;
	TraceLevel	tlTraceLevel;
	TCHAR		szTracePath[MAX_PATH + 1];
#endif

} *PSSS_SETTINGS;

// Constructor/destructor
CSSSOptions(HINSTANCE hInstance, CSSSPhone *pPhone, bool bFullScreen = true);
~CSSSOptions();

void	TodayDestroyed();

// Get methods
HINSTANCE		GetInstance();
HWND	GetWindowHandle();
bool	GetFullScreen();
DWORD	GetOptionsStyle();
void	GetTodayOptions(DWORD dwFlags);

bool	GetAllowAutoPINAfterInit();
bool	GetAllowAutoPINAfterRadioON();
bool	GetSingleLineDisplay();
bool	GetShowPhoneNumber();
bool	GetShowTSP();
DWORD	GetIconSet();
DWORD	GetTapAction();
DWORD	GetTAHAction();
DWORD	GetTodayIconTapAction();
DWORD	GetTodayIconTAHAction();
bool	GetLine1Bold();
bool	GetLine2Bold();
bool	GetHidePersonalInfo();

BOOL	SetupHelpFile(PWIVLANG lpHelpLanguage);

//DWORD	GetFlags();
//DWORD	SetFlags(const DWORD dwFlags);

// Set methods
void	SetInstance(HINSTANCE hInstance);
void	SetTodayClass(DWORD pToday);
void	SetFullScreen(const bool bFullScreen);
void	SetTitle(const LPCTSTR lpszTitle, bool bRefresh = false);
//	void			SetTitle(UINT nID, bool bRefresh = false);

// Association with option dialog created by system
void	AssociateWithOptionsDlg(HWND hWnd);
void	RefreshWindow(HWND hwWnd, BOOL blErase = TRUE);

protected :

}; //class CSSSOptions

// Global non-member functions
//int  CALLBACK		OptionsPropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
//BOOL CALLBACK		OptionsDlgProcAbout (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK		OptionsDlgProcDisplay(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK		OptionsDlgProcAppearance(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK		OptionsDlgProcActions(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK		OptionsDlgProcSecurity(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
//LRESULT	CALLBACK	OptionsDlgProcInformation(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//LRESULT	CALLBACK	OptionsDlgProcRegistration(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef WIV_DEBUG
//BOOL CALLBACK		OptionsDlgProcDebug(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
#endif


#endif // INC_SSS_OPTIONS_H
