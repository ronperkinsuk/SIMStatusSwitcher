//////////////////////////////////////////////////////////////////////
//
// SSSToday.h: Interface for the CSSSToday class.
//
//////////////////////////////////////////////////////////////////////

#ifndef INC_SSS_TODAY_H
#define INC_SSS_TODAY_H

//extern	UINT	IDFromIconSet(const DWORD dwIconSet, const UINT uIconType);
//extern	bool	InWideMode();
//extern	bool	IsTapAndHold (const HWND hWnd, const POINT pt);
//extern	LRESULT	ShowContextMenu (const HWND hWnd, const UINT uID, const POINT pt);
extern  LRESULT WINAPI CustomItemOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class CSSSToday  
{

friend LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
friend bool CALLBACK LicenseTodayNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig);

private:

	CSSSToday();
	// Member Variables

	// TodayWndProc - main message loop
	LRESULT CALLBACK TodayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Phone access
	bool	UpdateTodayItemData(const bool bRefresh = false);
	void	UpdateOptionsDialog(WPARAM wParam, LPARAM lParam);

	bool	IsOnTodayIcon(const POINT point);
	void	DrawTodayIcon(const HDC hDC);
	void	GetTodayDefaults();
	LRESULT ShowContextMenu (HWND hWnd, UINT uiMenuID, POINT ptPoint);
//	void	SetupPhoneStateFlags(const DWORD dwRadioSupport, const DWORD dwEquipmentState, const DWORD dwReadyState, const DWORD dwPhoneLockedState);
	bool	SetupPhoneStateFlags();
	void	ProcessLockedState(DWORD dwSIMLockedState);

	// Message handlers
	int		OnTodayCreate(LPCREATESTRUCT lpCreateStruct);
	void	OnTodayDestroy();
	void	OnTodayEraseBkgnd(const HDC hDC);
	BOOL	OnTodayCustomQueryRefreshCache(TODAYLISTITEM *pTodayListItem);
	BOOL	OnTodayCustomClearCache(TODAYLISTITEM *pTodayListItem);
	void	OnTodayLButtonDown(const UINT nFlags, const POINT point);
	void	OnTodayLButtonUp(const UINT nFlags, const POINT point);
	void	OnTodayPaint(const HDC hDC);
	void	OnTodaySettingChange(const UINT nFlags, LPCTSTR lpszSection);
	LRESULT OnTodayNotify(const UINT nID, NMHDR* pNMHDR);
	LRESULT OnTodayMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void	DoAction(const DWORD dwAction, const POINT point);

	void	DoSwitchSIM();
	void	DoToggleRadioState();
	void	DoShowOptions(DWORD dwFlags = 0);
	void	DoRefresh();
	void	DoPhoneSettings();

	// Update Window
	void	RefreshWindow(BOOL bShow = FALSE);
	void	FormatDisplayLines();

public:

	// Constructor/destructor Methods
	CSSSToday(HMODULE hInstance, CSSSPhone *pPhone, CSSSOptions *pOptions, LPCTSTR lpszClassName,
				LPCTSTR lpszWindowName);
	~CSSSToday();

	// Main Create method
	BOOL		TodayCreate(HWND hWndParent, TODAYLISTITEM *ptli, DWORD dwStyle = WS_VISIBLE | WS_CHILD);

	// Register/Unregister TodayWindow
	void		UnregisterTodayClass();
	void		RegisterTodayClass(WNDPROC wndProc);

	bool		TodayLicenseNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig);

	// Get methods
	HWND		GetTodayWindowHandle();
	HWND		GetParent();
	UINT		GetItemHeight();
	HMODULE		GetInstance();
	HICON		GetIcon();
	LPCTSTR		GetClassName();
	LPCTSTR		GetWindowName();

	// Set methods
	void		SetIconSet(const DWORD dwIconSet);
	BOOL		SetIcon(const UINT uID, const int xDrawAt = 2, const int yDrawAt = 1);
	void		SetItemHeight(const UINT nHeight);
	void		SetClassInfo(LPCTSTR lpszClassName, LPCTSTR lpszWindowName);
	void		SetInstance(const HMODULE hInstance);
	void		SetAutoPINAfterInit(const bool bAllowAutoPINAfterInit);
	void		SetAutoPINAfterRadioON(const bool bAllowAutoPINAfterRadioON);
	void		SetShowPhoneNumber(const bool bShowPhoneNumber);
	void		SetShowTSP(const bool bShowTSP);
	void		SetSingleLineDisplay(const bool bSingleLineDisplay);
	void		SetLine1BoldFont(const bool bLine1BoldFont);
	void		SetLine2BoldFont(const bool bLine2BoldFont);
	void		SetTapAction(const DWORD dwAction);
	void		SetTAHAction(const DWORD dwAction);
	void		SetTodayIconTapAction(const DWORD dwAction);
	void		SetTodayIconTAHAction(const DWORD dwAction);
	void		SetButtonAction(const DWORD dwAction);

	void		PhoneNotify(const DWORD dwNotifyCode, const LPVOID lpData = NULL);

protected:

}; // class CSSSToday

// Global non-member functions

#endif // #ifndef INC_SSS_TODAY_H
