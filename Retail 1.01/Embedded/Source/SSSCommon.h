//////////////////////////////////////////////////////////////////////
//
// SSSCommon.h: Common definitions.
//
//////////////////////////////////////////////////////////////////////

#ifndef INC_SSS_COMMON_H
#define INC_SSS_COMMON_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//=====================
// System Includes 
//=====================
#include <windows.h>
#include <todaycmn.h>
#include <aygshell.h>
#include <wincrypt.h>
#include <pm.h>
#include <msgqueue.h>

#ifdef WIV_DEBUG
#include <commdlg.h>                 // Common dialog box includes
#endif

#include "WiVDefs.h"

//TODO: refer to globals
#define	SSS_VERSION_TYPE		_T("Retail")
#define	SSS_VERSION_MAJOR		_T("1")
#define	SSS_VERSION_MINOR		_T("01")
#define	SSS_VERSION_BUILD		_T("0706")

//#define	SSS_WM_DOACTION			WM_APP + 10
#define SSS_WM_DOACTION    TEXT("SSS_WM_DOACTION")

class CSSSToday;
class CSSSPhone;
class CSSSOptions;

//=====================
// Identity
//=====================
class SSSCOMMON
{

public:

	TCHAR	szCompanyName[6];//5
	TCHAR	szCompanyURL[21];//20
	TCHAR	szCompanySupport[18];//17
	TCHAR	szRevSupport[18];//17
	TCHAR	szCompanyIdentity[15];//14

	TCHAR	szProductName[20];//19
	TCHAR	szRevProductName[20];//19
	TCHAR	szProductType[7];//6
	TCHAR	szProductVersion[5];//4
	TCHAR	szProductBuild[5];//4
	TCHAR	szProductShortName[7];//6

	TCHAR	szVersionLabel[8];//7
	TCHAR	szBuildLabel[6];//5
	TCHAR	szCopyrightLabel[10];//9
	TCHAR	szBetaLabel[5];//4
	TCHAR	szRetailLabel[7];//6
	TCHAR	szTrialLabel[6];//5
	TCHAR	szSpecialLabel[8];//7
	TCHAR	szLicenseLabel[8];//7
	TCHAR	szRegisteredLabel[11];//10
	TCHAR	szReservedLabel[20];//19
	TCHAR	szDevelopedByLabel[26];//25
	TCHAR	szNoLicenseLabel[19];//18
	TCHAR	szInvalidLabel[16];//15
	TCHAR	szExpiredLabel[8];//7

	TCHAR	szCustomFileHeader[25];//24

	SSSCOMMON::SSSCOMMON()
	{
		_zcpy(szCompanyName, _T("WiViT"));//, 5);
		_zcpy(szCompanyURL, _T("http://www.wivit.com"));//, 20);
		_zcpy(szCompanySupport, _T("support@wivit.com"));//, 17);
		_zcpy(szRevSupport, _T("moc.tiviw@troppus"));//, 17);
		_zcpy(szCompanyIdentity, _T("The WiViT Team"));//, 14);

		_zcpy(szProductName, _T("SIM Status Switcher"));// 19);
		_zcpy(szRevProductName, _T("rehctiwS sutatS MIS"));// 19);
		_zcpy(szProductType, SSS_VERSION_TYPE);// 6);
		_snwprintf(szProductVersion, 4, _T("%s.%s"), SSS_VERSION_MAJOR, SSS_VERSION_MINOR);// 4
		_zcpy(szProductBuild, SSS_VERSION_BUILD);// 4);
		_zcpy(szProductShortName, _T("WiVSSS"));// 6);
		
		_zcpy(szVersionLabel, _T("Version"));// 7);
		_zcpy(szBuildLabel, _T("Build"));// 5);
		_zcpy(szCopyrightLabel, _T("Copyright"));// 9);
		_zcpy(szBetaLabel, _T("Beta"));// 4);
		_zcpy(szRetailLabel, _T("Retail"));// 6);
		_zcpy(szTrialLabel, _T("Trial"));// 5);
		_zcpy(szSpecialLabel, _T("Special"));// 7);
		_zcpy(szLicenseLabel, _T("License"));// 7);
		_zcpy(szRegisteredLabel, _T("Registered"));// 10);
		_zcpy(szReservedLabel, _T("All rights reserved"));// 19);
		_zcpy(szDevelopedByLabel, _T("Designed and Developed by"));// 25);
		_zcpy(szNoLicenseLabel, _T("No License Entered"));// 18);
		_zcpy(szInvalidLabel, _T("Invalid License"));// 15);
		_zcpy(szExpiredLabel, _T("Expired"));// 7);

		_zcpy(szCustomFileHeader, _T("WiViT Customisation File"));// 24);
	}

	SSSCOMMON::~SSSCOMMON()
	{
	}

}; // class SSSCOMMON

class SSSCOMMON;
extern SSSCOMMON *g_pSSSData;

