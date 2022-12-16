//////////////////////////////////////////////////////////////////////
//
// SSSPhone.h: Interface for the CPhone class.
//
//////////////////////////////////////////////////////////////////////

#ifndef INC_SSS_PHONE_H
#define INC_SSS_PHONE_H
#include "ril.h"
// DCS - The following missing definition has been added to ril.h
//#define RIL_READYSTATE_SIMREADY		                (0x00000010)      // The SIM is ready

class CSSSPhone
{

friend void CALLBACK RILResultCallback(DWORD dwCode, HRESULT hrCmdID, const void *lpData, DWORD cbData, DWORD dwParam);
friend void CALLBACK RILNotifyCallback(DWORD dwCode, const void *lpData, DWORD cbData, DWORD dwParam);
friend bool CALLBACK LicensePhoneNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig);

private:

    CSSSPhone();

	bool			PhoneLicenseNotify(LPVOID lpLicenseData, DWORD dwLicenseConfig);
	void 			RILNotify(DWORD dwCode, const void *lpData, DWORD cbData, DWORD dwParam);
	void 			RILResult(DWORD dwCode, HRESULT hrCmdID, const void *lpData, DWORD cbData, DWORD dwParam);

	// Actions
    void			SetupPhoneData();
	HRESULT			ReadICCIDRecord(const HRIL hRil, const DWORD dwRecordType, const DWORD dwRecSize);
	void			ProcessRILResult(const HRESULT hrCmdID, LPCVOID lpData, const DWORD cbData);
	void			ProcessRILError(const HRESULT hrCmdID, const DWORD dwErrorWord, const DWORD dwFacility, const DWORD dwClass, const DWORD dwError);
#ifdef WIV_DEBUG
	LPCTSTR			GetErrorDetails(const DWORD dwFacility, const DWORD dwErrorClass, const DWORD dwErrorCode);
#endif
	HRESULT			RILInitialize(HRIL* lphRil);
	HRESULT			RILDeinitialize(HRIL hRil);

	HRESULT			RILEnableNotifications(const HRIL hRil, const DWORD dwNotificationClasses);
	HRESULT			RILDisableNotifications(const HRIL hRil, const DWORD dwNotificationClasses);

//	HRESULT			RILSendSimCmdReadICCIDRecord(HRIL hRil);
//	HRESULT			RILSendRestrictedSimCmd(HRIL hRil, DWORD dwCommand, const RILSIMCMDPARAMETERS* lpParameters, const BYTE* lpbData, DWORD dwSize);

	HRESULT			RILUnlockPhone(const HRIL hRil, LPCTSTR lpszPassword);
	HRESULT			RILChangeLockingPassword(HRIL hRil, DWORD dwFacility, DWORD dwOldPasswordType, LPCTSTR lpszOldPassword, LPCTSTR lpszNewPassword);
	HRESULT			RILRegisterOnNetwork(const HRIL hRil);
	HRESULT			RILUnregisterFromNetwork(const HRIL hRil);

	void			ProcessNetworkNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam);
	void			ProcessPhonebookNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam);
	void			ProcessMiscellaneousNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam);
	void			ProcessRadioStateChangeNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam);

    // Get Methods
	HRESULT			RILGetCurrentOperator(const HRIL hRil);
	HRESULT			RILGetUserIdentity(const HRIL hRil);
	HRESULT			RILGetCurrentAddressID(const HRIL hRil);
	HRESULT			RILGetSubscriberNumbers(const HRIL hRil);
	HRESULT			RILGetEquipmentInfo(const HRIL hRil);
	HRESULT			RILGetRadioPresence(const HRIL hRIL, DWORD* dwRadioPresence);
	HRESULT			RILGetEquipmentState(const HRIL hRil);
	HRESULT			RILGetPhonebookOptions(const HRIL hRil);
	HRESULT			RILGetSimICCIDRecordStatus(const HRIL hRil);
	HRESULT			RILGetDevCaps(const HRIL hRil, const DWORD dwCapsType);
	HRESULT			RILGetDriverVersion(const HRIL hRil, LPDWORD lpdwVersion);
	HRESULT			RILGetRegistrationStatus(const HRIL hRil);
	HRESULT			RILGetPhoneLockedState(const HRIL hRil);
//	HRESULT			RILGetSimRecordStatus(const HRIL hRil, const DWORD dwFileID);
	HRESULT			RILGetSIMLockingStatus(const HRIL hRil);
#ifdef SSS_V2_IMP
	HRESULT			RILGetSystemTime(const HRIL hRil);
	HRESULT			RILGetCellTowerInfo(const HRIL hRil);
	HRESULT			RILGetSignalQuality(const HRIL hRil);
	void			ProcessCallCtrlNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam);
	void			ProcessSMSMsgNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam);
	void			ProcessSupServiceNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam);
	void			ProcessSIMToolkitNotification(const DWORD dwCode, const void *lpData, const DWORD cbData, const DWORD dwParam);
#endif //#ifdef SSS_V2_IMP

    // Set Methods
	HRESULT			RILSetEquipmentState(const HRIL hRil, const DWORD dwEquipmentState);
	HRESULT			RILSetLockingStatus(const HRIL hRil, const DWORD dwFacility, LPCTSTR lpszPassword, const DWORD dwStatus);

public:

    // Contructor/destructor Methods
    CSSSPhone(const HINSTANCE hmInstance);
    ~CSSSPhone ();
	
	void			TodayDestroyed();

	// Actions
	DWORD			InitializeComms();
	DWORD			StartComms();
	DWORD			StopComms();

    DWORD			TurnPhoneOff(const bool bRestart = false);
    DWORD			TurnPhoneOn();
    DWORD			RegisterPhone();
    HRESULT			SwitchSIM();
    HRESULT			UnlockSIM();

    // Get Methods
	DWORD	GetSIMLockingStatus();
	void	GetPhoneState(DWORD &dwRadioSupport, DWORD &dwEquipmentState, DWORD &dwReadyState, DWORD &dwSIMLockedState, DWORD &dwLicense);
	DWORD	GetPhoneLockedState();
    LPCTSTR GetPhoneNumber ();
    LPCTSTR GetProvider ();
    LPCTSTR GetPIN ();
	void	GetPhonebookDetails();
	void	GetICCID(LPTSTR lpszICCID);
	void	GetSIMInfo(LPTSTR lpszSubscriber, LPTSTR lpszPBLocation, LPDWORD lpdwPBTotal, LPDWORD lpdwPBUsed);
	void	GetEquipmentInfo(LPTSTR lpszIMEI, LPTSTR lpszManufacturer = NULL, LPTSTR lpszModel = NULL,
								   LPTSTR lpszRevision = NULL);

    // Set Methods
	void	SetTodayClass(DWORD pToday);
    void	SetPIN( LPCTSTR szPIN );
	void	SetHidePersonalInfo(const bool blHidePersonalInfo);

#ifdef SSS_V2_IMP
	DWORD			RefreshSignalQuality();
#endif //#ifdef SSS_V2_IMP

protected:

}; // class CSSSPhone

// Global non-member functions

//void CALLBACK	RILResultCallback(DWORD dwCode, HRESULT hrCmdID, const void *lpData, DWORD cbData, DWORD dwParam);
//void CALLBACK	RILNotifyCallback(DWORD dwCode, const void *lpData, DWORD cbData, DWORD dwParam);

#endif // INC_SSS_PHONE_H
