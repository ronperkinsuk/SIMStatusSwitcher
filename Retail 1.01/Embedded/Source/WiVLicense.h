//////////////////////////////////////////////////////////////////////
//
// WiVLicense.h: Interface for the CWiVLicense class.
//
//////////////////////////////////////////////////////////////////////
#ifndef INC_WIV_LICENSE_H
#define INC_WIV_LICENSE_H

#include <wincrypt.h>
#include <todaycmn.h>

#include <pkfuncs.h>
#include <diskio.h>
#include <winioctl.h>

#ifdef WIV_DEBUG
#include	"WiVTrace.h"
#endif // WIV_DEBUG

//=========================================
// Obfuscation of some API calls to make it
// less obvious which APIs are being used.
// By using function pointers with ordinal
// values, the function does not appear in
// the import table.
//=========================================

// License Notification Callback
typedef bool (CALLBACK *LPFNNOTIFY)	(LPVOID, DWORD);

// Crypt APIs
// ----------

// Function definitions
typedef BOOL (CALLBACK *LPFNFUNCCAC)	(HCRYPTKEY *, LPCWSTR, LPCWSTR, DWORD, DWORD);
typedef BOOL (CALLBACK *LPFNFUNCCCH)	(HCRYPTPROV, ALG_ID, HCRYPTKEY, DWORD, HCRYPTHASH *);
typedef BOOL (CALLBACK *LPFNFUNCCHD)	(HCRYPTHASH, CONST BYTE *, DWORD, DWORD);
typedef BOOL (CALLBACK *LPFNFUNCCDK)	(HCRYPTPROV, ALG_ID, HCRYPTHASH, DWORD, HCRYPTKEY *);
typedef BOOL (CALLBACK *LPFNFUNCCSKP)	(HCRYPTKEY, DWORD, CONST BYTE *, DWORD);
typedef BOOL (CALLBACK *LPFNFUNCCDH)	(HCRYPTHASH);
typedef BOOL (CALLBACK *LPFNFUNCCRC)	(HCRYPTPROV, DWORD);
typedef BOOL (CALLBACK *LPFNFUNCCD)		(HCRYPTKEY, HCRYPTHASH, BOOL, DWORD, BYTE *, DWORD *);
typedef BOOL (CALLBACK *LPFNFUNCCE)		(HCRYPTKEY, HCRYPTHASH, BOOL, DWORD, BYTE *, DWORD *, DWORD);

// Ordinals for CoreDLL.dll
#define FNCAC							(LPCTSTR)MAKELONG(126,0)	// CryptAcquireContext
#define FNCCH							(LPCTSTR)MAKELONG(137,0)	// CryptCreateHash
#define FNCHD 							(LPCTSTR)MAKELONG(139,0)	// CryptHashData
#define FNCDK 							(LPCTSTR)MAKELONG(129,0)	// CryptDeriveKey
#define FNCSKP 							(LPCTSTR)MAKELONG(132,0)	// CryptSetKeyParms
#define FNCDH 							(LPCTSTR)MAKELONG(140,0)	// CryptDestroyHash
#define FNCRC 							(LPCTSTR)MAKELONG(127,0)	// CryptReleaseContext
#define FNCD 							(LPCTSTR)MAKELONG(136,0)	// CryptDecrypt
#define FNCE 							(LPCTSTR)MAKELONG(135,0)	// CryptEncrypt


// IoControl APIs
// --------------------

