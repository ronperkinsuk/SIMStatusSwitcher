//////////////////////////////////////////////////////////////////////
//
// SSSPhone.cpp: Implementation of the CSSSPhone class.
//
//////////////////////////////////////////////////////////////////////

extern struct SSS_GLOBALS *SSSGlobals;
#include "SSSCommon.h"
#include "SSSToday.h"
#include "WiVUtils.h"
#include "WiVLicense.h"
#include "WiVLang.h"
#include "SSSPhone.h"

using namespace WiV;

//extern SSS_GLOBALS *SSSGlobals;

// Member Variables
static	HMODULE		m_hmInstance;
static	CSSSToday	*m_pToday;
static	CSSSPhone	*m_pThis;

// Flags
union {
	DWORD	dwFlags;
	struct  {
		unsigned int	Flags						: 18;
		unsigned int	License						: 14;
	}Options;

	struct {
		unsigned int	HidePersonalInfo			: 1;
		unsigned int	SwitchingSIM				: 1;
		unsigned int	Unlocking					: 1;
		unsigned int	NotApplicable				: 15;

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
	}PhoneBits;
} PhoneFlags;


// RIL stuff
static	HRIL		m_hrilRIL = NULL;
static	DWORD		m_dwRILParam = 0x12344321; //TODO:
static	HRESULT		m_hrGetCurrentOperatorCmdID;
static	HRESULT		m_hrGetUserIdentityCmdID;
static	HRESULT		m_hrGetCurrentAddressIdCmdID;
static	HRESULT		m_hrGetSubscriberNumbersCmdID;
static	HRESULT		m_hrGetEquipmentInfoCmdID;
static	HRESULT		m_hrGetEquipmentStateCmdID;
static	HRESULT		m_hrSetEquipmentStateCmdID;
static	HRESULT		m_hrGetPhonebookOptionsCmdID;
static	HRESULT		m_hrGetPhoneLockedStateCmdID;
static	HRESULT		m_hrUnlockPhoneCmdID;
static	HRESULT		m_hrGetRegistrationStatusCmdID;
static	HRESULT		m_hrUnregisterFromNetworkCmdID;
static	HRESULT		m_hrRegisterOnNetworkCmdID;
static	HRESULT		m_hrRILGetSIMLockingStatusCmdID;
static	HRESULT		m_hrRILSetLockingStatusCmdID;
static	HRESULT		m_hrRILChangeLockingPasswordCmdID;

static	HRESULT		m_hrGetSimICCIDRecordStatusCmdID;
static	HRESULT		m_hrSendSimCmdReadICCIDRecordCmdID;

static	HRESULT		m_hrGetDevCapsLockFacilitiesCmdID;

#ifdef SSS_V2_IMP
static	HRESULT		m_hrRILGetSignalQualityCmdID;
static	HRESULT		m_hrRILGetCellTowerInfoCmdID;
static	HRESULT		m_hrRILGetSystemTimeCmdID;
static	HRESULT		m_hrGetDevCapsSignalQualityCmdID;
#endif //#ifdef SSS_V2_IMP

//Phone stuff
static	TCHAR		m_szPhoneNumber[SSS_MAX_PHONE_NUMBER + 1];
static	TCHAR		m_szPhonePIN[SSS_MAX_PIN + 1];
static	TCHAR		m_szOperatorLongName[MAXLENGTH_OPERATOR_LONG + 1];
static	TCHAR		m_szOperatorShortName[MAXLENGTH_OPERATOR_SHORT + 1];
static	TCHAR		m_szOperatorNumericID[MAXLENGTH_OPERATOR_NUMERIC + 1];
static	TCHAR		m_szUserID[MAXLENGTH_USERID + 1];
static	TCHAR		m_szManufacturer[MAXLENGTH_EQUIPINFO + 1];
static	TCHAR		m_szModel[MAXLENGTH_EQUIPINFO + 1];
static	TCHAR		m_szRevision[MAXLENGTH_EQUIPINFO + 1];
static	TCHAR		m_szSerialNumber[MAXLENGTH_EQUIPINFO + 1];
static	TCHAR		m_szICCID[SSS_MAX_ICCID + 1];
static	TCHAR		m_aszSubscriberNumbers[SSS_MAX_SUBSCRIBER_NUMBERS][MAXLENGTH_ADDRESS + 1];

static	DWORD		m_dwCurrentAddressID;
static	DWORD		m_dwRadioPresence;
static	DWORD		m_dwRadioSupport;
static	DWORD		m_dwEquipmentState;
static	DWORD		m_dwReadyState;
static	DWORD		m_dwPhoneLockedState;
static	DWORD		m_dwRegistrationStatus;
static	DWORD		m_dwGPRSRegistrationStatus;
static	DWORD		m_dwPBStoreLocation;
static	TCHAR		m_szPBLocation[SSS_MAX_PB_LOCATION_LENGTH + 1];
static	DWORD		m_dwPBUsed;
static	DWORD		m_dwPBTotal;
static	DWORD		m_dwICCIDRecordType;
static	DWORD		m_dwICCIDRecordItemCount;
static	DWORD		m_dwICCIDRecordSize;
static	DWORD		m_dwSIMLockingStatus;

static	DWORD		m_dwRilHandle			= 0;

static	DWORD		m_dwPhoneLicenseData		= 0;
static	DWORD		m_dwPhoneLicenseConfig		= 0;
static	DWORD		m_dwValidLicenseFound		= 0;

static	int			m_nRegisterCount			= 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSSSPhone::CSSSPhone()
{
	TraceEnter(_D("CSSSPhone::CSSSPhone (Default Constructor)"));
	TraceLeave(_D("CSSSPhone::CSSSPhone (Default Constructor)"));
}

CSSSPhone::CSSSPhone(HMODULE hmInstance)
{
	TraceEnter(_D("CSSSPhone::CSSSPhone (Constructor)"));
	TraceInfo(_D("CSSSPhone::CSSSPhone (Constructor): hmInstance = <%08X>"), hmInstance);

	m_hmInstance = hmInstance;
	m_pThis = this;
	m_hrilRIL = NULL;
	memset(&PhoneFlags, 0, sizeof(PhoneFlags));
	m_dwPhoneLicenseData		= (DWORD)m_pThis;
	m_dwPhoneLicenseConfig		= (DWORD)&m_hrilRIL;

	TraceDetail(_D("CSSSPhone::CSSSPhone (Constructor): Calling LicenseRegisterNotification with 0x%08X"), &LicensePhoneNotify);
	LicenseRegisterNotification(&LicensePhoneNotify);
	TraceDetail(_D("CSSSPhone::CSSSPhone (Constructor): Back from LicenseRegisterNotification"));
	
	TraceLeave(_D("CSSSPhone::CSSSPhone (Constructor)"));
}

CSSSPhone::~CSSSPhone()
{
	TraceEnter(_D("CSSSPhone::~CSSSPhone (Destructor)"));

	TraceDetail(_D("CSSSPhone::~CSSSPhone (Destructor): Calling LicenseDeregisterNotification with 0x%08X"), &LicensePhoneNotify);
	LicenseDeregisterNotification(&LicensePhoneNotify);
	TraceDetail(_D("CSSSPhone::~CSSSPhone (Destructor): Back from LicenseDeregisterNotification"));

	m_hmInstance=NULL;
	m_hrilRIL = NULL;
	TraceLeave(_D("CSSSPhone::~CSSSPhone (Destructor)"));
}

void CSSSPhone::TodayDestroyed()
{
	TraceEnter(_D("SSSPhone::TodayDestroyed"));
	TraceDetail(_D("CSSSPhone::TodayDestroyed: Calling LicenseDeregisterNotification with 0x%08X"), &LicensePhoneNotify);
	LicenseDeregisterNotification(&LicensePhoneNotify);
	TraceDetail(_D("CSSSPhone::TodayDestroyed: Back from LicenseDeregisterNotification"));
	TraceLeave(_D("CSSSPhone::TodayDestroyed"));
}

bool CSSSPhone::PhoneLicenseNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig)
{
	TraceEnter(_D("SSSPhone::LicenseNotify"));
	
	if (lpLicenseData == NULL)
	{
		PhoneFlags.Options.License = dwLicenseConfig;
		TraceDetail(_D("SSSPhone::LicenseNotify: PhoneFlags.Options.License = 0x%08X"), PhoneFlags.Options.License);
	}
	else
	{
		m_dwPhoneLicenseData = (DWORD)lpLicenseData;
		m_dwPhoneLicenseConfig = dwLicenseConfig;
	}

	TraceDetail(_D("SSSPhone::LicenseNotify: m_dwPhoneLicenseData = 0x%08X, m_dwPhoneLicenseConfig = 0x%08X"), m_dwPhoneLicenseData, m_dwPhoneLicenseConfig);

	TraceLeave(_D("SSSPhone::LicenseNotify"));
	return true;
}

DWORD CSSSPhone::InitializeComms()
{
	HRESULT hrResult;
	DWORD	dwNotifications = RIL_NCLASS_FUNCRESULT
							 | RIL_NCLASS_NETWORK
							 | RIL_NCLASS_PHONEBOOK
							 | RIL_NCLASS_MISC
							 | RIL_NCLASS_RADIOSTATE ;

	int		nLicenseType = WIV_LICTYPE_ERROR;

	TraceEnter(_D("CSSSPhone::InitializeComms"));

	SSSGlobals = GlobalsLoad(SSSGlobals);

//	TraceDetail(_D("CSSSPhone::InitializeComms: Calling LicenseGetRandom()"));
//	m_dwConfig = GetRandom();
//	TraceDetail(_D("CSSSPhone::InitializeComms: Back from LicenseGetRandom()"));

//	TraceInfo(_D("CSSSPhone::InitializeComms: dwRand = 0x%08X"), m_dwConfig);

	TraceDetail(_D("CSSSPhone::InitializeComms, calling RILInitialize"));
	hrResult = this->RILInitialize(&m_hrilRIL);
	TraceDetail(_D("CSSSPhone::InitializeComms, Back from RILInitialize, hrResult = <%08X>, m_hrilRIL = <%08X>"),
								hrResult, m_hrilRIL);
	if (m_hrilRIL != NULL)
	{
		m_dwRilHandle = (DWORD)m_hrilRIL;

		if (PhoneFlags.PhoneBits.LicenseAdvised == 0)
		{
			m_pToday->PhoneNotify(m_dwPhoneLicenseData, (LPVOID)m_dwPhoneLicenseConfig);
		}

		m_pToday->PhoneNotify(g_dwNotifyHandle ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwRilHandle) ^ m_dwPhoneLicenseConfig));

		TraceDetail(_D("CSSSPhone::InitializeComms, calling RILEnableNotifications"));
		hrResult = this->RILEnableNotifications(m_hrilRIL, RIL_NCLASS_ALL);// | RIL_NCLASS_DEVSPECIFIC);
		TraceDetail(_D("CSSSPhone::InitializeComms, back from RILEnableNotifications, hrResult = <%08X>"), hrResult);

		_zclr(m_szSerialNumber);

		// Attempt to obtain serial number using KernelIoControl
		if (GetSerialNumber(m_szSerialNumber))
		{
			TraceDetail(_D("CSSSPhone::InitializeComms: m_szSerialNumber = <%s>"), m_szSerialNumber);

			if (_tcslen(m_szSerialNumber) != 0)
			{
				// Generate licenses based on IMEI (also generates trial license)
				TraceDetail(_D("CSSSPhone::InitializeComms: Calling GenLicenses for %s"), m_szSerialNumber);
				GenLicenses(m_szSerialNumber);
				TraceDetail(_D("CSSSPhone::InitializeComms: Back from GenLicenses for %s"), m_szSerialNumber);
			}
		}

		// Read the license from registry (also returns license type)
		TraceDetail(_D("CSSSPhone::InitializeComms: Calling ReadLicense"));
		nLicenseType = ReadLicense(); // Returns m_pLicenseData->nLicType
		TraceDetail(_D("CSSSPhone::InitializeComms: Back from ReadLicense, nLicenseType = %d"), nLicenseType);

		// If not a valid license, use IMEI encoded in license to determine validity (until radio is on)
		if (nLicenseType < WIV_LICTYPE_FULL)
		{
			TraceDetail(_D("CSSSPhone::InitializeComms: Calling GenLicenses for NULL"));
			GenLicenses(NULL);
			TraceDetail(_D("CSSSPhone::InitializeComms: Back from GenLicenses for NULL"));

			TraceDetail(_D("CSSSPhone::InitializeComms: Calling CalcLicenseType"));
			nLicenseType = CalcLicenseType(GetLicense());
			TraceDetail(_D("CSSSPhone::InitializeComms: Back from CalcLicenseType, nLicenseType = %d"), nLicenseType);
		}

		_tcsncpy(m_szSerialNumber, GetIMEI(), MAXLENGTH_EQUIPINFO);
		
		// If valid license now found, advise Today
		if (nLicenseType >= WIV_LICTYPE_FULL)
		{
			if (m_dwValidLicenseFound == 0) m_dwValidLicenseFound = (DWORD)this;

			CheckLicense(WIV_CHK_DISK, GetLicense());
			
			TraceInfo(_D("CSSSPhone::InitializeComms: Calling m_pToday->PhoneNotify"));
			m_pToday->PhoneNotify(g_dwNotifySerialNumber ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szSerialNumber) ^ m_dwPhoneLicenseConfig));
			TraceInfo(_D("CSSSPhone::InitializeComms: Back from m_pToday->PhoneNotify"));
		}

		_zclr(m_szICCID);

		TraceDetail(_D("CSSSPhone::InitializeComms, calling RILGetSimICCIDRecordStatus"));
		hrResult = this->RILGetSimICCIDRecordStatus(m_hrilRIL);
		TraceDetail(_D("CSSSPhone::InitializeComms, back from RILGetSimICCIDRecordStatus, hrResult = <%08X>"), hrResult);

		TraceDetail(_D("CSSSPhone::InitializeComms, calling RILGetPhoneLockedState"));
		hrResult = this->RILGetPhoneLockedState(m_hrilRIL);
		TraceDetail(_D("CSSSPhone::InitializeComms, back from RILGetPhoneLockedState, hrResult = <%08X>"), hrResult);

		TraceDetail(_D("CSSSPhone::InitializeComms, calling RILGetEquipmentState"));
		hrResult = this->RILGetEquipmentState(m_hrilRIL);
		TraceDetail(_D("CSSSPhone::InitializeComms, back from RILGetEquipmentState, hrResult = <%08X>"), hrResult);
		
		TraceDetail(_D("CSSSPhone::InitializeComms, calling RILGetEquipmentInfo"));
		hrResult = this->RILGetEquipmentInfo(m_hrilRIL);
		TraceDetail(_D("CSSSPhone::InitializeComms, back from RILGetEquipmentInfo, hrResult = <%08X>"), hrResult);
	}

	TraceLeave(_D("CSSSPhone::InitializeComms"));
	return hrResult;
}

DWORD CSSSPhone::StartComms()
{
	HRESULT hrResult;
	DWORD	dwRadioPresence;
	DWORD	dwVersion;

	TraceEnter(_D("CSSSPhone::StartComms"));

	_zclr(m_szManufacturer);
	_zclr(m_szModel);
	_zclr(m_szRevision);
//	_zclr(m_szSerialNumber);

//	_zclr(m_szICCID);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetSimICCIDRecordStatus"));
	hrResult = this->RILGetSimICCIDRecordStatus(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetSimICCIDRecordStatus, hrResult = <%08X>"), hrResult);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetPhoneLockedState"));
	hrResult = this->RILGetPhoneLockedState(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetPhoneLockedState, hrResult = <%08X>"), hrResult);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetEquipmentState"));
	hrResult = this->RILGetEquipmentState(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetEquipmentState, hrResult = <%08X>"), hrResult);
	
	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetEquipmentInfo"));
	hrResult = this->RILGetEquipmentInfo(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetEquipmentInfo, hrResult = <%08X>"), hrResult);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetDriverVersion"));
	hrResult = this->RILGetDriverVersion(m_hrilRIL, &dwVersion);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetDriverVersion, hrResult = <%08X>, Driver Version = <%08X>"), hrResult, dwVersion);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetRadioPresence"));
	hrResult = this->RILGetRadioPresence(m_hrilRIL, &dwRadioPresence);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetRadioPresence, hrResult = <%08X>, Radio Presence = <%08X>"), hrResult, dwRadioPresence);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetSubscriberNumbers"));
	hrResult = this->RILGetSubscriberNumbers(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetSubscriberNumbers, hrResult = <%08X>"), hrResult);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILRegisterOnNetwork"));
	m_nRegisterCount = 0;
	hrResult = this->RILRegisterOnNetwork(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILRegisterOnNetwork, hrResult = <%08X>"), hrResult);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetCurrentOperator"));
	hrResult = this->RILGetCurrentOperator(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetCurrentOperator, hrResult = <%08X>"), hrResult);

	_zclr(m_szUserID);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetUserIdentity"));
	hrResult = this->RILGetUserIdentity(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetUserIdentity, hrResult = <%08X>"), hrResult);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetCurrentAddressID"));
	hrResult = this->RILGetCurrentAddressID(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetCurrentAddressID, hrResult = <%08X>"), hrResult);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetDevCapsCaps for RIL_CAPSTYPE_LOCKFACILITIES"));
	hrResult = this->RILGetDevCaps(m_hrilRIL, RIL_CAPSTYPE_LOCKFACILITIES);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetDevCaps, hrResult = <%08X>"), hrResult);

	TraceDetail(_D("CSSSPhone::StartComms, calling RILGetLockingStatus"));
	hrResult = this->RILGetSIMLockingStatus(m_hrilRIL);
	TraceDetail(_D("CSSSPhone::StartComms, back from RILGetLockingStatus, hrResult = <%08X>"), hrResult);

