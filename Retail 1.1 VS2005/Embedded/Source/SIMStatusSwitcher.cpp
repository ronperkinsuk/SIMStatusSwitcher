//////////////////////////////////////////////////////////////////////
//
// SIMStatusSwitcher.cpp: Implementation of SIM Status Switcher app.
//
//////////////////////////////////////////////////////////////////////

#include "SSSCommon.h"
#include "SSSGlobals.h"
#include "SIMStatusSwitcher.h"
#include "SSSToday.h"
#include "SSSOptions.h"
#include "SSSPhone.h"
#include "WiVUtils.h"
#include "WiVLicense.h"
#include "WiVLang.h"

using namespace WiV;

//=====================
// Class references
//=====================
SSSCOMMON	*g_pSSSData = NULL;

CSSSToday	*SSSToday = NULL;
CSSSOptions	*SSSOptions = NULL;
CSSSPhone	*SSSPhone = NULL;

HMODULE		gb_hmInstance;

#ifdef WIV_DEBUG
static bool	gb_blTraceInitialized = false;
#endif

static bool	gb_blInitThisProcess = false;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	BOOL	bRetVal = TRUE;

	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			if (gb_blInitThisProcess)
			{
				TraceDetail(_D("SSS:DLL_PROCESS_ATTACH: Already initialised"));
			}

			DisableThreadLibraryCalls((HMODULE)hModule);

			break;
		}

		case DLL_PROCESS_DETACH:
		{
			if (gb_blInitThisProcess)
			{
				TraceDetail(_D("SSS:DLL_PROCESS_DETACH: De-initialising for this process"));

				if (SSSPhone != NULL)
				{
//					TraceDetail(_D("SSS::DLL_PROCESS_DETACH: Calling SSSPhone->StopComms"));
//					SSSPhone->StopComms();
//					TraceDetail(_D("SSS::DLL_PROCESS_DETACH: Back from SSSPhone->StopComms"));
				}
				
				TraceDetail(_D("SSS::DLL_PROCESS_DETACH: Calling DisablePowerNotifications"));
				DisablePowerNotifications();
				TraceDetail(_D("SSS::DLL_PROCESS_DETACH: Back from DisablePowerNotifications"));
				
				if (SSSToday != NULL)
				{
					SSSToday->UnregisterTodayClass();

					delete SSSToday;
					SSSToday = NULL;
				}

				if (SSSOptions != NULL)
				{
					delete SSSOptions;
					SSSOptions = NULL;
				}

				if (SSSPhone != NULL)
				{
					delete SSSPhone;
					SSSPhone = NULL;
				}

				if (g_pSSSData != NULL)
				{
					delete g_pSSSData;
					g_pSSSData = NULL;
				}

#ifdef WIV_DEBUG
				if(gb_blTraceInitialized)
				{
					TraceStop();
					gb_blTraceInitialized = false;
				}
#endif
			}
			break;
		}
	}

	return bRetVal;
}