// Function definitions
typedef BOOL (CALLBACK *LPFNFUNCDIOC)	(HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL (CALLBACK *LPFNFUNCKIOC)	(DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD);

// Ordinals for CoreDLL.dll
#define FNDIOC 							(LPCTSTR)MAKELONG(179,0)	// DeviceIoControl
#define FNKIOC 							(LPCTSTR)MAKELONG(557,0)	// KernelIoControl


// File APIs
// ---------

// Function definitions
typedef BOOL (CALLBACK *LPFNFUNCFCD)	(LPCWSTR, LPSECURITY_ATTRIBUTES);
typedef BOOL (CALLBACK *LPFNFUNCWF)		(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef DWORD (CALLBACK *LPFNFUNCGFS)	(HANDLE, LPDWORD);
typedef BOOL (CALLBACK *LPFNFUNCFFB)	(HANDLE);
typedef BOOL (CALLBACK *LPFNFUNCGFT)	(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME);
typedef BOOL (CALLBACK *LPFNFUNCSFT)	(HANDLE, CONST FILETIME *, CONST FILETIME *, CONST FILETIME *);

// Ordinals for CoreDLL.dll
#define FNFCD 							(LPCTSTR)MAKELONG(160,0)	// CreateDirectory
#define FNWF 							(LPCTSTR)MAKELONG(171,0)	// WriteFile
#define FNGFS 							(LPCTSTR)MAKELONG(172,0)	// GetFileSize
#define FNFFB 							(LPCTSTR)MAKELONG(175,0)	// FlushFileBuffers
#define FNGFT 							(LPCTSTR)MAKELONG(176,0)	// GetFileTime
#define FNSFT							(LPCTSTR)MAKELONG(177,0)	// SetFileTime


// Random number APIs
// ------------------

// Function definitions
typedef DWORD	(CALLBACK *LPFNFUNCR)	();
typedef int		(CALLBACK *LPFNFUNCCR)	(void);
typedef void	(CALLBACK *LPFNFUNCCSR)	(unsigned int);

// Ordinals for CoreDLL.dll
#define FNR 							(LPCTSTR)MAKELONG(80,0)		// Random
#define FNCR 							(LPCTSTR)MAKELONG(1053,0)	// rand
#define FNCSR 							(LPCTSTR)MAKELONG(1061,0)	// srand


// Time APIs
// ---------

// Function definitions
typedef VOID	(CALLBACK* LPFNFUNCGST)	(LPSYSTEMTIME);
typedef VOID	(CALLBACK* LPFNFUNCGLT)	(LPSYSTEMTIME);
typedef BOOL	(CALLBACK* LPFNFUNCSTTFT)(const SYSTEMTIME *, LPFILETIME);
typedef DWORD	(CALLBACK* LPFNFUNCGTC)	(VOID);

// Ordinals for CoreDLL.dll
#define FNGST							(LPCTSTR)MAKELONG(25,0)		// GetSystemTime
#define FNGLT							(LPCTSTR)MAKELONG(23,0)		// GetLocalTime
#define FNSTTFT							(LPCTSTR)MAKELONG(19,0)		// SystemTimeToFileTime
#define FNGTC							(LPCTSTR)MAKELONG(535,0)	// GetTickCount


// Message Queue APIs
// ------------------

// Function definitions
typedef HANDLE	(CALLBACK* LPFNFUNCCRMQ)(LPCWSTR, LPMSGQUEUEOPTIONS);
typedef BOOL	(CALLBACK* LPFNFUNCRMQ)	(HANDLE, LPVOID, DWORD, LPDWORD, DWORD, DWORD *);
typedef BOOL	(CALLBACK* LPFNFUNCCLMQ)(HANDLE);

// Ordinals for CoreDLL.dll
#define FNCRMQ							(LPCTSTR)MAKELONG(1529,0)	// CreateMsgQueue
#define FNRMQ							(LPCTSTR)MAKELONG(1530,0)	// ReadMsgQueue
#define FNCLMQ							(LPCTSTR)MAKELONG(1533,0)	// CloseMsgQueue


// Notification APIs
// -----------------

// Function definitions
typedef HANDLE	(CALLBACK* LPFNFUNCRPN)	(HANDLE, DWORD);
typedef DWORD	(CALLBACK* LPFNFUNCSPN)	(HANDLE);

// Ordinals for CoreDLL.dll
#define FNRPN							(LPCTSTR)MAKELONG(1585,0)	// RequestPowerNotifications
#define FNSPN							(LPCTSTR)MAKELONG(1586,0)	// StopPowerNotifications


// Thread APIs
// -----------

// Function definitions
typedef HANDLE	(CALLBACK* LPFNFUNCCT)	(LPSECURITY_ATTRIBUTES, DWORD, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
typedef VOID	(CALLBACK* LPFNFUNCET)	(DWORD);
typedef BOOL	(CALLBACK* LPFNFUNCTT)	(HANDLE, DWORD);

// Ordinals for CoreDLL.dll
#define FNCT							(LPCTSTR)MAKELONG(492,0)	// CreateThread
#define FNET							(LPCTSTR)MAKELONG(6,0)		// ExitThread
#define FNTT							(LPCTSTR)MAKELONG(491,0)	// TerminateThread


// Other APIs
// ----------

// Function definitions
typedef HMODULE (CALLBACK* LPFNFUNCGMH)	(LPCWSTR);
typedef FARPROC (CALLBACK* LPFNFUNCGPA)	(HMODULE, LPCWSTR);
typedef DWORD	(CALLBACK* LPFNFUNCWSO)	(HANDLE, DWORD);

// Ordinals for CoreDLL.dll
#define FNGMH							(LPCTSTR)MAKELONG(1177,0)	// GetModuleHandle
#define FNGPA							(LPCTSTR)MAKELONG(1230,0)	// GetProcAddress
#define FNWSO							(LPCTSTR)MAKELONG(497,0)	// WaitForSingleObject



//============================
// Today Item dwFlags masks
//============================
#define WIV_FLAGS_INDEX_NUMBER			0
#define WIV_FLAGS_LICTYPE_NUMBER		1
#define WIV_FLAGS_DAYS_NUMBER			2
#define WIV_FLAGS_INSTALL_NUMBER		3

#define WIV_FLAGS_INDEX_MASK			0xF0000000 // 4 bits  0 -> 15
#define WIV_FLAGS_LICTYPE_MASK			0x0C000000 // 2 bits  0 -> 3
#define WIV_FLAGS_DAYS_MASK				0x03FFE000 // 13 bits 0 -> 8191
#define WIV_FLAGS_INSTALL_MASK			0x00001FFF // 13 bits 0 -> 8191

#define WIV_FLAGS_INDEX_SHIFT			28
#define WIV_FLAGS_LICTYPE_SHIFT			26
#define WIV_FLAGS_DAYS_SHIFT			13
#define WIV_FLAGS_INSTALL_SHIFT			0

#define WIV_MAX_OEM_BUF					8
#define WIV_MAX_DSK_NAME				15
#define WIV_MAX_SECTOR_BUF				1024
#define WIV_EPOCH						2450000

#define WIV_CRC_TABLE_SIZE				256
#define WIV_CRYPT_KEY_LEN				128
#define WIV_TRIAL_EXPIRY_PERIOD			(BYTE)15
#define WIV_BETA_EXPIRY_PERIOD			(BYTE)30
#define WIV_BASE16						16

#define WIV_TICKS_PER_SECOND			1000
#define WIV_TICKS_PER_MINUTE			(60*WIV_TICKS_PER_SECOND)
#define WIV_TICKS_PER_HOUR				(60*WIV_TICKS_PER_MINUTE)
#define WIV_TICKS_PER_DAY				(24*WIV_TICKS_PER_HOUR)
#define WIV_TICKS_PER_TRIAL_LICENSE		(WIV_TRIAL_EXPIRY_PERIOD*WIV_TICKS_PER_DAY)
#define WIV_TICKS_PER_BETA_LICENSE		(WIV_BETA_EXPIRY_PERIOD*WIV_TICKS_PER_DAY)


#define WIV_TIME_UNIT					WIV_TICKS_PER_MINUTE	// 1 minute as the time unit (60000 * 1ms = 60s)
#define WIV_TIME_UNITS_PER_DAY			(WIV_TICKS_PER_DAY/WIV_TIME_UNIT)
#define WIV_TIME_ALLOWED_FOR_TRIAL		(WIV_TRIAL_EXPIRY_PERIOD*WIV_TIME_UNITS_PER_DAY)
#define WIV_TIME_ALLOWED_FOR_BETA		(WIV_BETA_EXPIRY_PERIOD*WIV_TIME_UNITS_PER_DAY)

#define WIV_CHK_DISK					2
#define WIV_CHK_OFFSET					4
#define WIV_OEM_OFFSET					3

#define WIV_CHK_DSK_MS_DEFAULT			0x312E344E

#define WIV_CHK_FLAGS_NOT_INIT			0xFFFFFFFF
#define WIV_CHK_DSK_NOT_WRITTEN			0x00FFFFFF
#define WIV_CHK_REG_NOT_PRESENT			0xE158
#define WIV_CHK_FILE_NOT_PRESENT		(WIV_CHK_FILE_SIZE * -1)

#define WIV_CHK_MASK_FLAGS_NONE			0x00
#define WIV_CHK_MASK_FLAGS_NOT_INIT		0x01
#define WIV_CHK_MASK_DSK_NOT_WRITTEN	0x02
#define WIV_CHK_MASK_REG_NOT_PRESENT	0x04
#define WIV_CHK_MASK_FILE_NOT_PRESENT	0x08
#define WIV_CHK_MASK_FLAGS_ALL			0x0F
#define WIV_CHK_MASK_HARD_RESET			0x0D

#define	WIV_CHK_FILE_SIZE				250
#define WIV_CHK_REG_VALUE_SIZE			38

#define WIV_CHK_FILE_REG_POS_A			96
#define WIV_CHK_FILE_REG_POS_B			144
#define WIV_CHK_FILE_FLAGS_POS_H		192
#define WIV_CHK_FILE_FLAGS_POS_L		196
#define WIV_CHK_FILE_XOR_CHAR_POS_A		16
#define WIV_CHK_FILE_XOR_CHAR_POS_B		32

#define WIV_SHIFT_3_BYTES				24
#define WIV_SHIFT_2_BYTES				16
#define WIV_SHIFT_1_BYTE				8

#define WIV_MASK_BYTE_1					0x000000FF
#define WIV_MASK_BYTE_2					0x0000FF00
#define WIV_MASK_BYTE_3					0x00FF0000
#define WIV_MASK_BYTE_4					0xFF000000

#define WIV_PARAMS_TYPE_LSHIFT			29
#define WIV_PARAMS_EXPIRY_LSHIFT		24
#define WIV_PARAMS_TYPE_LMASK			0x00000007
#define WIV_PARAMS_EXPIRY_LMASK			0x0000001F
#define WIV_PARAMS_TYPE_RSHIFT			29
#define WIV_PARAMS_EXPIRY_RSHIFT		24
#define WIV_PARAMS_TYPE_RMASK			0xE0000000
#define WIV_PARAMS_EXPIRY_RMASK			0x1F000000

//=====================
// Default values
//=====================
#define WIV_DEFAULT_IMEI				{0x30,0x30,0x30,0x30,0x30,0x30,0x37,0x32,0x36,0x30,0x33,0x39,0x30,0x31,0x38}

#define	WCHK01							0x0A,0x00,0x00,0x00,0x4B,0x4B,0x49,0x41,0x4F,0x48,0x4A,0x4F,0x4F,0x00,0x11,0x24
#define	WCHK02							0x78,0xF0,0x11,0x24,0x0A,0x00,0x00,0x00,0x4B,0x4B,0x49,0x41,0x4F,0x48,0x4A,0x4F
#define	WCHK03							0x97,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0x00,0x65
#define	WCHK04							0x00,0x72,0x00,0x76,0x00,0x69,0x00,0x63,0x00,0x65,0x00,0x20,0x00,0x50,0x00,0x61
#define	WCHK05							0x00,0x63,0x00,0x6B,0x00,0x20,0x00,0x32,0x00,0x00,0x00,0x90,0x09,0x80,0xFC,0x74
#define	WCHK06							0x0A,0xC0,0xD2,0x90,0x00,0x00,0x52,0x53,0x41,0x31,0x00,0x04,0x00,0x00,0x01,0x00
#define	WCHK07							0x71,0x3A,0x3D,0x33,0x4C,0x3D,0x4C,0x3E,0x3C,0x27,0x3F,0x3C,0x49,0x3D,0x27,0x4C
#define	WCHK08							0x4C,0x3B,0x38,0x27,0x3F,0x38,0x49,0x32,0x27,0x3F,0x4C,0x49,0x4B,0x3E,0x4F,0x49
#define	WCHK09							0x3D,0x3E,0x4B,0x4E,0x4B,0x77,0x49,0x39,0x38,0x3D,0x27,0x3A,0x6E,0x33,0x6C,0x27
#define	WCHK10							0x03,0x48,0x4F,0x41,0x3E,0x4F,0x3E,0x4C,0x4E,0x55,0x4A,0x4D,0x49,0x4E,0x55,0x39
#define	WCHK11							0x40,0x3B,0x48,0x55,0x4B,0x48,0x49,0x4B,0x55,0x4B,0x3B,0x49,0x3B,0x4A,0x4D,0x48
#define	WCHK12							0x4C,0x4B,0x4C,0x48,0x4C,0x05,0x10,0x04,0x00,0x00,0x00,0x70,0xEE,0x09,0x01,0x00
#define	WCHK13							0x02,0x00,0x10,0xFF,0xC0,0xA8,0x1E,0x00,0x00,0x00,0x18,0xFC,0x74,0x0A,0xE2,0x64
#define	WCHK14							0x1F,0x79,0xB4,0xC3,0x1B,0x02,0x03,0x41,0x41,0x4E,0x39,0x49,0x3D,0x4F,0x4F,0x55
#define	WCHK15							0x4C,0x4E,0x4B,0x40,0x55,0x48,0x1C,0x19,0x1D,0x55,0x4C,0x3D,0x4B,0x40,0x55,0x4D
#define	WCHK16							0x3B,0x4B,0x4C,0x4D,0x48,0x4B,0x4C,0x4D,0x49,0x00

#define	WIV_DEFAULT_CHK_FILE_BUFFER		{WCHK01,WCHK02,WCHK03,WCHK04,WCHK05,WCHK06,WCHK07,WCHK08,WCHK09,WCHK10,WCHK11,WCHK12,WCHK13,WCHK14,WCHK15,WCHK16}

namespace WiV
{

	typedef union {
		BYTE	bRegLicense[WIV_MAX_LICENSE_LENGTH];

		struct  {
			union {
				BYTE	abLicense[WIV_MAX_ENCRYPTED_LICENSE_LENGTH];
				struct  {
					BYTE	abLicense1[WIV_MAX_ENCRYPTED_IMEI_LENGTH];
					BYTE	abLicense2[WIV_MAX_ENCRYPTED_LICENSE_LENGTH - WIV_MAX_ENCRYPTED_IMEI_LENGTH];
				}sLicense;

			}License;

			BYTE	IMEI[WIV_MAX_ENCRYPTED_IMEI_LENGTH];
		}RegLicense;

	}WIVLIC, *LPWIVLIC;

	
class CWiVLicense
{

// Friend functions
friend DWORD	CheckLicense(const int nDisk, LPWIVLIC lpLicense);
friend void		GenLicenses(LPTSTR lpszSignal);
friend DWORD	ReadLicense();
friend DWORD	WriteLicense();
friend LPWIVLIC	GetLicense();
friend void		SetLicense(LPWIVLIC lpLic, const bool blPending = false);
friend int		CalcLicenseType(LPWIVLIC lpLic);
friend int		GetLicenseType();
friend void		SetLicenseType(const int nType);
friend int		CalcRemainingTime(const UCHAR uchType, const int nUsedTime = 0);
friend int		CalcElapsedTime(const UCHAR uchType, const int nRemaining = 0);
friend int		CalcDays(const int nDays);
friend USHORT	CalcExpiryDay(const int nRemaining);
friend DWORD	GetRandom(short sIndex = -1, short sLower = 0, short sUpper = 15);
friend BOOL		Crypt(LPCTSTR lpszWith, const LPBYTE lpbIn, LPBYTE lpbOut, LPDWORD lpdwSize, bool bWay);
friend LPCTSTR	GetIMEI();
friend BOOL		GetSerialNumber(LPTSTR lpszSerialNumber);
friend void		EncodeParams(DWORD &dwParams, DWORD &dwConfig);
friend void		DecodeParams(DWORD &dwParams, int &nType, int &nDays, DWORD &dwConfig);
friend LPVOID	UpdateSources(TODAYLISTITEM *pTodayListItem, LPDWORD lpdwParam = NULL);
friend void		EnablePowerNotifications();
friend void		DisablePowerNotifications();
friend DWORD WINAPI	PowerNotificationsThread(LPVOID lpParms);
friend int		LicenseRegisterNotification(LPFNNOTIFY lpfnNotify);
friend int		LicenseDeregisterNotification(LPFNNOTIFY lpfnNotify);
friend			CWivLicenseData();

#ifdef WIV_DEBUG
#endif

public:

class CWivLicenseData
{

public:

	DWORD		dwSize;

	WIVLIC		EncryptedLicense;
	LPWIVLIC	lpLicenses;
	TCHAR		szLicType[WIV_MAX_NAME + 1];
	int			nLicType;
	TCHAR		szIMEI[WIV_MAX_NAME + 1];

	DWORD		dwConfig;
	DWORD		dwParams;
	HANDLE		hHandle;
	DWORD		dwLimited;
	DWORD		dwPattern;
	USHORT		uchStrange;

	bool		blLicenseRead;
	bool		blPending;

	HANDLE		hfFile;
	bool		blFileIsOpen;

	DWORD		dwLastTickCount;
	DWORD		dwUsedTime;
	int			nRemainingDays;
	int			nSuspendTicks;

	HANDLE		htMQueue;
	HANDLE		htPowerThread;
	HANDLE		hnwrNoti;
	DWORD		dwThreadID;
	bool		blKeepGoing;

	LPBYTE		lpbBuffer;
	int			nBufferSize;

	int			nAmberLimit;
	int			nRedLimit;

	CWivLicenseData::CWivLicenseData()
	{
		dwSize			= sizeof(CWivLicenseData);

		memset(&EncryptedLicense, 0, sizeof(EncryptedLicense));
		lpLicenses		= NULL;
		_zclr(szLicType);
		nLicType		= WIV_LICTYPE_TRIAL;
		_zclr(szIMEI);

		dwConfig		= 0;
		dwParams		= 0;
		hHandle			= NULL;
		dwLimited		= 0;
		dwPattern		= 0;
		uchStrange		= 0;

		blLicenseRead	= false;
		blPending		= false;

		hfFile			= NULL;
		blFileIsOpen	= false;

		dwLastTickCount = 0;
		dwUsedTime		= 0;
		nRemainingDays	= 0;
		nSuspendTicks	= 0;

		htMQueue		= NULL;
		htPowerThread	= NULL;
		hnwrNoti		= NULL;
		dwThreadID		= 0;
		blKeepGoing		= true;

		lpbBuffer		= NULL;
		nBufferSize		= 0;

		nAmberLimit		= -30;
		nRedLimit		= nAmberLimit + nAmberLimit;
	}

	CWivLicenseData::~CWivLicenseData()
	{
		if (lpLicenses != NULL)
		{
			LocalFree(lpLicenses);
			lpLicenses = NULL;
		}
	}

}; // class CWivLicenseData
	
private:
	
// Private functions	
UCHAR			LicenseSourcesLoad(const int nDisk, LPDWORD lpdwTodayFlags, LPDWORD lpdwDiskData, LPTSTR lpszRegA, LPTSTR lpszRegB, LPBYTE bFile);
int				LicenseSourcesDecode(const UCHAR uchIndicators, const DWORD dwTodayFlags, const DWORD dwDiskData,
									 LPCTSTR lpszRegA, LPCTSTR lpszRegB, LPBYTE lpbFile,
									 PUCHAR lpuchFlagsType, PUSHORT lpusFlagsElapsed, PUSHORT lpusFlagsInstall,
									 PUCHAR lpuchDiskType, PUSHORT lpusDiskRemaining, PUSHORT lpusDiskPeriod,
									 LPWIVLIC lpRegLicense, PUCHAR lpuchRegType, 	PUSHORT lpusRegInstall,
									 PUSHORT lpusRegRemaining, LPDWORD lpdwRegSession,
									 LPTSTR lpszFileRegA, LPTSTR lpszFileRegB, LPWIVLIC lpFileLicense,
									 PUCHAR lpuchFileType, PUSHORT lpusFileElapsed, PUSHORT lpusFileInstall,
									 PUSHORT lpusFileRemaining, PUSHORT lpusFilePeriod, LPDWORD lpdwFileSession);
int				LicenseSourcesEncode(LPDWORD lpdwTodayFlags, LPDWORD lpdwDiskData,
									 LPTSTR lpszRegA, LPTSTR lpszRegB, LPBYTE bFile, LPWIVLIC lpLicense,
									 const UCHAR uchType, const USHORT usElapsed,
									 const USHORT usInstall, const USHORT usRemaining,
									 const USHORT usPeriod, const DWORD dwSession);
int				LicenseSourcesSave(const int nDisk, const DWORD dwTodayFlags, const DWORD dwDiskData, LPCTSTR lpszRegA, LPCTSTR lpszRegB, LPBYTE bFile, const bool blLimited);

DWORD			LicenseFlagsRead();
int				LicenseFlagsDecode(const DWORD dwFlags, PUCHAR lpuchType, PUSHORT lpusElapsed, PUSHORT lpusInstall);
int				LicenseFlagsEncode(LPDWORD lpdwFlags, const UCHAR uchType, const USHORT usElapsed, const USHORT usInstall);
int				LicenseFlagsWrite(const DWORD dwFlags);

DWORD			LicenseDiskRead(const int nDisk);
int				LicenseDiskDecode(const DWORD dwData, PUCHAR lpuchType, PUSHORT lpusRemaining, PUSHORT lpusPeriod);
int				LicenseDiskEncode(LPDWORD lpdwData, const UCHAR uchType, const USHORT usRemaining, const USHORT usPeriod);
int				LicenseDiskWrite(const int nDisk, const DWORD dwData, const bool blLimited);

int				LicenseRegRead(LPTSTR lpszRegA, LPTSTR lpszRegB);
int				LicenseRegDecode(LPCTSTR lpszRegA, LPCTSTR lpszRegB, LPWIVLIC lpLicense,
								 PUCHAR lpuchType, PUSHORT lpusInstall, PUSHORT lpusRemaining,
								 LPDWORD lpdwSession = NULL);
int				LicenseRegEncode(LPTSTR lpszRegA, LPTSTR lpszRegB,
								 const LPWIVLIC lpLicense,
								 const UCHAR uchType, const USHORT usInstall, const USHORT usRemaining,
								 const DWORD dwSession = 0);
int				LicenseRegWrite(LPCTSTR lpszRegA, LPCTSTR lpszRegB);

int				LicenseFileOpen(const bool blCreate = false);
int				LicenseFileDecode(LPBYTE lpbFile, LPTSTR lpszRegA, LPTSTR lpszRegB, LPWIVLIC lpLicense,
								  PUCHAR lpuchType, PUSHORT lpusElapsed, PUSHORT lpusInstall,
								  PUSHORT lpusRemaining, PUSHORT lpusPeriod, LPDWORD lpdwSession);
int				LicenseFileEncode(LPCTSTR lpszRegA, LPCTSTR lpszRegB, LPBYTE lpbFile,
								  const UCHAR uchType, const USHORT usElapsed, const USHORT usInstall,
								  const USHORT usRemaining, const USHORT usPeriod, const DWORD dwSession);
int				LicenseFileRead(HANDLE hfFile, BYTE *pszDetails);
int				LicenseFileWrite(HANDLE hfFile, const BYTE *const pszDetails);
int				LicenseFileClose(HANDLE hfFile);
int				LicenseFileSetTime();

int				LicenseGetToday();
int				LicenseCalcInstallDay(const int nExpiryDay, const UCHAR uchType);
USHORT			LicenseCalcExpiryDay(const int nRemaining);
int				LicenseCalcRemainingTime(const UCHAR uchType, const int nUsedTime = 0);
int				LicenseCalcElapsedTime(const UCHAR uchType, const int nRemainingTime = 0);
int				LicenseCalcDays(const int nRemainingTime);
LPWIVLIC		LicenseCalcCode(const UCHAR uchType);
int				LicenseCalcType(LPWIVLIC lpLicense = NULL);
int				LicenseCalcType(const USHORT usPeriod);

DWORD			LicenseSourcesCheck(const int nDisk, LPWIVLIC lpLicense, const int nUsedTime = 0);
void			LicenseGenLicenses(LPTSTR lpszSignal);
LPBYTE			LicenseGenMap(LPWIVLIC &lpTrial);
__int64			LicenseMapByteCalc(const __int64 i64Value, const __int64 i64Modifier, const bool blAction = false);
void			LicenseGenName(LPTSTR lpszName);
DWORD			LicenseCodeRead();
DWORD			LicenseCodeWrite();
LPWIVLIC		LicenseGetValue();
LPWIVLIC		LicenseGetTrial();
void			LicenseSetValue(LPWIVLIC lpbLic, const bool blPending = false);
int				LicenseGetType();
void			LicenseSetType(const int nType);
short			LicenseGetPeriod(const UCHAR uchType);
USHORT			LicenseGetTimeAllowed(const UCHAR uchType);
DWORD			LicenseGetRandom(short sIndex = -2, short sLower = 0, short sUpper = 15);
LPVOID			LicenseElapsedUpdate(TODAYLISTITEM *pTodayListItem, LPDWORD lpdwParam);

HANDLE			StorageDiskOpen(int n);
int				StorageDiskGetInfo(const HANDLE hDsk, DISK_INFO &info);
int				StorageDiskRead(const int nDsk, const int nStartByte, const int nNumBytes,
								BYTE* pbBuffer);
int				StorageDiskWrite(const int nDsk, const int nStartByte, const int nNumBytes,
								 BYTE* pbBuffer);

LPCTSTR			GetCoreDLLName(LPTSTR szUnicode);
HMODULE			GetCoreDLLHandle();
FARPROC			GetFunctionAddress(LPCWSTR lpszFunction);

int				GetSuspendTicks();

LPCTSTR			LicenseGetIMEI();
BOOL			LicenseGetSerialNumber(LPTSTR lpszSerialNumber);

void			EncodeRILParams(DWORD &dwParams, DWORD &dwConfig);
void			DecodeRILParams(DWORD &dwParams, int &nType, int &nDays, DWORD &dwConfig);

BOOL			DataCrypt(LPCTSTR lpszWith, const LPBYTE lpbIn, LPBYTE lpbOut,
							 LPDWORD lpdwSize, bool bWay = false);
LPCTSTR			GetCryptProviderName(LPTSTR szUnicode);

BOOL			Decrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final,
						DWORD dwFlags, BYTE *pbData, DWORD *pdwDataLen);
BOOL			Encrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags,
						BYTE *pbData, DWORD *pdwDataLen, DWORD dwBufLen);
BOOL			AcquireContext(HCRYPTPROV *phProv, LPCWSTR szContainer, LPCWSTR szProvider,
							   DWORD dwProvType, DWORD dwFlags);
BOOL			CreateHash(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTKEY hKey, DWORD dwFlags,
						   HCRYPTHASH *phHash);
BOOL			HashData(HCRYPTHASH hHash, CONST BYTE *pbData, DWORD dwDataLen,
						 DWORD dwFlags);
BOOL			DeriveKey(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTHASH hBaseData,
						  DWORD dwFlags, HCRYPTKEY *phKey);
BOOL			SetKeyParameters(HCRYPTKEY hKey, DWORD dwParam, CONST BYTE *pbData,
								 DWORD dwFlags);
BOOL			DestroyHash(HCRYPTHASH hHash);
BOOL			ReleaseContext(HCRYPTPROV hProv, DWORD dwFlags);

BOOL			DeviceIO(HANDLE hDevice, DWORD dwIoControlCode,
						 LPVOID lpInBuf, DWORD nInBufSize,
						 LPVOID lpOutBuf, DWORD nOutBufSize,
						 LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);

#ifdef WIV_DEBUG
#endif

public:

// Constructors/Destructors
CWiVLicense();
~CWiVLicense();
CWiVLicense(HINSTANCE hmInstance);

#ifdef WIV_DEBUG
#endif

}; // class CWiVLicense

// Global non-member functions

#ifdef WIV_DEBUG
#endif

} // namespace WiV

#endif // INC_WIV_LICENSE_H