#ifdef SSS_V2_IMP
	TraceInfo(_D("CSSSPhone::StartComms, calling RILGetSignalQuality"));
	hrResult = this->RILGetSignalQuality(m_hrilRIL);
	TraceInfo(_D("CSSSPhone::StartComms, back from RILGetSignalQuality, hrResult = <%08X>"), hrResult);

	TraceInfo(_D("CSSSPhone::StartComms, calling RILSetLockingStatus"));
	hrResult = this->RILSetLockingStatus(m_hrilRIL, dwFacility, lpszPassword, dwStatus);
	TraceInfo(_D("CSSSPhone::StartComms, back from RILSetLockingStatus, hrResult = <%08X>"), hrResult);

	TraceInfo(_D("CSSSPhone::StartComms, calling RILChangeLockingPassword"));
	hrResult = this->RILChangeLockingPassword(m_hrilRIL, dwFacility, dwOldPasswordType, lpszOldPassword, lpszNewPassword);
	TraceInfo(_D("CSSSPhone::StartComms, back from RILChangeLockingPassword, hrResult = <%08X>"), hrResult);
	
	TraceInfo(_D("CSSSPhone::StartComms, calling RILGetDevCapsCaps for RIL_CAPSTYPE_SIGNALQUALITYIMPLEMENTATION"));
	hrResult = this->RILGetDevCaps(m_hrilRIL, RIL_CAPSTYPE_SIGNALQUALITYIMPLEMENTATION);
	TraceInfo(_D("CSSSPhone::StartComms, back from RILGetDevCaps, hrResult = <%08X>"), hrResult);

	TraceInfo(_D("CSSSPhone::StartComms, calling RILGetSystemTime"));
	hrResult = this->RILGetSystemTime(m_hrilRIL);
	TraceInfo(_D("CSSSPhone::StartComms, back from RILGetSystemTime, hrResult = <%08X>"), hrResult);
	if (hrResult < 0)
	{
		if (hrResult == E_NOTIMPL)
		{
			m_hrRILGetSystemTimeCmdID = 0;
			TraceError(_D("CSSSPhone::StartComms: RIL_GetSystemTime function is not implemented"));
		}
		else
		{
			m_pThis->ProcessRILError(hrResult, hrResult, HRESULT_FACILITY(hrResult), RILERRORCLASS(hrResult), hrResult & 0xff); 
		}
	}

	TraceInfo(_D("CSSSPhone::StartComms, calling RILGetCellTowerInfo"));
	hrResult = this->RILGetCellTowerInfo(m_hrilRIL);
	TraceInfo(_D("CSSSPhone::StartComms, back from RILGetCellTowerInfo, hrResult = <%08X>"), hrResult);
	if (hrResult < 0)
	{

		if (hrResult == E_NOTIMPL)
		{
			m_hrRILGetCellTowerInfoCmdID = 0;
			TraceError(_D("CSSSPhone::StartComms: RIL_GetCellTowerInfo function is not implemented"));
		}
		else
		{
			m_pThis->ProcessRILError(hrResult, hrResult, HRESULT_FACILITY(hrResult), RILERRORCLASS(hrResult), hrResult & 0xff); 
		}
	}
#endif //#ifdef SSS_V2_IMP

	TraceLeave(_D("CSSSPhone::StartComms"));
	return hrResult;
}

DWORD CSSSPhone::StopComms()
{
	HRESULT hrResult;

	TraceEnter(_D("CSSSPhone::StopComms"));

	TraceInfo(_D("CSSSPhone::StopComms, calling RILDisableNotifications"));
	hrResult = this->RILDisableNotifications(m_hrilRIL, RIL_NCLASS_ALL);
	TraceInfo(_D("CSSSPhone::StopComms, back from RILDisableNotifications, hrResult = <%08X>"), hrResult);

	TraceInfo(_D("CSSSPhone::StopComms, calling RILDeinitialize"));
	hrResult = RILDeinitialize(m_hrilRIL);
	TraceInfo(_D("CSSSPhone::StopComms, back from RILDeinitialize, hrResult = <%08X>"), hrResult);
	TraceInfo(_D("CSSSPhone::StopComms, back from RILDeinitialize"));

	TraceInfo(_D("CSSSPhone::StopComms, setting m_hrilRIL to NULL"));
	m_hrilRIL=NULL;

	TraceLeave(_D("CSSSPhone::StopComms"));
	return hrResult;
}

#ifdef SSS_V2_IMP
DWORD CSSSPhone::RefreshSignalQuality()
{
	HRESULT hrResult;

	TraceEnter(_D("CSSSPhone::RefreshSignalQuality"));

	TraceInfo(_D("CSSSPhone::RefreshSignalQuality, calling RILGetSignalQuality"));
	hrResult = this->RILGetSignalQuality(m_hrilRIL);
	TraceInfo(_D("CSSSPhone::RefreshSignalQuality, back from RILGetSignalQuality, hrResult = <%08X>"), hrResult);

	TraceLeave(_D("CSSSPhone::RefreshSignalQuality"), hrResult);

	return hrResult;

}
#endif //#ifdef SSS_V2_IMP

//////////////////////////////////////////////////////////////////////
// Get Methods
//////////////////////////////////////////////////////////////////////
LPCTSTR CSSSPhone::GetPhoneNumber()
{
	TraceEnter(_D("CSSSPhone::GetPhoneNumber"));
	TraceLeave(_D("CSSSPhone::GetPhoneNumber"), (PhoneFlags.PhoneBits.HidePersonalInfo ? g_szFakePhoneNumber : m_szPhoneNumber));
	return (PhoneFlags.PhoneBits.HidePersonalInfo ? g_szFakePhoneNumber : m_szPhoneNumber);
}

LPCTSTR CSSSPhone::GetProvider()
{
	TraceEnter(_D("CSSSPhone::GetProvider"));
	TraceLeave(_D("CSSSPhone::GetProvider"), m_szOperatorLongName);
	return m_szOperatorLongName;
}

LPCTSTR CSSSPhone::GetPIN()
{
	TraceEnter(_D("CSSSPhone::GetPIN"));
	TraceInfo(_D("CSSSPhone::GetPIN, m_szPhonePIN = <%s>"), m_szPhonePIN);
	TraceLeave(_D("CSSSPhone::GetPIN"), m_szPhonePIN);
	return m_szPhonePIN;
}

void CSSSPhone::GetPhonebookDetails()
{
	HRESULT	hrResult;
	TraceEnter(_D("CSSSPhone::GetPhonebookDetails"));
	TraceInfo(_D("CSSSPhone::GetPhonebookDetails, calling RILGetPhonebookOptions"));
	hrResult = this->RILGetPhonebookOptions(m_hrilRIL);
	TraceInfo(_D("CSSSPhone::GetPhonebookDetails, back from RILGetPhonebookOptions, hrResult = <%08X>"), hrResult);
	TraceLeave(_D("CSSSPhone::GetPhonebookDetails"));
	return;
}

//////////////////////////////////////////////////////////////////////
// Set Methods
//////////////////////////////////////////////////////////////////////

//======================================================================
// Save reference to Today class
//======================================================================
void CSSSPhone::SetTodayClass(DWORD pToday)
{
	TraceEnter(_D("CSSSPhone::SetTodayClass"));
	m_pToday = (CSSSToday *)pToday;
	TraceLeave(_D("CSSSPhone::SetTodayClass"));
}

void CSSSPhone::SetPIN(LPCTSTR szPIN)
{
	TraceEnter(_D("CSSSPhone::SetPIN"));
	TraceInfo(_D("CSSSPhone::SetPIN, Setting PIN to <%s>"), szPIN);
	_tcsncpy(m_szPhonePIN, szPIN, SSS_MAX_PIN);
	TraceLeave(_D("CSSSPhone::SetPIN"));
}

void CSSSPhone::SetHidePersonalInfo(const bool blHidePersonalInfo)
{
	TraceEnter(_D("CSSSPhone::SetHidePersonalInfo"));
	PhoneFlags.PhoneBits.HidePersonalInfo = blHidePersonalInfo;
	TraceLeave(_D("CSSSPhone::SetHidePersonalInfo"));
}

void CSSSPhone::SetupPhoneData()
{
	TraceEnter(_D("CSSPhone::SetupPhoneData"));

	HRESULT hrRetVal;

	if (m_hrilRIL != NULL)
	{
		hrRetVal = this->RILGetSubscriberNumbers(m_hrilRIL);
		hrRetVal = this->RILGetCurrentOperator(m_hrilRIL);
	}

	TraceLeave(_D("CSSPhone::SetupPhoneData"));
}

DWORD CSSSPhone::GetPhoneLockedState()
{
	TraceEnter(_D("CSSSPhone::GetPhoneLockedState"));
	TraceLeave(_D("CSSSPhone::GetPhoneLockedState"), m_dwPhoneLockedState);
	return m_dwPhoneLockedState;
}

HRESULT CSSSPhone::UnlockSIM()
{
	HRESULT	hrResult = 0;

	TraceEnter(_D("CSSSPhone::UnlockSIM"));

	TraceDetail(_D("CSSSPhone::UnlockSIM, m_dwPhoneLockedState = %08X"), m_dwPhoneLockedState);
/*
#define RIL_LOCKEDSTATE_UNKNOWN                     (0x00000000)      // @constdefine Locking state unknown
#define RIL_LOCKEDSTATE_READY                       (0x00000001)      // @constdefine ME not locked
#define RIL_LOCKEDSTATE_SIM_PIN                     (0x00000002)      // @constdefine ME awaiting PIN
#define RIL_LOCKEDSTATE_SIM_PUK                     (0x00000003)      // @constdefine ME awaiting PUK
#define RIL_LOCKEDSTATE_PH_SIM_PIN                  (0x00000004)      // @constdefine ME awaiting phone-to-sim password
#define RIL_LOCKEDSTATE_PH_FSIM_PIN                 (0x00000005)      // @constdefine ME awaiting phone-to-first-sim password
#define RIL_LOCKEDSTATE_PH_FSIM_PUK                 (0x00000006)      // @constdefine ME awaiting phone-to-first-sim PUK
#define RIL_LOCKEDSTATE_SIM_PIN2                    (0x00000007)      // @constdefine ME awaiting PIN2/CHV2
#define RIL_LOCKEDSTATE_SIM_PUK2                    (0x00000008)      // @constdefine ME awaiting PUK2
#define RIL_LOCKEDSTATE_PH_NET_PIN                  (0x00000009)      // @constdefine ME awaiting network personalization PIN
#define RIL_LOCKEDSTATE_PH_NET_PUK                  (0x0000000a)      // @constdefine ME awaiting network personalization PUK
#define RIL_LOCKEDSTATE_PH_NETSUB_PIN               (0x0000000b)      // @constdefine ME awaiting network subset personalization PIN
#define RIL_LOCKEDSTATE_PH_NETSUB_PUK               (0x0000000c)      // @constdefine ME awaiting network subset personalization PUK
#define RIL_LOCKEDSTATE_PH_SP_PIN                   (0x0000000d)      // @constdefine ME awaiting service provider PIN
#define RIL_LOCKEDSTATE_PH_SP_PUK                   (0x0000000e)      // @constdefine ME awaiting service provider PUK
#define RIL_LOCKEDSTATE_PH_CORP_PIN                 (0x0000000f)      // @constdefine ME awaiting corporate personalization PIN
#define RIL_LOCKEDSTATE_PH_CORP_PUK                 (0x00000010)      // @constdefine ME awaiting corporate personalization PUK
*/
	if (m_dwPhoneLockedState == RIL_LOCKEDSTATE_SIM_PIN)
	{
		TraceInfo(_D("CSSSPhone::UnlockSIM, m_szPhonePIN = <%s>"), m_szPhonePIN);

		hrResult = this->RILUnlockPhone(m_hrilRIL, m_szPhonePIN);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::UnlockSIM, Phone not waiting for PIN, so ignore request"));
	}

	TraceLeave(_D("CSSSPhone::UnlockSIM"), hrResult);

	return hrResult;
}

HRESULT CSSSPhone::SwitchSIM()
{
	HRESULT hrResult = g_dwPhoneAccessError;

	TraceEnter(_D("CSSSPhone::SwitchSIM"));

	// Turn phone off, sending flag to indicate that it should be restarted.
	TraceInfo(_D("CSSSPhone::SwitchSIM: Calling this->TurnPhoneOff"));
	hrResult = this->TurnPhoneOff(true);
	TraceInfo(_D("CSSSPhone::SwitchSIM: Back from this->TurnPhoneOff, hrResult = <%08x>"), hrResult);
	
		
	TraceLeave(_D("CSSSPhone::SwitchSIM"), hrResult);
	return hrResult;
}

DWORD CSSSPhone::TurnPhoneOff(const bool bRestart)
{
	DWORD	dwResult;

	TraceEnter(_D("CSSSPhone::TurnPhoneOff"));

	PhoneFlags.PhoneBits.SwitchingSIM = bRestart;
	TraceInfo(_D("CSSSPhone::TurnPhoneOff: Restart flag = <%d>"), PhoneFlags.PhoneBits.SwitchingSIM);

	dwResult = this->RILSetEquipmentState(m_hrilRIL, RIL_EQSTATE_MINIMUM);

	TraceLeave(_D("CSSSPhone::TurnPhoneOff"), dwResult);

	return dwResult;
}

DWORD CSSSPhone::TurnPhoneOn()
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::TurnPhoneOn"));
	
	TraceInfo(_D("CSSSPhone::TurnPhoneOn: Calling RILSetEquipmentState"));
	hrResult = this->RILSetEquipmentState(m_hrilRIL, RIL_EQSTATE_FULL);
	TraceInfo(_D("CSSSPhone::TurnPhoneOn: Back from RILSetEquipmentState, hrResult = <%08X>"), hrResult);

	TraceLeave(_D("CSSSPhone::TurnPhoneOn"), hrResult);
	return hrResult;
}

DWORD CSSSPhone::RegisterPhone()
{
	DWORD	dwResult;

	TraceEnter(_D("CSSSPhone::RegisterPhone"));

	m_nRegisterCount = 0;
	dwResult = this->RILRegisterOnNetwork(m_hrilRIL);

	TraceLeave(_D("CSSSPhone::RegisterPhone"), dwResult);

	return dwResult;
}

DWORD CSSSPhone::GetSIMLockingStatus()
{
	TraceEnter(_D("CSSSPhone::GetSIMLockingStatus"));

	TraceLeave(_D("CSSSPhone::GetSIMLockingStatus"), m_dwSIMLockingStatus);

	return m_dwSIMLockingStatus;
}

void CSSSPhone::GetPhoneState(DWORD &dwRadioSupport, DWORD &dwEquipmentState, DWORD &dwReadyState, DWORD &dwPhoneLockedState, DWORD &dwLicense)
{
	TraceEnter(_D("CSSSPhone::GetPhoneState"));

	dwRadioSupport		= m_dwRadioSupport;
	dwEquipmentState	= m_dwEquipmentState;
	dwReadyState		= m_dwReadyState;
	dwPhoneLockedState	= m_dwPhoneLockedState;
	dwLicense			= PhoneFlags.Options.License;

	TraceDetail(_D("CSSSPhone::GetPhoneState: Radio Support = <%08X>, Equipment State = <%08X>, Ready State = <%08X>, SIM Locked State = <%08X>, License = <%08X>"),
		dwRadioSupport, dwEquipmentState, dwReadyState, dwPhoneLockedState, dwLicense);
	TraceLeave(_D("CSSSPhone::GetPhoneState"));
}

//////////////////////////////////////////////////////////////////////
// Protected methods
//////////////////////////////////////////////////////////////////////

void CSSSPhone::GetICCID(LPTSTR lpszICCID)
{
	TraceEnter(_D("CSSSPhone::GetICCID"));
	_tcsncpy(lpszICCID, m_szICCID, SSS_MAX_ICCID);
	TraceLeave(_D("CSSSPhone::GetICCID"));
}

void CSSSPhone::GetSIMInfo(LPTSTR lpszSubscriber, LPTSTR lpszPBLocation, LPDWORD lpdwPBTotal, LPDWORD lpdwPBUsed)
{
	TraceEnter(_D("CSSSPhone::GetSIMInfo"));
	if (lpszSubscriber != NULL) 
	{
		_tcsncpy(lpszSubscriber, m_szUserID, MAXLENGTH_USERID);
	}

	if (lpszPBLocation != NULL) 
	{
		_tcsncpy(lpszPBLocation, m_szPBLocation, SSS_MAX_PB_LOCATION_LENGTH);
	}

	if (lpdwPBTotal != NULL) 
	{
		*lpdwPBTotal = m_dwPBTotal;
	}
	if (lpdwPBUsed != NULL) 
	{
		*lpdwPBUsed = m_dwPBUsed;
	}

	TraceLeave(_D("CSSSPhone::GetSIMInfo"));
	return;
}

void CSSSPhone::GetEquipmentInfo(LPTSTR lpszIMEI, LPTSTR lpszManufacturer, LPTSTR lpszModel, LPTSTR lpszRevision)
{
	TraceEnter(_D("CSSSPhone::GetEquipmentInfo"));

	if (lpszIMEI != NULL) 
	{
		if (PhoneFlags.PhoneBits.HidePersonalInfo)
		{
			_tcsncpy(lpszIMEI, g_szFakeIMEI, MAXLENGTH_EQUIPINFO);
		}
		else
		{

			_tcsncpy(lpszIMEI, m_szSerialNumber, MAXLENGTH_EQUIPINFO);
		}
	}
	
	if (lpszManufacturer != NULL) 
	{
		_tcsncpy(lpszManufacturer, m_szManufacturer, MAXLENGTH_EQUIPINFO);
	}

	if (lpszModel != NULL) 
	{
		_tcsncpy(lpszModel, m_szModel, MAXLENGTH_EQUIPINFO);
	}

	if (lpszRevision != NULL) 
	{
		_tcsncpy(lpszRevision, m_szRevision, MAXLENGTH_EQUIPINFO);
	}

	TraceLeave(_D("CSSSPhone::GetEquipmentInfo"));

	return;
}