HWND InitializeCustomItem(TODAYLISTITEM *pTodayListItem, HWND hWndParent)
{
	HWND	hwToday;
	bool	blResult = false;
	PWIVLANG	lpsDefaultLanguage = NULL;
	PWIVLANG	lpsCurrentLanguage = NULL;

	TCHAR	*pszDelim;
	TCHAR	szWindowsPath[MAX_PATH + 1];
	TCHAR	szInstallPath[MAX_PATH + 1];
	TCHAR	szTodayItemName[MAX_ITEMNAME + 1];

//	TraceEnter(_D("SSS:InitializeCustomItem"));
//	TraceDetail(_D("SSS:InitializeCustomItem: m_hmInstance = <%08X>"),
//			  gb_hmInstance);

	// If plugin is not enabled, don't bother doing anything
	if(!pTodayListItem->fEnabled)
	{
//		TraceInfo(_D("SSS:InitializeCustomItem: Plugin is not enabled."));
//		TraceLeave(_D("SSS:InitializeCustomItem"), (DWORD)NULL);
		return NULL;
	}

	if (!gb_blInitThisProcess)
	{
		gb_hmInstance = pTodayListItem->hinstDLL;

		if (g_pSSSData == NULL)
		{
			g_pSSSData = new SSSCOMMON;

			if (g_pSSSData == NULL)
			{
				return NULL;
			}
		}

		SSSGlobals = GlobalsLoad(SSSGlobals, true);

#ifdef WIV_DEBUG
		if(!gb_blTraceInitialized)
		{
			TraceStart();
			gb_blTraceInitialized = true;
		}
#endif
		TraceEnter(_D("SSS:InitializeCustomItem"));

		TraceDetail(_D("SSS:InitializeCustomItem: Initialising for new process"));

		TraceDetail(_D("SSS:InitializeCustomItem: ")
					_D("g_szCompanyIdentity = <%s>"), g_szCompanyIdentity);

		_zcpy(szTodayItemName, pTodayListItem->szName);
		_zcpy(szInstallPath, pTodayListItem->szDLLPath);

		pszDelim = _tcspbrk( _tcsrev(szInstallPath), g_szFormatSlashes);
		if (pszDelim)
		{
			_tcsncpy(szInstallPath, _tcsrev(pszDelim), MAX_PATH);
		}
		else
		{
			_tcsrev(szInstallPath);
			if (_tcslen(szInstallPath) == 0)
			{
				_tcsncpy(szInstallPath, g_szFormatSlash, MAX_PATH);
			}
		}

		TraceDetail(_D("SSS:InitializeCustomItem: Today Item Name = <%s>"), szTodayItemName);
		TraceDetail(_D("SSS:InitializeCustomItem: Install Path = <%s>"), szInstallPath);

		SetInstallPath(szInstallPath);
		SetTodayItemName(szTodayItemName);

		GetModuleFileName(::GetModuleHandle(_T("coredll.dll")), szWindowsPath, MAX_PATH);
		*_tcsrchr(szWindowsPath, _T('\\')) = _T('\0');
		_tcsncat(szWindowsPath, _T("\\"), MAX_PATH - _tcslen(szWindowsPath));
		TraceDetail(_D("SSS:InitializeCustomItem: Windows Path = <%s>"), szWindowsPath);

		SetWindowsPath(szWindowsPath);

		TraceDetail(_D("SSS:InitializeCustomItem: Calling LangGetDefaultLanguage"));
		lpsDefaultLanguage = LangGetDefaultLanguage();
		if (lpsDefaultLanguage != NULL)
		{
			TraceDetail(_D("SSS:InitializeCustomItem: Back from LangGetDefaultLanguage, ")
						_D("Language ID = %s"), lpsDefaultLanguage->ID);
		}

		TraceDetail(_D("SSS:InitializeCustomItem: Calling LangGetCurrentLanguage"));
		lpsCurrentLanguage = LangGetCurrentLanguage();
		if (lpsCurrentLanguage != NULL)
		{
			TraceDetail(_D("SSS:InitializeCustomItem: Back from LangGetCurrentLanguage, ")
						_D("Language ID = %s"), lpsCurrentLanguage->ID);
			
			TraceDetail(_D("SSS:InitializeCustomItem: Calling LangLoadTable, ")
						_D("Product = <%s> and Language = <%s>"), g_szProductShortName, lpsCurrentLanguage->ID);
			blResult = LangLoadTable(g_szProductShortName, lpsCurrentLanguage->ID);
			TraceDetail(_D("SSS:InitializeCustomItem: Back from LangLoadTable, Result = %d"), blResult);
		}

		TraceInfo(_D("SSS:InitializeCustomItem: ")
					_D("Creating CSSSPhone, CSSSToday and CSSSOptions class objects"));

		SSSPhone = new CSSSPhone(gb_hmInstance);
		SSSOptions = new CSSSOptions(gb_hmInstance, SSSPhone, TRUE);
		SSSToday = new CSSSToday(gb_hmInstance, SSSPhone, SSSOptions,
								g_szSSSTodayClass, g_szSSSTodayWnd);

		TraceInfo(_D("SSS:InitializeCustomItem: ")
					_D("CSSSPhone, CSSSToday and CSSSOptions class objects created"));

		TraceInfo(_D("SSS:InitializeCustomItem: Setting reference to Today class in SSSOptions"));
		SSSOptions->SetTodayClass((DWORD)SSSToday);

		if (lpsCurrentLanguage != NULL)
		{
			if (SSSOptions->SetupHelpFile(lpsCurrentLanguage) != 0)
			{
				TraceDetail(_D("SSS:InitializeCustomItem, Help file set up for language %s"), lpsCurrentLanguage->ID);
			}
			else
			{
				TraceDetail(_D("SSS:InitializeCustomItem, Help file for language %s, not found"), lpsCurrentLanguage->ID);
			}
		}
		
		TraceInfo(_D("SSS:InitializeCustomItem: Setting reference to Today class in SSSPhone"));
		SSSPhone->SetTodayClass((DWORD)SSSToday);
	
//		TraceInfo(_D("SSS::InitializeCustomItem, Calling SSSPhone->InitializeComms"));
//		SSSPhone->InitializeComms();
//		TraceInfo(_D("SSS::InitializeCustomItem, Back from SSSPhone->InitializeComms"));

//		TraceDetail(_D("SSS::InitializeCustomItem: Calling EnablePowerNotifications"));
//		EnablePowerNotifications();
//		TraceDetail(_D("SSS::InitializeCustomItem: Back from EnablePowerNotifications"));

	}

	TraceInfo(_D("SSS:InitializeCustomItem: Registering and creating Today window"));
	SSSToday->RegisterTodayClass((WNDPROC)WndProc);
	SSSToday->TodayCreate(hWndParent, pTodayListItem, WS_VISIBLE |WS_CHILD);

	hwToday = SSSToday->GetTodayWindowHandle();

	if (!gb_blInitThisProcess)
	{
		TraceDetail(_D("SSS:InitializeCustomItem: Calling EnablePowerNotifications"));
		EnablePowerNotifications();
		TraceDetail(_D("SSS:InitializeCustomItem: Back from EnablePowerNotifications"));

		gb_blInitThisProcess = true;
	}
	else
	{
		TraceDetail(_D("SSS:InitializeCustomItem: Already initialised for this process"));
	}

	TraceLeave(_D("SSS:InitializeCustomItem"), (DWORD)hwToday);

	return hwToday;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrResult;

	TraceEnter(_D("SSS::WndProc"));

	TraceDetail(_D("SSS::WndProc: Calling SSSToday->TodayWndProc"));
	lrResult = SSSToday->TodayWndProc(hWnd, uMsg, wParam, lParam);
	TraceDetail(_D("SSS::WndProc: Back from SSSToday->TodayWndProc, ")
				_D("lrResult = <%08X>"), lrResult);

	TraceLeave(_D("SSS::WndProc"), lrResult);
	return lrResult;
}