//=====================
// Safe String Defines
//=====================
#define STRSAFE_LIB
#define STRSAFE_NO_A_FUNCTIONS
#define STRSAFE_NO_CB_FUNCTIONS

//=======================
// Non-debug definitions 
//=======================
#ifndef WIV_DEBUG

#pragma warning( disable : 4003 )  // Disable 'not enough parameters' warning

//--------------------------------
// Make trace functions invisible 
//--------------------------------

#define		TraceStart()
#define		TraceStop()
#define		TraceFileSetPrefix(sz)
#define		TracePushLevel(l)
#define		TracePopLevel()
#define		TraceGetLevel()
#define		TraceSetLevel(n)
#define		TraceSetTabWidth(n)
#define		TraceSetLineWidth(n)
#define		TraceSetFilter(w)
#define		TraceSetCommit(f)
#define		TraceWrite(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceFatal(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceError(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceWarning(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceDebug(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceInfo(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceDetail(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceInternal(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceAlways(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceEnter(a,b)
#define		TraceLeave(a,b,c)
#define		TraceMessage(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceCodeStart(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceCodeEnd(a,b,c,d,e,f,g,h,i,j,k,l)
#define		TraceCode(a,b,c,d,e,f,g,h,i,j,k,l)

//-----------------------
// Property sheet pages 
//-----------------------
#define		SSS_MAX_PROP_SHEET_PAGES		6

#else

//============================
// Include Debug definitions 
//============================

#include	"WiVTrace.h"

#endif // WIV_DEBUG

//=====================
// Default values
//=====================

//-----------------
// Display
//-----------------
#define SSS_DEFAULT_SHOW_PHONE_NUMBER		true
#define SSS_DEFAULT_SHOW_TSP				true
#define SSS_DEFAULT_SINGLE_LINE_DISPLAY		false

//-----------------
// Appearance
//-----------------
#define SSS_DEFAULT_LINE_1_BOLD_FONT		true
#define SSS_DEFAULT_LINE_2_BOLD_FONT		false
#define SSS_DEFAULT_TODAY_ICON_SET			g_dwTodayIconSetStandardPhone

//-----------------
// Actions
//-----------------
#define SSS_DEFAULT_TAP_ACTION				g_dwTodayActionRefresh
#define SSS_DEFAULT_TAH_ACTION				g_dwTodayActionShowPopup
#define SSS_DEFAULT_TODAY_ICON_TAP_ACTION	g_dwTodayActionOptions
#define SSS_DEFAULT_TODAY_ICON_TAH_ACTION	g_dwTodayActionSwitchSIM
#define SSS_DEFAULT_BUTTON_ACTION			g_dwTodayActionSwitchSIM

//-----------------
// Security
//-----------------
#define SSS_DEFAULT_PIN						WIV_EMPTY_STRING
#define SSS_DEFAULT_PIN1					{0x00,0x00,0x00,0x00}

//=====================
// Maximum sizes
//=====================
#define SSS_MAX_STATUS_PROMPT				WIV_MAX_NAME
#define SSS_MAX_PHONE_NUMBER				MAXLENGTH_ADDRESS
#define SSS_MAX_PROVIDER					MAXLENGTH_OPERATOR_LONG
#define SSS_MAX_MFG							MAXLENGTH_EQUIPINFO
#define SSS_MAX_MODEL						MAXLENGTH_EQUIPINFO
#define SSS_MAX_REVISION					MAXLENGTH_EQUIPINFO
#define SSS_MAX_IMEI						MAXLENGTH_EQUIPINFO
#define SSS_MAX_SUBSCRIBER					MAXLENGTH_USERID
#define SSS_MAX_ICCID						WIV_MAX_NAME
#define SSS_MAX_PIN							8
#define SSS_MAX_BUFFSIZE					32768
#define	SSS_MAX_AVAILABLE_ACTIONS			6
#define	SSS_MAX_ACTION_NAME_LENGTH			20
#define	SSS_MAX_AVAILABLE_ICON_SETS			12
#define	SSS_MAX_ICON_SET_NAME_LENGTH		25
#define	SSS_MAX_SUBSCRIBER_NUMBERS			5
#define	SSS_MAX_OPERATORS					5
#define	SSS_MAX_PB_LOCATION_LENGTH			20
#define SSS_MAX_BRUSH_COUNT					16

//=====================
// Registry stuff
//=====================
#define	SSS_REG_ROOT_KEY				NULL

//=====================
// Today item stuff
//=====================
#define	SSS_FLAG_PLUGIN_ENABLED			0x00000001
#define	SSS_FLAG_SHOW_ABOUT_ONLY		0x00000002

//=====================
// Phone stuff
//=====================
#define SSS_PHONE_REGISTER_RETRY_MAX	10

//=====================
// Security stuff
//=====================

//=====================
// Notifications
//=====================

//=============================
//Include application headers
//=============================
#include "resource.h"

#include "SSSGlobals.h"
#include "SSSLang.h"

#endif // INC_SSS_COMMON_H