//******************************************************************************
// RIL Functions
//******************************************************************************
void CSSSPhone::ProcessRILError(const HRESULT hrCmdID, const DWORD dwErrorWord, const DWORD dwFacility, const DWORD dwClass, const DWORD dwError)
{
	TraceEnter(_D("CSSSPhone::ProcessRILError"));

	TraceInfo(_D("CSSSPhone::ProcessRILError: hrCmdID = <%08X>, Error = <%08X>"), hrCmdID, dwErrorWord);

	if (hrCmdID == m_hrGetUserIdentityCmdID)
	{
		m_hrGetUserIdentityCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetUserIdentity failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		_zclr(m_szUserID);
		m_pToday->PhoneNotify(g_dwNotifyIMSI ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szUserID) ^ m_dwPhoneLicenseConfig));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrGetCurrentOperatorCmdID)
	{
		m_hrGetCurrentOperatorCmdID = 0;

		if((dwClass == RIL_ERRORCLASS_RADIOUNAVAILABLE) && (dwErrorWord == RIL_E_RADIOOFF))
		{
			TraceWarning(_D("CSSSPhone::ProcessRILError: GetCurrentOperator failed: Radio not ready, so retrying"));
			::Sleep(500);
			this->RILGetCurrentOperator(m_hrilRIL);
		}
		else
		{
			TraceError(_D("CSSSPhone::ProcessRILError: GetCurrentOperator failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
			_zclr(m_szOperatorLongName);
			_zclr(m_szOperatorShortName);
			_zclr(m_szOperatorNumericID);
			m_pToday->PhoneNotify(g_dwNotifyOperator ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szOperatorLongName) ^ m_dwPhoneLicenseConfig));
			m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));
		}

	}else if (hrCmdID == m_hrGetCurrentAddressIdCmdID)
	{
		m_hrGetCurrentAddressIdCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetCurrentAddressId failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrGetSubscriberNumbersCmdID)
	{
		m_hrGetSubscriberNumbersCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetSubscriberNumbers failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		_tcsncpy(m_aszSubscriberNumbers[0], SSS_TEXT_PHONE_NUMBER_UNAVAILABLE, MAXLENGTH_ADDRESS);
		_tcsncpy(m_szPhoneNumber, m_aszSubscriberNumbers[0], MAXLENGTH_ADDRESS);
		m_pToday->PhoneNotify(g_dwNotifyNumber ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szPhoneNumber) ^ m_dwPhoneLicenseConfig));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrGetEquipmentInfoCmdID)
	{
		m_hrGetEquipmentInfoCmdID = 0;

		if((dwClass == RIL_ERRORCLASS_RADIOUNAVAILABLE) && (dwErrorWord == RIL_E_RADIOOFF))
		{
			TraceWarning(_D("CSSSPhone::ProcessRILError: GetEquipmentInfo failed: Radio not ready, so retrying"));
			//::Sleep(100);
			this->RILGetEquipmentInfo(m_hrilRIL);
		}
		else
		{
			TraceError(_D("CSSSPhone::ProcessRILError: GetEquipmentInfo failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
			_zclr(m_szManufacturer);
			_zclr(m_szModel);
			_zclr(m_szRevision);
//			_zclr(m_szSerialNumber);
			m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));
		}

	}else if (hrCmdID == m_hrGetEquipmentStateCmdID)
	{
		m_hrGetEquipmentStateCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetEquipmentState failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrSetEquipmentStateCmdID)
	{
		m_hrSetEquipmentStateCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: SetEquipmentState failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrGetPhonebookOptionsCmdID)
	{
		m_hrGetPhonebookOptionsCmdID = 0;

		if((dwClass == RIL_ERRORCLASS_SIM) && (dwErrorWord == RIL_E_SIMBUSY))
		{
			TraceWarning(_D("CSSSPhone::ProcessRILError: GetPhonebookOptions failed: SIM busy, so retrying"));
			::Sleep(500);
			this->RILGetPhonebookOptions(m_hrilRIL);
		}
		else
		{
			TraceError(_D("CSSSPhone::ProcessRILError: GetPhonebookOptions failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
			m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));
		}

	}else if (hrCmdID == m_hrUnlockPhoneCmdID)
	{
		m_hrUnlockPhoneCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: UnlockPhone failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));

		m_dwPhoneLockedState = RIL_LOCKEDSTATE_UNKNOWN;
		PhoneFlags.PhoneBits.Unlocking = false;

		m_pToday->PhoneNotify(g_dwNotifySIMUnlockError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrGetPhoneLockedStateCmdID)
	{
		m_hrGetPhoneLockedStateCmdID = 0;

		if((dwClass == RIL_ERRORCLASS_RADIOUNAVAILABLE) && (dwErrorWord == RIL_E_RADIOOFF))
		{
			TraceWarning(_D("CSSSPhone::ProcessRILError: GetPhoneLockedState failed: Radio not ready, so retrying"));
			//::Sleep(100);
			this->RILGetPhoneLockedState(m_hrilRIL);
		}
		else if ((dwClass == RIL_ERRORCLASS_SIM) && (dwErrorWord == RIL_E_SIMNOTINSERTED))
		{
			TraceWarning(_D("CSSSPhone::ProcessRILError: GetPhoneLockedState failed: SIM is missing"));
			m_dwPhoneLockedState = RIL_E_SIMNOTINSERTED;
			m_pToday->PhoneNotify(g_dwNotifySIMLock ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwPhoneLockedState) ^ m_dwPhoneLicenseConfig));
		}
		else if ((dwClass == RIL_ERRORCLASS_SIM) && (dwErrorWord == RIL_E_SIMWRONG))
		{
			TraceWarning(_D("CSSSPhone::ProcessRILError: GetPhoneLockedState failed: SIM is missing or invalid"));
			m_dwPhoneLockedState = RIL_E_SIMWRONG;
			m_pToday->PhoneNotify(g_dwNotifySIMLock ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwPhoneLockedState) ^ m_dwPhoneLicenseConfig));
		}
		else
		{
			TraceError(_D("CSSSPhone::ProcessRILError: GetPhoneLockedState failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
			m_dwPhoneLockedState = RIL_LOCKEDSTATE_UNKNOWN;
			m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwPhoneLockedState) ^ m_dwPhoneLicenseConfig));
		}

	}else if (hrCmdID == m_hrUnregisterFromNetworkCmdID)
	{
		m_hrUnregisterFromNetworkCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: UnregisterFromNetwork failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrRegisterOnNetworkCmdID)
	{
		m_hrRegisterOnNetworkCmdID = 0;

		if (m_nRegisterCount < SSS_PHONE_REGISTER_RETRY_MAX)
		{
			m_nRegisterCount += 1;

			TraceWarning(_D("CSSSPhone::ProcessRILError: RegisterOnNetwork failed, retry number %d"), m_nRegisterCount);
			::Sleep(500);
			this->RILRegisterOnNetwork(m_hrilRIL);
		}
		else
		{
			TraceError(_D("CSSSPhone::ProcessRILError: RegisterOnNetwork failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
			m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));
		}

	}else if (hrCmdID == m_hrGetRegistrationStatusCmdID)
	{
		m_hrGetRegistrationStatusCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetRegistrationStatus failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}
#ifdef SSS_V2_IMP
	else if (hrCmdID == m_hrGetDevCapsSignalQualityCmdID)
	{
		m_hrGetDevCapsSignalQualityCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetDevCaps failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}
#endif // #ifdef SSS_V2_IMP
	
	else if (hrCmdID == m_hrGetDevCapsLockFacilitiesCmdID)
	{
		m_hrGetDevCapsLockFacilitiesCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetDevCaps failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrGetSimICCIDRecordStatusCmdID)
	{
		m_hrGetSimICCIDRecordStatusCmdID = 0;

		_zclr(m_szICCID);

		if((dwClass == RIL_ERRORCLASS_RADIOUNAVAILABLE) && (dwErrorWord == RIL_E_RADIOOFF))
		{
			if ((m_dwPhoneLockedState != RIL_E_SIMNOTINSERTED) && (m_dwPhoneLockedState != RIL_E_SIMWRONG))
			{
				TraceWarning(_D("CSSSPhone::ProcessRILError: GetSimICCIDRecordStatus failed: Radio not ready, so retrying"));
				//::Sleep(100);
				this->RILGetSimICCIDRecordStatus(m_hrilRIL);
			}
		}
		else
		{
			TraceError(_D("CSSSPhone::ProcessRILError: GetSimICCIDRecordStatus failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
			m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));
		}

	}else if (hrCmdID == m_hrSendSimCmdReadICCIDRecordCmdID)
	{
		m_hrSendSimCmdReadICCIDRecordCmdID = 0;

		_zclr(m_szICCID);

		TraceError(_D("CSSSPhone::ProcessRILError: SendSimCmdReadICCIDRecord failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrRILGetSIMLockingStatusCmdID)
	{
		m_hrRILGetSIMLockingStatusCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetSIMLockingStatus failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrRILSetLockingStatusCmdID)
	{
		m_hrRILSetLockingStatusCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: SetLockingStatus failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrRILChangeLockingPasswordCmdID)
	{
		m_hrRILChangeLockingPasswordCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: ChangeLockingPassword failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}
#ifdef SSS_V2_IMP	
	else if (hrCmdID == m_hrRILGetSignalQualityCmdID)
	{
		m_hrRILGetSignalQualityCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetSignalQuality failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}
	else if (hrCmdID == m_hrRILGetCellTowerInfoCmdID)
	{
		m_hrRILGetCellTowerInfoCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetCellTowerInfo failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

	}else if (hrCmdID == m_hrRILGetSystemTimeCmdID)
	{
		m_hrRILGetSystemTimeCmdID = 0;

		TraceError(_D("CSSSPhone::ProcessRILError: GetSystemTime failed: %s"), GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));
	}
#endif //#ifdef SSS_V2_IMP
	else
	{
		TraceError(_D("CSSSPhone::ProcessRILError: Unrecognised hrCmdID (%08X) failed: %s"), hrCmdID, GetErrorDetails(dwFacility, dwClass, dwErrorWord));
		m_pToday->PhoneNotify(g_dwNotifyAPIError ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));
	}

	TraceLeave(_D("CSSSPhone::ProcessRILError"));

	return;
}