LRESULT WINAPI CustomItemOptionsDlgProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lrResult = 0;

	TraceEnter(_D("SSS::CustomItemOptionsDlgProc"));

	if (uiMsg == WM_INITDIALOG)
	{
		TraceInfo(_D("SSS::CustomItemOptionsDlgProc: WM_INITDIALOG, lParam = <%08X>"), lParam);
		
		// If plugin is not enabled, don't bother doing anything
		if (!((TODAYLISTITEM *)lParam)->fEnabled)
		{
			TraceInfo(_D("SSS::CustomItemOptionsDlgProc: Do nothing because plugin is not enabled"));
			TraceLeave(_D("SSS::CustomItemOptionsDlgProc"), (DWORD)lrResult);
			return lrResult;
		}
		else
		{
			lParam = (LPARAM)SSS_FLAG_PLUGIN_ENABLED;
		}
	}

	TraceInfo(_D("SSS::CustomItemOptionsDlgProc: uiMsg = ")
				_D("<%08x>, wParam = <%08x>, lParam = <%08x>"), uiMsg, wParam, lParam);

	TraceInfo(_D("SSS::CustomItemOptionsDlgProc: ")
				_D("Calling SSSOptions->TodayOptionsWndProc"));
	lrResult=SSSOptions->TodayOptionsWndProc(hDlg, uiMsg, wParam, lParam);
	TraceInfo(_D("SSS::CustomItemOptionsDlgProc: ")
			_D("Back from SSSOptions->TodayOptionsWndProc, lrResult = <%08X>"), lrResult);

	TraceLeave(_D("SSS::CustomItemOptionsDlgProc"), lrResult);
	return lrResult;
}