#ifdef WIV_DEBUG
LPCTSTR CSSSPhone::GetErrorDetails(const DWORD dwFacility, const DWORD dwErrorClass, const DWORD dwErrorCode)
{
	static TCHAR	szErrorDetails[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
	TCHAR			szFacility[WIV_MAX_STRING + 1];
	TCHAR			szErrorClass[WIV_MAX_STRING + 1];
	TCHAR			szErrorCode[WIV_MAX_STRING + 1];

	TraceEnter(_D("CSSSPhone::GetErrorDetails"));
	switch (dwFacility)
	{
	case FACILITY_RIL :
		{
			_tcsncpy(szFacility, _D("RIL"), WIV_MAX_STRING);
			break;
		}
	default :
		{
			_tcsncpy(szFacility, _D("Unknown"), WIV_MAX_STRING);
		}
	}

	switch (dwErrorClass)
	{
	case RIL_ERRORCLASS_NONE :
		{
			_tcsncpy(szErrorClass, _D("Misc error"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_PASSWORD :
		{
			_tcsncpy(szErrorClass, _D("Unspecified phone failure"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_SIM :
		{
			_tcsncpy(szErrorClass, _D("Problem with the SIM"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_NETWORKACCESS :
		{
			_tcsncpy(szErrorClass, _D("Can't access the network"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_NETWORK :
		{
			_tcsncpy(szErrorClass, _D("Error in the network"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_MOBILE :
		{
			_tcsncpy(szErrorClass, _D("Error in the mobile"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_NETWORKUNSUPPORTED :
		{
			_tcsncpy(szErrorClass, _D("Unsupported by the network"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_MOBILEUNSUPPORTED :
		{
            _tcsncpy(szErrorClass, _D("Unsupported by the mobile"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_BADPARAM :
		{
			_tcsncpy(szErrorClass, _D("An invalid parameter was supplied"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_STORAGE :
		{
			_tcsncpy(szErrorClass, _D("Error relating to storage"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_SMSC :
		{
			_tcsncpy(szErrorClass, _D("Error relates to the SMSC"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_DESTINATION :
		{
			_tcsncpy(szErrorClass, _D("Error in the destination mobile"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_DESTINATIONUNSUPPORTED :
		{
			_tcsncpy(szErrorClass, _D("Unsupported by destination mobile"), WIV_MAX_STRING);
			break;
		}
	case RIL_ERRORCLASS_RADIOUNAVAILABLE :
		{
			_tcsncpy(szErrorClass, _D("The Radio Module is Off or a radio module may not be present"), WIV_MAX_STRING);
			break;
		}
	default :
		{
			_tcsncpy(szErrorClass, _D("Unknown"), WIV_MAX_STRING);
		}
	}

	switch (dwErrorCode)
	{
	case RIL_E_PHONEFAILURE :
		{
			_tcsncpy(szErrorCode, _D("Unspecified phone failure"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NOCONNECTION :
		{
			_tcsncpy(szErrorCode, _D("RIL has no connection to the phone"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_LINKRESERVED :
		{
			_tcsncpy(szErrorCode, _D("RIL's link to the phone is reserved"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_OPNOTALLOWED :
		{
			_tcsncpy(szErrorCode, _D("Attempted operation isn't allowed"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_OPNOTSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Attempted operation isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_PHSIMPINREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("PH-SIM PIN is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_PHFSIMPINREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("PH-FSIM PIN is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_PHFSIMPUKREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("PH-FSIM PUK is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMNOTINSERTED :
		{
			_tcsncpy(szErrorCode, _D("SIM isn't inserted into the phone"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMPINREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("SIM PIN is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMPUKREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("SIM PUK is required to perform this operation"), WIV_MAX_STRING);
			break;
		}

	case RIL_E_SIMFAILURE :
		{
			_tcsncpy(szErrorCode, _D("SIM failure was detected"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMBUSY :
		{
			_tcsncpy(szErrorCode, _D("SIM is busy"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMWRONG :
		{
			_tcsncpy(szErrorCode, _D("Incorrect SIM was inserted"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INCORRECTPASSWORD :
		{
			_tcsncpy(szErrorCode, _D("Incorrect password was supplied"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMPIN2REQUIRED :
		{
			_tcsncpy(szErrorCode, _D("SIM PIN2 is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMPUK2REQUIRED :
		{
			_tcsncpy(szErrorCode, _D("SIM PUK2 is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MEMORYFULL :
		{
			_tcsncpy(szErrorCode, _D("Storage memory is full"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INVALIDINDEX :
		{
			_tcsncpy(szErrorCode, _D("Invalid storage index was supplied"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NOTFOUND :
		{
			_tcsncpy(szErrorCode, _D("A requested storage entry was not found"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MEMORYFAILURE :
		{
			_tcsncpy(szErrorCode, _D("Storage memory failure"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_TEXTSTRINGTOOLONG :
		{
			_tcsncpy(szErrorCode, _D("Supplied text string is too long"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INVALIDTEXTSTRING :
		{
			_tcsncpy(szErrorCode, _D("Supplied text string contains invalid characters"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_DIALSTRINGTOOLONG :
		{
			_tcsncpy(szErrorCode, _D("Supplied dial string is too long"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INVALIDDIALSTRING :
		{
			_tcsncpy(szErrorCode, _D("Supplied dial string contains invalid characters"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NONETWORKSVC :
		{
			_tcsncpy(szErrorCode, _D("Network service isn't available"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NETWORKTIMEOUT :
		{
			_tcsncpy(szErrorCode, _D("Network operation timed out"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_EMERGENCYONLY :
		{
			_tcsncpy(szErrorCode, _D("Network can only be used for emergency calls"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NETWKPINREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("Network Personalization PIN is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NETWKPUKREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("Network Personalization PUK is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SUBSETPINREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("Network Subset Personalization PIN is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SUBSETPUKREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("Network Subset Personalization PUK is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SVCPINREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("Service Provider Personalization PIN is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SVCPUKREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("Service Provider Personalization PUK is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_CORPPINREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("Corporate Personalization PIN is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_CORPPUKREQUIRED :
		{
			_tcsncpy(szErrorCode, _D("Corporate Personalization PUK is required to perform this operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_TELEMATICIWUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Telematic interworking isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SMTYPE0UNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Type 0 messages aren't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_CANTREPLACEMSG :
		{
			_tcsncpy(szErrorCode, _D("Existing message cannot be replaced"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_PROTOCOLIDERROR :
		{
			_tcsncpy(szErrorCode, _D("Uspecified error related to the message Protocol ID"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_DCSUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Specified message Data Coding Scheme isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MSGCLASSUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Specified message class isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_DCSERROR :
		{
			_tcsncpy(szErrorCode, _D("Unspecified error related to the message Data Coding Scheme"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_CMDCANTBEACTIONED :
		{
			_tcsncpy(szErrorCode, _D("Specified message Command cannot be executed"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_CMDUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Specified message Command isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_CMDERROR :
		{
			_tcsncpy(szErrorCode, _D("Unspecified error related to the message Command"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MSGBODYHEADERERROR :
		{
			_tcsncpy(szErrorCode, _D("Unspecified error related to the message Body or Header"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SCBUSY :
		{
			_tcsncpy(szErrorCode, _D("Message Service Center is busy"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NOSCSUBSCRIPTION :
		{
			_tcsncpy(szErrorCode, _D("No message Service Center subscription"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SCSYSTEMFAILURE :
		{
			_tcsncpy(szErrorCode, _D("Message service Center system failure occurred"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INVALIDADDRESS :
		{
			_tcsncpy(szErrorCode, _D("Specified address is invalid"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_DESTINATIONBARRED :
		{
			_tcsncpy(szErrorCode, _D("Message destination is barred"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_REJECTEDDUPLICATE :
		{
			_tcsncpy(szErrorCode, _D("Duplicate message was rejected"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_VPFUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Specified message Validity Period Format isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_VPUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Specified message Validity Period isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMMSGSTORAGEFULL :
		{
			_tcsncpy(szErrorCode, _D("Message storage on the SIM is full"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NOSIMMSGSTORAGE :
		{
			_tcsncpy(szErrorCode, _D("SIM isn't capable of storing messages"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMTOOLKITBUSY :
		{
			_tcsncpy(szErrorCode, _D("SIM Application Toolkit is busy"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SIMDOWNLOADERROR :
		{
			_tcsncpy(szErrorCode, _D("SIM data download error"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MSGSVCRESERVED :
		{
			_tcsncpy(szErrorCode, _D("Messaging service is reserved"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INVALIDMSGPARAM :
		{
			_tcsncpy(szErrorCode, _D("One of the message parameters is invalid"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_UNKNOWNSCADDRESS :
		{
			_tcsncpy(szErrorCode, _D("Unknown message Service Center address was specified"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_UNASSIGNEDNUMBER :
		{
			_tcsncpy(szErrorCode, _D("Specified message destination address is a currently unassigned phone number"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MSGBARREDBYOPERATOR :
		{
			_tcsncpy(szErrorCode, _D("Message sending was barred by an operator"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MSGCALLBARRED :
		{
			_tcsncpy(szErrorCode, _D("Message sending was prevented by outgoing calls barring"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MSGXFERREJECTED :
		{
			_tcsncpy(szErrorCode, _D("Sent message has been rejected by the receiving equipment"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_DESTINATIONOUTOFSVC :
		{
			_tcsncpy(szErrorCode, _D("Message could not be delivered because destination equipment is out of service"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_UNIDENTIFIEDSUBCRIBER :
		{
			_tcsncpy(szErrorCode, _D("Sender's mobile ID isn't registered"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SVCUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("Requested messaging service isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_UNKNOWNSUBSCRIBER :
		{
			_tcsncpy(szErrorCode, _D("Sender isn't recognized by the network"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NETWKOUTOFORDER :
		{
			_tcsncpy(szErrorCode, _D("Long-term network failure"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NETWKTEMPFAILURE :
		{
			_tcsncpy(szErrorCode, _D("Short-term network failure"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_CONGESTION :
		{
			_tcsncpy(szErrorCode, _D("Operation failed because of the high network traffic"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_RESOURCESUNAVAILABLE :
		{
			_tcsncpy(szErrorCode, _D("Unspecified resources weren't available"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SVCNOTSUBSCRIBED :
		{
			_tcsncpy(szErrorCode, _D("Sender isn't subscribed for the requested messaging service"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SVCNOTIMPLEMENTED :
		{
			_tcsncpy(szErrorCode, _D("Requested messaging service isn't implemented on the network"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INVALIDMSGREFERENCE :
		{
			_tcsncpy(szErrorCode, _D("Imvalid message reference value was used"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INVALIDMSG :
		{
			_tcsncpy(szErrorCode, _D("Message was determined to be invalid for unspecified reasons"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INVALIDMANDATORYINFO :
		{
			_tcsncpy(szErrorCode, _D("Mandatory message information is invalid or missing"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MSGTYPEUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("The message type is unsupported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_ICOMPATIBLEMSG :
		{
			_tcsncpy(szErrorCode, _D("Sent message isn't compatible with the network"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_INFOELEMENTUNSUPPORTED :
		{
			_tcsncpy(szErrorCode, _D("An information element specified in the message isn't supported"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_PROTOCOLERROR :
		{
			_tcsncpy(szErrorCode, _D("Unspefied protocol error"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NETWORKERROR :
		{
			_tcsncpy(szErrorCode, _D("Unspecified network error"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_MESSAGINGERROR :
		{
			_tcsncpy(szErrorCode, _D("Unspecified messaging error"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NOTREADY :
		{
			_tcsncpy(szErrorCode, _D("RIL isn't yet ready to perform the requested operation"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_TIMEDOUT :
		{
			_tcsncpy(szErrorCode, _D("Operation timed out"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_CANCELLED :
		{
			_tcsncpy(szErrorCode, _D("Operation was cancelled"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NONOTIFYCALLBACK :
		{
			_tcsncpy(szErrorCode, _D("Requested operation requires an RIL notification callback, which wasn't provided"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_OPFMTUNAVAILABLE :
		{
			_tcsncpy(szErrorCode, _D("Operator format isn't available"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_NORESPONSETODIAL :
		{
			_tcsncpy(szErrorCode, _D("Dial operation hasn't received a response for a long time"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_SECURITYFAILURE :
		{
			_tcsncpy(szErrorCode, _D("Security failure"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_RADIOFAILEDINIT :
		{
			_tcsncpy(szErrorCode, _D("Radio failed to initialize correctly"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_DRIVERINITFAILED :
		{
			_tcsncpy(szErrorCode, _D("There was a problem initializing the radio driver"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_RADIONOTPRESENT :
		{
			_tcsncpy(szErrorCode, _D("The Radio is not present"), WIV_MAX_STRING);
			break;
		}
	case RIL_E_RADIOOFF :
		{
            _tcsncpy(szErrorCode, _D("The Radio is in Off mode"), WIV_MAX_STRING);
			break;
		}
	default :
		{
			_tcsncpy(szErrorCode, _D("Unknown"), WIV_MAX_STRING);
		}
	}

	_snwprintf(szErrorDetails, WIV_MAX_STRING, _D("Facility: %04X (%s), Error Class: %02X (%s), Error Code: %02X (%s)"), dwFacility, szFacility, dwErrorClass, szErrorClass, dwErrorCode & 0xFF, szErrorCode);

	TraceDetail(_D("CSSSPhone::GetErrorDetails: Error = <%s>"), szErrorDetails);
	TraceLeave(_D("CSSSPhone::GetErrorDetails"), szErrorDetails);

	return szErrorDetails;
}
#endif //#ifdef WIV_DEBUG

void CSSSPhone::ProcessRILResult(const HRESULT hrCmdID, LPCVOID lpData, const DWORD cbData)
{ 

	TraceEnter(_D("CSSSPhone::ProcessRILResult"));
	TraceInfo(_D("CSSSPhone::ProcessRILResult: hrCmdID = <%08X>"), hrCmdID);

	if (hrCmdID == m_hrGetUserIdentityCmdID)
	{
		m_hrGetUserIdentityCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetUserIdentity, lpData is NULL"));
			_zclr(m_szUserID);
			m_pToday->PhoneNotify(g_dwNotifyIMSI ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szUserID) ^ m_dwPhoneLicenseConfig));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		TraceDetail(_D("CSSSPhone::ProcessRILResult: GetUserIdentity: cbData = %d"), cbData);
		TraceDetail(_D("CSSSPhone::ProcessRILResult: GetUserIdentity: lpData = <%s>"), BtoS((LPBYTE)lpData,strlen((char *)lpData)));

		// Convert ASCII to Unicode
		AtoU((char *)lpData, m_szUserID, strlen((char *)lpData));

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetUserIdentity: UserID = <%s>"), m_szUserID);
		if (PhoneFlags.PhoneBits.HidePersonalInfo)
		{
			_tcsncpy(m_szUserID, g_szFakeUserID, MAXLENGTH_USERID);
		}

		m_pToday->PhoneNotify(g_dwNotifyIMSI ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szUserID) ^ m_dwPhoneLicenseConfig));

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}

	if (hrCmdID == m_hrGetCurrentOperatorCmdID)
	{
		m_hrGetCurrentOperatorCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetCurrentOperator, lpData is NULL"));
			_zclr(m_szOperatorLongName);
			_zclr(m_szOperatorShortName);
			_zclr(m_szOperatorNumericID);
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		RILOPERATORNAMES *ril = (RILOPERATORNAMES *)lpData;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetCurrentOperator: dwParams = <%08X>"), ril->dwParams);

		if (ril->dwParams & RIL_PARAM_ON_LONGNAME)
		{
			AtoU(ril->szLongName, m_szOperatorLongName, strlen(ril->szLongName));
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetCurrentOperator: Long Name = <%s>"), m_szOperatorLongName);
			m_pToday->PhoneNotify(g_dwNotifyOperator ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szOperatorLongName) ^ m_dwPhoneLicenseConfig));
		}
		if (ril->dwParams & RIL_PARAM_ON_SHORTNAME)
		{
			AtoU(ril->szShortName, m_szOperatorShortName, strlen(ril->szShortName));
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetCurrentOperator: Short Name = <%s>"), m_szOperatorShortName);
			m_pToday->PhoneNotify(g_dwNotifyOperator ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szOperatorShortName) ^ m_dwPhoneLicenseConfig));
		}
		if (ril->dwParams & RIL_PARAM_ON_NUMNAME)
		{
			AtoU(ril->szNumName, m_szOperatorNumericID, strlen(ril->szNumName));
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetCurrentOperator: Numeric ID = <%s>"), m_szOperatorNumericID);
			m_pToday->PhoneNotify(g_dwNotifyOperator ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szOperatorNumericID) ^ m_dwPhoneLicenseConfig));
		}

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}

	if (hrCmdID == m_hrGetCurrentAddressIdCmdID)
	{
		m_hrGetCurrentAddressIdCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetCurrentAddressId, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		m_dwCurrentAddressID = *(LPDWORD)lpData;
		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetCurrentAddressId: CurrentAddressID = <%d>"), m_dwCurrentAddressID);

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}

	if (hrCmdID == m_hrGetSubscriberNumbersCmdID)
	{
		m_hrGetSubscriberNumbersCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetSubscriberNumbers, lpData is NULL"));
			_tcsncpy(m_aszSubscriberNumbers[0], SSS_TEXT_PHONE_NUMBER_UNAVAILABLE, MAXLENGTH_ADDRESS);
			_tcsncpy(m_szPhoneNumber, m_aszSubscriberNumbers[0], MAXLENGTH_ADDRESS);
			m_pToday->PhoneNotify(g_dwNotifyNumber ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szPhoneNumber) ^ m_dwPhoneLicenseConfig));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		LPRILSUBSCRIBERINFO lpril = (LPRILSUBSCRIBERINFO)lpData;
		
//#define RIL_PARAM_SI_DESCRIPTION                    (0x00000002) // @paramdefine
//#define RIL_PARAM_SI_SPEED                          (0x00000004) // @paramdefine
//#define RIL_PARAM_SI_SERVICE                        (0x00000008) // @paramdefine
//#define RIL_PARAM_SI_ITC                            (0x00000010) // @paramdefine
//#define RIL_PARAM_SI_ADDRESSID                      (0x00000020) // @paramdefine
		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSubscriberNumbers: dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_SI_ADDRESS)
		{
			int	nCount = cbData / sizeof(RILSUBSCRIBERINFO);

			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSubscriberNumbers: cbData = %d, Count = %d"), cbData, nCount);

			if (nCount > SSS_MAX_SUBSCRIBER_NUMBERS) nCount = SSS_MAX_SUBSCRIBER_NUMBERS;

			for (int i = 0 ; i < nCount ; ++i)
			{
				_tcsncpy(m_aszSubscriberNumbers[i], lpril[i].raAddress.wszAddress, MAXLENGTH_ADDRESS);
				TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSubscriberNumbers: Subscriber Number %d = <%s>"), i, m_aszSubscriberNumbers[i]);
			}

			_tcsncpy(m_szPhoneNumber, m_aszSubscriberNumbers[0], MAXLENGTH_ADDRESS);
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSubscriberNumbers: Phone Number = <%s>"), m_szPhoneNumber);
			m_pToday->PhoneNotify(g_dwNotifyNumber ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szPhoneNumber) ^ m_dwPhoneLicenseConfig));
		}

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}

	if (hrCmdID == m_hrGetEquipmentInfoCmdID)
	{
		m_hrGetEquipmentInfoCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetEquipmentInfo, lpData is NULL"));
			_zclr(m_szManufacturer);
			_zclr(m_szModel);
			_zclr(m_szRevision);
//			_zclr(m_szSerialNumber);
			m_pToday->PhoneNotify(g_dwNotifyEquipmentInfo ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(INVALID_HANDLE_VALUE) ^ m_dwPhoneLicenseConfig));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		LPRILEQUIPMENTINFO lpril = (LPRILEQUIPMENTINFO)lpData;

		TraceDetail(_D("CSSSPhone::ProcessRILResult: GetEquipmentInfo: dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_EI_MANUFACTURER)
		{
			AtoU(lpril->szManufacturer, m_szManufacturer, strlen(lpril->szManufacturer));
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetEquipmentInfo: Manufacturer = <%s>"), m_szManufacturer);
		}
		if (lpril->dwParams & RIL_PARAM_EI_MODEL)
		{
			AtoU(lpril->szModel, m_szModel, strlen(lpril->szModel));
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetEquipmentInfo: Model = <%s>"), m_szModel);
		}
		if (lpril->dwParams & RIL_PARAM_EI_REVISION)
		{
			AtoU(lpril->szRevision, m_szRevision, strlen(lpril->szRevision));
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetEquipmentInfo: Revision = <%s>"), m_szRevision);
		}
		if (lpril->dwParams & RIL_PARAM_EI_SERIALNUMBER)
		{
//			if (m_dwValidLicenseFound != (DWORD)this)
//			{
				int nLicenseType;

				AtoU(lpril->szSerialNumber, m_szSerialNumber, strlen(lpril->szSerialNumber));
				TraceDetail(_D("CSSSPhone::ProcessRILResult: GetEquipmentInfo: Serial Number = <%s>"), m_szSerialNumber);

				// Use actual IMEI for full/beta/special licenses
				TraceDetail(_D("CSSSPhone::ProcessRILResult: Calling GenLicenses for %s"), m_szSerialNumber);
				GenLicenses(m_szSerialNumber);
				TraceDetail(_D("CSSSPhone::ProcessRILResult: Back from GenLicenses for %s"), m_szSerialNumber);

				TraceDetail(_D("CSSSPhone::ProcessRILResult: Calling ReadLicense"));
				nLicenseType = ReadLicense(); // returns nLicType
				TraceDetail(_D("CSSSPhone::ProcessRILResult: Back from ReadLicense"));

				if (nLicenseType >= WIV_LICTYPE_FULL)
				{
					if (m_dwValidLicenseFound == 0) m_dwValidLicenseFound = (DWORD)this;
				}

//				TraceDetail(_D("Result: m_pToday %d"), m_dwValidLicenseFound);
				TraceInfo(_D("CSSSPhone::ProcessRILResult: Calling m_pToday->PhoneNotify"));
				m_pToday->PhoneNotify(g_dwNotifySerialNumber ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szSerialNumber) ^ m_dwPhoneLicenseConfig));
				TraceInfo(_D("CSSSPhone::ProcessRILResult: Back from m_pToday->PhoneNotify"));

//			}

//			TraceDetail(_D("CSSSPhone::ProcessRILResult: Calling EnablePowerNotifications"));
//			EnablePowerNotifications();
//			TraceDetail(_D("CSSSPhone::ProcessRILResult: Back from EnablePowerNotifications"));
		}

		//DWORD dwParams = lpril->dwParams;
//		TraceInfo(_D("CSSSPhone::ProcessRILResult: Calling EncodeParams"));
//		EncodeParams(lpril->dwParams, m_dwRilHandle);
//		TraceInfo(_D("CSSSPhone::ProcessRILResult: Back from EncodeParams"));

		TraceInfo(_D("CSSSPhone::ProcessRILResult: Calling m_pToday->PhoneNotify"));
		m_pToday->PhoneNotify(g_dwNotifyEquipmentInfo ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(lpril) ^ m_dwPhoneLicenseConfig));
		TraceInfo(_D("CSSSPhone::ProcessRILResult: Back from m_pToday->PhoneNotify"));

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}

	if (hrCmdID == m_hrGetEquipmentStateCmdID)
	{
		m_hrGetEquipmentStateCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetEquipmentState, lpData is NULL"));
			m_dwRadioSupport = RIL_RADIOSUPPORT_UNKNOWN;
			m_dwEquipmentState = RIL_EQSTATE_UNKNOWN;
			m_dwReadyState = RIL_READYSTATE_NONE;
			m_pToday->PhoneNotify(g_dwNotifyRadioState ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(INVALID_HANDLE_VALUE) ^ m_dwPhoneLicenseConfig));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		LPRILEQUIPMENTSTATE lpril = (LPRILEQUIPMENTSTATE)lpData;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetEquipmentState: dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_RADIOSUPPORT)
		{
			m_dwRadioSupport = lpril->dwRadioSupport;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetEquipmentState: Radio Support = <%08X>"), m_dwRadioSupport);
		}
		if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_EQSTATE)
		{
			m_dwEquipmentState = lpril->dwEqState;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetEquipmentState: Equipment State = <%08X>"), m_dwEquipmentState);
		}
		if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_READYSTATE)
		{
			m_dwReadyState = lpril->dwReadyState;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetEquipmentState: Ready State = <%08X>"), m_dwReadyState);
		}

		DWORD dwParams = lpril->dwParams;
		//EncodeRILParams(dwParams, m_dwRilHandle);

		m_pToday->PhoneNotify(g_dwNotifyRadioState ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(lpril) ^ m_dwPhoneLicenseConfig));

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrSetEquipmentStateCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: SetEquipmentState: OK"));

		m_hrSetEquipmentStateCmdID = 0;

		m_pToday->PhoneNotify(g_dwNotifyRadioState ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

		if (PhoneFlags.PhoneBits.SwitchingSIM)
		{
			TraceInfo(_D("CSSSPhone::ProcessRILResult: SetEquipmentState: Switching flag set, so turn phone on"));

			PhoneFlags.PhoneBits.SwitchingSIM = false;
			this->TurnPhoneOn();
		}

		this->RILGetPhoneLockedState(m_hrilRIL);

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrGetPhonebookOptionsCmdID)
	{
		m_hrGetPhonebookOptionsCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		LPRILPHONEBOOKINFO lpril = (LPRILPHONEBOOKINFO)lpData;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_PBI_STORELOCATION)
		{
			m_dwPBStoreLocation = lpril->dwStoreLocation;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Store Location = <%08X>"), m_dwPBStoreLocation);

			switch (m_dwPBStoreLocation)
			{
			case RIL_PBLOC_UNKNOWN :
				{
					TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Store Location: Unknown"));
					_tcsncpy(m_szPBLocation, SSS_TEXT_PHONEBOOK_UNKNOWN, SSS_MAX_PB_LOCATION_LENGTH);
					break;	
				}
			case RIL_PBLOC_SIMEMERGENCY :
				{
					TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Store Location: Emergency numbers"));
					_tcsncpy(m_szPBLocation, SSS_TEXT_PHONEBOOK_EMERGENCY, SSS_MAX_PB_LOCATION_LENGTH);
					break;	
				}
			case RIL_PBLOC_SIMFIXDIALING :
				{
					TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Store Location: Fixed dialing"));
					_tcsncpy(m_szPBLocation, SSS_TEXT_PHONEBOOK_FIXED, SSS_MAX_PB_LOCATION_LENGTH);
					break;	
				}
			case RIL_PBLOC_SIMLASTDIALING :
				{
					TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Store Location: Recent calls list"));
					_tcsncpy(m_szPBLocation, SSS_TEXT_PHONEBOOK_RECENT, SSS_MAX_PB_LOCATION_LENGTH);
					break;	
				}
			case RIL_PBLOC_OWNNUMBERS :
				{
					TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Store Location: Own numbers"));
					_tcsncpy(m_szPBLocation, SSS_TEXT_PHONEBOOK_OWN, SSS_MAX_PB_LOCATION_LENGTH);
					break;	
				}
			case RIL_PBLOC_SIMPHONEBOOK :
				{
					TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Store Location: SIM phonebook"));
					_tcsncpy(m_szPBLocation, SSS_TEXT_PHONEBOOK_SIM, SSS_MAX_PB_LOCATION_LENGTH);
					break;	
				}
			}
		}
		if (lpril->dwParams & RIL_PARAM_PBI_USED)
		{
			m_dwPBUsed = lpril->dwUsed;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Used Entries = <%08X>"), m_dwPBUsed);
		}
		if (lpril->dwParams & RIL_PARAM_PBI_TOTAL)
		{
			m_dwPBTotal = lpril->dwTotal;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhonebookOptions: Total Entries = <%08X>"), m_dwPBTotal);
		}

		//DWORD dwParams = lpril->dwParams;
//		EncodeParams(lpril->dwParams, m_dwRilHandle);

		m_pToday->PhoneNotify(g_dwNotifyPhonebook ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(lpril) ^ m_dwPhoneLicenseConfig));

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrUnlockPhoneCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: UnlockPhone: OK"));

		m_hrUnlockPhoneCmdID = 0;
		PhoneFlags.PhoneBits.Unlocking = false;

		m_pToday->PhoneNotify(g_dwNotifySIMLock ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrGetPhoneLockedStateCmdID)
	{
		m_hrGetPhoneLockedStateCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetPhoneLockedState, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		m_dwPhoneLockedState = *(LPDWORD)lpData;
		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetPhoneLockedState: m_dwPhoneLockedState = <%08X>"), m_dwPhoneLockedState);

		m_pToday->PhoneNotify(g_dwNotifySIMLock ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwPhoneLockedState) ^ m_dwPhoneLicenseConfig));

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrUnregisterFromNetworkCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: UnregisterFromNetwork: OK"));

		m_hrUnregisterFromNetworkCmdID = 0;

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrRegisterOnNetworkCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: RegisterOnNetwork: OK"));

		m_hrRegisterOnNetworkCmdID = 0;
		m_nRegisterCount = 0;

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrGetRegistrationStatusCmdID)
	{
		m_hrGetRegistrationStatusCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetRegistrationStatus, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		m_dwRegistrationStatus = *(LPDWORD)lpData;
		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetRegistrationStatus: m_dwRegistrationStatus = <%08X>"), m_dwRegistrationStatus);

		m_pToday->PhoneNotify(g_dwNotifyRegistration ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwRegistrationStatus) ^ m_dwPhoneLicenseConfig));

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
#ifdef SSS_V2_IMP	
	if (hrCmdID == m_hrGetDevCapsSignalQualityCmdID)
	{
		m_hrGetDevCapsSignalQualityCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetDevCaps, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}
/*
#define RIL_CAPS_SIGNALQUALITY_NOTIFICATION         (0x00000001)      // @constdefine The Radio Module can deliver unsolicited Signal Quality Notifications
#define RIL_CAPS_SIGNALQUALITY_POLLING              (0x00000002)      // @constdefine The Higher layers can poll the radio module in order to get the Signal Quality
*/

		DWORD dwSignalQuality = *(LPDWORD)lpData;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetDevCaps: Signal Quality Capabilities = %08X"), dwSignalQuality);

//		m_pToday->PhoneNotify(g_dwNotifySignalQuality, dwSignalQuality);

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
#endif //#ifdef SSS_V2_IMP

	if (hrCmdID == m_hrGetDevCapsLockFacilitiesCmdID)
	{
		m_hrGetDevCapsLockFacilitiesCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetDevCaps, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

/*
#define RIL_CAPS_LOCKFACILITY_CNTRL                 (0x00000001)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_PH_SIM                (0x00000002)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_PH_FSIM               (0x00000004)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_SIM                   (0x00000008)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_SIM_PIN2              (0x00000010)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_SIM_FIXEDIALING       (0x00000020)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_NETWORKPERS           (0x00000040)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_NETWORKSUBPERS        (0x00000080)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_SERVICEPROVPERS       (0x00000100)      // @constdefine TBD
#define RIL_CAPS_LOCKFACILITY_CORPPERS              (0x00000200)      // @constdefine TBD
*/
		DWORD dwLockFacilities = *(LPDWORD)lpData;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetDevCaps: dwLockFacilities = %08X"), dwLockFacilities);

//		m_pToday->PhoneNotify(g_dwNotifyLockFacilities, dwLockFacilities);

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}

	if (hrCmdID == m_hrGetSimICCIDRecordStatusCmdID)
	{
		m_hrGetSimICCIDRecordStatusCmdID = 0;

		_zclr(m_szICCID);

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetSimICCIDRecordStatus, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		LPRILSIMRECORDSTATUS lpril = (LPRILSIMRECORDSTATUS)lpData;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSimICCIDRecordStatus: dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_SRS_RECORDTYPE)
		{
			m_dwICCIDRecordType = lpril->dwRecordType;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSimICCIDRecordStatus: SIM Record Type = <%08X>"), m_dwICCIDRecordType);
		}
		if (lpril->dwParams & RIL_PARAM_SRS_ITEMCOUNT)
		{
			m_dwICCIDRecordItemCount = lpril->dwItemCount;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSimICCIDRecordStatus: SIM Record Count = <%08X>"), m_dwICCIDRecordItemCount);
		}
		if (lpril->dwParams & RIL_PARAM_SRS_SIZE)
		{
			m_dwICCIDRecordSize = lpril->dwSize;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSimICCIDRecordStatus: SIM Record Size = <%08X>"), m_dwICCIDRecordSize);
		}

		// Get the locked state
		this->RILGetPhoneLockedState(m_hrilRIL);

		// Now go read the ICCID record
		this->ReadICCIDRecord(m_hrilRIL, m_dwICCIDRecordType, m_dwICCIDRecordSize);

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrSendSimCmdReadICCIDRecordCmdID)
	{
		m_hrSendSimCmdReadICCIDRecordCmdID = 0;

		_zclr(m_szICCID);

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: SendSimCmdReadICCIDRecord, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		LPRILSIMRESPONSE	lpril = (LPRILSIMRESPONSE)lpData;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: SendSimCmdReadICCIDRecord: dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_SR_STATUSWORD1)
		{
			TraceInfo(_D("CSSSPhone::ProcessRILResult: SendSimCmdReadICCIDRecord: dwStatusWord1 = <%08X>"), lpril->dwStatusWord1);
		}
		if (lpril->dwParams & RIL_PARAM_SR_STATUSWORD2)
		{
			TraceInfo(_D("CSSSPhone::ProcessRILResult: SendSimCmdReadICCIDRecord: dwStatusWord2 = <%08X>"), lpril->dwStatusWord2);
		}

		if (lpril->dwParams & RIL_PARAM_SR_RESPONSE)
		{
			TraceInfo(_D("CSSSPhone::ProcessRILResult: SendSimCmdReadICCIDRecord: Response = <%s>"), BtoS(lpril->pbResponse, 9));

			// Convert binary ICCID into TCHAR string with byte nibbles reversed

			TCHAR	szICCID[SSS_MAX_ICCID + 1];
			int		i,j;

			memset(szICCID,0,sizeof(szICCID));
			_tcsncpy(szICCID, g_szICCIDPrefix, 2);

			m_dwICCIDRecordSize -= 1;

			TraceInfo(_D("CSSSPhone::ProcessRILResult: SendSimCmdReadICCIDRecord: Converting ICCID"));
			for (i= 0, j = 2 ; i < (int)m_dwICCIDRecordSize ; )
			{
				char c2 = ((lpril->pbResponse[i] & 0xf0) >> 4) + 0x30;
				char c1 = (lpril->pbResponse[i++] & 0x0f) + 0x30;

				szICCID[j++] = (TCHAR)c1;
				szICCID[j++] = (TCHAR)c2;
				TraceDetail(_D("CSSSPhone::ProcessRILResult: SendSimCmdReadICCIDRecord: Converting ICCID: c1 = <%c>, c2 = <%c>, j = <%d>"), c1, c2, j-2);
			}

			_tcsncpy(m_szICCID, szICCID, (_tcslen(szICCID) - 1));
			m_szICCID[_tcslen(szICCID)-1] = WIV_NULL_CHAR;

			TraceInfo(_D("CSSSPhone::ProcessRILResult: SendSimCmdReadICCIDRecord: ICCID length = %d, ICCID = <%s>"), _tcslen(m_szICCID), m_szICCID);

			m_pToday->PhoneNotify(g_dwNotifyICCID ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(m_szICCID) ^ m_dwPhoneLicenseConfig));
		}

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrRILGetSIMLockingStatusCmdID)
	{
/*
#define RIL_LOCKINGSTATUS_DISABLED                  (0x00000001)      // @constdefine Disable
#define RIL_LOCKINGSTATUS_ENABLED                   (0x00000002)      // @constdefine Enabled
*/
		m_hrRILGetSIMLockingStatusCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetLockingStatus, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		m_dwSIMLockingStatus = *(LPDWORD)lpData;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetLockingStatus: dwLockingStatus = <%08X>"), m_dwSIMLockingStatus);

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}

	if (hrCmdID == m_hrRILSetLockingStatusCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: SetLockingStatus: OK"));

		m_hrRILSetLockingStatusCmdID = 0;

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrRILChangeLockingPasswordCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: ChangeLockingPassword: OK"));

		m_hrRILChangeLockingPasswordCmdID = 0;

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
#ifdef SSS_V2_IMP	
	if (hrCmdID == m_hrRILGetSignalQualityCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSignalQuality: OK"));

		m_hrRILGetSignalQualityCmdID = 0;

		if (lpData == NULL)
		{
			TraceError(_D("CSSSPhone::ProcessRILResult: GetSignalQuality, lpData is NULL"));
			TraceLeave(_D("CSSSPhone::ProcessRILResult"));
			return;
		}

		LPRILSIGNALQUALITY lpril = (LPRILSIGNALQUALITY)lpData;

		int		nSignalStrength;
		int		nMinSignalStrength;
		int		nMaxSignalStrength;
		DWORD	dwBitErrorRate;
		int		nLowSignalStrength;
		int		nHighSignalStrength;

		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSignalQuality: dwParams = <%08X>"), lpril->dwParams);

		if (lpril->dwParams & RIL_PARAM_SQ_SIGNALSTRENGTH)
		{
			nSignalStrength = lpril->nSignalStrength;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSignalQuality: Signal Strength = <%08X>"), nSignalStrength);
		}

		if (lpril->dwParams & RIL_PARAM_SQ_MINSIGNALSTRENGTH)
		{
			nMinSignalStrength = lpril->nMinSignalStrength;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSignalQuality: Minimum Signal Strength = <%08X>"), nMinSignalStrength);
		}

		if (lpril->dwParams & RIL_PARAM_SQ_MAXSIGNALSTRENGTH)
		{
			nMaxSignalStrength = lpril->nMaxSignalStrength;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSignalQuality: Maximum Signal Strength = <%08X>"), nMaxSignalStrength);
		}

		if (lpril->dwParams & RIL_PARAM_SQ_BITERRORRATE)
		{
			dwBitErrorRate = lpril->dwBitErrorRate;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSignalQuality: Bit Error Rate = <%08X>"), dwBitErrorRate);
		}

		if (lpril->dwParams & RIL_PARAM_SQ_LOWSIGNALSTRENGTH)
		{
			nLowSignalStrength = lpril->nLowSignalStrength;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSignalQuality: Low Signal Strength = <%08X>"), nLowSignalStrength);
		}

		if (lpril->dwParams & RIL_PARAM_SQ_HIGHSIGNALSTRENGTH)
		{
			nHighSignalStrength = lpril->nHighSignalStrength;
			TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSignalQuality: High Signal Strength = <%08X>"), nHighSignalStrength);
		}
		
		m_pToday->PhoneNotify(g_dwNotifySignalQuality, lpril);

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrRILGetCellTowerInfoCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetCellTowerInfo: OK"));

		m_hrRILGetCellTowerInfoCmdID = 0;

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
	
	if (hrCmdID == m_hrRILGetSystemTimeCmdID)
	{
		TraceInfo(_D("CSSSPhone::ProcessRILResult: GetSystemTime: OK"));

		m_hrRILGetSystemTimeCmdID = 0;

		TraceLeave(_D("CSSSPhone::ProcessRILResult"));
		return;
	}
#endif //#ifdef SSS_V2_IMP	
	TraceError(_D("CSSSPhone::ProcessRILResult: Unrecognised hrCmdID (%08X)"), hrCmdID);

	TraceLeave(_D("CSSSPhone::ProcessRILResult"));
	return;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func RIL function result callback
//
// @comm This function is called to send a return value after and asynchronous
//       RIL function call
//
// -----------------------------------------------------------------------------
void CALLBACK RILResultCallback(DWORD dwCode, HRESULT hrCmdID, const void *lpData, DWORD cbData, DWORD dwParam)
{
	m_pThis->RILResult(dwCode, hrCmdID, lpData, cbData, dwParam);
	return;
}

void CSSSPhone::RILResult(DWORD dwCode, HRESULT hrCmdID, const void *lpData, DWORD cbData, DWORD dwParam)
{
//	SetTraceLevel(90);
	TraceEnter(_D("CSSSPhone::RILResult"));

	TraceInfo(_D("CSSSPhone::RILResult: dwCode=<%08x>, hrCmdID=<%08x>, cbData=%d, dwParam = %08x"),
								dwCode, hrCmdID, cbData, dwParam);

	switch (NCLASS_FROM_NOTIFICATION(dwCode))
	{
	// RIL_NCLASS_FUNCRESULT (0x00000000)
	// API call results
	case RIL_NCLASS_FUNCRESULT:
		{
			TraceDetail(_D("CSSSPhone::RILResult: Function Result: <%08x>"), dwCode & 0xff);
			switch (dwCode)
			{
			// RIL_RESULT_OK (0x00000001 | RIL_NCLASS_FUNCRESULT)
			// RIL API call succeeded; lpData points to data
			case RIL_RESULT_OK:
				{
					TraceDetail(_D("CSSSPhone::RILResult: RIL API call succeeded:"));

					m_pThis->ProcessRILResult(hrCmdID, lpData, cbData); 

					break;
				}

			// RIL_RESULT_NOCARRIER (0x00000002 | RIL_NCLASS_FUNCRESULT)
			// RIL API failed because no carrier was detected; lpData is NULL
			case RIL_RESULT_NOCARRIER:
				{
					TraceDetail(_D("CSSSPhone::RILResult: RIL API failed because no carrier was detected"));
					break;
				}

			// RIL_RESULT_ERROR (0x00000003 | RIL_NCLASS_FUNCRESULT)
			// RIL API failed; lpData points to RIL_E_* constant
			case RIL_RESULT_ERROR:
				{
					if (lpData == NULL)
					{
						TraceError(_D("CSSSPhone::RILResult: hrCmdID = <%08X>, RIL API failed, but unable to determine error because lpData is NULL"), hrCmdID);
					}
					else
					{
						DWORD	dwErrorWord = *(LPDWORD)lpData;
						DWORD	dwError = dwErrorWord & 0xff;
						DWORD	dwClass = RILERRORCLASS(dwErrorWord);
						DWORD	dwFacility = HRESULT_FACILITY(dwErrorWord);

						TraceWarning(_D("CSSSPhone::RILResult: RIL API failed, processing"));
						m_pThis->ProcessRILError(hrCmdID, dwErrorWord, dwFacility, dwClass, dwError); 
					}
					break;
				}

			// RIL_RESULT_NODIALTONE (0x00000004 | RIL_NCLASS_FUNCRESULT)
			// RIL API failed because no dialtone was detected; lpData is NULL
			case RIL_RESULT_NODIALTONE:
				{
					TraceError(_D("CSSSPhone::RILResult: RIL API failed because no dial tone was detected"));
					break;
				}

			// RIL_RESULT_BUSY (0x00000005 | RIL_NCLASS_FUNCRESULT)
			// RIL API failed because the line was busy; lpData is NULL
			case RIL_RESULT_BUSY:
				{
					TraceError(_D("CSSSPhone::RILResult: RIL API failed because the line was busy"));
					break;
				}

			// RIL_RESULT_NOANSWER (0x00000006 | RIL_NCLASS_FUNCRESULT)
			// RIL API failed because of the lack of answer; lpData is NULL
			case RIL_RESULT_NOANSWER:
				{
					TraceError(_D("CSSSPhone::RILResult: RIL API failed because of no answer"));
					break;
				}

			// RIL_RESULT_CALLABORTED (0x00000007 | RIL_NCLASS_FUNCRESULT)
			// RIL API failed because it was cancelled prior to completion; lpData is NULL
			case RIL_RESULT_CALLABORTED:
				{
					TraceError(_D("CSSSPhone::RILResult: RIL API failed because it was cancelled prior to completion"));
					break;
				}
			}
			break;
		}
	}	
	
	TraceLeave(_D("CSSSPhone::RILResult"));
	return;
}
// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func RIL notification callback
//
// @comm This function is called when the radio sends an unsolicited notification
//
// -----------------------------------------------------------------------------
void CALLBACK RILNotifyCallback(DWORD dwCode, const void *lpData, DWORD cbData, DWORD dwParam)
{
	m_pThis->RILNotify(dwCode, lpData, cbData, dwParam);
	return;
}

void CSSSPhone::RILNotify(DWORD dwCode, const void *lpData, DWORD cbData, DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::RILNotify"));

	TraceInfo(_D("CSSSPhone::RILNotify: dwCode=<%08x>, cbData=%d, dwParam = %08x"),
								dwCode,	cbData, dwParam);

	switch (NCLASS_FROM_NOTIFICATION(dwCode))
	{

	// RIL_NCLASS_NETWORK (0x00040000)
	// Network-related notifications
	case RIL_NCLASS_NETWORK:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: Network-related Notification: <%08x>"), dwCode & 0xff);
			m_pThis->ProcessNetworkNotification(dwCode, lpData, cbData, dwParam);
			break;
		}

	// RIL_NCLASS_PHONEBOOK (0x00100000)
	// Phone book notifications
	case RIL_NCLASS_PHONEBOOK:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: Phone book Notification: <%08x>"), dwCode & 0xff);
			m_pThis->ProcessPhonebookNotification(dwCode, lpData, cbData, dwParam);
			break;
		}

	// RIL_NCLASS_MISC (0x00400000)
	// Miscellaneous notifications
	case RIL_NCLASS_MISC:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: Miscellaneous Notification: <%08x>"), dwCode & 0xff);
			m_pThis->ProcessMiscellaneousNotification(dwCode, lpData, cbData, dwParam);
			break;
		}

	// RIL_NCLASS_RADIOSTATE (0x00800000)
	// Notifications Pertaining to changes in Radio State
	case RIL_NCLASS_RADIOSTATE:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: Radio State Change Notification: <%08x>"), dwCode & 0xff);
			m_pThis->ProcessRadioStateChangeNotification(dwCode, lpData, cbData, dwParam);
			break;
		}

#ifdef SSS_V2_IMP
	// RIL_NCLASS_CALLCTRL (0x00010000)
	// Call control notifications
	case RIL_NCLASS_CALLCTRL:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: Call Control Notification: <%08x>"), dwCode & 0xff);
			m_pThis->ProcessCallCtrlNotification(dwCode, lpData, cbData, dwParam);
			break;
		}

	// RIL_NCLASS_MESSAGE (0x00020000)
	// Messaging notifications
	case RIL_NCLASS_MESSAGE:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: SMS Message Notification: <%08x>"), dwCode & 0xff);
			m_pThis->ProcessSMSMsgNotification(dwCode, lpData, cbData, dwParam);
			break;
		}

	// RIL_NCLASS_SUPSERVICE (0x00080000)
	// Supplementary service notifications
	case RIL_NCLASS_SUPSERVICE:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: Supplementary service Notification: <%08x>"), dwCode & 0xff);
			m_pThis->ProcessSupServiceNotification(dwCode, lpData, cbData, dwParam);
			break;
		}

	// RIL_NCLASS_SIMTOOLKIT (0x00200000)
	// SIM Toolkit notifications
	case RIL_NCLASS_SIMTOOLKIT:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: SIM Toolkit Notification: <%08x>"), dwCode & 0xff);
			m_pThis->ProcessSIMToolkitNotification(dwCode, lpData, cbData, dwParam);
			break;
		}

	// RIL_NCLASS_DEVSPECIFIC (0x80000000)
	// Reserved for device specific notifications
	case RIL_NCLASS_DEVSPECIFIC:
		{
			TraceInfo(_D("CSSSPhone::RILNotify: Device specific Notification: <%08x>"), dwCode & 0xff);
			break;
		}
#endif // SSS_V2_IMP
	}

	TraceLeave(_D("CSSSPhone::RILNotify"));
//	SetTraceLevel(0);
	return;
}

void CSSSPhone::ProcessRadioStateChangeNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::ProcessRadioStateChangeNotification"));

	switch (dwCode)
	{
	// RIL_NOTIFY_RADIOEQUIPMENTSTATECHANGED (0x00000001 | RIL_NCLASS_RADIOSTATE)
	// Carries a STRUCT (RILEQUIPMENTSTATE) stating The Radio equipment state has changed, also notifies a driver defined Radio ON or OFF state
	case RIL_NOTIFY_RADIOEQUIPMENTSTATECHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessRadioStateChangeNotification: lpData is NULL"));
				break;
			}

			LPRILEQUIPMENTSTATE lpril = (LPRILEQUIPMENTSTATE)lpData;

			TraceInfo(_D("CSSSPhone::ProcessRadioStateChangeNotification: dwParams = <%08X>"), lpril->dwParams);
			TraceInfo(_D("CSSSPhone::ProcessRadioStateChangeNotification: The Radio equipment state has changed"));

			if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_RADIOSUPPORT)
			{
				TraceInfo(_D("CSSSPhone::ProcessRadioStateChangeNotification: Old Radio Support = <%08X>, New Radio Support = <%08X>"),
										m_dwRadioSupport, lpril->dwRadioSupport);
				m_dwRadioSupport = lpril->dwRadioSupport;
			}
			if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_EQSTATE)
			{
				TraceInfo(_D("CSSSPhone::ProcessRadioStateChangeNotification: Old Equipment State = <%08X>, New Equipment State = <%08X>"),
										m_dwEquipmentState, lpril->dwEqState);

				if ((lpril->dwEqState != m_dwEquipmentState) && (lpril->dwEqState & RIL_EQSTATE_FULL))
				{
					HRESULT	hrResult;
					TraceInfo(_D("CSSSPhone::ProcessRadioStateChangeNotification: Calling RILGetSimICCIDRecordStatus"));
//					hrResult = this->RILGetEquipmentState(m_hrilRIL);
					hrResult = this->RILGetSimICCIDRecordStatus(m_hrilRIL);
//					hrResult = this->RILGetPhoneLockedState(m_hrilRIL);
//					hrResult = this->RILGetEquipmentInfo(m_hrilRIL);
					TraceInfo(_D("CSSSPhone::ProcessRadioStateChangeNotification: Back from RILGetSimICCIDRecordStatus, hrResult = <%08X>"), hrResult);
				}

				m_dwEquipmentState = lpril->dwEqState;
			}

			if (lpril->dwParams & RIL_PARAM_EQUIPMENTSTATE_READYSTATE)
			{
				TraceInfo(_D("CSSSPhone::ProcessRadioStateChangeNotification: Old Ready State = <%08X>, New Ready State = <%08X>"),
										m_dwReadyState, lpril->dwReadyState);
				m_dwReadyState = lpril->dwReadyState;
			}

			if ((!(m_dwRadioSupport & RIL_RADIOSUPPORT_ON))
				|| (!(m_dwEquipmentState & RIL_EQSTATE_FULL))
				|| (!(m_dwReadyState & RIL_READYSTATE_UNLOCKED)))
			{
				_zclr(m_szPhoneNumber);
				_zclr(m_szOperatorLongName);
			}

			//DWORD dwParams = lpril->dwParams;
//			EncodeParams(lpril->dwParams, m_dwRilHandle);

			m_pToday->PhoneNotify(g_dwNotifyRadioState ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(lpril) ^ m_dwPhoneLicenseConfig));
			break;
		}

	// RIL_NOTIFY_RADIOPRESENCECHANGED (0x00000002 | RIL_NCLASS_RADIOSTATE)
	// Carries a dword (RIL_RADIOPRESENCE_*) stating that a Radio Module/Driver has been changed (removed, inserted, etc)
	case RIL_NOTIFY_RADIOPRESENCECHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessRadioStateChangeNotification: lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessRadioStateChangeNotification: Radio Module/Driver has been changed (removed, inserted, etc), old value = <%08X>, new value = <%08X>"),
										m_dwRadioPresence, *(LPDWORD)lpData);

			m_dwRadioPresence = *(LPDWORD)lpData;

			m_pToday->PhoneNotify(g_dwNotifyRadioPresence ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwRadioPresence) ^ m_dwPhoneLicenseConfig));
			break;
		}
	}

	TraceLeave(_D("CSSSPhone::ProcessRadioStateChangeNotification"));

	return;
}

void CSSSPhone::ProcessMiscellaneousNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::ProcessMiscellaneousNotification"));

	switch (dwCode)
	{
	// RIL_NOTIFY_SIMNOTACCESSIBLE (0x00000001 | RIL_NCLASS_MISC)
	// SIM card has been removed or has failed to respond; lpData is NULL
	case RIL_NOTIFY_SIMNOTACCESSIBLE:
		{
			TraceWarning(_D("CSSSPhone::ProcessMiscellaneousNotification: SIM card has been removed or has failed to respond"));

			m_dwPhoneLockedState = RIL_E_SIMNOTINSERTED;
			m_pToday->PhoneNotify(g_dwNotifySIMLock ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwPhoneLockedState) ^ m_dwPhoneLicenseConfig));
			break;
		}

	// RIL_NOTIFY_DTMFSIGNAL (0x00000002 | RIL_NCLASS_MISC)
	// A DTMF signal has been detected; lpData points to char
	case RIL_NOTIFY_DTMFSIGNAL:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessMiscellaneousNotification: RIL_NOTIFY_DTMFSIGNAL, lpData is NULL"));
				break;
			}

			char cChar = *(char *)lpData;
			TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: A DTMF signal has been detected, character = <%c>"), cChar);
			break;
		}

	// RIL_NOTIFY_GPRSCLASS_NETWORKCHANGED (0x00000003 | RIL_NCLASS_MISC)
	// Network has indicated a change in GPRS class
    // lpData points to a DWORD containing the new RIL_GPRSCLASS_* value
	case RIL_NOTIFY_GPRSCLASS_NETWORKCHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessMiscellaneousNotification: RIL_NOTIFY_GPRSCLASS_NETWORKCHANGED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: Network has indicated a change in GPRS class"));
			break;
		}

	// RIL_NOTIFY_GPRSCLASS_RADIOCHANGED (0x00000004 | RIL_NCLASS_MISC)
	// The radio has indicated a change in GPRS class
    // lpData points to a DWORD containing the new RIL_GPRSCLASS_* value
	case RIL_NOTIFY_GPRSCLASS_RADIOCHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessMiscellaneousNotification: RIL_NOTIFY_GPRSCLASS_RADIOCHANGED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: The radio has indicated a change in GPRS class"));
			break;
		}

	// RIL_NOTIFY_SIGNALQUALITY (0x00000005 | RIL_NCLASS_MISC)
	// Signal Quality Notification
    // lpData points to a RILSIGNALQUALITY structure
	case RIL_NOTIFY_SIGNALQUALITY:
		{
#ifdef SSS_V2_IMP
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessMiscellaneousNotification: RIL_NOTIFY_SIGNALQUALITY, lpData is NULL"));
				break;
			}

			LPRILSIGNALQUALITY lpril = (LPRILSIGNALQUALITY)lpData;

			TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: Signal Quality, dwParams = <%08X>"), lpril->dwParams);

			if (lpril->dwParams & RIL_PARAM_SQ_SIGNALSTRENGTH)
			{
				int nSignalStrength = lpril->nSignalStrength;
				TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: Signal Quality, Signal Strength = <%08X>"), nSignalStrength);
			}

			if (lpril->dwParams & RIL_PARAM_SQ_MINSIGNALSTRENGTH)
			{
				int nMinSignalStrength = lpril->nMinSignalStrength;
				TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: Signal Quality, Minimum Signal Strength = <%08X>"), nMinSignalStrength);
			}

			if (lpril->dwParams & RIL_PARAM_SQ_MAXSIGNALSTRENGTH)
			{
				int nMaxSignalStrength = lpril->nMaxSignalStrength;
				TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: Signal Quality, Maximum Signal Strength = <%08X>"), nMaxSignalStrength);
			}

			if (lpril->dwParams & RIL_PARAM_SQ_BITERRORRATE)
			{
				DWORD dwBitErrorRate = lpril->dwBitErrorRate;
				TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: Signal Quality, Bit Error Rate = <%08X>"), dwBitErrorRate);
			}

			if (lpril->dwParams & RIL_PARAM_SQ_LOWSIGNALSTRENGTH)
			{
				int nLowSignalStrength = lpril->nLowSignalStrength;
				TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: Signal Quality, Low Signal Strength = <%08X>"), nLowSignalStrength);
			}

			if (lpril->dwParams & RIL_PARAM_SQ_HIGHSIGNALSTRENGTH)
			{
				int nHighSignalStrength = lpril->nHighSignalStrength;
				TraceInfo(_D("CSSSPhone::ProcessMiscellaneousNotification: Signal Quality, High Signal Strength = <%08X>"), nHighSignalStrength);
			}
#endif // #ifdef SSS_V2_IMP
			break;
		}
	}

	TraceLeave(_D("CSSSPhone::ProcessMiscellaneousNotification"));

	return;
}

void CSSSPhone::ProcessPhonebookNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::ProcessPhonebookNotification"));

	switch (dwCode)
	{
	// RIL_NOTIFY_PHONEBOOKENTRYSTORED (0x00000001 | RIL_NCLASS_PHONEBOOK)
	// A phonebook entry has been added to storage; lpData points to the storage
    // index assigned to the new entry (ifdwIndex is RIL_PBINDEX_FIRSTAVAILABLE, the new entry was stored in the first available location)
	case RIL_NOTIFY_PHONEBOOKENTRYSTORED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessPhonebookNotification: RIL_NOTIFY_PHONEBOOKENTRYSTORED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessPhonebookNotification: A phonebook entry has been added to storage"));
			break;
		}

	// RIL_NOTIFY_PHONEBOOKENTRYDELETED (0x00000002 | RIL_NCLASS_PHONEBOOK)
	// A phonebook entry has been deleted from storage; lpData points to the storage index occupied by the deleted entry
	case RIL_NOTIFY_PHONEBOOKENTRYDELETED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessPhonebookNotification: RIL_NOTIFY_PHONEBOOKENTRYDELETED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessPhonebookNotification: A phonebook entry has been deleted from storage"));
			break;
		}

	// RIL_NOTIFY_PHONEBOOKSTORAGECHANGED (0x00000003 | RIL_NCLASS_PHONEBOOK)
	// Phonebook storage location has been changed; lpData points to RIL_PBLOC_* constant
	case RIL_NOTIFY_PHONEBOOKSTORAGECHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessPhonebookNotification: RIL_NOTIFY_PHONEBOOKSTORAGECHANGED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessPhonebookNotification: Phonebook storage location has been changed"));
			break;
		}
	}

	TraceLeave(_D("CSSSPhone::ProcessPhonebookNotification"));

	return;
}

void CSSSPhone::ProcessNetworkNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::ProcessNetworkNotification"));

	switch (dwCode)
	{
	// RIL_NOTIFY_REGSTATUSCHANGED (0x00000001 | RIL_NCLASS_NETWORK)
	// Network registration status has changed; lpData points to the new status (RIL_REGSTAT_* constant)
	case RIL_NOTIFY_REGSTATUSCHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessNetworkNotification: RIL_NOTIFY_REGSTATUSCHANGED, lpData is NULL"));
				break;
			}

			TraceDetail(_D("CSSSPhone::ProcessNetworkNotification: Network registration status has changed, old status = <%08X>, new status = <%08X>"),
										m_dwRegistrationStatus, *(LPDWORD)lpData);
			m_dwRegistrationStatus = *(LPDWORD)lpData;

			m_pToday->PhoneNotify(g_dwNotifyRegistration ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(&m_dwRegistrationStatus) ^ m_dwPhoneLicenseConfig));
			break;
		}

	// RIL_NOTIFY_CALLMETER (0x00000002 | RIL_NCLASS_NETWORK)
	// Call meter has changed; lpData points to a DWORD containing new current call meter value
	case RIL_NOTIFY_CALLMETER:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessNetworkNotification: RIL_NOTIFY_CALLMETER, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessNetworkNotification: Call meter has changed"));
			break;
		}

	// RIL_NOTIFY_CALLMETERMAXREACHED (0x00000003 | RIL_NCLASS_NETWORK)
	// Call meter maximum has been reached; lpData is NULL
	case RIL_NOTIFY_CALLMETERMAXREACHED:
		{
			TraceInfo(_D("CSSSPhone::ProcessNetworkNotification: Call meter maximum has been reached"));
			break;
		}

	// RIL_NOTIFY_GPRSREGSTATUSCHANGED (0x00000004 | RIL_NCLASS_NETWORK)
	// Network registration status has changed; lpData points to the new status (RIL_REGSTAT_* constant)
	case RIL_NOTIFY_GPRSREGSTATUSCHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessNetworkNotification: RIL_NOTIFY_GPRSREGSTATUSCHANGED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessNetworkNotification: GPRS Network registration status has changed, old status = <%08X>, new status = <%08X>"),
										m_dwGPRSRegistrationStatus, *(LPDWORD)lpData);
			m_dwGPRSRegistrationStatus = *(LPDWORD)lpData;
			break;
		}
	}

	TraceLeave(_D("CSSSPhone::ProcessNetworkNotification"));

	return;
}

#ifdef SSS_V2_IMP
void CSSSPhone::ProcessSIMToolkitNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::ProcessSIMToolkitNotification"));

	switch (dwCode)
	{
	// RIL_NOTIFY_SIMTOOLKITCMD (0x00000001 | RIL_NCLASS_SIMTOOLKIT)
	// A SIM Toolkit command was not handled by the radio; lpData points to array of bytes containing the command
	case RIL_NOTIFY_SIMTOOLKITCMD:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSIMToolkitNotification: RIL_NOTIFY_SIMTOOLKITCMD, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSIMToolkitNotification: A SIM Toolkit command was not handled by the radio"));
			TraceInfo(_D("CSSSPhone::ProcessSIMToolkitNotification: Toolkit command = <%s>"), BtoS((LPBYTE)lpData, cbData));
			break;
		}

	// RIL_NOTIFY_SIMTOOLKITCALLSETUP (0x00000002 | RIL_NCLASS_SIMTOOLKIT)
	// SIM Toolkit is trying to set up a call and call conditions were successfully checked by the radio;
    // lpData points to a DWORD containing the redial timeout for the call (in milliseconds)
	case RIL_NOTIFY_SIMTOOLKITCALLSETUP:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSIMToolkitNotification: RIL_NOTIFY_SIMTOOLKITCALLSETUP, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSIMToolkitNotification: SIM Toolkit is trying to set up a call and call conditions were successfully checked by the radio"));
			break;
		}

 	// RIL_NOTIFY_SIMTOOLKITEVENT (0x00000003 | RIL_NCLASS_SIMTOOLKIT)
	// A SIM Toolkit command was handled by the radio or the radio sent a SIm Toolkit command response to the SIM;
    // lpData points to array of bytes containing the command or response sent
	case RIL_NOTIFY_SIMTOOLKITEVENT:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSIMToolkitNotification: RIL_NOTIFY_SIMTOOLKITEVENT, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSIMToolkitNotification: A SIM Toolkit command was handled by the radio or the radio sent a SIm Toolkit command response to the SIM"));
			TraceInfo(_D("CSSSPhone::ProcessSIMToolkitNotification: Toolkit command = <%s>"), BtoS((LPBYTE)lpData, cbData));
			break;
		}

	// RIL_NOTIFY_SIMTOOLKITSESSIONEND (0x00000004 | RIL_NCLASS_SIMTOOLKIT)
	// A SIM Toolkit command session is ending
	case RIL_NOTIFY_SIMTOOLKITSESSIONEND:
		{
			TraceInfo(_D("CSSSPhone::ProcessSIMToolkitNotification: A SIM Toolkit command session is ending"));
			break;
		}
	}

	TraceLeave(_D("CSSSPhone::ProcessSIMToolkitNotification"));

	return;
}

void CSSSPhone::ProcessSupServiceNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::ProcessSupServiceNotification"));

	switch (dwCode)
	{
	// RIL_NOTIFY_CALLERID (0x00000001 | RIL_NCLASS_SUPSERVICE)
	// Incoming call CallerID information; lpData points to RILREMOTEPARTYINFO
	case RIL_NOTIFY_CALLERID:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSupServiceNotification: RIL_NOTIFY_CALLERID, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSupServiceNotification: Incoming call CallerID information"));
			break;
		}

	// RIL_NOTIFY_DIALEDID (0x00000002 | RIL_NCLASS_SUPSERVICE)
	// Initiated call DialedID information; lpData points to RILREMOTEPARTYINFO
	case RIL_NOTIFY_DIALEDID:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSupServiceNotification: RIL_NOTIFY_DIALEDID, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSupServiceNotification: Initiated call DialedID information"));
			break;
		}

	// RIL_NOTIFY_CALLWAITING (0x00000003 | RIL_NCLASS_SUPSERVICE)
	// Call Waiting information; lpData points to RILCALLWAITINGINFO
	case RIL_NOTIFY_CALLWAITING:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSupServiceNotification: RIL_NOTIFY_CALLWAITING, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSupServiceNotification: Call Waiting information"));
			break;
		}

	// RIL_NOTIFY_SUPSERVICEDATA (0x00000004 | RIL_NCLASS_SUPSERVICE)
	// Ustructured supplementary service data; lpData points to RILSUPSERVICEDATA
	case RIL_NOTIFY_SUPSERVICEDATA:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSupServiceNotification: RIL_NOTIFY_SUPSERVICEDATA, lpData is NULL"));
				break;
			}

			LPRILSUPSERVICEDATA lpril = (LPRILSUPSERVICEDATA)lpData;

			TraceInfo(_D("CSSSPhone::ProcessSupServiceNotification: Unstructured supplementary service data, cbSize = %d, dwParams = <%08X>, dwStatus = <%08X>"),
				lpril->cbSize, lpril->dwParams, lpril->dwStatus);

			break;
		}
	}

	TraceLeave(_D("CSSSPhone::ProcessSupServiceNotification"));

	return;
}

void CSSSPhone::ProcessSMSMsgNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::ProcessSMSMsgNotification"));

	switch (dwCode)
	{
	// RIL_NOTIFY_MESSAGE (0x00000001 | RIL_NCLASS_MESSAGE)
	// Incoming message; lpData points to RILMESSAGE
	case RIL_NOTIFY_MESSAGE:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_MESSAGE, lpData is NULL"));
				break;
			}

			RILMESSAGE *prm = (RILMESSAGE *)lpData;

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: Incoming message"));
			SYSTEMTIME t = prm->msgInDeliver.stSCReceiveTime;
			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: Date: %04d/%02d/%02d, Time: %02d:%02d:%02d, Day of week: %d"),
						t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wDayOfWeek);

			break;
		}

	// RIL_NOTIFY_STATUSMESSAGE (0x00000003 | RIL_NCLASS_MESSAGE)
	// Incoming status-report message; lpData points to RILMESSAGE
	case RIL_NOTIFY_STATUSMESSAGE:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_STATUSMESSAGE, lpData is NULL"));
				break;
			}

			RILMESSAGE *prm = (RILMESSAGE *)lpData;

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: Incoming status report message"));
			break;
		}

	// RIL_NOTIFY_BCMESSAGE (0x00000002 | RIL_NCLASS_MESSAGE)
	// Incoming broadcast message; lpData points to RILMESSAGE
	case RIL_NOTIFY_BCMESSAGE:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_BCMESSAGE, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: Incoming broadcast message"));
			break;
		}

	// RIL_NOTIFY_MSGSTORED (0x00000004 | RIL_NCLASS_MESSAGE)
	// A message has been added to storage; lpData points to the storage index assigned to the new message
	case RIL_NOTIFY_MSGSTORED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_MSGSTORED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: A message has been added to storage"));
			break;
		}

	// RIL_NOTIFY_MSGDELETED (0x00000005 | RIL_NCLASS_MESSAGE)
	// A message has been deleted from storage; lpData points to the storage index occupied by the deleted message
	case RIL_NOTIFY_MSGDELETED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_MSGDELETED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: A message has been deleted from storage"));
			break;
		}

	// RIL_NOTIFY_MSGSTORAGECHANGED (0x00000006 | RIL_NCLASS_MESSAGE)
	// One of the message storage locations has been changed; lpData points to RILMSGSTORAGEINFO
	case RIL_NOTIFY_MSGSTORAGECHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_MSGSTORAGECHANGED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: One of the message storage locations has been changed"));
			break;
		}

	// RIL_NOTIFY_MESSAGE_IN_SIM (0x00000007 | RIL_NCLASS_MESSAGE)
	// Incoming message stored to SIM; lpData points to the storage RILMESSAGE_IN_SIM
	case RIL_NOTIFY_MESSAGE_IN_SIM:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_MESSAGE_IN_SIM, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: Incoming message stored to SIM"));
			break;
		}

	// RIL_NOTIFY_BCMESSAGE_IN_SIM (0x00000008 | RIL_NCLASS_MESSAGE)
	// Incoming broadcast message stored to SIM; lpData points to RILMESSAGE_IN_SIM
	case RIL_NOTIFY_BCMESSAGE_IN_SIM:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_BCMESSAGE_IN_SIM, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: Incoming broadcast message stored to SIM"));
			break;
		}

	// RIL_NOTIFY_STATUSMESSAGE_IN_SIM (0x00000009 | RIL_NCLASS_MESSAGE)
	// Incoming status-report message stored to SIM; lpData points to RILMESSAGE_IN_SIM
	case RIL_NOTIFY_STATUSMESSAGE_IN_SIM:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessSMSMsgNotification: RIL_NOTIFY_STATUSMESSAGE_IN_SIM, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessSMSMsgNotification: Incoming status-report message stored to SIM"));
			break;
		}
	}
	TraceLeave(_D("CSSSPhone::ProcessSMSMsgNotification"));
	return;
}

void CSSSPhone::ProcessCallCtrlNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam)
{
	TraceEnter(_D("CSSSPhone::ProcessCallCtrlNotification"));

	switch (dwCode)
	{
	// RIL_NOTIFY_RING (0x00000001 | RIL_NCLASS_CALLCTRL)
	// Incoming call; lpData points to RILRINGINFO
	case RIL_NOTIFY_RING:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessCallCtrlNotification: RIL_NOTIFY_RING, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: Incoming call"));
			break;
		}

	// RIL_NOTIFY_CONNECT (0x00000002 | RIL_NCLASS_CALLCTRL)
	// Data/voice connection has been established; lpData points to RILCONNECTINFO
	case RIL_NOTIFY_CONNECT:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessCallCtrlNotification: RIL_NOTIFY_CONNECT, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: Data/voice connection has been established"));
			break;
		}

	// RIL_NOTIFY_DISCONNECT (0x00000003 | RIL_NCLASS_CALLCTRL)
	// Data/voice connection has been terminated; lpData points to RIL_DISCINIT_* constant
	case RIL_NOTIFY_DISCONNECT:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessCallCtrlNotification: RIL_NOTIFY_DISCONNECT, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: Data/voice connection has been terminated"));
			break;
		}

	// RIL_NOTIFY_DATASVCNEGOTIATED (0x00000004 | RIL_NCLASS_CALLCTRL)
	// Data connection service has been negotiated; lpData points to RILSERVICEINFO
	case RIL_NOTIFY_DATASVCNEGOTIATED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessCallCtrlNotification: RIL_NOTIFY_DATASVCNEGOTIATED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: Data connection service has been negotiated"));
			break;
		}

	// RIL_NOTIFY_CALLSTATECHANGED (0x00000005 | RIL_NCLASS_CALLCTRL)
	// RIL has performed an operation that may have changed state of existing calls; lpData is NULL
	case RIL_NOTIFY_CALLSTATECHANGED:
		{
			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: RIL has performed an operation that may have changed state of existing calls"));
			break;
		}

	// RIL_NOTIFY_EMERGENCYMODEENTERED (0x00000006 | RIL_NCLASS_CALLCTRL)
	// RIL has enetered emergency mode; lpData is NULL
	case RIL_NOTIFY_EMERGENCYMODEENTERED:
		{
			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: RIL has entered emergency mode"));
			break;
		}

	// RIL_NOTIFY_EMERGENCYMODEEXITED (0x00000007 | RIL_NCLASS_CALLCTRL)
	// RIL has exited emergency mode; lpData is NULL
	case RIL_NOTIFY_EMERGENCYMODEEXITED:
		{
			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: RIL has exited emergency mode"));
			break;
		}

	// RIL_NOTIFY_EMERGENCYHANGUP (0x00000008 | RIL_NCLASS_CALLCTRL)
	// Existing calls (if any) were hung up in RIL emergency mode; lpData is NULL
	case RIL_NOTIFY_EMERGENCYHANGUP:
		{
			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: Existing calls (if any) were hung up in RIL emergency mode"));
			break;
		}

	// RIL_NOTIFY_HSCSDPARAMSNEGOTIATED (0x00000009 | RIL_NCLASS_CALLCTRL)
	// HSCSD parameters for a call has been negotiated; lpData points to RILCALLHSCSDINFO
	case RIL_NOTIFY_HSCSDPARAMSNEGOTIATED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessCallCtrlNotification: RIL_NOTIFY_HSCSDPARAMSNEGOTIATED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: HSCSD parameters for a call has been negotiated"));
			break;
		}

	// RIL_NOTIFY_DIAL (0x0000000A | RIL_NCLASS_CALLCTRL)
	// Outgoing call; lpData points to RILDIALINFO
	case RIL_NOTIFY_DIAL:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessCallCtrlNotification: RIL_NOTIFY_DIAL, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: Outgoing call"));
			break;
		}

	// RIL_NOTIFY_CALLPROGRESSINFO (0x0000000B | RIL_NCLASS_CALLCTRL)
	// CPI notification; lpData points to RILCALLINFO
	case RIL_NOTIFY_CALLPROGRESSINFO:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessCallCtrlNotification: RIL_NOTIFY_CALLPROGRESSINFO, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: Call Progress notification"));
			break;
		}

	// RIL_NOTIFY_CURRENTLINECHANGED (0x0000000C | RIL_NCLASS_CALLCTRL)
	// Current line has changed notification; lpData points to DWORD with new current address id
	case RIL_NOTIFY_CURRENTLINECHANGED:
		{
			if (lpData == NULL)
			{
				TraceError(_D("CSSSPhone::ProcessCallCtrlNotification: RIL_NOTIFY_CURRENTLINECHANGED, lpData is NULL"));
				break;
			}

			TraceInfo(_D("CSSSPhone::ProcessCallCtrlNotification: Current line has changed notification"));
			break;
		}
	}
	TraceLeave(_D("CSSSPhone::ProcessCallCtrlNotification"));
	return;
}
#endif// SSS_V2_IMP

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Initializes RIL for use by this client
//
// @comm Synchronous
//	    RIL only supports single threaded RIL handles.
//	    The RIL validates the application's RIL handle before using it.
//       	    No application can use/close a RIL handle that it does not own.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILInitialize(HRIL* lphRil)
{

	HRESULT	hrResult;

	DWORD	dwNotifications = RIL_NCLASS_FUNCRESULT; // = 0, RIL_NCLASS_ALL = 0x00ff0000
	DWORD	dwParam = m_dwRILParam;

	TraceEnter(_D("CSSSPhone::RILInitialize"));

	hrResult = RIL_Initialize(1, RILResultCallback, RILNotifyCallback, dwNotifications, dwParam, lphRil);

	TraceInfo(_D("CSSSPhone::RILInitialize: RIL_Initialize result = <%08x>, handle = <%08x>"), hrResult, m_hrilRIL);

	if (hrResult > 0) hrResult = 0;

	TraceLeave(_D("CSSSPhone::RILInitialize"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Deinitializes RIL
//
// @comm Synchronous
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILDeinitialize(HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILDeinitialize"));

	hrResult =  RIL_Deinitialize(hRil);

	TraceInfo(_D("CSSSPhone::RILDeinitialize: RIL_Deinitialize result = <%08x>"), hrResult);

	if (hrResult > 0) hrResult = 0;

	TraceLeave(_D("CSSSPhone::RILDeinitialize"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Enables additional classes of notifications for this client
//
// @comm Synchronous
//
// -----------------------------------------------------------------------------

HRESULT CSSSPhone::RILEnableNotifications(const HRIL hRil, const DWORD dwNotificationClasses)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILEnableNotifications"));

	hrResult =  RIL_EnableNotifications(hRil, dwNotificationClasses);

	TraceInfo(_D("CSSSPhone::RILEnableNotifications: RIL_EnableNotifications result = <%08x>, handle = <%08x>, dwNotificationClasses = <%08X>"), hrResult, hRil, dwNotificationClasses);

	if (hrResult > 0) hrResult = 0;

	TraceLeave(_D("CSSSPhone::RILEnableNotifications"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Disables classes of notifications for this client
//
// @comm Synchronous
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILDisableNotifications(const HRIL hRil, const DWORD dwNotificationClasses)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILDisableNotifications"));

	hrResult =  RIL_DisableNotifications(hRil, dwNotificationClasses);

	TraceInfo(_D("CSSSPhone::RILDisableNotifications: RIL_DisableNotifications result = <%08x>, handle = <%08x>, dwNotificationClasses = <%08X>"), hrResult, hRil, dwNotificationClasses);

	if (hrResult > 0) hrResult = 0;

	TraceLeave(_D("CSSSPhone::RILDisableNotifications"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves the operator the ME is currently registered with
//
// @comm Asynchronous.  <lpData> points to a <RILOPERATORNAMES> structure.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetCurrentOperator(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetCurrentOperator"));

	if (m_hrGetCurrentOperatorCmdID == 0)
	{
		m_hrGetCurrentOperatorCmdID =  RIL_GetCurrentOperator(hRil, RIL_OPFORMAT_LONG);
		TraceInfo(_D("CSSSPhone::RILGetCurrentOperator: RIL_GetCurrentOperator result = <%08x>, handle = <%08x>"), m_hrGetCurrentOperatorCmdID, hRil);

		hrResult = (m_hrGetCurrentOperatorCmdID >= 0 ? 0 : m_hrGetCurrentOperatorCmdID);

	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetCurrentOperator: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetCurrentOperator"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetCurrentOperator"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves manufacturer equipment information
//
// @comm Asynchronous.  <lpData> points to a <RILEQUIPMENTINFO> structure.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetEquipmentInfo(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetEquipmentInfo"));

	if (m_hrGetEquipmentInfoCmdID == 0)
	{
		m_hrGetEquipmentInfoCmdID =  RIL_GetEquipmentInfo(hRil);
		TraceDetail(_D("CSSSPhone::RILGetEquipmentInfo: RIL_GetEquipmentInfo result = <%08x>, handle = <%08x>"), m_hrGetEquipmentInfoCmdID, hRil);

		hrResult = (m_hrGetEquipmentInfoCmdID >= 0 ? 0 : m_hrGetEquipmentInfoCmdID);
	}
	else
	{
		TraceDetail(_D("CSSSPhone::RILGetEquipmentInfo: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetEquipmentInfo"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetEquipmentInfo"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves currently set equipment state
//
// @comm Asynchronous.  <lpData> points to a <RILEQUIPMENTSTATE> structure.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetEquipmentState(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetEquipmentState"));

	if (m_hrGetEquipmentStateCmdID == 0)
	{
		m_hrGetEquipmentStateCmdID =  RIL_GetEquipmentState(hRil);
		TraceInfo(_D("CSSSPhone::RILGetEquipmentState: RIL_GetEquipmentState result = <%08x>, handle = <%08x>"), m_hrGetEquipmentStateCmdID, hRil);
		
		hrResult = (m_hrGetEquipmentStateCmdID >= 0 ? 0 : m_hrGetEquipmentStateCmdID);
	}
	else
	{
		TraceDetail(_D("CSSSPhone::RILGetEquipmentState: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetEquipmentState"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetEquipmentState"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Sets the equipment to the specified state
//
// @comm Asynchronous.  <lpData> is NULL.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILSetEquipmentState(const HRIL hRil, const DWORD dwEquipmentState)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILSetEquipmentState"));

	if (m_hrSetEquipmentStateCmdID == 0)
	{
		m_hrSetEquipmentStateCmdID =  RIL_SetEquipmentState(hRil, dwEquipmentState);
		TraceInfo(_D("CSSSPhone::RILSetEquipmentState: RIL_SetEquipmentState result = <%08x>, handle = <%08x>"), m_hrSetEquipmentStateCmdID, hRil);

		hrResult = (m_hrSetEquipmentStateCmdID >= 0 ? 0 : m_hrSetEquipmentStateCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILSetEquipmentState: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILSetEquipmentState"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILSetEquipmentState"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Proxy API to determine if the Radio is present or Not (Is the RIL driver Loaded?)
//
// @comm Synchronous
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetRadioPresence(const HRIL hRil, LPDWORD lpdwRadioPresence)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetRadioPresence"));

	hrResult =  RIL_GetRadioPresence(hRil, lpdwRadioPresence);
	TraceInfo(_D("CSSSPhone::RILGetRadioPresence: RIL_GetRadioPresence result = <%08x>, dwRadioPresence = <%08x>"), hrResult, *lpdwRadioPresence);

	if (hrResult > 0) hrResult = 0;

	TraceLeave(_D("CSSSPhone::RILGetRadioPresence"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves International Mobile Subscriber Identity of the phone user
//
// @comm Asynchronous.  <lpData> points to an array of <char>s
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetUserIdentity(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetUserIdentity"));

	if (m_hrGetUserIdentityCmdID == 0)
	{
		m_hrGetUserIdentityCmdID =  RIL_GetUserIdentity(hRil);
		TraceInfo(_D("CSSSPhone::RILGetUserIdentity: RIL_GetUserIdentity result = <%08x>, handle = <%08x>"), m_hrGetUserIdentityCmdID, hRil);

		hrResult = (m_hrGetUserIdentityCmdID >= 0 ? 0 : m_hrGetUserIdentityCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetUserIdentity: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetUserIdentity"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetUserIdentity"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves the current address identifier (see RILSUBSCRIBERINFO)
//
// @comm Asynchronous.  <lpData> points to a DWORD identifying the current address ID.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetCurrentAddressID(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetCurrentAddressID"));

	if (m_hrGetCurrentAddressIdCmdID == 0)
	{
		m_hrGetCurrentAddressIdCmdID =  RIL_GetCurrentAddressId(hRil);
		TraceInfo(_D("CSSSPhone::RILGetCurrentAddressID: RIL_GetCurrentAddressId result = <%08x>, handle = <%08x>"), m_hrGetCurrentAddressIdCmdID, hRil);

		hrResult = (m_hrGetCurrentAddressIdCmdID >= 0 ? 0 : m_hrGetCurrentAddressIdCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetCurrentAddressID: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetCurrentAddressID"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetCurrentAddressID"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Restrieves information about subscriber numbers
//
// @comm Asynchronous.  <lpData> points to an array of <RILSUBSCRIBERINFO> structures.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetSubscriberNumbers(const HRIL hRil) 
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetSubscriberNumbers"));

	if (m_hrGetSubscriberNumbersCmdID == 0)
	{
		m_hrGetSubscriberNumbersCmdID =  RIL_GetSubscriberNumbers(hRil);
		TraceInfo(_D("CSSSPhone::RILGetSubscriberNumbers: RIL_GetSubscriberNumbers result = <%08x>, handle = <%08x>"), m_hrGetSubscriberNumbersCmdID, hRil);
		
		hrResult = (m_hrGetSubscriberNumbersCmdID >= 0 ? 0 : m_hrGetSubscriberNumbersCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetSubscriberNumbers: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetSubscriberNumbers"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetSubscriberNumbers"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves currently set phonebook options
//
// @comm Asynchronous.  <lpData> points to a <RILPHONEBOOKINFO> structure.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetPhonebookOptions(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetPhonebookOptions"));

	if (m_hrGetPhonebookOptionsCmdID == 0)
	{
		m_hrGetPhonebookOptionsCmdID =  RIL_GetPhonebookOptions(hRil);
		TraceInfo(_D("CSSSPhone::RILGetPhonebookOptions: RILGetPhonebookOptions result = <%08x>, handle = <%08x>"), m_hrGetPhonebookOptionsCmdID, hRil);

		hrResult = (m_hrGetPhonebookOptionsCmdID >= 0 ? 0 : m_hrGetPhonebookOptionsCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetPhonebookOptions: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetPhonebookOptions"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetPhonebookOptions"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves SIM Record Status
//
// @comm Asynchronous.  <lpData> points to RILSIMRECORDSTATUS
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetSimICCIDRecordStatus(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetSimICCIDRecordStatus"));

	if (m_hrGetSimICCIDRecordStatusCmdID == 0)
	{
		if (_tcslen(m_szICCID) != 0)
		{
			_zclr(m_szICCID);
			m_pToday->PhoneNotify(g_dwNotifyICCID ^ m_dwPhoneLicenseData, (LPVOID)((DWORD)(NULL) ^ m_dwPhoneLicenseConfig));
		}

		TraceInfo(_D("CSSSPhone::RILGetSimICCIDRecordStatus, Clearing PIN"));
		_zclr(m_szPhonePIN);

		m_hrGetSimICCIDRecordStatusCmdID =  RIL_GetSimRecordStatus(hRil, g_dwICCIDNumber);
		TraceInfo(_D("CSSSPhone::RILGetSimICCIDRecordStatus: RIL_GetSimRecordStatus result = <%08x>, handle = <%08x>"), m_hrGetSimICCIDRecordStatusCmdID, hRil);
		
		hrResult = (m_hrGetSimICCIDRecordStatusCmdID >= 0 ? 0 : m_hrGetSimICCIDRecordStatusCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetSimICCIDRecordStatus: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetSimICCIDRecordStatus"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetSimICCIDRecordStatus"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Sends a specified restricted command to the SIM
//
// @comm Asynchronous.  <lpData> points to a <RILSIMRESPONSE> structure.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::ReadICCIDRecord(const HRIL hRil, const DWORD dwRecordType, const DWORD dwRecSize)
{
	RILSIMCMDPARAMETERS	rilcmd;
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::ReadICCIDRecord"));

	memset(&rilcmd, 0, sizeof(RILSIMCMDPARAMETERS));
	rilcmd.cbSize = sizeof(RILSIMCMDPARAMETERS);
	rilcmd.dwParams = RIL_PARAM_SCP_ALL;
	rilcmd.dwFileID = g_dwICCIDNumber;
	rilcmd.dwParameter1 = 0;
	rilcmd.dwParameter2 = dwRecordType;
	rilcmd.dwParameter3 = dwRecSize-1;

	if (m_hrSendSimCmdReadICCIDRecordCmdID == 0)
	{
		m_hrSendSimCmdReadICCIDRecordCmdID =  RIL_SendRestrictedSimCmd(hRil, RIL_SIMCMD_READBINARY, &rilcmd, NULL, 0);
		TraceInfo(_D("CSSSPhone::ReadICCIDRecord: RIL_SendRestrictedSimCmd result = <%08x>, handle = <%08x>"), m_hrSendSimCmdReadICCIDRecordCmdID, hRil);

		hrResult = (m_hrSendSimCmdReadICCIDRecordCmdID >= 0 ? 0 : m_hrSendSimCmdReadICCIDRecordCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::ReadICCIDRecord: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::ReadICCIDRecord"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::ReadICCIDRecord"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves current locked state of the phone
//
// @comm Asynchronous.  <lpData> points to a DWORD containing a <RIL_LOCKEDSTATE_> constant
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetPhoneLockedState(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetPhoneLockedState"));

	if (m_hrGetPhoneLockedStateCmdID == 0)
	{
		m_hrGetPhoneLockedStateCmdID =  RIL_GetPhoneLockedState(hRil);
		TraceInfo(_D("CSSSPhone::RILGetPhoneLockedState: RIL_GetPhoneLockedState result = <%08x>, handle = <%08x>"), m_hrGetPhoneLockedStateCmdID, hRil);

		hrResult = (m_hrGetPhoneLockedStateCmdID >= 0 ? 0 : m_hrGetPhoneLockedStateCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetPhoneLockedState: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetPhoneLockedState"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetPhoneLockedState"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Removes current lock applied to the phone
//
// @comm Asynchronous.  <lpData> is NULL.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILUnlockPhone(const HRIL hRil, LPCTSTR lpszPassword)
{
	char	szPassword[MAXLENGTH_PASSWORD + 1];
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILUnlockPhone"));

	if (_tcslen(lpszPassword) == 0)
	{
		TraceWarning(_D("CSSSPhone::RILUnlockPhone: Password is empty, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILUnlockPhone"), -1);
		return -1;
	}

	if (PhoneFlags.PhoneBits.Unlocking)
	{
		TraceWarning(_D("CSSSPhone::RILUnlockPhone: Already trying to unlock phone, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILUnlockPhone"), -1);
		return -1;
	}

	TraceInfo(_D("CSSSPhone::RILUnlockPhone: Password = <%s>"), lpszPassword);

	UtoA(lpszPassword, szPassword, _tcslen(lpszPassword));

	PhoneFlags.PhoneBits.Unlocking = true;

	if (m_hrUnlockPhoneCmdID == 0)
	{
		m_hrUnlockPhoneCmdID =  RIL_UnlockPhone(hRil, szPassword, szPassword);
		TraceInfo(_D("CSSSPhone::RILUnlockPhone: RIL_UnlockPhone result = <%08x>, handle = <%08x>"), m_hrUnlockPhoneCmdID, hRil);

		hrResult = (m_hrUnlockPhoneCmdID >= 0 ? 0 : m_hrUnlockPhoneCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILUnlockPhone: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILUnlockPhone"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILUnlockPhone"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Registers the ME with a network operator
//
// @comm Asynchronous.  <lpData> is NULL.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILRegisterOnNetwork(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILRegisterOnNetwork"));

	if (m_hrRegisterOnNetworkCmdID == 0)
	{
		m_hrRegisterOnNetworkCmdID =  RIL_RegisterOnNetwork(hRil, RIL_OPSELMODE_AUTOMATIC, NULL);
		TraceInfo(_D("CSSSPhone::RILRegisterOnNetwork: RIL_RegisterOnNetwork result = <%08x>, handle = <%08x>"), m_hrRegisterOnNetworkCmdID, hRil);
		
		hrResult = (m_hrRegisterOnNetworkCmdID >= 0 ? 0 : m_hrRegisterOnNetworkCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILRegisterOnNetwork: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILRegisterOnNetwork"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILRegisterOnNetwork"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Unregisters the ME from the current newtwork operator
//
// @comm Asynchronous.  <lpData> is NULL.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILUnregisterFromNetwork(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILUnregisterFromNetwork"));

	if (m_hrUnregisterFromNetworkCmdID == 0)
	{
		m_hrUnregisterFromNetworkCmdID =  RIL_UnregisterFromNetwork(hRil);
		TraceInfo(_D("CSSSPhone::RILUnregisterFromNetwork: RIL_UnregisterFromNetwork result = <%08x>, handle = <%08x>"), m_hrUnregisterFromNetworkCmdID, hRil);
		
		hrResult = (m_hrUnregisterFromNetworkCmdID >= 0 ? 0 : m_hrUnregisterFromNetworkCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILUnregisterFromNetwork: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILUnregisterFromNetwork"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILUnregisterFromNetwork"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves the current phone registration status
//
// @comm Asynchronous.  <lpData> points to a <RIL_REGSTAT_> constant.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetRegistrationStatus(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetRegistrationStatus"));

	if (m_hrGetRegistrationStatusCmdID == 0)
	{
		m_hrGetRegistrationStatusCmdID =  RIL_GetRegistrationStatus(hRil);
		TraceInfo(_D("CSSSPhone::RILGetRegistrationStatus: RIL_GetRegistrationStatus result = <%08x>, handle = <%08x>"), m_hrGetRegistrationStatusCmdID, hRil);
		
		hrResult = (m_hrGetRegistrationStatusCmdID >= 0 ? 0 : m_hrGetRegistrationStatusCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetRegistrationStatus: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetRegistrationStatus"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetRegistrationStatus"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves specified device capabilities
//
// @comm Asynchronous. <dwCapsType> (<RIL_CAPSTYPE_>) <lpData>
//                     <*_DIAL>                       points to a <RILCAPSDIAL> structure
//                     <*_DTMFDURATIONRANGE>          points to a <RILRANGE> structure (values in milliseconds)
//                     <*_CALLMGTCMDS>                points to DWORD containing a combination of <RIL_CAPS_CALLCMD_> constants
//                     <*_BEARERSERVICE>              points to a <RILCAPSBEARERSVC> structure
//                     <*_RLP>                        points to an array of <RILAPSRLP> structures
//                     <*_EQUIPMENTSTATES>            points to a DWORD containing a combination of <RIL_CAPS_EQSTATE_> constants
//                     <*_PBSTORELOCATIONS>           points to a DWORD containing a combination of <RIL_CAPS_PBLOC_> constants
//                     <*_PBINDEXRANGE>               points to a <RILRANGE> structure
//                     <*_PBENTRYLENGTH>              points to a <RILCAPSPBENTRYLENGTH> strcuture
//                     <*_MSGSERVICETYPES>            points to a DWORD containing a combination of <RIL_CAPS_MSGSVCTYPE_> constants
//                     <*_MSGMEMORYLOCATIONS>         points to a <RILCAPSMSGMEMORYLOCATIONS> structure
//                     <*_BROADCASTMSGLANGS>          points to a DWORD containing a combination of <RIL_CAPS_DCSLANG_> constants
//                     <*_MSGCONFIGINDEXRANGE>        points to a <RILRANGE> structure
//                     <*_MSGSTATUSVALUES>            points to a DWORD containing a combination of <RIL_CAPS_MSGSTATUS_> constants
//                     <*_PREFOPINDEXRANGE>           points to a <RILRANGE> structure
//                     <*_LOCKFACILITIES>             points to a DWORD containing a combination of <RIL_CAPS_LOCKFACILITY_> constants
//                     <*_LOCKINGPWDLENGTHS>          points to a array of <RILCAPSLOCKINGPWDLENGTH> structures
//                     <*_BARRTYPES>		          points to a DWORD containing a combination of <RIL_CAPS_BARRTYPE_> constants
//                     <*_BARRINGPWDLENGTHS>          points to a array of <RILCAPSBARRINGPWDLENGTH> structures
//                     <*_FORWARDINGREASONS>          points to a DWORD containing a combination of <RIL_CAPS_FWDREASON_> constants
//                     <*_SIMTOOLKITNOTIFICATIONS>    points to a <TBD> SIMTOOLKIT structure 
//                     <*_INFOCLASSES>                points to a DWORD containing a combination of <RIL_CAPS_INFOCLASS_> constants
//                     <*_HSCSD>                      points to a <RILCAPSHSCSD> structure
//                     <*_GPRS>                       points to a <RILCAPSGPRS> structure

HRESULT CSSSPhone::RILGetDevCaps(const HRIL hRil, const DWORD dwCapsType)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetDevCaps"));


	switch (dwCapsType)
	{
#ifdef SSS_V2_IMP
	case RIL_CAPSTYPE_SIGNALQUALITYIMPLEMENTATION :
		{
			if (m_hrGetDevCapsSignalQualityCmdID == 0)
			{
				m_hrGetDevCapsSignalQualityCmdID =  RIL_GetDevCaps(hRil, dwCapsType);
				TraceInfo(_D("CSSSPhone::RILGetDevCaps: RIL_GetDevCaps, dwCapsType = <%08X>, result = <%08x>, handle = <%08x>"), dwCapsType, m_hrGetDevCapsSignalQualityCmdID, hRil);
				
				hrResult = (m_hrGetDevCapsSignalQualityCmdID >= 0 ? 0 : m_hrGetDevCapsSignalQualityCmdID);
				break;
			}
			else
			{
				TraceInfo(_D("CSSSPhone::RILGetDevCaps: Command still running, so do nothing"));
				TraceLeave(_D("CSSSPhone::RILGetDevCaps"), -1);
				return -1;
			}
		}
#endif //#ifdef SSS_V2_IMP
	case RIL_CAPSTYPE_LOCKFACILITIES :
		{

			if (m_hrGetDevCapsLockFacilitiesCmdID == 0)
			{
				m_hrGetDevCapsLockFacilitiesCmdID =  RIL_GetDevCaps(hRil, dwCapsType);
				TraceInfo(_D("CSSSPhone::RILGetDevCaps: RIL_GetDevCaps, dwCapsType = <%08X>, result = <%08x>, handle = <%08x>"), dwCapsType, m_hrGetDevCapsLockFacilitiesCmdID, hRil);
				
				hrResult = (m_hrGetDevCapsLockFacilitiesCmdID >= 0 ? 0 : m_hrGetDevCapsLockFacilitiesCmdID);
				break;
			}
			else
			{
				TraceInfo(_D("CSSSPhone::RILGetDevCaps: Command still running, so do nothing"));
				TraceLeave(_D("CSSSPhone::RILGetDevCaps"), -1);
				return -1;
			}
		}
	}

	TraceLeave(_D("CSSSPhone::RILGetDevCaps"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves the driver version
//
// @parm DWORD *pdwVersion - pointer to version.
//       HIWORD is major version, LOWORD is minor version
//
// @comm Synchronous
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetDriverVersion(const HRIL hRil, LPDWORD lpdwVersion)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetDriverVersion"));

	hrResult =  RIL_GetDriverVersion(hRil, lpdwVersion);
	TraceInfo(_D("CSSSPhone::RILGetDriverVersion: RIL_GetDriverVersion result = <%08x>, Major Version = <%08x>, Minor Version = <%08x>"), hrResult, HIWORD(*lpdwVersion), LOWORD(*lpdwVersion));

	if (hrResult > 0) hrResult = 0;

	TraceLeave(_D("CSSSPhone::RILGetDriverVersion"), hrResult);
	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves locking status for the specified facility
//
// @comm Asynchronous.  <p lpData> points to a <t DWORD> containing a <def RIL_LOCKINGSTATUS_> constant.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetSIMLockingStatus(const HRIL hRil)
{
	HRESULT	hrResult;
/*
#define RIL_LOCKFACILITY_CNTRL                      (0x00000001)      // @constdefine Lock control curface
#define RIL_LOCKFACILITY_PH_SIM                     (0x00000002)      // @constdefine Lock phone to SIM card
#define RIL_LOCKFACILITY_PH_FSIM                    (0x00000003)      // @constdefine Lock phone to first SIM card
#define RIL_LOCKFACILITY_SIM                        (0x00000004)      // @constdefine Lock SIM card
#define RIL_LOCKFACILITY_SIM_PIN2                   (0x00000005)      // @constdefine SIM PIN2 (only for RIL_ChangeLockingPassword())
#define RIL_LOCKFACILITY_SIM_FIXEDIALING            (0x00000006)      // @constdefine SIM fixed dialing memory
#define RIL_LOCKFACILITY_NETWORKPERS                (0x00000007)      // @constdefine Network personalization
#define RIL_LOCKFACILITY_NETWORKSUBPERS             (0x00000008)      // @constdefine Network subset personalization
#define RIL_LOCKFACILITY_SERVICEPROVPERS            (0x00000009)      // @constdefine Service provider personalization
#define RIL_LOCKFACILITY_CORPPERS                   (0x0000000a)      // @constdefine Corporate personalization
*/
	TraceEnter(_D("CSSSPhone::RILGetSIMLockingStatus"));

	if (m_hrRILGetSIMLockingStatusCmdID == 0)
	{
		m_hrRILGetSIMLockingStatusCmdID =  RIL_GetLockingStatus(hRil, RIL_LOCKFACILITY_SIM, NULL);
		TraceInfo(_D("CSSSPhone::RILGetSIMLockingStatus: RIL_GetLockingStatus result = <%08x>, handle = <%08x>"), m_hrRILGetSIMLockingStatusCmdID, hRil);
		hrResult = (m_hrRILGetSIMLockingStatusCmdID >= 0 ? 0 : m_hrRILGetSIMLockingStatusCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetSIMLockingStatus: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetSIMLockingStatus"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetSIMLockingStatus"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Enables or disables locking status for the specified facility
//
// @comm Asynchronous.  <p lpData> is <def NULL>.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILSetLockingStatus(const HRIL hRil, const DWORD dwFacility, LPCTSTR lpszPassword, const DWORD dwStatus)
{
	char	szPassword[MAXLENGTH_PASSWORD + 1];
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILSetLockingStatus"));

	if (_tcslen(lpszPassword) == 0)
	{
		TraceWarning(_D("CSSSPhone::RILSetLockingStatus: Password is empty, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILSetLockingStatus"), -1);
		return -1;
	}

	UtoA(lpszPassword, szPassword, _tcslen(lpszPassword));

	if (m_hrRILSetLockingStatusCmdID == 0)
	{
		m_hrRILSetLockingStatusCmdID =  RIL_SetLockingStatus(hRil, dwFacility, szPassword, dwStatus);
		TraceInfo(_D("CSSSPhone::RILSetLockingStatus: RIL_SetLockingStatus result = <%08x>, handle = <%08x>"), m_hrRILSetLockingStatusCmdID, hRil);
		hrResult = (m_hrRILSetLockingStatusCmdID >= 0 ? 0 : m_hrRILSetLockingStatusCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILSetLockingStatus: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILSetLockingStatus"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILSetLockingStatus"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Changes locking password for the specified facility
//
// @comm Asynchronous.  <p lpData> is <def NULL>.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILChangeLockingPassword(HRIL hRil, DWORD dwFacility, DWORD dwOldPasswordType, LPCTSTR lpszOldPassword, LPCTSTR lpszNewPassword)
{
	char	szOldPassword[MAXLENGTH_PASSWORD + 1];
	char	szNewPassword[MAXLENGTH_PASSWORD + 1];
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILChangeLockingPassword"));

	if (_tcslen(lpszNewPassword) == 0)
	{
		TraceWarning(_D("CSSSPhone::RILChangeLockingPassword: Password is empty, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILChangeLockingPassword"), -1);
		return -1;
	}

//	if (PhoneFlags.PhoneBits.Unlocking)
//	{
//		TraceWarning(_D("CSSSPhone::RILChangeLockingPassword: Already trying to unlock phone, so do nothing"));
//		TraceLeave(_D("CSSSPhone::RILChangeLockingPassword"), -1);
//		return -1;
//	}

	TraceInfo(_D("CSSSPhone::RILChangeLockingPassword: Old Password = <%s>, New Password = <%s>"), lpszOldPassword, lpszOldPassword);

	UtoA(lpszOldPassword, szOldPassword, _tcslen(lpszOldPassword));
	UtoA(lpszNewPassword, szNewPassword, _tcslen(lpszNewPassword));

//	PhoneFlags.PhoneBits.Unlocking = true;

	if (m_hrRILChangeLockingPasswordCmdID == 0)
	{
		m_hrRILChangeLockingPasswordCmdID =  RIL_ChangeLockingPassword(hRil, dwFacility, dwOldPasswordType, szOldPassword, szNewPassword);
		TraceInfo(_D("CSSSPhone::RILChangeLockingPassword: RIL_ChangeLockingPassword result = <%08x>, handle = <%08x>"), m_hrRILChangeLockingPasswordCmdID, hRil);
		hrResult = (m_hrRILChangeLockingPasswordCmdID >= 0 ? 0 : m_hrRILChangeLockingPasswordCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILChangeLockingPassword: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILChangeLockingPassword"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILChangeLockingPassword"), hrResult);

	return hrResult;
}

bool CALLBACK LicensePhoneNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig)
{
	return m_pThis->PhoneLicenseNotify(lpLicenseData, dwLicenseConfig);
}

#ifdef SSS_V2_IMP
// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves information about the received signal quality
//
// @comm Asynchronous.  <p lpData> points to a <t RILSIGNALQUALITY> structure.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetSignalQuality(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetSignalQuality"));

	if (m_hrRILGetSignalQualityCmdID == 0)
	{
		m_hrRILGetSignalQualityCmdID =  RIL_GetSignalQuality(hRil);
		TraceInfo(_D("CSSSPhone::RILGetSignalQuality: RIL_GetSignalQuality result = <%08x>, handle = <%08x>"), m_hrRILGetSignalQualityCmdID, hRil);
		hrResult = (m_hrRILGetSignalQualityCmdID >= 0 ? 0 : m_hrRILGetSignalQualityCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetSignalQuality: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetSignalQuality"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetSignalQuality"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves information about the cell tower currently used by the phone
//
// @comm Asynchronous.  <p lpData> points to a <t RILCELLTOWERINFO> structure.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetCellTowerInfo(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetCellTowerInfo"));

	if (m_hrRILGetCellTowerInfoCmdID == 0)
	{
		m_hrRILGetCellTowerInfoCmdID =  RIL_GetCellTowerInfo(hRil);
		TraceInfo(_D("CSSSPhone::RILGetCellTowerInfo: RIL_GetCellTowerInfo result = <%08x>, handle = <%08x>"), m_hrRILGetCellTowerInfoCmdID, hRil);
		hrResult = (m_hrRILGetCellTowerInfoCmdID >= 0 ? 0 : m_hrRILGetCellTowerInfoCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetCellTowerInfo: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetCellTowerInfo"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetCellTowerInfo"), hrResult);

	return hrResult;
}

// -----------------------------------------------------------------------------
//
// @doc EXTERNAL
//
// @func Retrieves the systemtime from the network
//
// @comm Asynchronous.  <p lpData> points to a <t SYSTEMTIME> structure (containing the UTC time).
//       This feature is currently not used and is untested.
//
// -----------------------------------------------------------------------------
HRESULT CSSSPhone::RILGetSystemTime(const HRIL hRil)
{
	HRESULT	hrResult;

	TraceEnter(_D("CSSSPhone::RILGetSystemTime"));

	if (m_hrRILGetSystemTimeCmdID == 0)
	{
		m_hrRILGetSystemTimeCmdID =  RIL_GetSystemTime(hRil);
		TraceInfo(_D("CSSSPhone::RILGetSystemTime: RIL_GetSystemTime result = <%08x>, handle = <%08x>"), m_hrRILGetSystemTimeCmdID, hRil);
		hrResult = (m_hrRILGetSystemTimeCmdID >= 0 ? 0 : m_hrRILGetSystemTimeCmdID);
	}
	else
	{
		TraceInfo(_D("CSSSPhone::RILGetSystemTime: Command still running, so do nothing"));
		TraceLeave(_D("CSSSPhone::RILGetSystemTime"), -1);
		return -1;
	}

	TraceLeave(_D("CSSSPhone::RILGetSystemTime"), hrResult);

	return hrResult;
}
#endif //#ifdef SSS_V2_IMP
