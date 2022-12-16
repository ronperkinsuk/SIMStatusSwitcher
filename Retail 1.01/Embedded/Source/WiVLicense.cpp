//////////////////////////////////////////////////////////////////////
//
// WiVLicense.cpp: Implementation of the CWiVLicense class.
//
//////////////////////////////////////////////////////////////////////

extern struct SSS_GLOBALS *SSSGlobals;
#include <Cmnintrin.h>
#include "SSSCommon.h"
#include "WiVDefs.h"
#include "WiVUtils.h"
#include "WiVLicense.h"
#include "WiVReg.h"
#include "ril.h"

namespace WiV
{

static	CWiVLicense	WivLicense;

static	int			m_nClientCount = 0;
static	LPFNNOTIFY	m_fnClients[10];

// File scope variables
static	HMODULE		m_hmInstance;
static	CWiVLicense	*m_pThis;

static CWiVLicense::CWivLicenseData *m_pLicenseData = NULL;

CWiVLicense::CWiVLicense()
{
	SSSGlobals = GlobalsLoad(SSSGlobals);
	m_pLicenseData = NULL;
}

CWiVLicense::~CWiVLicense()
{
	if (m_pLicenseData)
	{
		delete m_pLicenseData;
		m_pLicenseData = NULL;
	}

	m_nClientCount = 0;
}

CWiVLicense::CWiVLicense(HINSTANCE hmInstance)
{

	m_nClientCount = 0;
	m_hmInstance = hmInstance;
}

LPWIVLIC CWiVLicense::LicenseGetTrial()
{
	TraceEnter(_D("CWiVLicense::LicenseGetTrial"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseGetTrial, License Data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseGetTrial"), (DWORD)NULL, tlInternal);
		return NULL;
	}

	TraceLeave(_D("CWiVLicense::LicenseGetTrial"), (DWORD)&m_pLicenseData->lpLicenses[0], tlInternal);
	return &m_pLicenseData->lpLicenses[0];
}

LPWIVLIC CWiVLicense::LicenseGetValue()
{
	TraceEnter(_D("CWiVLicense::LicenseGetValue"), tlInternal);

	if (m_pLicenseData)
	{
		TraceInternal(_D("CWiVLicense::LicenseGetValue, m_pLicenseData->abLic =  <%s>"), BtoS((LPBYTE)&m_pLicenseData->EncryptedLicense, sizeof(WIVLIC)));
		if (*(__int64*)(&m_pLicenseData->EncryptedLicense) == 0)
		{
			TraceInternal(_D("CWiVLicense::LicenseGetValue, m_pLicenseData->abLic is 0"));
			memcpy(&m_pLicenseData->EncryptedLicense, LicenseGetTrial(), sizeof(WIVLIC));
		}
		TraceLeave(_D("CWiVLicense::LicenseGetValue"), (DWORD)&m_pLicenseData->EncryptedLicense, tlInternal);
		return &m_pLicenseData->EncryptedLicense;
	}

	TraceError(_D("CWiVLicense::LicenseGetValue, License Data is NULL"));

	TraceLeave(_D("CWiVLicense::LicenseGetValue"), (DWORD)NULL, tlInternal);
	return NULL;
}

void CWiVLicense::LicenseSetValue(LPWIVLIC lpLic, const bool blPending)
{
	TraceEnter(_D("CWiVLicense::LicenseSetValue"), tlInternal);

	if (m_pLicenseData)
	{
		if (lpLic != NULL)
		{
			memcpy(&m_pLicenseData->EncryptedLicense, lpLic, sizeof(WIVLIC));
			TraceInternal(_D("CWiVLicense::LicenseSetValue, License set to <%s>"), BtoS((LPBYTE)&m_pLicenseData->EncryptedLicense, sizeof(WIVLIC)));

			if (blPending)
			{
				m_pLicenseData->blPending = true;
			}

			if (!m_pLicenseData->blPending)
			{
				LicenseSetType(LicenseCalcType(lpLic));
			}
		}
	}
	else
	{
		TraceError(_D("CWiVLicense::LicenseSetValue, m_pLicenseDatais NULL"));
	}

	TraceLeave(_D("CWiVLicense::LicenseSetValue"), tlInternal);
}

int CWiVLicense::LicenseGetType()
{
	int nRetVal = WIV_LICTYPE_TRIAL;

	TraceEnter(_D("CWiVLicense::LicenseGetType"), tlInternal);

	if (m_pLicenseData)
	{
		nRetVal = m_pLicenseData->nLicType;
	}
	else
	{
		TraceError(_D("CWiVLicense::LicenseGetType, m_pLicenseDatais NULL"));
	}

	TraceLeave(_D("CWiVLicense::LicenseGetType"), nRetVal, tlInternal);
	return nRetVal;
}

void CWiVLicense::LicenseSetType(const int nType)
{
	int nLicType = nType;// + 1;

	TraceEnter(_D("CWiVLicense::LicenseSetType"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseSetType: nType = %d"), nType);

	if (m_pLicenseData)
	{
		m_pLicenseData->nLicType = nType;

		if (nLicType == WIV_LICTYPE_FULL)
		{
			TraceInternal(_D("CWiVLicense::LicenseSetType: Full"));
			_tcsncpy(m_pLicenseData->szLicType, g_szRegisteredLabel, WIV_MAX_NAME);

		}else if (nLicType == WIV_LICTYPE_BETA)
		{
			TraceInternal(_D("CWiVLicense::LicenseSetType: Beta"));
			_tcsncpy(m_pLicenseData->szLicType, g_szBetaLabel, WIV_MAX_NAME);

		}else if (nLicType == WIV_LICTYPE_TRIAL)
		{
			TraceInternal(_D("CWiVLicense::LicenseSetType: Trial"));
			_tcsncpy(m_pLicenseData->szLicType, g_szTrialLabel, WIV_MAX_NAME);

		}else if (nLicType == WIV_LICTYPE_SPECIAL)
		{
			TraceInternal(_D("CWiVLicense::LicenseSetType: Special"));
			_tcsncpy(m_pLicenseData->szLicType, g_szSpecialLabel, WIV_MAX_NAME);
		}else
		{
			TraceInternal(_D("CWiVLicense::LicenseSetType: Error"));
			_tcsncpy(m_pLicenseData->szLicType, WIV_EMPTY_STRING, WIV_MAX_NAME);
		}

	}
	else
	{
		TraceError(_D("CWiVLicense::LicenseSetType, m_pLicenseDatais NULL"));
	}

	TraceLeave(_D("CWiVLicense::LicenseSetType"), tlInternal);
	return;
}

void CWiVLicense::LicenseGenLicenses(LPTSTR lpszSignal)
{
	TCHAR	szUnicode[WIV_MAX_STRING + 1] = _T("");
	BYTE	bIMEI[WIV_MAX_ENCRYPTED_IMEI_LENGTH];

	char	szOrd[5] = "";
	char	szSubst[49] = "";
	char	szFirst[WIV_MAX_DWORD + 1] = "";
	char	szSecond[WIV_MAX_DWORD + 1] = "";

	char	cPad;
	char	szTemp[WIV_MAX_NAME + 1] = "";
	char	szPad[WIV_MAX_NAME + 1] = "";
	char	szIn[WIV_MAX_NAME + 1] = "";
	char	szIn2[WIV_MAX_NAME + 1] = "";
	char	szIMEI[WIV_MAX_NAME + 1] = "";
	DWORD	dwFirst;
	DWORD	dwSecond;

	DWORD	dwSize;

	int		nType;
	int		nIndex;
	int		nCount;
	int		nLength;

	LPBYTE	lpbMap64 = NULL;

	TraceEnter(_D("CWiVLicense::LicenseGenLicenses"), tlInternal);

	if (m_pLicenseData == NULL)
	{
		m_pLicenseData = new CWivLicenseData;
		if (m_pLicenseData == NULL)
		{
			TraceError(_D("CWiVLicense::LicenseGenLicenses: Could not allocate storage for m_pLicenseData"));
			TraceLeave(_D("CWiVLicense::LicenseGenLicenses"), tlInternal);
			return;
		}

		// Generate random minutes for timeouts
		m_pLicenseData->nAmberLimit	= (LicenseGetRandom(-1, 5, 60) * -1);
		m_pLicenseData->nRedLimit = m_pLicenseData->nAmberLimit + (LicenseGetRandom(-1, 30, 90) * -1);
		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: m_pLicenseData->nAmberLimit = %d"), m_pLicenseData->nAmberLimit);
		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: m_pLicenseData->nRedLimit   = %d"), m_pLicenseData->nRedLimit);
	}

	lpbMap64 = LicenseGenMap(m_pLicenseData->lpLicenses);
	TraceInternal(_D("CWiVLicense::LicenseGenLicenses: m_pLicenseData->lpabLicenses = 0x%08X"), m_pLicenseData->lpLicenses);

	if (lpbMap64 != NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: pbMap64 = <%s>"), BtoS(lpbMap64, 128));
		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: dwSize = %d, m_pLicenseData->lpabLicenses[%d] = <%s>"), sizeof(WIVLIC), 0, BtoS((LPBYTE)LicenseGetTrial(), sizeof(WIVLIC)));
	}
	else
	{
		TraceError(_D("CWiVLicense::LicenseGenLicenses: pbMap64 is NULL"));
		if (m_pLicenseData != NULL)
		{
			delete m_pLicenseData;
		}

		TraceLeave(_D("CWiVLicense::LicenseGenLicenses"), tlInternal);
		return;
	}

	// If passed pointer is NULL, extract IMEI from the license read from registry
	if (lpszSignal == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: IMEI is NULL"));
		
		// Get IMEI number from read license
		memcpy(bIMEI, m_pLicenseData->EncryptedLicense.RegLicense.IMEI, WIV_MAX_ENCRYPTED_IMEI_LENGTH);
		for (int i = 0; i < WIV_MAX_ENCRYPTED_IMEI_LENGTH; i++)
		{
			bIMEI[i] ^= m_pLicenseData->EncryptedLicense.RegLicense.License.sLicense.abLicense1[i];
		}

		_tcsncpy(m_pLicenseData->szIMEI, BtoS(bIMEI, WIV_MAX_ENCRYPTED_IMEI_LENGTH), WIV_MAX_NAME);
	}
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: IMEI = %s"), lpszSignal);

		_tcsncpy(m_pLicenseData->szIMEI, lpszSignal, WIV_MAX_NAME);
	}

	// Convert from unicode
	UtoA(m_pLicenseData->szIMEI, szIn, _tcslen(m_pLicenseData->szIMEI));

	TraceInternal(_D("CWiVLicense::LicenseGenLicenses: szIn = <%S>"), szIn);

	nLength = WIV_MAX_LIC_STRING_LENGTH;

	TraceInternal(_D("CWiVLicense::LicenseGenLicenses: Type = %d, License = <%S>, Encrypted License = <%s>"), 0, "BMKKBOUZNLXOPOBCNCOZ", BtoS((LPBYTE)LicenseGetTrial(), sizeof(WIVLIC)));

	for (nType = 1; nType < (WIV_MAX_LICENSES); nType++)
	{
		strncpy(szIn2, szIn, WIV_MAX_NAME);
		strncpy(szIMEI, szIn, WIV_MAX_NAME);

		// Add extra digit conditioned on license type, unless no IMEI passed
		// Type 1  = Full 1,    digit = '0'
		// Type 2  = Full 2,    digit = '1'
		// Type 3  = Beta 1,    digit = '2'
		// Type 4  = Beta 2,    digit = '3'
		// Type 5  = Special 1, digit = '4'
		// Type 6  = Special 2, digit = '5'

		cPad = 0x30 + (nType - 1);

		szIMEI[15] = cPad;
		szIMEI[16] = 0;
		szIn2[15] = 0x35;
		szIn2[16] = 0;

		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: szIMEI = <%S>, szIn2 = <%S>"), szIMEI, szIn2);
		
		// Do the sums on first 8 bytes
		memset(szFirst,0,sizeof(szFirst));
		memcpy(szFirst,&szIMEI[0],8);
		dwFirst = atol(szFirst);
		dwFirst *= (10 + nType);
		dwFirst /= (2 + nType);

		_ultoa(dwFirst, szFirst, 10);

		// Do the sums on second 8 bytes
		memset(szSecond,0,sizeof(szSecond));
		memcpy(szSecond,&szIMEI[8],8);
		dwSecond = atol(szSecond);
		dwSecond *= (10 + nType);
		dwSecond /= (5 + nType);

		_ultoa(dwSecond, szSecond, 10);

		// Rejoin to make 16 bytes
		strcpy(szIMEI, szFirst);
		strcat(szIMEI, szSecond);

		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: IMEI Mult = %s"), BtoS((LPBYTE)szIMEI, strlen(szIMEI)));

		// Insert pad characters starting at array index = type - 1, until required total length reached
		nCount = nLength - strlen(szIMEI);

		memset(szTemp, 0, sizeof(szTemp));
		memset(szPad, 0, sizeof(szPad));
		strncpy(szTemp, szIMEI, nLength);

		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: type = %d, szTemp = <%s>, count = %d, pad = <%c>"), nType, AtoU(szTemp, szUnicode, strlen(szTemp)), nCount, cPad);

		// Construct pad string
		for (nIndex = 0 ; nIndex < nCount; nIndex++)
		{
			szPad[nIndex] = cPad;
		}

		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: Pad string = <%s>"), AtoU(szPad, szUnicode));

		// Reconstruct padded IMEI string
		memset(szIMEI, 0, WIV_MAX_NAME);
		strncpy(szIMEI, szTemp, nType - 1);
		strcat(szIMEI, szPad);
		strcat(szIMEI, &szTemp[nType - 1]);

		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: Padded IMEI = <%s>"), AtoU(szIMEI, szUnicode, strlen(szIMEI)));

		// Exclusive or each byte with byte from character table
		for (nCount = 0; nCount < (int)strlen(szIMEI); nCount++)
		{
			szIMEI[nCount] ^= lpbMap64[nCount];
		}

		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: Xored IMEI = %s"),BtoS((LPBYTE)szIMEI, strlen(szIMEI)));

		char szDecimal[65] = "";

		// Get ordinal for each character, making sure leading zeros used to pad to 3 characters
		for (nCount = strlen(szIMEI)-1; nCount >=0; nCount--)
		{
			memset(szOrd, 0, sizeof(szOrd));
			_itoa(szIMEI[nCount], szOrd, 10);
			int j = strlen(szOrd);
			for ( ; j < 3 ; j++)
			{
				strcat(szDecimal, "0");
			}

			strcat(szDecimal, szOrd);
		}

		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: Ordinal values IMEI = %s"), BtoS((LPBYTE)szDecimal, strlen(szDecimal)));

		memset(szSubst, 0, sizeof(szSubst));

		// Substitute characters for characters from table, indexed by ordinals
		for (nCount = 0; nCount <=(int)strlen(szDecimal) - 1; nCount+=3)
		{
			char szChar[2];

			memset(szChar, 0, sizeof(szChar));
			memset(szOrd, 0, sizeof(szOrd));
			memcpy(szOrd, &szDecimal[nCount],3);
			memcpy(szChar, &lpbMap64[atoi(szOrd)], 1);
			strcat(szSubst,szChar);
		}

		dwSize = strlen(szSubst);

		BYTE	bLic[sizeof(WIVLIC)];
		
		// Encrypt into temporary buffer
		DataCrypt(NULL, (LPBYTE)szSubst, bLic, &dwSize, true);

		// Convert IMEI string to byte nibbles
		for (int j = 0, i = WIV_MAX_ENCRYPTED_LICENSE_LENGTH; j <= (int)strlen(szIn2) - 1;)
		{
			unsigned short s1 = 0;
			unsigned short s2 = 0;

			s1 = ((szIn2[j++] - 0x30) << 4);

			if (j <= (int)strlen(szIn2) - 1)
			{
				s2 = szIn2[j++] - 0x30;
			}

			bLic[i] = (s1 | s2) ^ bLic[i - WIV_MAX_ENCRYPTED_LICENSE_LENGTH];
			i++;

			if (j > (int)strlen(szIn2) - 1)
				break;
		}

		memcpy(&m_pLicenseData->lpLicenses[nType], bLic, sizeof(WIVLIC));
		
		TraceInternal(_D("CWiVLicense::LicenseGenLicenses: Type = %d, License = <%S>, Encrypted License = <%s>"), nType, szSubst, BtoS((LPBYTE)&m_pLicenseData->lpLicenses[nType], sizeof(WIVLIC)));

	} // for (nType = 1; nType < (WIV_MAX_SIGNAL_STATES + 1); nType++)

	if (lpbMap64 != NULL)
	{
		LocalFree(lpbMap64);
	}

	m_pLicenseData->dwLimited = LicenseGetRandom();
	m_pLicenseData->dwPattern = m_pLicenseData->dwLimited;

	TraceInternal(_D("CWiVLicense::LicenseGenLicenses: m_pLicenseData->dwLimited = 0x%08X"), m_pLicenseData->dwLimited);
	TraceInternal(_D("CWiVLicense::LicenseGenLicenses: m_pLicenseData->dwPattern = 0x%08X"), m_pLicenseData->dwPattern);

	TraceInternal(_D("CWiVLicense::LicenseGenLicenses, Licenses generated"));
	TraceLeave(_D("CWiVLicense::LicenseGenLicenses"), tlInternal);

	return;

}

LPBYTE CWiVLicense::LicenseGenMap(LPWIVLIC &lpTrial)
{
	LPBYTE	lpbMap64;
	__int64 i64;
	BYTE	bBuff[sizeof(WIVLIC)];
	BYTE	bDefIMEI[WIV_MAX_ENCRYPTED_IMEI_LENGTH * 2] = WIV_DEFAULT_IMEI;
	BYTE	bIMEI[WIV_MAX_ENCRYPTED_IMEI_LENGTH];

	TraceEnter(_D("CWiVLicense::LicenseGenMap"), tlInternal);

	lpbMap64 = (PBYTE)LocalAlloc (LPTR, 128);

	if (lpbMap64 == NULL)
	{
		TraceError(_D("CWiVLicense::LicenseGenMap: Cannot allocate memory for data map"));
		TraceLeave(_D("CWiVLicense::LicenseGenMap"), (DWORD)lpbMap64, tlInternal);
		return lpbMap64;
	}

	if (lpTrial == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseGenMap: Requesting %d bytes of zero initialised memory for license codes"), (WIV_MAX_LICENSES * sizeof(WIVLIC)));
		lpTrial = (LPWIVLIC)LocalAlloc (LPTR, (WIV_MAX_LICENSES * sizeof(WIVLIC)));
	}

	if (lpTrial == NULL)
	{
		TraceError(_D("CWiVLicense::LicenseGenMap: Cannot allocate memory for license codes"));
		if (lpbMap64 != NULL)
		{
			LocalFree(lpbMap64);
			lpbMap64 = NULL;
		}

		TraceLeave(_D("CWiVLicense::LicenseGenMap"), (DWORD)lpbMap64, tlInternal);
		return lpbMap64;
	}

	TraceInternal(_D("CWiVLicense::LicenseGenMap: %d bytes of memory allocated for license codes"), LocalSize(lpTrial));

	// Generate license substitution data map
	i64 = _byteswap_uint64(3472328296227680304);

	i64 = LicenseMapByteCalc(i64, 2382732025437623572);
	memcpy(&lpbMap64[sizeof(__int64) * 0], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 1004043236438770411, true);
	memcpy(&lpbMap64[sizeof(__int64) * 1], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 359178511300298998);
	memcpy(&lpbMap64[sizeof(__int64) * 2], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 288787849955511813);
	memcpy(&lpbMap64[sizeof(__int64) * 3], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 429257044704751860);
	memcpy(&lpbMap64[sizeof(__int64) * 4], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 792372941755907078, true);
	memcpy(&lpbMap64[sizeof(__int64) * 5], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 1080041506137502740);
	memcpy(&lpbMap64[sizeof(__int64) * 6], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 1363181173349808655, true);
	memcpy(&lpbMap64[sizeof(__int64) * 7], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 210535538837750030);
	memcpy(&lpbMap64[sizeof(__int64) * 8], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 1373596804133816322);
	memcpy(&lpbMap64[sizeof(__int64) * 9], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 1441712545823457799, true);
	memcpy(&lpbMap64[sizeof(__int64) * 10], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 287966575200371973, true);
	memcpy(&lpbMap64[sizeof(__int64) * 11], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 794625836888228611);
	memcpy(&lpbMap64[sizeof(__int64) * 12], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 721724839803355141, true);
	memcpy(&lpbMap64[sizeof(__int64) * 13], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 646557882581711884);
	memcpy(&lpbMap64[sizeof(__int64) * 14], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 503829170128421102);
	memcpy(&lpbMap64[sizeof(__int64) * 15], &i64, sizeof(__int64));

	// Generate encrypted Trial license data
	i64 = LicenseMapByteCalc(i64, 5096405956244103200, true);
	memcpy(&bBuff[sizeof(__int64) * 0], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 1504788337283507025);
	memcpy(&bBuff[sizeof(__int64) * 1], &i64, sizeof(__int64));

	i64 = LicenseMapByteCalc(i64, 2586173651626374068);
	memcpy(&bBuff[sizeof(__int64) * 2], &i64, sizeof(__int64));


	// Generate encrypted IMEI
	bDefIMEI[sizeof(bDefIMEI) - 1] = 0x35;

	// Convert IMEI string to byte nibbles
	for (int j = 0, i = 0 ; j <= (sizeof(bDefIMEI) - 1);)
	{
		unsigned short s1 = 0;
		unsigned short s2 = 0;

		s1 = ((bDefIMEI[j++] - 0x30) << 4);

		if (j <= sizeof(bDefIMEI) - 1)
		{
			s2 = bDefIMEI[j++] - 0x30;
		}

		bIMEI[i++] = s1 | s2;

		if (j > sizeof(bDefIMEI) - 1)
			break;
	}

	TraceInternal(_D("CWiVLicense::LicenseGenMap: bDefIMEI = <%s>"), BtoS(bDefIMEI, sizeof(bDefIMEI)));
	TraceInternal(_D("CWiVLicense::LicenseGenMap: bIMEI = <%s>"), BtoS(bIMEI, sizeof(bIMEI)));

	for (i = 0; i < WIV_MAX_ENCRYPTED_IMEI_LENGTH; i++)
	{
		bIMEI[i] ^= bBuff[i]; 		
	}

	memcpy(&bBuff[sizeof(__int64) * 3], bIMEI, WIV_MAX_ENCRYPTED_IMEI_LENGTH);

	memcpy(&lpTrial[0], bBuff, sizeof(bBuff));
	
	TraceInternal(_D("CWiVLicense::LicenseGenMap: lpTrial = <%s>"), BtoS((LPBYTE)lpTrial, sizeof(WIVLIC)*WIV_MAX_LICENSES));

	TraceLeave(_D("CWiVLicense::LicenseGenMap"), (DWORD)lpbMap64, tlInternal);
	return lpbMap64;
}

//#pragma optimize("", off)
__int64 CWiVLicense::LicenseMapByteCalc(const __int64 i64Value, const __int64 i64Modifier, const bool blAction)
{
	__int64 i64;
	
	i64 = _byteswap_uint64(i64Value);
	if (blAction)
	{
		i64 -= i64Modifier;
	}
	else
	{
		i64 += i64Modifier;
	}

	return _byteswap_uint64(i64);
}
//#pragma optimize("", on)

void CWiVLicense::LicenseGenName(LPTSTR lpszName)
{
	_tcsncpy(lpszName, _T("License_"), WIV_MAX_NAME);

	lpszName[0] = g_pSSSData->szNoLicenseLabel[3];		// L
	lpszName[1] = g_pSSSData->szCompanyName[1];			// i
	lpszName[2] = g_pSSSData->szCompanyURL[17];			// c
	lpszName[3] = g_pSSSData->szCompanyIdentity[11];	// e
	lpszName[4] = g_pSSSData->szDevelopedByLabel[5];	// n
	lpszName[5] = g_pSSSData->szCompanySupport[0];		// s
	lpszName[6] = g_pSSSData->szProductName[17];		// e
	lpszName[7] = g_pSSSData->szProductVersion[0];		// 1

	return;
}

DWORD CWiVLicense::LicenseCodeRead()
{
	DWORD	dwResult = 0;
	DWORD	dwSize = 0;
	HKEY	hKey;
	TCHAR	szLicName[WIV_MAX_NAME +1];

	TraceEnter(_D("CWiVLicense::LicenseCodeRead"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseCodeRead: License data is NULL"));
		dwResult = WIV_LICTYPE_ERROR;
		TraceLeave(_D("CWiVLicense::LicenseCodeRead"), dwResult, tlInternal);
		return dwResult;
	}

	if (!g_pSSSData)
	{
		TraceError(_D("CWiVLicense::LicenseCodeRead: SSS data is NULL"));
		dwResult = WIV_LICTYPE_ERROR;
		TraceLeave(_D("CWiVLicense::LicenseCodeRead"), dwResult, tlInternal);
		return dwResult;
	}

	m_pLicenseData->nLicType = WIV_LICTYPE_TRIAL;
	_tcsncpy(m_pLicenseData->szLicType, g_szTrialLabel, WIV_MAX_NAME);

	LicenseGenName(szLicName);

	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Getting license from registry"));

	// Open the root Key [HKEY_LOCAL_MACHINE\SOFTWARE\WiViT\SIM Status Switcher]
	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Calling RegKeyOpen for root key"));
	dwResult = RegKeyOpen(SSS_REG_ROOT_KEY, &hKey);
	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Back from RegKeyOpen, dwResult = <%08x>, hKey = <%08x>"), dwResult, hKey);

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("CWiVLicense::LicenseCodeRead: Error opening root key, dwResult = <%08x>"), dwResult);
		dwResult = WIV_LICTYPE_ERROR;
		TraceLeave(_D("CWiVLicense::LicenseCodeRead"), dwResult, tlInternal);
		return dwResult;
	}

	// Get license from registry
	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Calling RegGetValue for <%s>"), szLicName);
	dwResult = RegGetValue(hKey, szLicName, (LPBYTE)&m_pLicenseData->EncryptedLicense, sizeof(WIVLIC), NULL);
	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Back from RegGetValue, dwResult = %d"), dwResult);
	if (dwResult == sizeof(WIVLIC))
	{
		TraceInternal(_D("CWiVLicense::LicenseCodeRead: License read from registry"));
	}
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseCodeRead: No license found, so use trial licence"));
		memcpy(&m_pLicenseData->EncryptedLicense, LicenseGetTrial(), sizeof(WIVLIC));
	}

	TraceInternal(_D("CWiVLicense::LicenseCodeRead: License = %s"), BtoS((LPBYTE)&m_pLicenseData->EncryptedLicense, sizeof(WIVLIC)));
	
	m_pLicenseData->blLicenseRead = true;

	m_pLicenseData->blPending = false;

	LicenseSetValue(&m_pLicenseData->EncryptedLicense);
	TraceInternal(_D("CWiVLicense::LicenseCodeRead: m_pLicenseData->nLicType = <%08x>"), m_pLicenseData->nLicType);

	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Calling LicenseSourcesCheck (<%s>)"), BtoS((LPBYTE)&m_pLicenseData->EncryptedLicense, sizeof(WIVLIC)));
	LicenseSourcesCheck(WIV_CHK_DISK, &m_pLicenseData->EncryptedLicense);
	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Back from LicenseSourcesCheck"));

	// Close the root Key
	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Calling RegKeyClose"));
	dwResult = RegKeyClose(hKey);
	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Back from RegKeyClose, dwResult = <%08x>"), dwResult);

//	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Calling LicenseCalcType"));
//	m_pLicenseData->nLicType = LicenseCalcType(m_pLicenseData->abLic);
//	TraceInternal(_D("CWiVLicense::LicenseCodeRead: Back from LicenseCalcType"));

	dwResult = m_pLicenseData->nLicType;
	TraceLeave(_D("CWiVLicense::LicenseCodeRead"), dwResult, tlInternal);

	return dwResult;
}

DWORD CWiVLicense::LicenseCodeWrite()
{
	HKEY	hKey;
	DWORD	dwResult;
	TCHAR	szLicName[WIV_MAX_NAME +1];

	TraceEnter(_D("CWiVLicense::LicenseCodeWrite"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseCodeWrite: License data is NULL"));
		dwResult = -1;
		TraceLeave(_D("CWiVLicense::LicenseCodeWrite"), dwResult, tlInternal);
		return dwResult;
	}

	if (!g_pSSSData)
	{
		TraceError(_D("CWiVLicense::LicenseCodeWrite: SSS data is NULL"));
		dwResult = -1;
		TraceLeave(_D("CWiVLicense::LicenseCodeWrite"), dwResult, tlInternal);
		return dwResult;
	}

	LicenseGenName(szLicName);

	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Writing license to registry"));

	// Open the Key [HKEY_LOCAL_MACHINE\SOFTWARE\WiViT\SIM Status Switcher]
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Calling RegKeyOpen for root key"));
	dwResult = RegKeyOpen(SSS_REG_ROOT_KEY, &hKey);
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Back from RegKeyOpen, dwResult = <%08x>, hKey = <%08x>"), dwResult, hKey);

	if (dwResult != ERROR_SUCCESS)
	{
		TraceError(_D("CWiVLicense::LicenseCodeWrite: Error opening root key, dwResult = <%08x>"), dwResult);
		TraceLeave(_D("CWiVLicense::LicenseCodeWrite"), dwResult, tlInternal);
		return dwResult;
	}
	
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: m_pLicenseData->abLic = <%s>"), BtoS((LPBYTE)&m_pLicenseData->EncryptedLicense, sizeof(WIVLIC)));
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: m_pLicenseData->szLicType = <%s>"), m_pLicenseData->szLicType);

	// Write license code to registry
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Calling RegSetValue for <%s>"), szLicName);
	dwResult = RegSetValue(hKey, szLicName, (LPBYTE)&m_pLicenseData->EncryptedLicense, sizeof(WIVLIC));
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Calling RegSetValue for <%s>"), g_szRegLicenseType);
	dwResult = RegSetValue(hKey, g_szRegLicenseType, m_pLicenseData->szLicType);
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Back from RegSetValue, dwResult = <%08x>"), dwResult);

	// Close the root Key
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Calling RegKeyClose"));
	dwResult = RegKeyClose(hKey);
	TraceInternal(_D("CWiVLicense::LicenseCodeWrite: Back from RegKeyClose, dwResult = <%08x>"), dwResult);

	TraceLeave(_D("CWiVLicense::LicenseCodeWrite"), dwResult, tlInternal);

	return dwResult;
}

LPWIVLIC CWiVLicense::LicenseCalcCode(const UCHAR uchType)
{
	char		chType = (char)uchType;
	LPWIVLIC	lpLicense = NULL;

	TraceEnter(_D("CWiVLicense::LicenseCalcCode"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseCalcCode: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseCalcCode"), (DWORD)lpLicense, tlInternal);
		return lpLicense;
	}

	if (m_pLicenseData->blPending)
	{
		TraceInternal(_D("CWiVLicense::LicenseCalcCode: License pending"));
		TraceLeave(_D("CWiVLicense::LicenseCalcCode"), (DWORD)lpLicense, tlInternal);
		return lpLicense;
	}

	TraceInternal(_D("CWiVLicense::LicenseCalcCode: Type = %d"), chType);

	if (chType == WIV_LICTYPE_TRIAL)
	{
		lpLicense = LicenseGetTrial();
		TraceInternal(_D("CWiVLicense::LicenseCalcCode: lpbLicense Trial = <%s>"), BtoS((LPBYTE)lpLicense, sizeof(WIVLIC)));

	} else if (chType >= WIV_LICTYPE_FULL)
	{
		lpLicense = &m_pLicenseData->lpLicenses[(chType + 1)];
		TraceInternal(_D("CWiVLicense::LicenseCalcCode: lpbLicense Full = <%s>"), BtoS((LPBYTE)lpLicense, sizeof(WIVLIC)));
	}
	else
	{
		lpLicense = NULL;
		TraceInternal(_D("CWiVLicense::LicenseCalcCode: lpbLicense = NULL"));
	}


	TraceLeave(_D("CWiVLicense::LicenseCalcCode"), (DWORD)lpLicense, tlInternal);
	return lpLicense;
}

int CWiVLicense::LicenseCalcType(const USHORT usPeriod)
{
	int		nState = WIV_LICTYPE_NONE;

	TraceEnter(_D("CWiVLicense::LicenseCalcType"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseCalcType: Period = %d"), usPeriod);

	if (usPeriod == WIV_TRIAL_EXPIRY_PERIOD)
	{
		nState = WIV_LICTYPE_TRIAL;

	} else if (usPeriod == WIV_BETA_EXPIRY_PERIOD)
	{
		nState = WIV_LICTYPE_BETA;
	}

	TraceInternal(_D("CWiVLicense::LicenseCalcType: nState = %d"), nState);

	TraceLeave(_D("CWiVLicense::LicenseCalcType"), (DWORD)nState, tlInternal);
	return nState;
}

int CWiVLicense::LicenseCalcType(LPWIVLIC lpLicense)
{
	int		nState = WIV_LICTYPE_TRIAL;
	WIVLIC	wivLicense;

	TraceEnter(_D("CWiVLicense::LicenseCalcType"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseCalcType: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseCalcType"), (DWORD)nState, tlInternal);
		return nState;
	}

	if (m_pLicenseData->blPending)
	{
		TraceInternal(_D("CWiVLicense::LicenseCalcType: License pending"));
		TraceLeave(_D("CWiVLicense::LicenseCalcType"), (DWORD)m_pLicenseData->nLicType, tlInternal);
		return m_pLicenseData->nLicType;
	}

	if (lpLicense == NULL)
	{
		memcpy(&wivLicense, LicenseGetTrial(), sizeof(WIVLIC));
	}
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseCalcType: lpLicense = <%s>"), BtoS((LPBYTE)lpLicense, sizeof(WIVLIC)));
		memcpy(&wivLicense, lpLicense, sizeof(WIVLIC));
	}

	TraceInternal(_D("CWiVLicense::LicenseCalcType: License = <%s>"), BtoS((LPBYTE)&wivLicense, sizeof(WIVLIC)));

	if (memcmp(&wivLicense, LicenseGetTrial(), sizeof(WIVLIC)) == 0)
	{
		TraceInternal(_D("CWiVLicense::LicenseCalcType: Trial"));
		nState = WIV_LICTYPE_TRIAL;
	}
	else
	{

		for (nState = 1; nState < WIV_MAX_LICENSES; nState++)
		{
			TraceInternal(_D("CWiVLicense::LicenseCalcType: nState = %d, m_pLicenseData->lpLicenses[nState] = <%s>"), nState, BtoS((LPBYTE)&m_pLicenseData->lpLicenses[nState], sizeof(WIVLIC)));
			if (memcmp(&wivLicense, &m_pLicenseData->lpLicenses[nState], sizeof(WIVLIC)) == 0) break;
		}

		TraceInternal(_D("CWiVLicense::LicenseCalcType: nState = %d"), nState);

		if (nState >= WIV_MAX_LICENSES)
		{
			TraceInternal(_D("CWiVLicense::LicenseCalcType: Error"));
			nState = WIV_LICTYPE_ERROR;

		}else if (nState >= 1 && nState <= 2)
		{
			TraceInternal(_D("CWiVLicense::LicenseCalcType: Full"));
			nState = WIV_LICTYPE_FULL;

		}else if (nState >= 3 && nState <= 4)
		{
			TraceInternal(_D("CWiVLicense::LicenseCalcType: Beta"));
			nState = WIV_LICTYPE_BETA;

		}else if (nState >= 5 && nState <= 6)
		{
			TraceInternal(_D("CWiVLicense::LicenseCalcType: Special"));
			nState = WIV_LICTYPE_SPECIAL;
		}else
		{
			TraceInternal(_D("CWiVLicense::LicenseCalcType: Error"));
			nState = WIV_LICTYPE_ERROR;
		}
	}

	TraceLeave(_D("CWiVLicense::LicenseCalcType"), (DWORD)nState, tlInternal);
	return nState;
}

// Calculate install day
int	CWiVLicense::LicenseCalcInstallDay(const int nExpiryDay, const UCHAR uchType)
{
	int			nJInstall = 0;

	TraceEnter(_D("CWiVLicense::LicenseCalcInstallTime"), tlInternal);

	if (uchType == WIV_LICTYPE_TRIAL) nJInstall = nExpiryDay - WIV_TRIAL_EXPIRY_PERIOD;
	else if (uchType == WIV_LICTYPE_BETA) nJInstall = nExpiryDay - WIV_BETA_EXPIRY_PERIOD;

	TraceInternal(_D("CWiVLicense::LicenseCalcInstallTime: Install day = %d (0x%08X)"), nJInstall, nJInstall);

	TraceLeave(_D("CWiVLicense::LicenseCalcInstallTime"), (DWORD)nJInstall, tlInternal);

	return	nJInstall;
}

// Calculate expiry day
USHORT	CWiVLicense::LicenseCalcExpiryDay(const int nRemaining)
{
	int			nJToDay;
	int			nJExpDay = 0;

	TraceEnter(_D("CWiVLicense::LicenseCalcExpiryDay"), tlInternal);

	nJToDay = LicenseGetToday();

	TraceInternal(_D("CWiVLicense::LicenseCalcExpiryDay: Today's day = %d (0x%08X)"), nJToDay, nJToDay);

	nJExpDay = nJToDay + LicenseCalcDays(nRemaining);

	TraceInternal(_D("CWiVLicense::LicenseCalcExpiryDay: Expiry day = %d (0x%08X)"), nJExpDay, nJExpDay);

	TraceLeave(_D("CWiVLicense::LicenseCalcExpiryDay"), (DWORD)nJExpDay, tlInternal);

	return	nJExpDay;
}

int	CWiVLicense::LicenseCalcDays(const int nRemainingTime)
{
	double		dRemainingDays;

	TraceEnter(_D("CWiVLicense::LicenseCalcDays"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseCalcDays: nRemainingTime = %d"), nRemainingTime, tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseCalcDays: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseCalcDays"), (DWORD)0, tlInternal);
		return 0;
	}

	dRemainingDays = (double)(nRemainingTime)/(double)WIV_TIME_UNITS_PER_DAY;

	if (dRemainingDays < 0)
	{
		m_pLicenseData->nRemainingDays = (int)floor(dRemainingDays);
	}
	else
	{
		m_pLicenseData->nRemainingDays = (int)ceil(dRemainingDays);
	}

	TraceInternal(_D("CWiVLicense::LicenseCalcDays: Remaining days = %d"), m_pLicenseData->nRemainingDays);

	TraceLeave(_D("CWiVLicense::LicenseCalcDays"), m_pLicenseData->nRemainingDays, tlInternal);
	return m_pLicenseData->nRemainingDays;
}

short CWiVLicense::LicenseGetPeriod(const UCHAR uchType)
{
	TraceEnter(_D("CWiVLicense::LicenseGetPeriod"), tlInternal);

	short sRetVal = 0;

	if (uchType == WIV_LICTYPE_TRIAL)
	{
		sRetVal = WIV_TRIAL_EXPIRY_PERIOD;

	}else if (uchType == WIV_LICTYPE_BETA)
	{
		sRetVal = WIV_BETA_EXPIRY_PERIOD;
	}

	TraceLeave(_D("CWiVLicense::LicenseGetPeriod"), (DWORD)sRetVal, tlInternal);
	return sRetVal;
}

USHORT CWiVLicense::LicenseGetTimeAllowed(const UCHAR uchType)
{
	TraceEnter(_D("CWiVLicense::LicenseGetTimeAllowed"), tlInternal);

	USHORT usRetVal = 0;

	if (uchType == WIV_LICTYPE_TRIAL)
	{
		usRetVal = WIV_TIME_ALLOWED_FOR_TRIAL;

	}else if (uchType == WIV_LICTYPE_BETA)
	{
		usRetVal = WIV_TIME_ALLOWED_FOR_BETA;
	}

	TraceLeave(_D("CWiVLicense::LicenseGetTimeAllowed"), (DWORD)usRetVal, tlInternal);
	return usRetVal;
}

int	CWiVLicense::LicenseCalcElapsedTime(const UCHAR uchType, const int nRemainingTime)
{
	int nAllowedTime;
	int nElapsedTime = 0;
	int nRemaining;

	TraceEnter(_D("CWiVLicense::LicenseCalcElapsedTime"), tlInternal);

	nAllowedTime = (uchType == WIV_LICTYPE_TRIAL) ? WIV_TIME_ALLOWED_FOR_TRIAL : WIV_TIME_ALLOWED_FOR_BETA;
	TraceInternal(_D("CWiVLicense::LicenseCalcElapsedTime: Allowed Time = %d"), nAllowedTime);

	if (nRemainingTime == 0)
	{
		nRemaining = nAllowedTime;
	}
	else
	{
		nRemaining = nRemainingTime;
	}

	TraceInternal(_D("CWiVLicense::LicenseCalcElapsedTime: Remaining Time = %d"), nRemaining);

	nElapsedTime = nAllowedTime - nRemaining;
	TraceInternal(_D("CWiVLicense::LicenseCalcElapsedTime: Elapsed Time = %d"), nElapsedTime);

	TraceLeave(_D("CWiVLicense::LicenseCalcElapsedTime"), nElapsedTime, tlInternal);
	return nElapsedTime;
}

int CWiVLicense::LicenseCalcRemainingTime(const UCHAR uchType, const int nUsedTime)
{
	int nAllowedTime;
	int nRemainingTime = 0;
	int nUsed;

	TraceEnter(_D("CWiVLicense::LicenseGetRemainingTime"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseGetRemainingTime: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseGetRemainingTime"), (DWORD)0, tlInternal);
		return 0;
	}

	if (nUsedTime == 0)
	{
		nUsed = m_pLicenseData->dwUsedTime;
	}
	else
	{
		nUsed = nUsedTime;
	}

	TraceInternal(_D("CWiVLicense::LicenseGetRemainingTime: Used Time = %d"), nUsed);

	nAllowedTime = (uchType == WIV_LICTYPE_TRIAL) ? WIV_TIME_ALLOWED_FOR_TRIAL : WIV_TIME_ALLOWED_FOR_BETA;

	nRemainingTime = nAllowedTime - nUsed;
	TraceInternal(_D("CWiVLicense::LicenseGetRemainingTime: Remaining Time = %d"), nRemainingTime);

	TraceLeave(_D("CWiVLicense::LicenseGetRemainingTime"), nRemainingTime, tlInternal);
	return nRemainingTime;
}

DWORD CWiVLicense::LicenseGetRandom(short sIndex, short sLower, short sUpper)
{
	LPFNFUNCR		lpfnR;
	LPFNFUNCCR		lpfnCR;
	LPFNFUNCCSR		lpfnCSR;
	BOOL			bResult = FALSE;
    UCHAR			uRand;
    const int		nLowerLimit = sLower;
    const int		nUpperLimit = sUpper;
	double			dRandNum;
	static DWORD	dwRnd = 0;

	TraceEnter(_D("CWiVLicense::LicenseGetRandom"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseGetRandom: Requested Index = %d"), sIndex);

	lpfnR = (LPFNFUNCR)GetFunctionAddress(FNR);
	if (lpfnR == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseGetRandom: Could not get address of Random function"));
		TraceLeave(_D("CWiVLicense::LicenseGetRandom"), dwRnd, tlInternal);
		return dwRnd;
	}

	lpfnCSR = (LPFNFUNCCSR)GetFunctionAddress(FNCSR);
	if (lpfnCSR == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseGetRandom: Could not get address of srand function"));
		TraceLeave(_D("CWiVLicense::LicenseGetRandom"), dwRnd, tlInternal);
		return dwRnd;
	}

	lpfnCR = (LPFNFUNCCR)GetFunctionAddress(FNCR);
	if (lpfnCR == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseGetRandom: Could not get address of rand function"));
		TraceLeave(_D("CWiVLicense::LicenseGetRandom"), dwRnd, tlInternal);
		return dwRnd;
	}

	if (sIndex < 0)
	{
		do
		{
			if (dwRnd == 0)
			{
				dwRnd = lpfnR();
				lpfnCSR(dwRnd);
				TraceInternal(_D("CWiVLicense::LicenseGetRandom: Seed = 0x%08X"), dwRnd);
			}

			dRandNum = nUpperLimit * (lpfnCR() / (RAND_MAX + 1.0));

			for (int i = 0; i < (int)(dwRnd & 0x0000000F); i++)
			{
				dRandNum = nUpperLimit * (lpfnCR() / (RAND_MAX + 1.0));
			}

			uRand = (int) dRandNum;

		} while(uRand < sLower);

		dwRnd = uRand;
	}
	else
	{
		uRand = (UCHAR)sIndex;
	}

	if (sIndex == -2)
	{
		if (uRand > 15)
		{
			dwRnd = uRand;
		}
		else
		{
			dwRnd = *(LPDWORD)&SSSGlobals->bRandData[uRand * sizeof(DWORD)];
		}
	}

	TraceInternal(_D("CWiVLicense::LicenseGetRandom: Requested Index = %d, Returned Index = %d, Random = 0x%08X"), sIndex, uRand, dwRnd);

	TraceLeave(_D("CWiVLicense::LicenseGetRandom"), dwRnd, tlInternal);
    return dwRnd;
}

// Get Julian day for today
int CWiVLicense::LicenseGetToday()
{
	LPFNFUNCGLT	lpfnGLT;
	int			nJToday = 0;
	SYSTEMTIME	stLocalTime;

	TraceEnter(_D("CWiVLicense::LicenseGetToday"), tlInternal);

	lpfnGLT = (LPFNFUNCGLT)GetFunctionAddress(FNGLT);
	if (lpfnGLT == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseGetToday: Could not get address of GetLocalTime function"));
		TraceLeave(_D("CWiVLicense::LicenseGetToday"), nJToday, tlInternal);
		return nJToday;
	}

	lpfnGLT(&stLocalTime);

	nJToday = GetJulianDay(stLocalTime.wDay, stLocalTime.wMonth, stLocalTime.wYear);

	TraceLeave(_D("CWiVLicense::LicenseGetToday"), nJToday, tlInternal);
	return nJToday;
}

LPVOID CWiVLicense::LicenseElapsedUpdate(TODAYLISTITEM *pTodayListItem, LPDWORD lpdwParam)
{
	
	LPVOID		lpRetVal = INVALID_HANDLE_VALUE;
	DWORD		dwCyp = pTodayListItem->cyp;
	DWORD		dwParam = *lpdwParam;
	LPFNFUNCGTC	lpfnGTC;
	DWORD		dwTimeCount;
	DWORD		dwTickCount;
	DWORD		dwElapsedTime;
	DWORD		dwElapsedTicks;
	int			nRemainingTime;
	int			nAllowedTime;
	static WIVLIC wivLicense;

	TraceEnter(_D("CWiVLicense::LicenseElapsedUpdate"), tlInternal);

	pTodayListItem->cyp = 0;
	*lpdwParam = 0;
	
	if (m_pLicenseData == NULL)
	{
		m_pLicenseData = new CWivLicenseData;
		if (m_pLicenseData == NULL)
		{
			TraceError(_D("CWiVLicense::LicenseElapsedUpdate: Could not allocate storage for m_pLicenseData"));
			TraceLeave(_D("CWiVLicense::LicenseElapsedUpdate"), (DWORD)lpRetVal, tlInternal);
			return lpRetVal;
		}
	}

	lpfnGTC = (LPFNFUNCGTC)GetFunctionAddress(FNGTC);
	if (lpfnGTC == NULL)
	{
		TraceError(_D("CWiVLicense::LicenseElapsedUpdate: Could not get address of GetTickCount function"));
		TraceLeave(_D("CWiVLicense::LicenseElapsedUpdate"), (DWORD)lpRetVal, tlInternal);
		return lpRetVal;
	}

	if (!m_pLicenseData->blLicenseRead)
	{
		TraceWarning(_D("CWiVLicense::LicenseElapsedUpdate: License not read yet"));
		TraceLeave(_D("CWiVLicense::LicenseElapsedUpdate"), (DWORD)lpRetVal,  tlInternal);
		return lpRetVal;
	}
/*
	//TODO: Change method of checking for full license
	if ((LicenseGetType() != WIV_LICTYPE_TRIAL) && (LicenseGetType() != WIV_LICTYPE_BETA))
	{
		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: Not time limited, so no check necessary"));
		TraceLeave(_D("CWiVLicense::LicenseElapsedUpdate"), (DWORD)m_pLicenseData,  tlInternal);

		if (dwCyp != dwParam)
		{
			dwCyp = dwParam;
			*lpdwParam = 1;
		}

		pTodayListItem->cyp = dwCyp;
		return m_pLicenseData;
	}
*/
	if (m_pLicenseData->dwLastTickCount == 0)
	{
		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: m_dwLastTickCount is zero"));
		memcpy(&wivLicense, LicenseGetValue(), sizeof(WIVLIC));

		m_pLicenseData->dwLastTickCount = lpfnGTC();

		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: Calling LicenseSourcesCheck (NULL)"));
		m_pLicenseData->dwUsedTime = LicenseSourcesCheck(WIV_CHK_DISK, NULL);
		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: m_dwUsedTime = %d"), m_pLicenseData->dwUsedTime);
	}

	dwTickCount = lpfnGTC();

	DWORD dwSuspendTicks = GetSuspendTicks();
	dwElapsedTicks = (dwTickCount + dwSuspendTicks) - m_pLicenseData->dwLastTickCount;

	if (dwSuspendTicks != 0)
	{
		TraceInternal(WIV_EMPTY_STRING);
		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: dwSuspendTicks    = %08d"), dwSuspendTicks);
	}

	TraceInternal(WIV_EMPTY_STRING);
	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: dwTickCount       = %08d"), dwTickCount);
	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: m_dwLastTickCount = %08d"), m_pLicenseData->dwLastTickCount);
	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate:                     --------"));
	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: Elapsed ticks     = %08d"), dwElapsedTicks);

	dwTimeCount = dwTickCount/WIV_TIME_UNIT;
	TraceInternal(WIV_EMPTY_STRING);
	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: dwTimeCount    = %d"), dwTimeCount);

	dwElapsedTime = dwElapsedTicks/WIV_TIME_UNIT;
	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: Elapsed time   = %d"), dwElapsedTime);

	if (dwElapsedTime > 0)
	{
		m_pLicenseData->dwUsedTime += dwElapsedTime;
		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: Used Time      = %d"), m_pLicenseData->dwUsedTime);

		nAllowedTime = LicenseGetTimeAllowed(LicenseGetType());
		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: Allowed Time   = %d"), nAllowedTime);

		nRemainingTime = nAllowedTime - m_pLicenseData->dwUsedTime;
		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: Remaining Time = %d"), nRemainingTime);

		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: Calling LicenseSourcesCheck (<%s>, %d)"), BtoS((LPBYTE)&wivLicense, sizeof(WIVLIC)), m_pLicenseData->dwUsedTime);
		LicenseSourcesCheck(WIV_CHK_DISK, &wivLicense, m_pLicenseData->dwUsedTime);

		m_pLicenseData->dwLastTickCount = dwTickCount;
		TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: m_dwLastTickCount reset to %d"), m_pLicenseData->dwLastTickCount);
	}

	pTodayListItem->prgbCachedData = (LPBYTE)m_pLicenseData;
	pTodayListItem->cbCachedData = m_pLicenseData->dwSize;

	if (dwCyp != dwParam)
	{
		dwCyp = dwParam;
		*lpdwParam = 1;
	}

	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate, setting pTodayListItem->cyp = %d"), dwCyp);
	pTodayListItem->cyp = dwCyp;

//	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: pTodayListItem->grfFlags       = 0x%08X"), pTodayListItem->grfFlags);
//	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: pTodayListItem->hinstDLL       = 0x%08X"), pTodayListItem->hinstDLL);
//	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: pTodayListItem->hwndCustom     = 0x%08X"), pTodayListItem->hwndCustom);
//	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: pTodayListItem->prgbCachedData = 0x%08X"), pTodayListItem->prgbCachedData);
//	TraceInternal(_D("CWiVLicense::LicenseElapsedUpdate: pTodayListItem->cbCachedData   = %d"), pTodayListItem->cbCachedData);
/*
	for (int i = 0; i < m_nClientCount; i++)
	{
		LPFNNOTIFY lpfn = m_fnClients[i];
		lpfn(m_pLicenseData,2);
	}
*/	
	TraceLeave(_D("CWiVLicense::LicenseElapsedUpdate"), (DWORD)m_pLicenseData, tlInternal);
	return m_pLicenseData;
}

//////////////////////////////////////////////////////////////////////////
// Sort Out License, Expiry, etc.
// LicenseSourcesCheck
//////////////////////////////////////////////////////////////////////////

DWORD CWiVLicense::LicenseSourcesCheck(const int nDisk, LPWIVLIC lpLicense, const int nUsedTime)
{
	int				nResult;
	bool			blFirstInstall = false;
	bool			blAfterHardReset = false;
	DWORD			dwUsedTime = 0;
	WIVLIC			wivStoredLicenseCode;

	// Indicators of missing/incorrect license information
	static UCHAR	uchLastIndicators = WIV_CHK_MASK_FLAGS_ALL;
	UCHAR			uchIndicators;

	// Final values that will be written back to all sources
	UCHAR			uchFinalType;											// Type
	USHORT			usFinalElapsed;											// Elapsed time
	USHORT			usFinalInstall;											// Install day
	USHORT			usFinalRemaining;										// Remaining time
	USHORT			usFinalPeriod;											// Max days
	WIVLIC			wivFinalLicense;										// License code
	DWORD			dwFinalSession;											// Session

	// Today flags
	DWORD			dwTodayFlags;
	UCHAR			uchFlagsType;
	USHORT			usFlagsElapsed;
	USHORT			usFlagsInstall;

	// Disk data
	LPDWORD			lpdwDiskData = NULL;
	DWORD			dwDiskData;
	UCHAR			uchDiskType;
	USHORT			usDiskRemaining;
	USHORT			usDiskPeriod;

	// Registry information
	TCHAR			szRegA[WIV_CHK_REG_VALUE_SIZE + 1] = WIV_EMPTY_STRING;
	TCHAR			szRegB[WIV_CHK_REG_VALUE_SIZE + 1] = WIV_EMPTY_STRING;
	UCHAR			uchRegType;
	USHORT			usRegInstall;
	USHORT			usRegRemaining;
	WIVLIC			wivRegLicense;
	DWORD			dwRegSession;

	// File information
	byte			bFile[WIV_CHK_FILE_SIZE] = WIV_DEFAULT_CHK_FILE_BUFFER;
	UCHAR			uchFileType;
	USHORT			usFileElapsed;
	USHORT			usFileInstall;
	USHORT			usFileRemaining;
	USHORT			usFilePeriod;
	TCHAR			szFileRegA[WIV_CHK_REG_VALUE_SIZE + 1] = WIV_EMPTY_STRING;
	TCHAR			szFileRegB[WIV_CHK_REG_VALUE_SIZE + 1] = WIV_EMPTY_STRING;
	WIVLIC			wivFileLicense;
	DWORD			dwFileSession;

	TraceEnter(_D("CWiVLicense::LicenseSourcesCheck"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseSourcesCheck: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseSourcesCheck"), (DWORD)0, tlInternal);
		return 0;
	}

	if (!m_pLicenseData->blLicenseRead)
	{
		TraceWarning(_D("CWiVLicense::LicenseSourcesCheck: License not read yet"));
		TraceLeave(_D("CWiVLicense::LicenseSourcesCheck"), (DWORD)0, tlInternal);
		return 0;
	}

	if (lpLicense == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: lpLicense is NULL"));
	}
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: lpLicense = <%s>"), BtoS((LPBYTE)lpLicense, sizeof(WIVLIC)));
	}

	memset(&wivStoredLicenseCode, 0, sizeof(WIVLIC));
	memset(&wivFinalLicense, 0, sizeof(WIVLIC));
	memset(&wivRegLicense, 0, sizeof(WIVLIC));
	memset(&wivFileLicense, 0, sizeof(WIVLIC));

//==============================================
// First attempt to obtain any existing license
// information from all the sources
//==============================================

	uchIndicators = LicenseSourcesLoad(nDisk, &dwTodayFlags, &dwDiskData, szRegA, szRegB, bFile);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Back from LicenseSourcesLoad, uchIndicators = %02X, dwTodayFlags = %08X, ")
  				  _D("usDiskData = %08X, szRegA = <%s>, szRegB = <%s>, bFile = <%s>"),
						uchIndicators, dwTodayFlags, dwDiskData, szRegA, szRegB, BtoS(bFile, sizeof(bFile)));


	// If no sources present, we have to assume new install, else if only the
	// disk source and no other sources were present, then there has been a hard reset
	if ((uchIndicators & WIV_CHK_MASK_FLAGS_ALL) == WIV_CHK_MASK_FLAGS_ALL)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Potential first install"));
		blFirstInstall = true;

	}else if ((uchIndicators & WIV_CHK_MASK_HARD_RESET) == WIV_CHK_MASK_HARD_RESET)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: After hard reset"));
		blAfterHardReset = true;
	}

	// Decode the sources data
	nResult = LicenseSourcesDecode(uchIndicators, dwTodayFlags, dwDiskData, szRegA, szRegB, bFile,
									&uchFlagsType, &usFlagsElapsed, &usFlagsInstall, 
									&uchDiskType, &usDiskRemaining, &usDiskPeriod, 
									&wivRegLicense, &uchRegType, &usRegInstall,
									&usRegRemaining, &dwRegSession,
									szFileRegA, szFileRegB, &wivFileLicense,
									&uchFileType, &usFileElapsed, &usFileInstall,
									&usFileRemaining, &usFilePeriod, &dwFileSession);

	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: LicenseSourcesDecode nResult  = %d"), nResult);

	TraceInternal(WIV_EMPTY_STRING);
	TraceInternal(_D("Result = %d"), nResult);
	TraceInternal(_D("uchFlagsType = %d, usFlagsElapsed = 0x%04X, usFlagsInstall = 0x%04X"), uchFlagsType, usFlagsElapsed, usFlagsInstall);
	TraceInternal(_D("uchDiskType = %d, usDiskRemaining = 0x%04X, usDiskPeriod = 0x%04X"), uchDiskType, usDiskRemaining, usDiskPeriod);
	TraceInternal(_D("uchRegType = %d, usRegRemaining = 0x%04X, usRegInstall = 0x%04X, wivRegLicense = <%s>, dwRegSession= 0x%08X"), uchRegType, usRegRemaining, usRegInstall, BtoS((LPBYTE)&wivRegLicense, sizeof(WIVLIC)), dwRegSession);
	TraceInternal(_D("uchFileType = %d, usFileElapsed = 0x%04X, usFileRemaining = 0x%04X, usFileInstall = 0x%04X, wivFileLicense = <%s>, dwFileSession = 0x%08X, szFileRegA = <%s>, szFileRegB = <%s>"), uchFileType, usFileElapsed, usFileRemaining, usFileInstall, BtoS((LPBYTE)&wivFileLicense, sizeof(WIVLIC)), dwFileSession, szFileRegA, szFileRegB);
	TraceInternal(WIV_EMPTY_STRING);


//==============================================
// Determine whether all retrieved data matches
// and if not, decide which to use to set up
// the data that will be written back to all of
// the sources 
//==============================================

//=====================================
// Check decoded results
//=====================================

// Type
// uchStoredType, uchFlagsType, uchDiskType, uchRegType, uchFileType: uchFinalType
//

	if (blFirstInstall)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: First install, so use trial licence"));
		uchFinalType = WIV_LICTYPE_TRIAL;

	} else if (blAfterHardReset)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: After hard reset, so calculate licence type"));
		uchFinalType = LicenseCalcType(usDiskPeriod);

	} else if ((lpLicense == NULL) || (*(__int64*)(lpLicense) == 0))
	{
		// If passed license is empty, get stored license
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Passed License code is empty"));

		if (*(__int64*)(LicenseGetValue()) == 0)
		{
			// If no license stored, use trial license
			TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: No stored License code, so use trial licence"));
			uchFinalType = WIV_LICTYPE_TRIAL;
		}
		else
		{
			// Calculate type for stored license
			TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Calculate license type for stored license <%s>"), BtoS((LPBYTE)LicenseGetValue(), sizeof(WIVLIC)));
			uchFinalType = LicenseCalcType(LicenseGetValue());
		}

	} else
	{
		// Calculate type for passed license
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Calculate license type for passed license <%s>"), BtoS((LPBYTE)lpLicense, sizeof(WIVLIC)));
		uchFinalType = LicenseCalcType(lpLicense);
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Type = %d"), (char)uchFinalType);
	}

// License
// szStoredLicenseCode, szRegLicense, szFileLicense: szFinalLicense
//

	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: m_pLicenseData->blPending = %d"), m_pLicenseData->blPending);
	if (m_pLicenseData->blPending)
	{
		memcpy(&wivStoredLicenseCode, LicenseGetValue(), sizeof(WIVLIC));
	}
	else
	{

		// If invalid license
		if ((char)uchFinalType < WIV_LICTYPE_FULL)
		{
			if (lpLicense != NULL)
			{
				memcpy(&wivStoredLicenseCode, lpLicense, sizeof(WIVLIC));
			}
			else
			{
				memcpy(&wivStoredLicenseCode, LicenseGetValue(), sizeof(WIVLIC));
			}
		}
		else
		{
			// Calculate license code from type
			TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Calculating license code for type = %d"), (char)uchFinalType);
			memcpy(&wivStoredLicenseCode, LicenseCalcCode(uchFinalType), sizeof(WIVLIC));
		}
	}

	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Setting License value to <%s>"), BtoS((LPBYTE)&wivStoredLicenseCode, sizeof(WIVLIC)));
	LicenseSetValue(&wivStoredLicenseCode);

	memcpy(&wivFinalLicense, &wivStoredLicenseCode, sizeof(WIVLIC));

//TODO: change how and where this is done
	bool blLimited = !(uchFinalType == 0);

	TraceInternal(WIV_EMPTY_STRING);

	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: wivStoredLicenseCode = <%s>"), BtoS((LPBYTE)&wivStoredLicenseCode, sizeof(WIVLIC)));

// Registry
// szRegA, szRegB, szFileRegA, szFileRegB
//
// Elapsed
// usFlagsElapsed, usFileElapsed: usFinalElapsed
//

	if (nUsedTime == 0)
	{
		usFinalElapsed = usFileElapsed;
		usFinalElapsed = MAX((short)usFinalElapsed, (short)usFlagsElapsed);
	}
	else
	{
		usFinalElapsed = nUsedTime;
	}

// Remaining
// usDiskRemaining, usRegRemaining, usFileRemaining: usFinalRemaining
//

	if (nUsedTime == 0)
	{
		// Use disk as master as it survives a hard reset
		usFinalRemaining = usDiskRemaining;
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: usFinalRemaining set to %d"), (short)usDiskRemaining);

		// If no other sources were available, then there has been a hard reset
		if (uchIndicators == WIV_CHK_MASK_HARD_RESET)
		{
			TraceInternal(WIV_EMPTY_STRING);
			TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Recovering from hard reset, usDiskRemaining = %d"), (short)usDiskRemaining);
			
			// Adjust elapsed time
			usFinalElapsed = LicenseCalcElapsedTime(uchFinalType, usDiskRemaining);
		}
		else
		{
			usFinalRemaining = LicenseCalcRemainingTime(uchFinalType, usFinalElapsed);
			TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: LicenseCalcRemainingTime (usFinalElapsed = %d) returned %d"), (short)usFinalElapsed, (short)usFinalRemaining);
			if (uchIndicators != WIV_CHK_MASK_FLAGS_NONE)
			{
				usFinalRemaining = min((short)usFinalRemaining, (short)usFileRemaining);
				usFinalRemaining = min((short)usFinalRemaining, (short)usRegRemaining);
				TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: usFinalRemaining set to min = %d"), (short)usFinalRemaining);
			}	
		}
	}
	else
	{
		usFinalRemaining = LicenseCalcRemainingTime(uchFinalType, nUsedTime);
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: LicenseCalcRemainingTime returned %d"), (short)usFinalRemaining);
	}

// Install
// usFlagsInstall, usRegInstall, usFileInstall: usFinalInstall
//

	usFinalInstall = LicenseCalcInstallDay(LicenseCalcExpiryDay(usFinalRemaining), uchFinalType);

// Max days
// usDiskPeriod, usFilePeriod: usFinalPeriod
//

	usFinalPeriod = LicenseGetPeriod(uchFinalType);

// Session
// dwRegSession, dwFileSession: dwFinalSession
//

//==============================================
// For all the sources, set up the data,
// generate the values and save them
//==============================================

	// Get session number
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Calling LicenseGetRandom()"));
	dwFinalSession = LicenseGetRandom();
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Back from LicenseGetRandom()"));

//TODO: change how and where this is done
	if (!blLimited)
	{
		usFinalPeriod = 31;
		usFinalRemaining = 9417;
		usFinalInstall = 0x0C98;
		usFinalElapsed = 0127;
	}
	
//usFinalRemaining = 1;
//usFinalInstall = 0x0C98;
//usFinalElapsed = 43199;

	nResult = LicenseSourcesEncode(&dwTodayFlags, &dwDiskData, szRegA, szRegB, bFile, &wivFinalLicense,
											uchFinalType, usFinalElapsed, usFinalInstall,
											usFinalRemaining, usFinalPeriod, dwFinalSession);

	nResult = LicenseSourcesSave(nDisk, dwTodayFlags, dwDiskData, szRegA, szRegB, bFile, blLimited);

	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: uchIndicators = %04X, nLastIndicators = %08X"), uchIndicators, uchLastIndicators);

	if (uchIndicators > uchLastIndicators)
	{
		m_pLicenseData->uchStrange = uchLastIndicators;
		dwUsedTime = usFinalElapsed;
	}
	else
	{
		m_pLicenseData->uchStrange = 0;
		dwUsedTime = usFinalElapsed;
	}

	uchLastIndicators = uchIndicators;

	TraceInternal(WIV_EMPTY_STRING);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: uchFinalType     = 0x%02X (%d)"), uchFinalType, (char)uchFinalType);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: blLimited        = %d"), blLimited);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: usFinalElapsed   = 0x%04X (%d)"), usFinalElapsed, (short)usFinalElapsed);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: usFinalRemaining = 0x%04X (%d)"), usFinalRemaining, (short)usFinalRemaining);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: usFinalInstall   = 0x%04X (%d)"), usFinalInstall, (short)usFinalInstall);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: usFinalPeriod    = 0x%04X (%d)"), usFinalPeriod, (short)usFinalPeriod);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: dwFinalSession   = 0x%08X"), dwFinalSession);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: bFinalLicense    = <%s>"), BtoS((LPBYTE)&wivFinalLicense, sizeof(WIVLIC)));
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: nUsedTime        = %d"), nUsedTime);
	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: dwUsedTime       = %d"), dwUsedTime);
	
	LicenseSetValue(&wivFinalLicense);
	LicenseCodeWrite();

	TraceAlways(_D("CWiVLicense::LicenseSourcesCheck: Remaining = %d, Elapsed = %d"), (short)usFinalRemaining, (short)usFinalElapsed);
	TraceAlways(_D("CWiVLicense::LicenseSourcesCheck: m_pLicenseData->nAmberLimit = %d"),(short)m_pLicenseData->nAmberLimit);
	TraceAlways(_D("CWiVLicense::LicenseSourcesCheck: m_pLicenseData->nRedLimit   = %d"), (short)m_pLicenseData->nRedLimit);


	//	WIV_BETA_EXPIRY_PERIOD
	//	WIV_TIME_ALLOWED_FOR_TRIAL	
	
	USHORT usTest = usFinalRemaining;

	if ((usFinalPeriod % 10) == 0)
	{
		if (usFinalElapsed < (USHORT)WIV_TIME_ALLOWED_FOR_TRIAL)
		{
			usTest = usFinalRemaining - (USHORT)WIV_TIME_ALLOWED_FOR_TRIAL;
		}
	}	

	TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: usTest = %d (0x%04X)"), (short)usTest, usTest);

	// Notify each registered client of the state of the license
	for (int i = 0; i < ARRAYSIZE(m_fnClients); i++)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Client %d, m_fnClients[%d] = 0x%08X"), i + 1, i, m_fnClients[i]);
		if (m_fnClients[i] != NULL)
		{
			TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: usFinalRemaining = %d, nAmberLimit = %d, nRedLimit = %d"), (short)usFinalRemaining, m_pLicenseData->nAmberLimit, m_pLicenseData->nRedLimit);
			LPFNNOTIFY lpfnNotify = m_fnClients[i];
			TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Notifying client %d"), i + 1);
			if ((char)uchFinalType < 0)
			{
				TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Notifying client %d with WHITE (0x%08X)"), i + 1, WIV_LICENSE_CONDITION_WHITE);
				lpfnNotify(NULL, WIV_LICENSE_CONDITION_WHITE);

			} else if ((short)usTest < (short)m_pLicenseData->nAmberLimit)
			{
				if ((short)usTest < (short)m_pLicenseData->nRedLimit)
				{
					TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Notifying client %d with RED (0x%08X)"), i + 1, WIV_LICENSE_CONDITION_RED);
					lpfnNotify(NULL, WIV_LICENSE_CONDITION_RED);
				}
				else
				{
					TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Notifying client %d with AMBER (0x%08X)"), i + 1, WIV_LICENSE_CONDITION_AMBER);
					lpfnNotify(NULL, WIV_LICENSE_CONDITION_AMBER);
				}
			}
			else
			{
				TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Notifying client %d with GREEN (0x%08X)"), i + 1, WIV_LICENSE_CONDITION_GREEN);
				lpfnNotify(NULL, WIV_LICENSE_CONDITION_GREEN);
				TraceInternal(_D("CWiVLicense::LicenseSourcesCheck: Notifying client %d with remaining (%d)"), i + 1, (short)usFinalRemaining);
				lpfnNotify(m_pLicenseData, usFinalRemaining);
			}
		}
	}
	
	TraceLeave(_D("CWiVLicense::LicenseSourcesCheck"), dwUsedTime, tlInternal);

	return dwUsedTime;

}

UCHAR CWiVLicense::LicenseSourcesLoad(const int nDisk, LPDWORD lpdwTodayFlags, LPDWORD lpdwDiskData, LPTSTR lpszRegA, LPTSTR lpszRegB, LPBYTE bFile)
{
	UCHAR	uchIndicators;
	int		nResult = 0;

	TraceEnter(_D("CWiVLicense::LicenseSourcesLoad"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseSourcesLoad: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseSourcesLoad"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	// Reset all the 'not present' bits
	uchIndicators = WIV_CHK_MASK_FLAGS_NONE;

	// Get flags from Today Item registry entry
	*lpdwTodayFlags = LicenseFlagsRead();

	// If flags = 0xFFFFFFFF, then they indicate a first time run
	if (*lpdwTodayFlags == WIV_CHK_FLAGS_NOT_INIT)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesLoad: Today flags not initialized"));
		// Set the bit that says flags not initialized
		uchIndicators |= WIV_CHK_MASK_FLAGS_NOT_INIT;
	}

	// Check to see if storage disk has our info on it
	*lpdwDiskData = LicenseDiskRead(nDisk);

	TraceInternal(_D("CWiVLicense::LicenseSourcesLoad: *lpdwDiskData = %d, WIV_CHK_DSK_NOT_WRITTEN = %d, WIV_CHK_DSK_MS_DEFAULT = %d"),*lpdwDiskData, WIV_CHK_DSK_NOT_WRITTEN, WIV_CHK_DSK_MS_DEFAULT);
	// If no info on disk, then it indicates a first time run
	if (*lpdwDiskData >= WIV_CHK_DSK_NOT_WRITTEN)
	{
//		if (*lpdwDiskData != WIV_CHK_DSK_MS_DEFAULT)
//		{
			TraceInternal(_D("CWiVLicense::LicenseSourcesLoad: Disk data not written"));
			// Set the bit that says disk not written
			uchIndicators |= WIV_CHK_MASK_DSK_NOT_WRITTEN;
//		}
	}

	// Check to see if registry has our special entry in it
	nResult = LicenseRegRead(lpszRegA, lpszRegB);

	// If no registry entry, then it indicates a first time run
	if (nResult == WIV_CHK_REG_NOT_PRESENT)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesLoad: Registry entry not present"));
		// Set the bit that says reg not present
		uchIndicators |= WIV_CHK_MASK_REG_NOT_PRESENT;
	}

	// Check to see if our special file exists,
	// If so read its contents
	nResult = LicenseFileOpen();
	if (nResult == 0)
	{
		nResult = LicenseFileRead(m_pLicenseData->hfFile, bFile);
		TraceInternal(_D("CWiVLicense::LicenseSourcesLoad: After LicenseFileRead, nResult = %d, bFile = <%s>"), nResult, BtoS(bFile, sizeof(bFile)));
		LicenseFileClose(m_pLicenseData->hfFile);
	}

	// If no file, then it indicates a first time run
	if (nResult == WIV_CHK_FILE_NOT_PRESENT)
	{
		TraceInternal(_D("CWiVLicense::LicenseSourcesLoad: File not present"));
		// Set the bit that says file not present
		uchIndicators |= WIV_CHK_MASK_FILE_NOT_PRESENT;
	}

	TraceLeave(_D("CWiVLicense::LicenseSourcesLoad"), (DWORD)uchIndicators, tlInternal);
	return uchIndicators;
}

int	CWiVLicense::LicenseSourcesDecode(const UCHAR uchIndicators, const DWORD dwTodayFlags, const DWORD dwDiskData,
									  LPCTSTR lpszRegA, LPCTSTR lpszRegB, LPBYTE lpbFile,
									  PUCHAR lpuchFlagsType, PUSHORT lpusFlagsElapsed, PUSHORT lpusFlagsInstall,
									  PUCHAR lpuchDiskType, PUSHORT lpusDiskRemaining, PUSHORT lpusDiskPeriod,
									  LPWIVLIC lpRegLicense, PUCHAR lpuchRegType, PUSHORT lpusRegInstall,
									  PUSHORT lpusRegRemaining, LPDWORD lpdwRegSession,
									  LPTSTR lpszFileRegA, LPTSTR lpszFileRegB, LPWIVLIC lpFileLicense,
									  PUCHAR lpuchFileType, PUSHORT lpusFileElapsed, PUSHORT lpusFileInstall,
									  PUSHORT lpusFileRemaining, PUSHORT lpusFilePeriod, LPDWORD lpdwFileSession)
{

	BYTE	bLicense[sizeof(WIVLIC)];
	TCHAR	szReg[WIV_CHK_REG_VALUE_SIZE + 1];
	DWORD	dwSession;
	UCHAR	uchType;
	USHORT	usElapsed;
	USHORT	usInstall;
	USHORT	usRemaining;
	USHORT	usPeriod;
	int		nResult		= -4;

	TraceEnter(_D("CWiVLicense::LicenseSourcesDecode"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseSourcesDecode: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseSourcesDecode"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	// Set up default data for when a source doesn't exist
	memset(bLicense, 0, sizeof(bLicense));
	memcpy(bLicense, LicenseGetTrial(), sizeof(WIVLIC));

	_tcsncpy(szReg, g_szEmptyReg, WIV_CHK_REG_VALUE_SIZE);
	TraceInternal(_D("CWiVLicense::LicenseSourcesDecode: szReg = <%s>"), szReg);

	uchType		= WIV_LICTYPE_TRIAL;
	usElapsed	= 0;
	usInstall	= LicenseGetToday();
	usRemaining	= LicenseGetTimeAllowed(uchType);
	usPeriod	= LicenseGetPeriod(uchType);
	dwSession	= LicenseGetRandom();

	// Check Today Item source
	if (uchIndicators & WIV_CHK_MASK_FLAGS_NOT_INIT)
	{
		*lpuchFlagsType = uchType;
		*lpusFlagsElapsed = usElapsed;
		*lpusFlagsInstall = usInstall;
	}
	else
	{
		LicenseFlagsDecode(dwTodayFlags, lpuchFlagsType, lpusFlagsElapsed, lpusFlagsInstall);
		TraceInternal(_D("CWiVLicense::LicenseSourcesDecode::LicenseFlagsDecode: *lpuchFlagsType = %d, ")
					_D("*lpusFlagsElapsed = 0x%04X, *lpusFlagsInstall = 0x%04X"),
						*lpuchFlagsType, *lpusFlagsElapsed, *lpusFlagsInstall);

		nResult += 1;
	}

	// Check disk source
	if (uchIndicators & WIV_CHK_MASK_DSK_NOT_WRITTEN)
	{
		*lpuchDiskType = uchType;
		*lpusDiskRemaining = usRemaining;
		*lpusDiskPeriod = usPeriod;
	}
	else
	{
		LicenseDiskDecode(dwDiskData, lpuchDiskType, lpusDiskRemaining, lpusDiskPeriod);
		TraceInternal(_D("CWiVLicense::LicenseSourcesDecode::LicenseDiskDecode: *lpuchDiskType = %d, *lpusDiskRemaining = 0x%04X, *lpusDiskPeriod = 0x%04X"),
							*lpuchDiskType, *lpusDiskRemaining, *lpusDiskPeriod);

		nResult += 1;
	}

	// Check registry source
	if (uchIndicators & WIV_CHK_MASK_REG_NOT_PRESENT)
	{
		memcpy(lpRegLicense, bLicense, sizeof(WIVLIC));
		*lpuchRegType = uchType;
		*lpusRegInstall = usInstall;
		*lpusRegRemaining = usRemaining;
		*lpdwRegSession = dwSession;
	}
	else
	{
		LicenseRegDecode(lpszRegA, lpszRegB, lpRegLicense, lpuchRegType,
									lpusRegInstall, lpusRegRemaining, lpdwRegSession);
		TraceInternal(_D("CWiVLicense::LicenseSourcesDecode::LicenseRegDecode: lpbRegLicense = <%s>, *lpuchRegType = %d, ")
					_D("*lpusRegInstall = 0x%04X, *lpusRegRemaining = 0x%04X, *lpdwRegSession = 0x%08X"),
						BtoS((LPBYTE)lpRegLicense, sizeof(WIVLIC)), *lpuchRegType, *lpusRegInstall, *lpusRegRemaining, *lpdwRegSession);

		nResult += 1;
	}

	// Check file source
	if (uchIndicators & WIV_CHK_MASK_FILE_NOT_PRESENT)
	{
		memcpy(lpFileLicense, bLicense, sizeof(WIVLIC));
		_tcsncpy(lpszFileRegA, szReg, WIV_CHK_REG_VALUE_SIZE);
		_tcsncpy(lpszFileRegB, szReg, WIV_CHK_REG_VALUE_SIZE);
		*lpuchFileType = uchType;
		*lpusFileElapsed = usElapsed;
		*lpusFileInstall = usInstall;
		*lpusFileRemaining = usRemaining;
		*lpusFilePeriod = usPeriod;
		*lpdwFileSession = dwSession;
	}
	else
	{
		LicenseFileDecode(lpbFile, lpszFileRegA, lpszFileRegB, lpFileLicense,
									lpuchFileType, lpusFileElapsed, lpusFileInstall,
									lpusFileRemaining, lpusFilePeriod, lpdwFileSession);
		TraceInternal(_D("CWiVLicense::LicenseSourcesDecode::LicenseFileDecode: lpszFileRegA = <%s>, lpszFileRegB = <%s>, ")
					_D("lpbFileLicense = <%s>, *lpuchFileType = %d, *lpusFileElapsed = 0x%04X, ")
					_D("*lpusFileInstall = 0x%04X, *lpusFileRemaining = 0x%04X, *lpusFilePeriod = 0x%04X, *lpdwFileSession = 0x%08X"),
						lpszFileRegA, lpszFileRegB, BtoS((LPBYTE)lpFileLicense, sizeof(WIVLIC)), *lpuchFileType, *lpusFileElapsed,
						*lpusFileInstall, *lpusFileRemaining, *lpusFilePeriod, *lpdwFileSession);

		nResult += 1;
	}

	TraceLeave(_D("CWiVLicense::LicenseSourcesDecode"), (DWORD)nResult, tlInternal);
	return nResult;
}

int	CWiVLicense::LicenseSourcesEncode(LPDWORD lpdwTodayFlags, LPDWORD lpdwDiskData,
									  LPTSTR lpszRegA, LPTSTR lpszRegB, LPBYTE bFile, LPWIVLIC lpLicense,
									  const UCHAR uchType, const USHORT usElapsed,
									  const USHORT usInstall, const USHORT usRemaining,
									  const USHORT usPeriod, const DWORD dwSession)
{
	int	nResult = -1;

	TraceEnter(_D("CWiVLicense::LicenseSourcesEncode"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseSourcesEncode: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseSourcesEncode"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	TraceInternal(_D("CWiVLicense::LicenseSourcesEncode: lpbLicense = <%s>, uchType = %d, usRemaining = 0x%04X, usInstall = 0x%04X, dwSession = 0x%08X"),
						BtoS((LPBYTE)lpLicense, sizeof(WIVLIC)), uchType, usRemaining, usInstall, dwSession);

	TraceInternal(_D("CWiVLicense::LicenseSourcesEncode: Encode Flags"));
	nResult = LicenseFlagsEncode(lpdwTodayFlags, uchType, usElapsed, usInstall);

	TraceInternal(_D("CWiVLicense::LicenseSourcesEncode: Encode Disk"));
	nResult = LicenseDiskEncode(lpdwDiskData, uchType, usRemaining, usPeriod);

	TraceInternal(_D("CWiVLicense::LicenseSourcesEncode: Encode Registry"));
	nResult = LicenseRegEncode(lpszRegA, lpszRegB, lpLicense,
								uchType, usInstall, usRemaining, dwSession);

	TraceInternal(_D("CWiVLicense::LicenseSourcesEncode: Encode File"));
	nResult = LicenseFileEncode(lpszRegA, lpszRegB, bFile,
								uchType, usElapsed, usInstall, usRemaining, usPeriod, dwSession);

	TraceLeave(_D("CWiVLicense::LicenseSourcesEncode"), (DWORD)nResult, tlInternal);
	return nResult;
}

int	CWiVLicense::LicenseSourcesSave(const int nDisk, const DWORD dwTodayFlags, const DWORD dwDiskData, LPCTSTR lpszRegA, LPCTSTR lpszRegB, LPBYTE bFile, const bool blLimited)
{
	int	nResult = -1;

	TraceEnter(_D("CWiVLicense::LicenseSourcesSave"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseSourcesSave: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseSourcesSave"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	TraceInternal(_D("CWiVLicense::LicenseSourcesSave: dwTodayFlags = 0x%08X, dwDiskData = 0x%08X, ")
				_D("lpszRegA = <%s>, lpszRegB = <%s>, bFile = <%s>"),
						dwTodayFlags, dwDiskData, lpszRegA, lpszRegB, BtoS(bFile, WIV_CHK_FILE_SIZE));

	TraceInternal(_D("CWiVLicense::LicenseSourcesSave: Rewrite Flags"));
	nResult = LicenseFlagsWrite(dwTodayFlags);

	TraceInternal(_D("CWiVLicense::LicenseSourcesSave: Rewrite Disk"));
	nResult = LicenseDiskWrite(nDisk, dwDiskData, blLimited);

	TraceInternal(_D("CWiVLicense::LicenseSourcesSave: Rewrite Registry"));
	nResult = LicenseRegWrite(lpszRegA, lpszRegB);

	TraceInternal(_D("CWiVLicense::LicenseSourcesSave: Rewrite File"));
	nResult = LicenseFileOpen(true);
	nResult = LicenseFileWrite(m_pLicenseData->hfFile, bFile);
	nResult = LicenseFileSetTime();
	nResult = LicenseFileClose(m_pLicenseData->hfFile);

	TraceLeave(_D("CWiVLicense::LicenseSourcesSave"), (DWORD)nResult, tlInternal);
	return nResult;
}

// Get Today item flags
DWORD CWiVLicense::LicenseFlagsRead()
{
	DWORD		nResult;
	HKEY		hKey;
	DWORD		dwFlags;

	TraceEnter(_D("CWiVLicense::LicenseFlagsRead"), tlInternal);

	// Get Today flags from registry
	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Getting Today flags from registry"));

	// Open the Today Item Key
	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Calling RegKeyOpen for Today Item key"));
	nResult = RegKeyOpen(GetTodayItemName(), &hKey, g_szTodayItemsKey);
	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Back from RegKeyOpen, nResult = <%08x>, hKey = <%08x>"), nResult, hKey);

	if (nResult != ERROR_SUCCESS)
	{
		TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Error opening Today Item key, nResult = <%08x>"), nResult);
		TraceLeave(_D("CWiVLicense::LicenseFlagsRead"), nResult, tlInternal);
		return nResult;
	}

	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Calling RegGetValue for Flags"));
	dwFlags = RegGetValue(hKey, g_szTodayItemFlagsKey, (DWORD)0);
	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Back from RegGetValue, Flags = <%08X>"), dwFlags);

	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Flags read from registry"));

	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Flags = <%08X>"), dwFlags);

	// Close the Today Item Key
	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Calling RegKeyClose"));
	nResult = RegKeyClose(hKey);
	TraceInternal(_D("CWiVLicense::LicenseFlagsRead: Back from RegKeyClose, nResult = <%08x>"), nResult);

	TraceLeave(_D("CWiVLicense::LicenseFlagsRead"), dwFlags, tlInternal);
	return dwFlags;
}

int	CWiVLicense::LicenseFlagsDecode(const DWORD dwFlags, PUCHAR lpuchType, PUSHORT lpusElapsed, PUSHORT lpusInstall)
{
	int			nResult = 0;


	union {
		DWORD  dwFlags;
		struct {
			unsigned int Type		: 3;
			unsigned int Elapsed	: 16;
			unsigned int Install	: 13;
		}TBits;
	} TFlags;

	TraceEnter(_D("CWiVLicense::LicenseFlagsDecode"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseFlagsDecode: dwFlags = 0x%08X"), dwFlags);

	memset(&TFlags, 0, sizeof(TFlags));

	TFlags.dwFlags = dwFlags;

	TraceInternal(_D("CWiVLicense::LicenseFlagsDecode: TFlags = <%s>"), BtoS((LPBYTE)&TFlags, sizeof(TFlags)));

	UCHAR	uchType		= TFlags.TBits.Type;
	USHORT	usElapsed	= TFlags.TBits.Elapsed;
	USHORT	usInstall	= TFlags.TBits.Install;

	*lpuchType		= uchType;
	*lpusElapsed	= usElapsed;
	*lpusInstall	= usInstall;

	TraceInternal(WIV_EMPTY_STRING);

	TraceInternal(_D("CWiVLicense::LicenseFlagsDecode: uchType    = 0x%02X"), uchType);
	TraceInternal(_D("CWiVLicense::LicenseFlagsDecode: usElapsed  = 0x%04X"), usElapsed);
	TraceInternal(_D("CWiVLicense::LicenseFlagsDecode: usInstall  = 0x%04X"), usInstall);

	nResult = 0;

	TraceLeave(_D("CWiVLicense::LicenseFlagsDecode"), (DWORD)nResult, tlInternal);
	return nResult;
}

// Generate item flags
int CWiVLicense::LicenseFlagsEncode(LPDWORD lpdwFlags, const UCHAR uchType, const USHORT usElapsed, const USHORT usInstall)
{
	int		nResult = WIV_CHK_FLAGS_NOT_INIT;

	union {
		DWORD  dwFlags;
		struct {
			unsigned int Type		: 3;
			unsigned int Elapsed	: 16;
			unsigned int Install	: 13;
		}TBits;
	} TFlags;

	TraceEnter(_D("CWiVLicense::LicenseFlagsEncode"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseFlagsEncode: lpdwFlags = 0x%08X, usElapsed = 0x%04X, usInstall = 0x%04X, uchType = %d"),
					lpdwFlags, usElapsed, usInstall, uchType);

	memset(&TFlags, 0, sizeof(TFlags));
	TFlags.TBits.Type		= uchType;
	TFlags.TBits.Elapsed	= usElapsed;
	TFlags.TBits.Install	= usInstall;

	TraceInternal(_D("CWiVLicense::LicenseFlagsEncode: TFlags.dwFlags = 0x%08X"), TFlags.dwFlags);
	*lpdwFlags = TFlags.dwFlags;
	TraceInternal(_D("CWiVLicense::LicenseFlagsEncode: *lpdwFlags = 0x%08X"), *lpdwFlags);

	nResult = 0;

	TraceLeave(_D("CWiVLicense::LicenseFlagsEncode"), (DWORD)nResult, tlInternal);
	return nResult;
}

// Set Today item flags
int CWiVLicense::LicenseFlagsWrite(const DWORD dwFlags)
{
	DWORD		nResult;
	HKEY		hKey;

	TraceEnter(_D("CWiVLicense::LicenseFlagsWrite"), tlInternal);

	// Set Today flags in registry
	TraceInternal(_D("CWiVLicense::LicenseFlagsWrite: Setting Today flags in registry"));

	TraceInternal(_D("CWiVLicense::LicenseFlagsWrite: Flags = <%08X>"), dwFlags);

	// Open the Today Item Key
	TraceInternal(_D("CWiVLicense::LicenseFlagsWrite: Calling RegKeyOpen for Today Item key"));
	nResult = RegKeyOpen(GetTodayItemName(), &hKey, g_szTodayItemsKey);
	TraceInternal(_D("CWiVLicense::LicenseFlagsWrite: Back from RegKeyOpen, nResult = <%08x>, hKey = <%08x>"), nResult, hKey);

	if (nResult != ERROR_SUCCESS)
	{
		TraceInternal(_D("CWiVLicense::LicenseFlagsWrite: Error opening Today Item key, nResult = <%08x>"), nResult);
		TraceLeave(_D("CWiVLicense::LicenseFlagsWrite"), nResult, tlInternal);
		return nResult;
	}

	TraceInternal(_D("CSSSOptions::LicenseFlagsWrite: Calling RegSetValue for Flags = <%08X>"), dwFlags);
	nResult = RegSetValue(hKey, g_szTodayItemFlagsKey, dwFlags);
	TraceInternal(_D("CSSSOptions::LicenseFlagsWrite: Back from RegSetValue, nResult = <%08x>"), nResult);

	// Close the Today Item Key
	TraceInternal(_D("CWiVLicense::LicenseFlagsWrite: Calling RegKeyClose"));
	nResult = RegKeyClose(hKey);
	TraceInternal(_D("CWiVLicense::LicenseFlagsWrite: Back from RegKeyClose, nResult = <%08x>"), nResult);

	TraceLeave(_D("CWiVLicense::LicenseFlagsWrite"), nResult, tlInternal);
	return nResult;
}

DWORD CWiVLicense::LicenseDiskRead(const int nDisk)
{
	int		nResult = WIV_CHK_DSK_NOT_WRITTEN;
	DWORD	dwNumBytes;
	BYTE	bOEMbuf[WIV_MAX_OEM_BUF + 1];
	DWORD	dwRetVal = WIV_CHK_DSK_NOT_WRITTEN;

	TraceEnter(_D("CWiVLicense::LicenseDiskRead"), tlInternal);
// ===========================
	static bool blDone1 = false;
	static bool blDone2 = false;

	if (!blDone1)
	{
		memset(bOEMbuf, 0, sizeof(bOEMbuf));
		dwNumBytes = sizeof(bOEMbuf) - 1;
		nResult = StorageDiskRead(1, WIV_OEM_OFFSET, dwNumBytes, bOEMbuf);
		blDone1 = true;
	}
// ========================================
	memset(bOEMbuf, 0, sizeof(bOEMbuf));
	dwNumBytes = sizeof(bOEMbuf) - 1;

	nResult = StorageDiskRead(nDisk, WIV_OEM_OFFSET, dwNumBytes, bOEMbuf);

// ========================================
	if (!blDone2)
	{
		blDone2 = true;
	}
// ==============

	TraceInternal(_D("CWiVLicense::LicenseDiskRead: Disk %d back from StorageDiskRead, nResult = %d, bOEMbuf = <%s>"), nDisk, nResult, BtoS(bOEMbuf, dwNumBytes));

	if (nResult <= 0)
	{
		TraceInternal(_D("CWiVLicense::LicenseDiskRead: Could not read disk device"));
	}
	else
	{
		memcpy(&dwRetVal, &bOEMbuf[WIV_CHK_OFFSET], WIV_MAX_OEM_BUF - WIV_CHK_OFFSET);
		TraceInternal(_D("CWiVLicense::LicenseDiskRead: Disk %d Read: dwRetVal = %d (0x%08X)"), nDisk, dwRetVal, dwRetVal);
	}

	TraceLeave(_D("CWiVLicense::LicenseDiskRead"), dwRetVal, tlInternal);
	return dwRetVal;
}

int	CWiVLicense::LicenseDiskDecode(const DWORD dwData, PUCHAR lpuchType, PUSHORT lpusRemaining, PUSHORT lpusPeriod)
{
	int			nResult = WIV_CHK_DSK_NOT_WRITTEN;
// D245F500
	union {
		DWORD  dwData;
		struct {
			unsigned int Type		: 3;
			unsigned int Remaining	: 16;
			unsigned int Period	: 5;
			unsigned int NotApplic	: 8;
		}DBits;
	} DData;

	TraceEnter(_D("CWiVLicense::LicenseDiskDecode"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseDiskDecode: dwData = 0x%08X"), dwData);

	memset(&DData, 0, sizeof(DData));

	if (dwData != WIV_CHK_DSK_MS_DEFAULT)
	{
		DData.dwData = dwData;
	}

	TraceInternal(_D("CWiVLicense::LicenseDiskDecode: DData = <%s>"), BtoS((LPBYTE)&DData, sizeof(DData)));

	UCHAR	uchType		= DData.DBits.Type;
	USHORT	usRemaining	= DData.DBits.Remaining;
	USHORT	usPeriod	= DData.DBits.Period;

	*lpuchType		= uchType;
	*lpusRemaining  = usRemaining;
	*lpusPeriod	= usPeriod;

	TraceInternal(WIV_EMPTY_STRING);

	TraceInternal(_D("CWiVLicense::LicenseDiskDecode: uchType     = 0x%02X"), uchType);
	TraceInternal(_D("CWiVLicense::LicenseDiskDecode: usRemaining = 0x%08X"), usRemaining);
	TraceInternal(_D("CWiVLicense::LicenseDiskDecode: usPeriod    = 0x%04X"), usPeriod);

	nResult = 0;

	TraceLeave(_D("CWiVLicense::LicenseDiskDecode"), (DWORD)nResult, tlInternal);
	return nResult;
}

int	CWiVLicense::LicenseDiskEncode(LPDWORD lpdwData, const UCHAR uchType, const USHORT usRemaining, const USHORT usPeriod)
{
	int			nResult = WIV_CHK_DSK_NOT_WRITTEN;

	union {
		DWORD  dwData;
		struct {
			unsigned int Type		: 3;
			unsigned int Remaining	: 16;
			unsigned int Period	: 5;
			unsigned int NotApplic	: 8;
		}DBits;
	} DData;

	TraceEnter(_D("CWiVLicense::LicenseDiskEncode"), tlInternal);

	memset(&DData, 0, sizeof(DData));

	DData.DBits.Type		= uchType;
	DData.DBits.Remaining	= usRemaining;
	DData.DBits.Period		= usPeriod;

	TraceInternal(_D("CWiVLicense::LicenseDiskEncode: uchType = %d, usRemaining = 0x%04X, usPeriod = 0x%04X"), uchType, usRemaining, usPeriod);

	*lpdwData = DData.dwData;

	nResult = 0;

	TraceLeave(_D("CWiVLicense::LicenseDiskEncode"), (DWORD)nResult, tlInternal);
	return nResult;
}

int CWiVLicense::LicenseDiskWrite(const int nDisk, const DWORD dwData, const bool blLimited)
{
	DWORD		dwNumBytes;
	BYTE		bOEMbuf[WIV_MAX_OEM_BUF + 1];
	DWORD		dwValue = dwData;
	int			nResult = 0;

	TraceEnter(_D("CWiVLicense::LicenseDiskWrite"), tlInternal);

	memset(bOEMbuf, 0, sizeof(bOEMbuf));
	dwNumBytes = sizeof(bOEMbuf) - 1;

	nResult = StorageDiskRead(nDisk, WIV_OEM_OFFSET, dwNumBytes, bOEMbuf);
	TraceInternal(_D("CWiVLicense::LicenseDiskWrite: Disk %d Read: nResult = %d (0x%08X), bOEMbuf = <%s>"), nDisk, nResult, nResult, BtoS(bOEMbuf, dwNumBytes));

	if (blLimited)
	{
		memcpy(&bOEMbuf[WIV_CHK_OFFSET], &dwValue, WIV_MAX_OEM_BUF - WIV_CHK_OFFSET);
	}
	else
	{
		UtoA(g_szMSDefaultOEM, (PSZ)bOEMbuf, WIV_MAX_OEM_BUF);
	}

	nResult = StorageDiskWrite(nDisk, WIV_OEM_OFFSET, dwNumBytes, bOEMbuf);
	TraceInternal(_D("CWiVLicense::LicenseDiskWrite: Disk %d Write: nResult = %d (0x%08X), bOEMbuf = <%s>"), nDisk, nResult, nResult, BtoS(bOEMbuf, dwNumBytes));

	nResult = StorageDiskRead(nDisk, WIV_OEM_OFFSET, dwNumBytes, bOEMbuf);
	TraceInternal(_D("CWiVLicense::LicenseDiskWrite: Disk %d Read after write: nResult = %d (0x%04X), bOEMbuf = <%s>"), nDisk, nResult, nResult, BtoS(bOEMbuf, dwNumBytes));

	TraceLeave(_D("CWiVLicense::LicenseDiskWrite"), (DWORD)nResult, tlInternal);
	return nResult;
	
}

// Get special registry entry
int CWiVLicense::LicenseRegRead(LPTSTR lpszRegA, LPTSTR lpszRegB)
{
	DWORD		nResult;
	HKEY		hKey;
	TCHAR		szReg[WIV_MAX_STRING + 1];

	TraceEnter(_D("CWiVLicense::LicenseRegRead"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseRegRead: Getting Special entry from registry"));

	// Open the Special Key
	//[HKEY_CLASSES_ROOT\CLSID\{28D3086B-6F51-4ca6-B555-5D98A2234BA8}\TypeLib]
	TraceInternal(_D("CWiVLicense::LicenseRegRead: Calling RegKeyOpen for Special key"));
	nResult = RegKeyOpen(g_szTypeLibKey, &hKey, g_szSpecialKey, HKEY_CLASSES_ROOT);
	TraceInternal(_D("CWiVLicense::LicenseRegRead: Back from RegKeyOpen, nResult = <%08x>, hKey = <%08x>"), nResult, hKey);

	if (nResult != ERROR_SUCCESS)
	{
		TraceInternal(_D("CWiVLicense::LicenseRegRead: Error opening Special key, nResult = <%08x>"), nResult);
		TraceLeave(_D("CWiVLicense::LicenseRegRead"), WIV_CHK_REG_NOT_PRESENT, tlInternal);
		return WIV_CHK_REG_NOT_PRESENT;
	}

	// (Default) with default of ""
	TraceInternal(_D("CWiVLicense::LicenseRegRead: Calling RegGetValue for (Default)"));
	nResult = RegGetValue(hKey, g_szDefaultKey, szReg, WIV_MAX_STRING, WIV_EMPTY_STRING);
	TraceInternal(_D("CWiVLicense::LicenseRegRead: Back from RegGetValue, nResult = <%08X>, (Default) = <%s>"), nResult, szReg);

	TraceInternal(_D("CWiVLicense::LicenseRegRead: Special (Default) read from registry"));
	
	_tcsncpy(lpszRegA, szReg, WIV_CHK_REG_VALUE_SIZE);

	if (_tcslen(szReg) == 0)
	{
		TraceInternal(_D("CWiVLicense::LicenseRegRead: Default value not present"));
		TraceLeave(_D("CWiVLicense::LicenseRegRead"), WIV_CHK_REG_NOT_PRESENT, tlInternal);
		return WIV_CHK_REG_NOT_PRESENT;
	}

	// 2.0 with default of ""
	TraceInternal(_D("CWiVLicense::LicenseRegRead: Calling RegGetValue for 2.0"));
	nResult = RegGetValue(hKey, g_szSecondKey, szReg, WIV_MAX_STRING, WIV_EMPTY_STRING);
	TraceInternal(_D("CWiVLicense::LicenseRegRead: Back from RegGetValue, nResult = <%08X>, 2.0 = <%s>"), nResult, szReg);

	TraceInternal(_D("CWiVLicense::LicenseRegRead: Special 2.0 read from registry"));

	_tcsncpy(lpszRegB, szReg, WIV_CHK_REG_VALUE_SIZE);

	if (_tcslen(szReg) == 0)
	{
		TraceInternal(_D("CWiVLicense::LicenseRegRead: 2.0 value not present"));
		TraceLeave(_D("CWiVLicense::LicenseRegRead"), WIV_CHK_REG_NOT_PRESENT, tlInternal);
		return WIV_CHK_REG_NOT_PRESENT;
	}

	// Close the Special Key
	TraceInternal(_D("CWiVLicense::LicenseRegRead: Calling RegKeyClose"));
	nResult = RegKeyClose(hKey);
	TraceInternal(_D("CWiVLicense::LicenseRegRead: Back from RegKeyClose, nResult = <%08x>"), nResult);

	TraceLeave(_D("CWiVLicense::LicenseRegRead"), nResult, tlInternal);
	return nResult;
}

int	CWiVLicense::LicenseRegDecode(LPCTSTR lpszRegA, LPCTSTR lpszRegB, LPWIVLIC lpLicense,
								  PUCHAR lpuchType, PUSHORT lpusInstall, PUSHORT lpusRemaining,
								  LPDWORD lpdwSession)
{
	int		nResult = WIV_CHK_REG_NOT_PRESENT;
	char	szDword[(sizeof(DWORD)*2) + 1] = "";
	char	szUshort[(sizeof(USHORT)*2) + 1] = "";
	USHORT	usSessionA;
	USHORT	usSessionB;
	DWORD	dwSession;
	char	szRegA[WIV_CHK_REG_VALUE_SIZE + 1];
	char	szRegB[WIV_CHK_REG_VALUE_SIZE + 1];
	BYTE	bLicense[WIV_MAX_ENCRYPTED_LICENSE_LENGTH];
	char	szWorkA[WIV_MAX_ENCRYPTED_LICENSE_LENGTH + 1]	= "";
	char	szWorkB[WIV_MAX_ENCRYPTED_LICENSE_LENGTH + 1]	= "";
	BYTE	b1a;
	BYTE	b1b;
	BYTE	b2a;
	BYTE	b2b;

	union {
		USHORT  usFlagsA;
		struct {
			unsigned int Type		: 3;
			unsigned int Install	: 13;
		}RBitsA;
	} RFlagsA;

	union {
		USHORT  usFlagsB;
		struct {
			unsigned int Remaining	: 16;
		}RBitsB;
	} RFlagsB;

	TraceEnter(_D("CWiVLicense::LicenseRegDecode"), tlInternal);

	if (_tcslen(lpszRegA) != WIV_CHK_REG_VALUE_SIZE
	|| _tcslen(lpszRegB) != WIV_CHK_REG_VALUE_SIZE)
	{
		if (_tcslen(lpszRegA) != WIV_CHK_REG_VALUE_SIZE)
		{
			TraceInternal(_D("CWiVLicense::LicenseRegDecode: Default registry value is wrong length (%d)"), _tcslen(lpszRegA));
		}

		if (_tcslen(lpszRegB) != WIV_CHK_REG_VALUE_SIZE)
		{
			TraceInternal(_D("CWiVLicense::LicenseRegDecode: Second registry value is wrong length (%d)"), _tcslen(lpszRegB));
		}

		TraceLeave(_D("CWiVLicense::LicenseRegDecode"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	UtoA(lpszRegA, szRegA, WIV_CHK_REG_VALUE_SIZE);
	UtoA(lpszRegB, szRegB, WIV_CHK_REG_VALUE_SIZE);

	memcpy(szUshort, &szRegA[1], sizeof(USHORT)*2);
	TraceInternal(_D("CWiVLicense::LicenseRegDecode: szUshort (A) = %s"), BtoS((LPBYTE)szUshort, sizeof(szUshort)));
	usSessionA = (USHORT)strtoul(szUshort, NULL, WIV_BASE16);

	memcpy(szUshort, &szRegB[1], sizeof(USHORT)*2);
	TraceInternal(_D("CWiVLicense::LicenseRegDecode: szUshort (B) = %s"), BtoS((LPBYTE)szUshort, sizeof(szUshort)));
	usSessionB = (USHORT)strtoul(szUshort, NULL, WIV_BASE16);

	dwSession = (usSessionA << 16) + usSessionB;

	*lpdwSession = dwSession;
	TraceInternal(_D("CWiVLicense::LicenseRegDecode: dwSession = %08X"), dwSession);

	memcpy(&szUshort, &szRegA[5], sizeof(USHORT)*2);
	RFlagsA.usFlagsA = (USHORT)strtoul(szUshort, NULL, WIV_BASE16);
	*lpuchType		= RFlagsA.RBitsA.Type;
	*lpusInstall	= RFlagsA.RBitsA.Install;

	memcpy(szUshort, &szRegB[5], sizeof(USHORT)*2);
	RFlagsB.usFlagsB = (USHORT)strtoul(szUshort, NULL, WIV_BASE16);
	*lpusRemaining	= RFlagsB.RBitsB.Remaining;

// {8D860C9B-0B93-300F-2C2D-EA2320754514}
// {BE340003-4E1D-0D74-4459-345036C74128}

	memcpy(szWorkA, &szRegA[10], 4);
	memcpy(&szWorkA[4], &szRegA[15], 4);
	memcpy(&szWorkA[8], &szRegA[20], 4);
	memcpy(&szWorkA[12], &szRegA[25], 12);

	memcpy(szWorkB, &szRegB[10], 4);
	memcpy(&szWorkB[4], &szRegB[15], 4);
	memcpy(&szWorkB[8], &szRegB[20], 4);
	memcpy(&szWorkB[12], &szRegB[25], 12);

	TraceInternal(_D("CWiVLicense::LicenseRegDecode: szWorkA = <%S>, szWorkB = <%S>"), szWorkA, szWorkB);

	b1a	= (BYTE)((dwSession >> WIV_SHIFT_3_BYTES) & WIV_MASK_BYTE_1);
	b1b	= (BYTE)((dwSession >> WIV_SHIFT_2_BYTES) & WIV_MASK_BYTE_1);
	b2a	= (BYTE)((dwSession >> WIV_SHIFT_1_BYTE) & WIV_MASK_BYTE_1);
	b2b	= (BYTE)(dwSession & WIV_MASK_BYTE_1);

	TraceInternal(_D("CWiVLicense::LicenseRegDecode: b1a = %02X, b1b = %02X, b2a = %02X, b2b = %02X"), b1a, b1b, b2a, b2b);

	for (int j = 0, i = 0 ; j < WIV_MAX_ENCRYPTED_LICENSE_LENGTH ; )
	{
		BYTE s1 = 0;
		BYTE s2 = 0;

		s1 = szWorkA[j++];
		s2 = szWorkA[j++];

		s1 -= (s1 < 0x41) ? 0x30 : 0x37;
		s1 <<= 4;
		s2 -= (s2 < 0x41) ? 0x30 : 0x37;

		TraceInternal(_D("CWiVLicense::LicenseRegDecode: (A) s1 = %02X, s2 = %02X"), s1, s2);
		BYTE chL = (s1 | s2);
		TraceInternal(_D("CWiVLicense::LicenseRegDecode: (A) chL = %02X"), chL);

		 chL ^= (i & 1) ? b1b : b1a;
		TraceInternal(_D("CWiVLicense::LicenseRegDecode: (A) (i & 1) = %d, chL = %02X"), (i & 1), chL);

		bLicense[i++] = chL;
	}

	for (j = 0, i = WIV_MAX_ENCRYPTED_LICENSE_LENGTH/2 ; j < WIV_MAX_ENCRYPTED_LICENSE_LENGTH ; )
	{
		BYTE s1 = 0;
		BYTE s2 = 0;

		s1 = szWorkB[j++];
		s2 = szWorkB[j++];

		s1 -= (s1 < 0x41) ? 0x30 : 0x37;
		s1 <<= 4;
		s2 -= (s2 < 0x41) ? 0x30 : 0x37;

		TraceInternal(_D("CWiVLicense::LicenseRegDecode: (B) s1 = %02X, s2 = %02X"), s1, s2);
		BYTE chL = (s1 | s2);
		TraceInternal(_D("CWiVLicense::LicenseRegDecode: (B) chL = %02X"), chL);

		chL ^= (i & 1) ? b2b : b2a;
		TraceInternal(_D("CWiVLicense::LicenseRegDecode: (B) (i & 1) = %d, chL = %02X"), (i & 1), chL);

		bLicense[i++] = chL;
	}

	TraceInternal(_D("CWiVLicense::LicenseRegDecode: License code = <%s>"), BtoS(bLicense, WIV_MAX_ENCRYPTED_LICENSE_LENGTH));

 	memcpy(lpLicense, bLicense, WIV_MAX_ENCRYPTED_LICENSE_LENGTH);

	TraceInternal(WIV_EMPTY_STRING);

	TraceInternal(_D("CWiVLicense::LicenseRegDecode: *lpuchType     = 0x%04X"), *lpuchType);
	TraceInternal(_D("CWiVLicense::LicenseRegDecode: *lpusInstall   = 0x%04X"), *lpusInstall);
	TraceInternal(_D("CWiVLicense::LicenseRegDecode: *lpusRemaining = 0x%04X"), *lpusRemaining);
	TraceInternal(_D("CWiVLicense::LicenseRegDecode: lpbLicense    = <%s>"), BtoS((LPBYTE)lpLicense, WIV_MAX_ENCRYPTED_LICENSE_LENGTH));

	nResult = 0;

	TraceLeave(_D("CWiVLicense::LicenseRegDecode"), (DWORD)nResult, tlInternal);
	return nResult;
}

int CWiVLicense::LicenseRegEncode(LPTSTR lpszRegA, LPTSTR lpszRegB,
								  const LPWIVLIC lpLicense,
								  const UCHAR uchType, const USHORT usInstall,
								  const USHORT usRemaining, const DWORD dwSession)
{
	BYTE	bWork[WIV_MAX_ENCRYPTED_LICENSE_LENGTH];
	BYTE	*pbWA		= bWork;
	BYTE	*pbWB		= &bWork[WIV_MAX_ENCRYPTED_LICENSE_LENGTH/2];

	TCHAR	szWorkA[WIV_MAX_ENCRYPTED_LICENSE_LENGTH + 1]	= WIV_EMPTY_STRING;
	TCHAR	szWorkB[WIV_MAX_ENCRYPTED_LICENSE_LENGTH + 1]	= WIV_EMPTY_STRING;

	USHORT	usSessionA = HIWORD(dwSession);
	USHORT	usSessionB = LOWORD(dwSession);
	BYTE	b1a;
	BYTE	b1b;
	BYTE	b2a;
	BYTE	b2b;

	int		nResult		= 0;

	union {
		USHORT  usFlagsA;
		struct {
			unsigned int Type		: 3;
			unsigned int Install	: 13;
		}RBitsA;
	} RFlagsA;

	union {
		USHORT  usFlagsB;
		struct {
			unsigned int Remaining	: 16;
		}RBitsB;
	} RFlagsB;

	TraceEnter(_D("CWiVLicense::LicenseRegEncode"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseRegEncode: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseRegEncode"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	b1a			= (BYTE)((dwSession >> WIV_SHIFT_3_BYTES) & WIV_MASK_BYTE_1);
	b1b			= (BYTE)((dwSession >> WIV_SHIFT_2_BYTES) & WIV_MASK_BYTE_1);
	b2a			= (BYTE)((dwSession >> WIV_SHIFT_1_BYTE) & WIV_MASK_BYTE_1);
	b2b			= (BYTE)(dwSession & WIV_MASK_BYTE_1);

	TraceInternal(_D("CWiVLicense::LicenseRegEncode: dwSession = %08X"), dwSession);
	TraceInternal(_D("CWiVLicense::LicenseRegEncode: b1a = %02X, b1b = %02X, b2a = %02X, b2b = %02X"), b1a, b1b, b2a, b2b);

	RFlagsA.RBitsA.Type = uchType;
	RFlagsA.RBitsA.Install = usInstall;

	RFlagsB.RBitsB.Remaining = usRemaining;

	if ((char)uchType >= WIV_LICTYPE_FULL)
	{
		memcpy(bWork, lpLicense, WIV_MAX_ENCRYPTED_LICENSE_LENGTH);
	}
	else
	{
		memcpy(bWork, LicenseGetTrial(), WIV_MAX_ENCRYPTED_LICENSE_LENGTH);
	}

	for (int i = 0; i < WIV_MAX_ENCRYPTED_LICENSE_LENGTH/2; i++)
	{
		bWork[i++] ^= b1a;
		bWork[i] ^= b1b;
	}

	for (i = WIV_MAX_ENCRYPTED_LICENSE_LENGTH/2; i < (int)sizeof(bWork); i++)
	{
		bWork[i++] ^= b2a;
		bWork[i] ^= b2b;
	}
	
	TraceInternal(_D("CWiVLicense::LicenseRegEncode: bWork = <%s>"), BtoS(bWork, sizeof(bWork)));
	TraceInternal(_D("CWiVLicense::LicenseRegEncode: pbWA  = <%s>"), BtoS(pbWA, WIV_MAX_ENCRYPTED_LICENSE_LENGTH/2));
	TraceInternal(_D("CWiVLicense::LicenseRegEncode: pbWB  = <%s>"), BtoS(pbWB, WIV_MAX_ENCRYPTED_LICENSE_LENGTH/2));

	_tcsncpy(szWorkA, BtoS((LPBYTE)pbWA, WIV_MAX_ENCRYPTED_LICENSE_LENGTH/2), WIV_MAX_ENCRYPTED_LICENSE_LENGTH);
	_tcsncpy(szWorkB, BtoS((LPBYTE)pbWB, WIV_MAX_ENCRYPTED_LICENSE_LENGTH/2), WIV_MAX_ENCRYPTED_LICENSE_LENGTH);

	TraceInternal(_D("CWiVLicense::LicenseRegEncode: szWorka = <%s>"), szWorkA);
	TraceInternal(_D("CWiVLicense::LicenseRegEncode: szWorkb = <%s>"), szWorkB);

	_snwprintf(lpszRegA, WIV_CHK_REG_VALUE_SIZE, g_szFormatRegValue, usSessionA, RFlagsA.usFlagsA, szWorkA, &szWorkA[4], &szWorkA[8], &szWorkA[12]);
	_snwprintf(lpszRegB, WIV_CHK_REG_VALUE_SIZE, g_szFormatRegValue, usSessionB, RFlagsB.usFlagsB, szWorkB, &szWorkB[4], &szWorkB[8], &szWorkB[12]);

	TraceInternal(_D("CWiVLicense::LicenseRegEncode: lpszRegA = <%s>"), lpszRegA);
	TraceInternal(_D("CWiVLicense::LicenseRegEncode: lpszRegB = <%s>"), lpszRegB);

	TraceLeave(_D("CWiVLicense::LicenseRegEncode"), nResult, tlInternal);

	return nResult;
}

// Set special registry entry
int CWiVLicense::LicenseRegWrite(LPCTSTR lpszRegA, LPCTSTR lpszRegB)
{
	DWORD		nResult;
	HKEY		hKey;

	TraceEnter(_D("CWiVLicense::LicenseRegWrite"), tlInternal);

	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Setting Special entry in registry"));

	// Open the Special Key
	//[HKEY_CLASSES_ROOT\CLSID\{28D3086B-6F51-4ca6-B555-5D98A2234BA8}\TypeLib]
	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Calling RegKeyOpen for Special key"));
	nResult = RegKeyOpen(g_szTypeLibKey, &hKey, g_szSpecialKey, HKEY_CLASSES_ROOT);
	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Back from RegKeyOpen, nResult = <%08x>, hKey = <%08x>"), nResult, hKey);

	if (nResult != ERROR_SUCCESS)
	{
		TraceInternal(_D("CWiVLicense::LicenseRegWrite: Error opening Special key, nResult = <%08x>"), nResult);
		TraceLeave(_D("CWiVLicense::LicenseRegWrite"), nResult, tlInternal);
		return nResult;
	}

	// (Default) value
	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Calling RegSetValue for (Default) = <%s>"), lpszRegA);
	nResult = RegSetValue(hKey, g_szDefaultKey, lpszRegA);

	// Second value
	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Calling RegSetValue for 2.0 = <%s>"), lpszRegB);
	nResult = RegSetValue(hKey, g_szSecondKey, lpszRegB);

	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Back from RegSetValue, nResult = <%08x>"), nResult);


	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Special written to registry"));

	// Close the Special Key
	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Calling RegKeyClose"));
	nResult = RegKeyClose(hKey);
	TraceInternal(_D("CWiVLicense::LicenseRegWrite: Back from RegKeyClose, nResult = <%08x>"), nResult);

	TraceLeave(_D("CWiVLicense::LicenseRegWrite"), nResult, tlInternal);
	return nResult;
}

// Create/Open special file
int CWiVLicense::LicenseFileOpen(const bool blCreate)
{
	int				nResult = WIV_CHK_FILE_NOT_PRESENT;
	TCHAR			szFileName[MAX_PATH + 1];
    DWORD			dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    DWORD			dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	DWORD			dwCreationDisposition = OPEN_EXISTING;
    WIN32_FIND_DATA fdFindData;
	TCHAR			szFolder[MAX_PATH + 1] = WIV_EMPTY_STRING;
	HANDLE			hFind = NULL;
	int				nRes = 0;
	bool			blFolderFound = false;

	TraceEnter(_D("CWiVLicense::LicenseFileOpen"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileOpen: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseFileOpen"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	if (!m_pLicenseData->blFileIsOpen)
	{
		_snwprintf(szFolder, MAX_PATH, _T("%s%s"), GetWindowsPath(), g_szConfigMgrDir);
		
		TraceInternal(_D("CWiVLicense::LicenseFileOpen: szFolder = <%s>"), szFolder);
		
		// Find matching files.
		hFind = FindFirstFile (szFolder, &fdFindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do {
				// See if a folder.
				if (fdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					TraceInternal(_D("CWiVLicense::LicenseFileOpen: Folder found"));
					blFolderFound = true;
					break;
				}
				nRes = FindNextFile (hFind, &fdFindData);
			} while (nRes);
			
			FindClose (hFind);
		}
		
		if (!blFolderFound)
		{
			TraceInternal(_D("CWiVLicense::LicenseFileOpen: Folder not found, creating it"));

			LPFNFUNCFCD	lpfnFCD = (LPFNFUNCFCD)GetFunctionAddress(FNFCD);

			if (lpfnFCD == NULL)
			{
				TraceInternal(_D("CWiVLicense::LicenseFileOpen: Could not get address of CreateDirectory function"));
				TraceLeave(_D("CWiVLicense::LicenseFileOpen"), nResult, tlInternal);
				return nResult;
			}
			
			if (!lpfnFCD(szFolder, NULL))
			{
				DWORD dwError = GetLastError();
				TraceInternal(_D("CWiVLicense::LicenseFileOpen: CreateDirectory failed, Error = %08X"), dwError);
				TraceLeave(_D("CWiVLicense::LicenseFileOpen"), nResult, tlInternal);
				return nResult;
			}
		}
		
	}
	// If file already open, close it.
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseFileOpen: File already open, so close it"));
		CloseHandle (m_pLicenseData->hfFile);
		m_pLicenseData->hfFile = NULL;
		m_pLicenseData->blFileIsOpen = false;
	}

	_snwprintf(szFileName, MAX_PATH, _T("%s\\%s"), szFolder, g_szConfigMgrFile);
	TraceInternal(_D("CWiVLicense::LicenseFileOpen: szFileName = <%s>"), szFileName);
	
	TraceInternal(_D("CWiVLicense::LicenseFileOpen: File = %s"), szFileName);

	if (blCreate)
	{
		dwDesiredAccess = GENERIC_WRITE;
		dwShareMode = 0;
		dwCreationDisposition = CREATE_ALWAYS;
	}

	// Create/Open the file.
	m_pLicenseData->hfFile = CreateFile (szFileName, dwDesiredAccess,
						   dwShareMode, NULL, dwCreationDisposition,
						   FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);

	if (m_pLicenseData->hfFile != INVALID_HANDLE_VALUE && m_pLicenseData->hfFile != NULL)
	{
		if (blCreate)
		{
			TraceInternal(_D("CWiVLicense::LicenseFileOpen: Special file created"));
		}
		else
		{
			TraceInternal(_D("CWiVLicense::LicenseFileOpen: Special file opened"));
		}

		m_pLicenseData->blFileIsOpen = true;
		nResult = 0;
	}
	else
	{
		if (blCreate)
		{
			TraceInternal(_D("CWiVLicense::LicenseFileOpen: Cannot create special file"));
		}
		else
		{
			TraceInternal(_D("CWiVLicense::LicenseFileOpen: Cannot open special file"));
		}

		m_pLicenseData->hfFile = NULL;
	}

	TraceLeave(_D("CWiVLicense::LicenseFileOpen"), (DWORD)nResult, tlInternal);
	return nResult;
}

// Read special file details
int CWiVLicense::LicenseFileRead(HANDLE hfFile, BYTE *pszDetails)
{
	LPFNFUNCGFS	lpfnGFS;
	int			nResult = WIV_CHK_FILE_NOT_PRESENT;
	DWORD		cBytes;
	int			nFileSize;

	TraceEnter(_D("CWiVLicense::LicenseFileRead"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseFileRead: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseFileRead"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	// Check that file is open
	if (hfFile == INVALID_HANDLE_VALUE || hfFile == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileRead: File is not open"));
		TraceLeave(_D("CWiVLicense::LicenseFileRead"), nResult, tlInternal);
		return nResult;
	}

	lpfnGFS = (LPFNFUNCGFS)GetFunctionAddress(FNGFS);
	if (lpfnGFS == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileRead: Could not get address of GetFileSize function"));
		TraceLeave(_D("CWiVLicense::LicenseFileRead"), nResult, tlInternal);
		return nResult;
	}

	// Get the size of the file
	nFileSize = (int)lpfnGFS(hfFile, NULL);

	TraceInternal(_D("CWiVLicense::LicenseFileRead: File size = %d"), nFileSize);

	// Check file size
	if (nFileSize != WIV_CHK_FILE_SIZE)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileRead: Invalid file size"));
		TraceLeave(_D("CWiVLicense::LicenseFileRead"), nResult, tlInternal);
		return nResult;
	}

	memset(pszDetails, 0, nFileSize);

	if (!ReadFile (m_pLicenseData->hfFile, pszDetails, nFileSize, &cBytes, NULL))
	{
		TraceInternal(_D("CWiVLicense::LicenseFileRead: ReadFile failed"));

		TraceLeave(_D("CWiVLicense::LicenseFileRead"), nResult, tlInternal);
		return nResult;
	}

	if (cBytes != WIV_CHK_FILE_SIZE)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileRead: Invalid number of bytes read"));
		TraceLeave(_D("CWiVLicense::LicenseFileRead"), nResult, tlInternal);
		return nResult;
	}

	nResult = 0;

	TraceLeave(_D("CWiVLicense::LicenseFileRead"), (DWORD)nResult, tlInternal);

	return nResult;
}

int	CWiVLicense::LicenseFileDecode(LPBYTE lpbFile, LPTSTR lpszRegA, LPTSTR lpszRegB, LPWIVLIC lpLicense,
								   PUCHAR lpuchType, PUSHORT lpusElapsed, PUSHORT lpusInstall,
								   PUSHORT lpusRemaining, PUSHORT lpusPeriod, LPDWORD lpdwSession)
{
	int		nResult = WIV_CHK_FILE_NOT_PRESENT;
	char	szRegA[WIV_CHK_REG_VALUE_SIZE + 1];
	char	szRegB[WIV_CHK_REG_VALUE_SIZE + 1];
	UCHAR	uchType;
	USHORT	usInstall;
	USHORT	usRemaining;

	union {
		struct {
			DWORD  dwFlagsLow;
			DWORD  dwFlagsHigh;
		}FData;
		struct {
			unsigned int Type		: 3;
			unsigned int Elapsed	: 16;
			unsigned int Install	: 13;
			unsigned int Remaining	: 16;
			unsigned int Period	: 5;
			unsigned int NotApplic	: 11;
		}FBits;
	} FileData;

	TraceEnter(_D("CWiVLicense::LicenseFileDecode"), tlInternal);

	memset(szRegA, 0, sizeof(szRegA));
	memset(szRegB, 0, sizeof(szRegB));

	memcpy(szRegA, &lpbFile[WIV_CHK_FILE_REG_POS_A], WIV_CHK_REG_VALUE_SIZE);
	memcpy(szRegB, &lpbFile[WIV_CHK_FILE_REG_POS_B], WIV_CHK_REG_VALUE_SIZE);

	memcpy(&FileData.FData.dwFlagsLow, &lpbFile[WIV_CHK_FILE_FLAGS_POS_L], sizeof(DWORD));
	memcpy(&FileData.FData.dwFlagsHigh, &lpbFile[WIV_CHK_FILE_FLAGS_POS_H], sizeof(DWORD));

	for (int i = 0; i < WIV_CHK_REG_VALUE_SIZE; i++)
	{
		szRegA[i] ^= lpbFile[WIV_CHK_FILE_XOR_CHAR_POS_A];
		szRegB[i] ^= lpbFile[WIV_CHK_FILE_XOR_CHAR_POS_B];
	}

	TraceInternal(_D("CWiVLicense::LicenseFileDecode: szRegA = <%S>, szRegB = <%S>"), szRegA, szRegB);

	AtoU(szRegA, lpszRegA, WIV_CHK_REG_VALUE_SIZE);
	AtoU(szRegB, lpszRegB, WIV_CHK_REG_VALUE_SIZE);

	LicenseRegDecode(lpszRegA, lpszRegB, lpLicense, &uchType, &usInstall, &usRemaining, lpdwSession);

	*lpuchType		= FileData.FBits.Type;
	*lpusElapsed	= FileData.FBits.Elapsed;
	*lpusInstall	= FileData.FBits.Install;
	*lpusRemaining	= FileData.FBits.Remaining;
	*lpusPeriod		= FileData.FBits.Period;

	TraceInternal(WIV_EMPTY_STRING);

	TraceInternal(_D("CWiVLicense::LicenseFileDecode: *lpuchType     = 0x%02X"), *lpuchType);
	TraceInternal(_D("CWiVLicense::LicenseFileDecode: *lpusElapsed   = 0x%04X"), *lpusElapsed);
	TraceInternal(_D("CWiVLicense::LicenseFileDecode: *lpusInstall   = 0x%02X"), *lpusInstall);
	TraceInternal(_D("CWiVLicense::LicenseFileDecode: *lpusRemaining = 0x%04X"), *lpusRemaining);
	TraceInternal(_D("CWiVLicense::LicenseFileDecode: *lpusPeriod    = 0x%02X"), *lpusPeriod);
	TraceInternal(_D("CWiVLicense::LicenseFileDecode: *lpdwSession   = 0x%02X"), *lpdwSession);
	TraceInternal(_D("CWiVLicense::LicenseFileDecode: lpbLicense     = <%s>"), BtoS((LPBYTE)lpLicense, WIV_MAX_ENCRYPTED_LICENSE_LENGTH));
	TraceInternal(_D("CWiVLicense::LicenseFileDecode: lpszRegA       = <%s>"), lpszRegA);
	TraceInternal(_D("CWiVLicense::LicenseFileDecode: lpszRegB       = <%s>"), lpszRegB);

	TraceLeave(_D("CWiVLicense::LicenseFileDecode"), (DWORD)nResult, tlInternal);
	return nResult;
}

// Generate special file contents
int	CWiVLicense::LicenseFileEncode(LPCTSTR lpszRegA, LPCTSTR lpszRegB, LPBYTE lpbFile,
								   const UCHAR uchType, const USHORT usElapsed, const USHORT usInstall,
								   const USHORT usRemaining, const USHORT usPeriod, const DWORD dwSession)
{
	int		nResult = WIV_CHK_FILE_NOT_PRESENT;
	char	szRegA[WIV_CHK_REG_VALUE_SIZE + 1];
	char	szRegB[WIV_CHK_REG_VALUE_SIZE + 1];

	union {
		struct {
			DWORD  dwFlagsLow;
			DWORD  dwFlagsHigh;
		}FData;
		struct {
			unsigned int Type		: 3;
			unsigned int Elapsed	: 16;
			unsigned int Install	: 13;
			unsigned int Remaining	: 16;
			unsigned int Period		: 5;
			unsigned int NotApplic	: 11;
		}FBits;
	} FileData;

	TraceEnter(_D("CWiVLicense::LicenseFileEncode"), tlInternal);
	
	TraceInternal(_D("CWiVLicense::LicenseFileEncode: lpszRegA = <%s>"), lpszRegA);
	TraceInternal(_D("CWiVLicense::LicenseFileEncode: lpszRegB = <%s>"), lpszRegB);

	UtoA(lpszRegA, szRegA, WIV_CHK_REG_VALUE_SIZE);
	UtoA(lpszRegB, szRegB, WIV_CHK_REG_VALUE_SIZE);

	TraceInternal(_D("CWiVLicense::LicenseFileEncode: szRegA = <%S>"), szRegA);
	TraceInternal(_D("CWiVLicense::LicenseFileEncode: szRegB = <%S>"), szRegB);
	TraceInternal(_D("CWiVLicense::LicenseFileEncode: lpbFile[0] = %02X, lpbFile[16] = %02X"), lpbFile[WIV_CHK_FILE_XOR_CHAR_POS_A], lpbFile[WIV_CHK_FILE_XOR_CHAR_POS_B]);

	for (int i = 0; i < WIV_CHK_REG_VALUE_SIZE; i++)
	{
		szRegA[i] ^= lpbFile[WIV_CHK_FILE_XOR_CHAR_POS_A];
		szRegB[i] ^= lpbFile[WIV_CHK_FILE_XOR_CHAR_POS_B];
	}

	memcpy(&lpbFile[WIV_CHK_FILE_REG_POS_A], szRegA, WIV_CHK_REG_VALUE_SIZE);
	memcpy(&lpbFile[WIV_CHK_FILE_REG_POS_B], szRegB, WIV_CHK_REG_VALUE_SIZE);

	FileData.FBits.Type			= uchType;
	FileData.FBits.Elapsed		= usElapsed;
	FileData.FBits.Install		= usInstall;
	FileData.FBits.Remaining	= usRemaining;
	FileData.FBits.Period		= usPeriod;

	memcpy(&lpbFile[WIV_CHK_FILE_FLAGS_POS_L], &FileData.FData.dwFlagsLow, sizeof(DWORD));
	memcpy(&lpbFile[WIV_CHK_FILE_FLAGS_POS_H], &FileData.FData.dwFlagsHigh, sizeof(DWORD));

	nResult = 0;

	TraceLeave(_D("CWiVLicense::LicenseFileEncode"), (DWORD)nResult, tlInternal);
	return nResult;
}

// Write special file details
int CWiVLicense::LicenseFileWrite(HANDLE hfFile, const BYTE *const pszDetails)
{
	LPFNFUNCWF		lpfnWF;
	LPFNFUNCFFB		lpfnFFB;
	int				nResult = WIV_CHK_FILE_NOT_PRESENT;
	int				nFileSize = WIV_CHK_FILE_SIZE;
	DWORD			dwBytes;

	TraceEnter(_D("CWiVLicense::LicenseFileWrite"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseFileWrite: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseFileWrite"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	// Check that file is open
	if (hfFile == INVALID_HANDLE_VALUE || hfFile == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileWrite: File is not open"));
		TraceLeave(_D("CWiVLicense::LicenseFileWrite"), nResult, tlInternal);
		return nResult;
	}

	TraceInternal(_D("CWiVLicense::LicenseFileWrite: Buffer = <%s>"), BtoS((LPBYTE)pszDetails, -WIV_CHK_FILE_NOT_PRESENT));

	lpfnWF = (LPFNFUNCWF)GetFunctionAddress(FNWF);
	if (lpfnWF == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileWrite: Could not get address of WriteFile function"));
		TraceLeave(_D("CWiVLicense::LicenseFileWrite"), nResult, tlInternal);
		return nResult;
	}

	lpfnFFB = (LPFNFUNCFFB)GetFunctionAddress(FNFFB);
	if (lpfnFFB == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileWrite: Could not get address of FlushFileBuffers function"));
		TraceLeave(_D("CWiVLicense::LicenseFileWrite"), nResult, tlInternal);
		return nResult;
	}

	if (!lpfnWF(m_pLicenseData->hfFile, pszDetails, nFileSize, &dwBytes, NULL))
	{
		DWORD dwError = GetLastError();

		TraceInternal(_D("CWiVLicense::LicenseFileWrite: WriteFile failed, Error = %08X"), dwError);

		TraceLeave(_D("CWiVLicense::LicenseFileWrite"), nResult, tlInternal);
		return nResult;
	}

	lpfnFFB(m_pLicenseData->hfFile);

	TraceInternal(_D("CWiVLicense::LicenseFileWrite: %d bytes written"), dwBytes);

	if (dwBytes != WIV_CHK_FILE_SIZE)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileWrite: Invalid number of bytes written"));
		TraceLeave(_D("CWiVLicense::LicenseFileWrite"), nResult, tlInternal);
		return nResult;
	}

	nResult = 0;

	TraceLeave(_D("CWiVLicense::LicenseFileWrite"), (DWORD)nResult, tlInternal);
	return nResult;
}

int CWiVLicense::LicenseFileClose(HANDLE hfFile)
{
	int		nResult = 0;

	TraceEnter(_D("CWiVLicense::LicenseFileClose"), tlInternal);

	if (hfFile != INVALID_HANDLE_VALUE && hfFile != NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileClose: Closing file"), tlInternal);
		CloseHandle(hfFile);
		hfFile = NULL;
		m_pLicenseData->blFileIsOpen = false;
	}
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseFileClose: File not open"), tlInternal);
	}

	TraceLeave(_D("CWiVLicense::LicenseFileClose"), (DWORD)nResult, tlInternal);
	return nResult;
}

int CWiVLicense::LicenseFileSetTime()
{
	LPFNFUNCGFT	lpfnGFT;
	LPFNFUNCSFT	lpfnSFT;
	int			nResult = -1;
	TCHAR		szFileName[MAX_PATH + 1];
	HANDLE		hfFile = NULL;

    DWORD		dwDesiredAccess = GENERIC_READ;
    DWORD		dwShareMode = FILE_SHARE_READ;
	DWORD		dwCreationDisposition = OPEN_EXISTING;
	FILETIME	ftCreationTime; 
	FILETIME	ftLastAccessTime; 
	FILETIME	ftLastWriteTime;

	TraceEnter(_D("CWiVLicense::LicenseFileSetTime"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseFileSetTime: License data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseFileSetTime"), (DWORD)nResult, tlInternal);
		return nResult;
	}

	lpfnGFT = (LPFNFUNCGFT)GetFunctionAddress(FNGFT);
	if (lpfnGFT == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileSetTime: Could not get address of GetFileTime function"));
		TraceLeave(_D("CWiVLicense::LicenseFileSetTime"), nResult, tlInternal);
		return nResult;
	}

	lpfnSFT = (LPFNFUNCSFT)GetFunctionAddress(FNSFT);
	if (lpfnSFT == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseFileSetTime: Could not get address of SetFileTime function"));
		TraceLeave(_D("CWiVLicense::LicenseFileSetTime"), nResult, tlInternal);
		return nResult;
	}

	_snwprintf(szFileName, MAX_PATH, _T("%s%s"), GetWindowsPath(), g_szSMSReceiver);

	// Open the file.
	hfFile = CreateFile (szFileName, dwDesiredAccess,
					   dwShareMode, NULL, dwCreationDisposition,
						   FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfFile != INVALID_HANDLE_VALUE && hfFile != NULL)
	{
		BOOL	blRes = 0;

		TraceInternal(_D("CWiVLicense::LicenseFileSetTime: %s opened"), szFileName);

		blRes = lpfnGFT(hfFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime);

		if (blRes != 0)
		{
			TraceInternal(_D("CWiVLicense::LicenseFileSetTime: File time retrieved from %s"), szFileName);

			blRes = lpfnSFT(m_pLicenseData->hfFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime); 

			if (blRes != 0)
			{
				TraceInternal(_D("CWiVLicense::LicenseFileSetTime: File time set for special file"));
				nResult = 0;

			}else
			{
				TraceInternal(_D("CWiVLicense::LicenseFileSetTime: Could not set file time for special file"));
			}
		}
		else
		{
			TraceInternal(_D("CWiVLicense::LicenseFileSetTime: Could not get file time from %s"), szFileName);
		}

		TraceInternal(_D("CWiVLicense::LicenseFileSetTime: Closing file %s"), szFileName);
		CloseHandle(hfFile);
		hfFile = NULL;
	}
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseFileSetTime: Cannot open %s"), szFileName);
	}

	TraceLeave(_D("CWiVLicense::LicenseFileSetTime"), (DWORD)nResult, tlInternal);
	return nResult;
}

HANDLE CWiVLicense::StorageDiskOpen(int nDsk)
{
    TCHAR szDsk[WIV_MAX_DSK_NAME + 1];
	HANDLE hDsk;

	TraceEnter(_D("CWiVLicense::StorageDiskOpen"), tlInternal);

    if (nDsk >= 0 && nDsk <= 256)
	{
        _snwprintf(szDsk, WIV_MAX_DSK_NAME,  _T("%s%u:"), g_szDiskLabel, nDsk);
	}
    else
	{
        _snwprintf(szDsk, WIV_MAX_DSK_NAME, _T("%s:"), g_szDiskLabel);
	}

	szDsk[WIV_MAX_DSK_NAME] = WIV_NULL_CHAR;

    hDsk = CreateFile(szDsk, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0 );

    if (hDsk == NULL || hDsk == INVALID_HANDLE_VALUE)
    {
		TraceLeave(_D("CWiVLicense::StorageDiskOpen"), (DWORD)NULL, tlInternal);
        return NULL;
    }

	TraceLeave(_D("CWiVLicense::StorageDiskOpen"), (DWORD)hDsk, tlInternal);
    return hDsk;
}

int CWiVLicense::StorageDiskGetInfo(const HANDLE hDsk, DISK_INFO &info)
{
    DWORD nReturned;

	TraceEnter(_D("CWiVLicense::StorageDiskGetInfo"), tlInternal);

    // Various CE versions use all possible combinations, so we try 'm all.
    if (DeviceIO(hDsk, IOCTL_DISK_GETINFO, &info, sizeof(info), NULL, 0, &nReturned, NULL))
	{
		TraceLeave(_D("CWiVLicense::StorageDiskGetInfo"), (DWORD)1, tlInternal);
        return 1;
	}
    if (DeviceIO(hDsk, IOCTL_DISK_GETINFO, NULL, 0, &info, sizeof(info), &nReturned, NULL))
	{
		TraceLeave(_D("CWiVLicense::StorageDiskGetInfo"), (DWORD)2, tlInternal);
        return 2;
	}
    if (DeviceIO(hDsk, DISK_IOCTL_GETINFO, &info, sizeof(info), NULL, 0, &nReturned, NULL))
	{
		TraceLeave(_D("CWiVLicense::StorageDiskGetInfo"), (DWORD)3, tlInternal);
        return 3;
	}
    if (DeviceIO(hDsk, DISK_IOCTL_GETINFO, NULL, 0, &info, sizeof(info), &nReturned, NULL))
	{
		TraceLeave(_D("CWiVLicense::StorageDiskGetInfo"), (DWORD)4, tlInternal);
        return 4;
	}

	TraceLeave(_D("CWiVLicense::StorageDiskGetInfo"), (DWORD)0, tlInternal);
    return 0;
}

int CWiVLicense::StorageDiskRead(const int nDsk, const int nStartByte, const int nNumBytes, BYTE* pbBuffer)
{
    HANDLE		hDsk;
	HRESULT		hrResult;
    DISK_INFO	diInfo;
    SG_REQ		sgReq;
	BYTE		bSectorBuffer[WIV_MAX_SECTOR_BUF + 1];
    DWORD		dwBytesRead			= 0;
	int			nResult				= 0;

	TraceEnter(_D("CWiVLicense::StorageDiskRead"), tlInternal);

	hDsk = StorageDiskOpen(nDsk);

    if (hDsk == NULL)
	{
		hrResult = GetLastError();
		TraceInternal(_D("CWiVLicense::StorageDiskRead: StorageDiskOpen failed, hrResult = 0x%08X"), hrResult);
		TraceLeave(_D("CWiVLicense::StorageDiskRead"), hrResult, tlInternal);
		return hrResult;
	}

	nResult = StorageDiskGetInfo(hDsk, diInfo);
	TraceInternal(_D("CWiVLicense::StorageDiskRead: Back from StorageDiskGetInfo: nResult = %d"), nResult);

    if (nResult <= 0)
    {
        hrResult= GetLastError();
		TraceInternal(_D("CWiVLicense::StorageDiskRead: StorageDiskGetInfo failed, hrResult = 0x%08X"), hrResult);
        CloseHandle(hDsk);
		hDsk = NULL;
		TraceLeave(_D("CWiVLicense::StorageDiskRead"), hrResult, tlInternal);
        return hrResult;
    }

    sgReq.sr_start				= 0;
    sgReq.sr_num_sec			= 1;
    sgReq.sr_num_sg				= 1;
    sgReq.sr_status				= 0;
    sgReq.sr_callback			= NULL;
    sgReq.sr_sglist[0].sb_len	= min(diInfo.di_bytes_per_sect, WIV_MAX_SECTOR_BUF);
    sgReq.sr_sglist[0].sb_buf	= bSectorBuffer;

    if (!DeviceIO(hDsk, DISK_IOCTL_READ, &sgReq, sizeof(sgReq), NULL, NULL, &dwBytesRead, NULL))
	{
		hrResult = GetLastError();
		TraceInternal(_D("CWiVLicense::StorageDiskRead: DeviceIO failed, hrResult = 0x%08X"), hrResult);
	    CloseHandle(hDsk);
		hDsk = NULL;
		TraceLeave(_D("CWiVLicense::StorageDiskRead"), -1, tlInternal);
        return -1;
	}

	memcpy(pbBuffer, &bSectorBuffer[nStartByte], nNumBytes);
	nResult = nNumBytes;

	TraceInternal(_D("CWiVLicense::StorageDiskRead: nNumBytes = %d"), nNumBytes);
	TraceInternal(_D("CWiVLicense::StorageDiskRead: pbBuffer = <%s>"), BtoS(pbBuffer, nNumBytes));

    CloseHandle(hDsk);
	hDsk = NULL;

	TraceLeave(_D("CWiVLicense::StorageDiskRead"), nResult, tlInternal);
	return nResult;
}

int CWiVLicense::StorageDiskWrite(const int nDsk, const int nStartByte, const int nNumBytes, BYTE* pbBuffer)
{
    HANDLE		hDsk;
	HRESULT		hrResult;
    DISK_INFO	diInfo;
    SG_REQ		sgReq;
	BYTE		bSectorBuffer[WIV_MAX_SECTOR_BUF + 1];
    DWORD		dwBytesWritten = 0;
	int			nResult = 0;
	DWORD		dwRes;

	TraceEnter(_D("CWiVLicense::StorageDiskWrite"), tlInternal);

	memset(bSectorBuffer, 0, sizeof(bSectorBuffer));
	dwRes = StorageDiskRead(nDsk, 0, sizeof(bSectorBuffer)-1, bSectorBuffer);
	memcpy(&bSectorBuffer[nStartByte], pbBuffer, nNumBytes);

	hDsk = StorageDiskOpen(nDsk);

    if (hDsk == NULL)
	{
		hrResult = GetLastError();
		TraceLeave(_D("CWiVLicense::StorageDiskWrite"), hrResult, tlInternal);
		return hrResult;
	}

    if (!StorageDiskGetInfo(hDsk, diInfo))
    {
        hrResult = GetLastError();
        CloseHandle(hDsk);
		hDsk = NULL;
		TraceLeave(_D("CWiVLicense::StorageDiskWrite"), hrResult, tlInternal);
        return hrResult;
    }

    sgReq.sr_start				= 0;
    sgReq.sr_num_sec			= 1;
    sgReq.sr_num_sg				= 1;
    sgReq.sr_status				= 0;
    sgReq.sr_callback			= NULL;
    sgReq.sr_sglist[0].sb_len	= min(diInfo.di_bytes_per_sect, WIV_MAX_SECTOR_BUF);//diInfo.di_bytes_per_sect;
    sgReq.sr_sglist[0].sb_buf	= bSectorBuffer;

    if (!DeviceIO(hDsk, DISK_IOCTL_WRITE, &sgReq, sizeof(sgReq), NULL, NULL, &dwBytesWritten, NULL))
	{
	    CloseHandle(hDsk);
		TraceLeave(_D("CWiVLicense::StorageDiskWrite"), -1, tlInternal);
        return -1;
	}

	memcpy(pbBuffer, &bSectorBuffer[nStartByte], nNumBytes);

    CloseHandle(hDsk);

	TraceLeave(_D("CWiVLicense::StorageDiskWrite"), dwBytesWritten, tlInternal);
	return dwBytesWritten;
}

LPCTSTR CWiVLicense::LicenseGetIMEI()
{
	TraceEnter(_D("CWiVLicense::LicenseGetIMEI"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::LicenseGetIMEI, License Data is NULL"));
		TraceLeave(_D("CWiVLicense::LicenseGetIMEI"), (DWORD)NULL, tlInternal);
		return NULL;
	}

	TraceLeave(_D("CWiVLicense::LicenseGetIMEI"), (DWORD)m_pLicenseData->szIMEI, tlInternal);
	return m_pLicenseData->szIMEI;
}

BOOL CWiVLicense::LicenseGetSerialNumber(LPTSTR lpszSerialNumber)
{
	LPFNFUNCKIOC	lpfnKIOC;

	byte		bDevinfo[64];
	DWORD		dwDevInfo;
	BOOL		blResult = FALSE;
	DWORD		dwOutBytes = 0;
	const int	nBuffSize = 256;

	struct {
		DEVICE_ID	dID;
		byte		abOutBuff[nBuffSize];
	} dInfo;

	TraceEnter(_D(""), tlInternal);

	lpfnKIOC = (LPFNFUNCKIOC)GetFunctionAddress(FNKIOC);
	if (lpfnKIOC == NULL)
	{
		TraceInternal(_D("CWiVLicense::LicenseGetSerialNumber: Could not get address of KernelIoControl function"));
		TraceLeave(_D("CWiVLicense::LicenseGetSerialNumber"), (DWORD)blResult, tlInternal);
		return blResult;
	}

	dwDevInfo = SPI_GETOEMINFO;

	blResult = lpfnKIOC(IOCTL_HAL_GET_DEVICE_INFO, &dwDevInfo, 4, &bDevinfo, sizeof(bDevinfo), &dwOutBytes);
	if (!blResult) 
	{
		DWORD dwError = GetLastError();
		TraceInternal(_D("CWiVLicense::LicenseGetSerialNumber: IOCTL_HAL_GET_DEVICE_INFO failed, dwError = 0x%08X"), dwError);
	}
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseGetSerialNumber: SPI_GETOEMINFO, blResult = %d, dwOutBytes = %d, bDevinfo = <%s>"),
								blResult, dwOutBytes, (LPCTSTR)&bDevinfo);
	}

	memset(&bDevinfo, 0, sizeof(bDevinfo));
	dwDevInfo = SPI_GETPLATFORMTYPE;

	blResult = lpfnKIOC(IOCTL_HAL_GET_DEVICE_INFO, &dwDevInfo, 4, &bDevinfo, sizeof(bDevinfo), &dwOutBytes);
	if (!blResult) 
	{
		DWORD dwError = GetLastError();
		TraceInternal(_D("CWiVLicense::LicenseGetSerialNumber: IOCTL_HAL_GET_DEVICE_INFO failed, dwError = 0x%08X"), dwError);
	}
	else
	{
		TraceInternal(_D("CWiVLicense::LicenseGetSerialNumber: SPI_GETPLATFORMTYPE, blResult = %d, dwOutBytes = %d, bDevinfo = <%s>"),
								blResult, dwOutBytes, (LPCTSTR)&bDevinfo);
	}

	_zclr(lpszSerialNumber);
	memset(&dInfo, 0, sizeof(dInfo));
	dInfo.dID.dwSize = sizeof(dInfo);

//IOCTL_GET_IMEI_NUMBER
//#define IOCTL_HAL_GET_DEVICEID                  CTL_CODE(FILE_DEVICE_HAL, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)
	
	blResult = lpfnKIOC(IOCTL_HAL_GET_DEVICEID, &dInfo, sizeof(dInfo), &dInfo, nBuffSize, &dwOutBytes);
	if (!blResult)
	{
		DWORD dwError = GetLastError();
		TraceInternal(_D("CWiVLicense::LicenseGetSerialNumber: IOCTL_HAL_GET_DEVICEID failed, dwError = 0x%08X"), dwError);
		TraceLeave(_D("CWivLicense::LicenseGetSerialNumber"), (DWORD)blResult, tlInternal);
		return blResult;
	}

	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: dInfo = <%s>"), BtoS((LPBYTE)&dInfo, dwOutBytes));

//	byte x[256] = {0x28,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x0E,0x61,0x6D,0x6F,0xA8,0x41,0xF1,0xC1,0x48,0x00,0x00,0x00,0x00,0x50,0xBF,0x3F,0x51,0x73,0x00,0x00};
//	memcpy(&dInfo, x, dwOutBytes);
//	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: abOutBuff = <%s>"), BtoS((LPBYTE)&dInfo, dwOutBytes));

	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: dInfo.dID.dwSize             = %d"), dInfo.dID.dwSize);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: dInfo.dID.dwPresetIDOffset   = %d"), dInfo.dID.dwPresetIDOffset);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: dInfo.dID.dwPresetIDBytes    = %d"), dInfo.dID.dwPresetIDBytes);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: dInfo.dID.dwPlatformIDOffset = %d"), dInfo.dID.dwPlatformIDOffset);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: dInfo.dID.dwPlatformIDBytes  = %d"), dInfo.dID.dwPlatformIDBytes);

	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: PresetID   = <%s>"), BtoS(&dInfo.abOutBuff[dInfo.dID.dwPresetIDOffset - sizeof(DEVICE_ID)], dInfo.dID.dwPresetIDBytes));
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: PlatformID = <%s>"), BtoS(&dInfo.abOutBuff[dInfo.dID.dwPlatformIDOffset - sizeof(DEVICE_ID)], dInfo.dID.dwPlatformIDBytes));

	union {
		struct {
			__int64	i64Data;
			__int64	i64Data2;
		} stData;
		struct {
			unsigned int Spare1		: 4;
			unsigned int Check		: 4;
			unsigned int Serial		: 24;
			unsigned int TAC		: 8;
			unsigned int Type		: 16;
			unsigned int Country2b	: 4;
			unsigned int Country1b	: 4;

			unsigned int Country2a	: 4;
			unsigned int Country1a	: 4;
			unsigned int Spare2		: 8;
			unsigned int Spare3		: 16;
			unsigned int Spare4		: 32;
		} dDev;
	} uDev;

	memset(&uDev, 0, sizeof(uDev));

	memcpy(&uDev.stData, &dInfo.abOutBuff[dInfo.dID.dwPresetIDOffset - sizeof(DEVICE_ID)], dInfo.dID.dwPresetIDBytes);

	byte	bCountry1 = ((uDev.dDev.Country1a) * 16) + uDev.dDev.Country1b;
	byte	bCountry2 = ((uDev.dDev.Country2a) * 16) + uDev.dDev.Country2b;

	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: sizeof(uDev)       = %d"), sizeof(uDev));
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Spare1   = 0x%01X"), uDev.dDev.Spare1);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Check    = 0x%01X"), uDev.dDev.Check);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Serial   = 0x%06X"), uDev.dDev.Serial);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.TAC      = 0x%02X"), uDev.dDev.TAC);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Type     = 0x%04X"), uDev.dDev.Type);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Country1 = 0x%02X"), bCountry1);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Country2 = 0x%02X"), bCountry2);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Spare2   = 0x%02X"), uDev.dDev.Spare2);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Spare3   = 0x%04X"), uDev.dDev.Spare3);
	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: uDev.dDev.Spare4   = 0x%08X"), uDev.dDev.Spare4);

	_snwprintf(lpszSerialNumber, WIV_MAX_STRING, g_szFormatSerialNumber,
		bCountry1, uDev.dDev.Type, uDev.dDev.TAC, uDev.dDev.Serial, uDev.dDev.Check);

	TraceInternal(_D("CWivLicense::LicenseGetSerialNumber: SerialNumber = <%s>"), lpszSerialNumber);

	TraceLeave(_D("CWivLicense::LicenseGetSerialNumber"), (DWORD)blResult, tlInternal);
	return blResult;
}

void CWiVLicense::EncodeRILParams(DWORD &dwParams, DWORD &dwConfig)
{
	TraceEnter(_D("EncodeRILParams"), tlInternal);

	TraceInternal(_D("EncodeRILParams: On entry, dwParams = 0x%08X, dwConfig = 0x%08X"), dwParams, dwConfig);

	dwParams |= ((LicenseGetType() & WIV_PARAMS_TYPE_LMASK) << WIV_PARAMS_TYPE_LSHIFT);
	
	TraceInternal(_D("EncodeRILParams: License = <%s>"), BtoS((LPBYTE)LicenseGetValue(), sizeof(WIVLIC)));

	TraceInternal(_D("EncodeRILParams: Calling CalcDays"));
	dwParams |= ((CalcDays(CalcRemainingTime(LicenseGetType())) & WIV_PARAMS_EXPIRY_LMASK) << WIV_PARAMS_EXPIRY_LSHIFT);
	TraceInternal(_D("EncodeRILParams: Back from CalcDays, dwParams = 0x%08X"), dwParams);

	dwParams ^= dwConfig;
	TraceInternal(_D("EncodeRILParams: After XOR with dwConfig, dwParams = 0x%08X"), dwParams);

	TraceLeave(_D("EncodeRILParams"), tlInternal);
	return;
}

void CWiVLicense::DecodeRILParams(DWORD &dwParams, int &nType, int &nDays, DWORD &dwConfig)
{
	TraceEnter(_D("DecodeRILParams"), tlInternal);

	TraceInternal(_D("DecodeRILParams: dwParams = 0x%08X, dwConfig = 0x%08X"), dwParams, dwConfig);
	dwParams ^= dwConfig;
	TraceInternal(_D("DecodeRILParams: After XOR with dwConfig, dwParams = 0x%08X"), dwParams);

	nType = (dwParams & WIV_PARAMS_TYPE_RMASK) >> WIV_PARAMS_TYPE_RSHIFT;
	if (nType > (WIV_PARAMS_TYPE_LMASK - 4)) nType -= (WIV_PARAMS_TYPE_LMASK + 1);
	nDays = (dwParams & WIV_PARAMS_EXPIRY_RMASK) >> WIV_PARAMS_EXPIRY_RSHIFT;
	if (nDays >= WIV_PARAMS_EXPIRY_LMASK) nDays -= (WIV_PARAMS_TYPE_LMASK + 1);

	dwParams &= 0x00FFFFFF;
	TraceInternal(_D("DecodeRILParams: After decode, dwParams = 0x%08X, nType = %d, nDays = %d"), dwParams, nType, nDays);

	TraceLeave(_D("DecodeRILParams"), tlInternal);
	return;
}

//======================================================================
// DataCrypt - Data Encrypt/Decrypt
//======================================================================
BOOL CWiVLicense::DataCrypt(LPCTSTR lpszWith, const LPBYTE lpbIn, LPBYTE lpbOut, LPDWORD lpdwSize, bool bWay)
{
	
	HCRYPTPROV	hProv;
	HCRYPTHASH	hHash;
	HCRYPTKEY	hKey;
	DWORD		dwCount;
	BOOL		bResult = FALSE;
	BYTE		bB[WIV_MAX_BUFFER + 1];
	DWORD		dwResult;
	TCHAR		szW[WIV_MAX_NAME + 1];
	char		szWith[WIV_MAX_NAME + 1] = "SIGNALLEVEL";
	TCHAR		szTemp[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	TraceEnter(_D("CWiVLicense::DataCrypt"), tlInternal);

	if (!g_pSSSData)
	{
		TraceError(_D("CWiVLicense::DataCrypt: SSS data is NULL"));
		TraceLeave(_D("CWiVLicense::DataCrypt"), (DWORD)FALSE, tlInternal);
		return FALSE;
	}

	szW[0] = g_pSSSData->szCompanyName[2];		// V
	szW[1] = g_pSSSData->szProductName[8];		// u
	szW[2] = g_pSSSData->szCompanyURL[3];		// p

	dwCount = *lpdwSize;

	TraceInternal(_D("CWiVLicense::DataCrypt: lpszWith = <%s>, In buffer = <%s>"), lpszWith, BtoS(lpbIn, dwCount));

	TraceInternal(_D("CWiVLicense::DataCrypt: Copying %d bytes of data from in buffer to internal buffer"), dwCount);
	memcpy(bB, lpbIn, dwCount);

	szW[3] = g_pSSSData->szCompanyIdentity[13];	// a
	szW[4] = g_pSSSData->szProductName[2];		// M
	szW[5] = g_pSSSData->szCompanyName[4];		// T

	// Acquire a CSP
	bResult = AcquireContext(&hProv, NULL, GetCryptProviderName(szTemp), PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	if (!bResult)
	{
		dwResult = GetLastError();
		TraceInternal(_D("CWiVLicense::DataCrypt: AcquireContext returned = <%08x>"), dwResult);
	}

	szW[6] = g_pSSSData->szCustomFileHeader[1];	// i
	szW[7] = g_pSSSData->szProductShortName[3];	// r
	szW[8] = g_pSSSData->szCompanyIdentity[4];	// W

	// Create an MD5 hash object, to hash the password.
	bResult = CreateHash(hProv, CALG_MD5, 0, 0, &hHash);
	if (!bResult)
	{
		dwResult = GetLastError();
		TraceInternal(_D("CWiVLicense::DataCrypt: CreateHash returned = <%08x>"), dwResult);
	}

	szW[9] = g_pSSSData->szCompanySupport[7];	// @
	szW[10] = g_pSSSData->szProductName[15];		// c
	szW[11] = WIV_NULL_CHAR;

	if (lpszWith != NULL)
	{
		bResult = HashData(hHash, (PBYTE)lpszWith, (_tcslen(lpszWith) * sizeof(TCHAR)), 0);
	}
	else
	{
		UtoA(szW, szWith, _tcslen(szW));
		bResult = HashData(hHash, (PBYTE)szWith, strlen(szWith), 0);
	}

	if (!bResult)
	{
		dwResult = GetLastError();
		TraceInternal(_D("CWiVLicense::DataCrypt: HashData returned = <%08x>"), dwResult);
	}

	// Derive a 3DES session key from the password hash, to encrypt and decrypt data.
	bResult = DeriveKey(hProv, CALG_3DES, hHash, 0, &hKey);
	if (!bResult)
	{
		dwResult = GetLastError();
		TraceInternal(_D("CWiVLicense::DataCrypt: DeriveKey returned = <%08x>"), dwResult);
	}

	DWORD dwKeyLen = WIV_CRYPT_KEY_LEN;
	SetKeyParameters(hKey, KP_EFFECTIVE_KEYLEN, (BYTE*) &dwKeyLen, 0);

	if (bWay)
	{
		// Encrypt data
		bResult = Encrypt(hKey, 0, TRUE, 0, bB, &dwCount, sizeof(bB));

		if (!bResult)
		{
			dwResult = GetLastError();
			TraceInternal(_D("CWiVLicense::DataCrypt: Encrypt returned = <%08x>"), dwResult);
		}
		else
		{
			TraceInternal(_D("CWiVLicense::DataCrypt: Encrypt returned = TRUE"));
		}
	}
	else
	{
		// Decrypt data
		bResult = Decrypt(hKey, 0, TRUE, 0, bB, &dwCount);

		if (!bResult)
		{
			dwResult = GetLastError();
			TraceInternal(_D("CWiVLicense::DataCrypt: Decrypt returned = <%08x>"), dwResult);
		}
		else
		{
			TraceInternal(_D("CWiVLicense::DataCrypt: Decrypt returned TRUE"));
		}
	}

	// Destroy the hash object.
	if(hHash) DestroyHash(hHash);

	// Release the CSP handle.
	if(hProv) ReleaseContext(hProv,0);

	if (bResult)
	{
		TraceInternal(_D("CWiVLicense::DataCrypt: Copying %d bytes of data from internal buffer to out buffer"), dwCount);
		memcpy(lpbOut, bB, dwCount);

		TraceInternal(_D("CWiVLicense::DataCrypt: Out buffer = <%s>"), BtoS(lpbOut, dwCount));
		memcpy(lpbOut, bB, dwCount);
		*lpdwSize = dwCount;
		TraceLeave(_D("CWiVLicense::DataCrypt"), (DWORD)TRUE, tlInternal);
		return TRUE;
	}
	else
	{
		TraceLeave(_D("CWiVLicense::DataCrypt"), (DWORD)FALSE, tlInternal);
		return FALSE;
	}
}

// Acquire Context
BOOL CWiVLicense::AcquireContext(HCRYPTPROV *phProv, LPCWSTR szContainer, LPCWSTR szProvider, DWORD dwProvType, DWORD dwFlags)
{
	LPFNFUNCCAC		lpfnCAC;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::AcquireContext"), tlInternal);

	lpfnCAC = (LPFNFUNCCAC)GetFunctionAddress(FNCAC);
	if (lpfnCAC == NULL)
	{
		TraceInternal(_D("CWiVLicense::AcquireContext: Could not get address of CryptAcquireContext function"));
		TraceLeave(_D("CWiVLicense::AcquireContext"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCAC(phProv, szContainer, szProvider, dwProvType, dwFlags);

	TraceLeave(_D("CWiVLicense::AcquireContext"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Create Hash
BOOL CWiVLicense::CreateHash(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTKEY hKey, DWORD dwFlags, HCRYPTHASH *phHash)
{
	LPFNFUNCCCH		lpfnCCH;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::CreateHash"), tlInternal);

	lpfnCCH = (LPFNFUNCCCH)GetFunctionAddress(FNCCH);
	if (lpfnCCH == NULL)
	{
		TraceInternal(_D("CWiVLicense::CreateHash: Could not get address of CryptCreateHash function"));
		TraceLeave(_D("CWiVLicense::CreateHash"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCCH(hProv, Algid, hKey, dwFlags, phHash);

	TraceLeave(_D("CWiVLicense::CreateHash"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Hash Data
BOOL CWiVLicense::HashData(HCRYPTHASH hHash, CONST BYTE *pbData, DWORD dwDataLen, DWORD dwFlags)
{
	LPFNFUNCCHD		lpfnCHD;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::HashData"), tlInternal);

	lpfnCHD = (LPFNFUNCCHD)GetFunctionAddress(FNCHD);
	if (lpfnCHD == NULL)
	{
		TraceInternal(_D("CWiVLicense::HashData: Could not get address of CryptHashData function"));
		TraceLeave(_D("CWiVLicense::HashData"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCHD(hHash, pbData, dwDataLen, dwFlags);

	TraceLeave(_D("CWiVLicense::HashData"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Derive Key
BOOL CWiVLicense::DeriveKey(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTHASH hBaseData, DWORD dwFlags, HCRYPTKEY *phKey)
{
	LPFNFUNCCDK		lpfnCDK;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::DeriveKey"), tlInternal);

	lpfnCDK = (LPFNFUNCCDK)GetFunctionAddress(FNCDK);
	if (lpfnCDK == NULL)
	{
		TraceInternal(_D("CWiVLicense::DeriveKey: Could not get address of CryptDeriveKey function"));
		TraceLeave(_D("CWiVLicense::DeriveKey"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCDK(hProv, Algid, hBaseData, dwFlags, phKey);

	TraceLeave(_D("CWiVLicense::DeriveKey"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Set Key Parameters
BOOL CWiVLicense::SetKeyParameters(HCRYPTKEY hKey, DWORD dwParam, CONST BYTE *pbData, DWORD dwFlags)
{
	LPFNFUNCCSKP	lpfnCSKP;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::SetKeyParameters"), tlInternal);

	lpfnCSKP = (LPFNFUNCCSKP)GetFunctionAddress(FNCSKP);
	if (lpfnCSKP == NULL)
	{
		TraceInternal(_D("CWiVLicense::SetKeyParameters: Could not get address of CryptSetKeyParms function"));
		TraceLeave(_D("CWiVLicense::SetKeyParameters"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCSKP(hKey, dwParam, pbData, dwFlags);

	TraceLeave(_D("CWiVLicense::SetKeyParameters"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Destroy Hash
BOOL CWiVLicense::DestroyHash(HCRYPTHASH hHash)
{
	LPFNFUNCCDH		lpfnCDH;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::DestroyHash"), tlInternal);

	lpfnCDH = (LPFNFUNCCDH)GetFunctionAddress(FNCDH);
	if (lpfnCDH == NULL)
	{
		TraceInternal(_D("CWiVLicense::DestroyHash: Could not get address of CryptDestroyHash function"));
		TraceLeave(_D("CWiVLicense::DestroyHash"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCDH(hHash);

	TraceLeave(_D("CWiVLicense::DestroyHash"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Release Context
BOOL CWiVLicense::ReleaseContext(HCRYPTPROV hProv, DWORD dwFlags)
{
	LPFNFUNCCRC		lpfnCRC;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::ReleaseContext"), tlInternal);

	lpfnCRC = (LPFNFUNCCRC)GetFunctionAddress(FNCRC);
	if (lpfnCRC == NULL)
	{
		TraceInternal(_D("CWiVLicense::ReleaseContext: Could not get address of CryptReleaseContext function"));
		TraceLeave(_D("CWiVLicense::ReleaseContext"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCRC(hProv, dwFlags);

	TraceLeave(_D("CWiVLicense::ReleaseContext"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Decrypt
BOOL CWiVLicense::Decrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, BYTE *pbData, DWORD *pdwDataLen)
{
	LPFNFUNCCD		lpfnCD;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::Decrypt"), tlInternal);

	lpfnCD = (LPFNFUNCCD)GetFunctionAddress(FNCD);
	if (lpfnCD == NULL)
	{
		TraceInternal(_D("CWiVLicense::Decrypt: Could not get address of CryptDecryptt function"));
		TraceLeave(_D("CWiVLicense::Decrypt"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCD(hKey, hHash, Final, dwFlags, pbData, pdwDataLen);

	TraceLeave(_D("CWiVLicense::Decrypt"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Encrypt
BOOL CWiVLicense::Encrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, BYTE *pbData, DWORD *pdwDataLen, DWORD dwBufLen)
{
	LPFNFUNCCE		lpfnCE;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::Encrypt"), tlInternal);

	lpfnCE = (LPFNFUNCCE)GetFunctionAddress(FNCE);
	if (lpfnCE == NULL)
	{
		TraceInternal(_D("CWiVLicense::Encrypt: Could not get address of CryptEncrypt function"));
		TraceLeave(_D("CWiVLicense::Encrypt"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnCE(hKey, hHash, Final, dwFlags, pbData, pdwDataLen, dwBufLen);

	TraceLeave(_D("CWiVLicense::Encrypt"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Device IO Control
BOOL CWiVLicense::DeviceIO(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize,
		LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
	LPFNFUNCDIOC	lpfnDIOC;
	BOOL			bResult = FALSE;

	TraceEnter(_D("CWiVLicense::DeviceIO"), tlInternal);

	lpfnDIOC = (LPFNFUNCDIOC)GetFunctionAddress(FNDIOC);
	if (lpfnDIOC == NULL)
	{
		TraceInternal(_D("CWiVLicense::DeviceIO: Could not get address of DeviceIoControl function"));
		TraceLeave(_D("CWiVLicense::DeviceIO"), (DWORD)bResult, tlInternal);
		return bResult;
	}

	bResult = lpfnDIOC(hDevice, dwIoControlCode, lpInBuf, nInBufSize,
						lpOutBuf, nOutBufSize, lpBytesReturned, lpOverlapped);

	TraceLeave(_D("CWiVLicense::DeviceIO"), (DWORD)bResult, tlInternal);
	return bResult;
}

// Get Core DLL name
LPCTSTR CWiVLicense::GetCoreDLLName(LPTSTR szUnicode)
{
	BYTE	bBuf[] = {0xA5,0xE6,0xEA,0xF7,0xE0,0xE1,0xE9,0xE9,0x95};

	TraceEnter(_D("CWiVLicense::GetCoreDLLName"), tlInternal);

	for (int i = 1; i < (int)sizeof(bBuf) - 1; i++)
		bBuf[i] ^= bBuf[0];

	bBuf[sizeof(bBuf) - 1] = 0x00;

	TraceInternal(_D("CWiVLicense::GetCoreDLLName: sizeof string = %d, bBuf = <%S>"), strlen((char *)&bBuf[1]), (char *)&bBuf[1]);

	AtoU((char *)&bBuf[1], szUnicode);

	TraceInternal(_D("CWiVLicense::GetCoreDLLName: szUnicode = <%s>"),szUnicode);

	TraceLeave(_D("CWiVLicense::GetCoreDLLName"), szUnicode, tlInternal);

	return szUnicode;

}

// Get Core DLL Handle
HMODULE CWiVLicense::GetCoreDLLHandle()
{
	TCHAR	szTemp[WIV_MAX_NAME + 1] = WIV_EMPTY_STRING;
	TraceEnter(_D("CWiVLicense::GetCoreDLLHandle"), tlInternal);
	TraceLeave(_D("CWiVLicense::GetCoreDLLHandle"), tlInternal);
	return GetModuleHandle(GetCoreDLLName(szTemp));
}

// Get function address
FARPROC CWiVLicense::GetFunctionAddress(LPCWSTR lpszFunction)
{
	HINSTANCE	hDLL;
	FARPROC		fp = NULL;

	TraceEnter(_D("CWiVLicense::GetFunctionAddress"), tlInternal);

	hDLL = GetCoreDLLHandle();
	if (hDLL == NULL)
	{
		TraceInternal(_D("CWiVLicense::GetFunctionAddress: Could not get handle to CoreDll.dll"));
		TraceLeave(_D("CWiVLicense::GetFunctionAddress"), DWORD(fp), tlInternal);
		return fp;
	}

	fp = GetProcAddress(hDLL, lpszFunction);
	if (fp == NULL)
	{
		TraceError(_D("CWiVLicense::GetFunctionAddress: Could not address of requested function (0x%08X)"), lpszFunction);
		TraceLeave(_D("CWiVLicense::GetFunctionAddress"), DWORD(fp), tlInternal);
		return fp;
	}

	TraceLeave(_D("CWiVLicense::GetFunctionAddress"), DWORD(fp), tlInternal);
	return fp;
}

// Get Cryptography Provider name
LPCTSTR CWiVLicense::GetCryptProviderName(LPTSTR szUnicode)
{
	BYTE	bBuf[] = {0xA5,0xE8,0xCC,0xC6,0xD7,0xCA,0xD6,0xCA,
					  0xC3,0xD1,0x85,0xE0,0xCB,0xCD,0xC4,0xCB,
					  0xC6,0xC0,0xC1,0x85,0xE6,0xD7,0xDC,0xD5,
					  0xD1,0xCA,0xC2,0xD7,0xC4,0xD5,0xCD,0xCC,
					  0xC6,0x85,0xF5,0xD7,0xCA,0xD3,0xCC,0xC1,
					  0xC0,0xD7,0x85,0xD3,0x94,0x8B,0x95,0x76};

	TraceEnter(_D("CWiVLicense::GetCryptProviderName"), tlInternal);

	for (int i = 1; i < (int)sizeof(bBuf) - 1; i++)
		bBuf[i] ^= bBuf[0];

	bBuf[sizeof(bBuf) - 1] = 0x00;

	TraceInternal(_D("CWiVLicense::GetCryptProviderName: sizeof string = %d, bBuf = <%S>"), strlen((char *)&bBuf[1]), (char *)&bBuf[1]);

	AtoU((char *)&bBuf[1], szUnicode);

	TraceInternal(_D("CWiVLicense::GetCryptProviderName: szUnicode = <%s>"),szUnicode);

	TraceLeave(_D("CWiVLicense::GetCryptProviderName"), szUnicode, tlInternal);

	return szUnicode;

}

int CWiVLicense::GetSuspendTicks()
{
	if (!m_pLicenseData)
	{
		TraceError(_D("CWiVLicense::GetSuspendTicks: License data is NULL"));
		return 0;
	}

	int nSuspendTicks = m_pLicenseData->nSuspendTicks;
	m_pLicenseData->nSuspendTicks = 0;
	return nSuspendTicks;
}


//======================================================================
// Global non-member functions
//======================================================================
void GenLicenses(LPTSTR lpszSignal)
{
	TraceEnter(_D("GenLicenses"), tlInternal);
	m_pThis->LicenseGenLicenses(lpszSignal);
	TraceLeave(_D("GenLicenses"), tlInternal);
	return;
}

LPWIVLIC GetLicense()
{
	LPWIVLIC lpRetVal;

	TraceEnter(_D("GetLicense"), tlInternal);
	lpRetVal = m_pThis->LicenseGetValue();
	TraceLeave(_D("GetLicense"), (DWORD)lpRetVal, tlInternal);
	return lpRetVal;
}

void SetLicense(LPWIVLIC lpLic, const bool blPending)
{
	TraceEnter(_D("SetLicense"), tlInternal);
	if (lpLic != NULL)
	{
		TraceInternal(_D("SetLicense: Setting License value to <%s>"), BtoS((LPBYTE)lpLic, sizeof(WIVLIC)));
		m_pThis->LicenseSetValue(lpLic, blPending);
	}

	TraceLeave(_D("SetLicense"), tlInternal);
	return;
}

int CalcLicenseType(LPWIVLIC lpLic)
{
	int nRetVal;

	TraceEnter(_D("CalcLicenseType"), tlInternal);
	nRetVal = m_pThis->LicenseCalcType(lpLic);
	TraceLeave(_D("CalcLicenseType"), nRetVal, tlInternal);
	return nRetVal;
}

int GetLicenseType()
{
	int nRetVal;

	TraceEnter(_D("GetLicenseType"), tlInternal);
	nRetVal = m_pThis->LicenseGetType();
	TraceLeave(_D("GetLicenseType"), nRetVal, tlInternal);
	return nRetVal;
}

void SetLicenseType(const int nType)
{
	TraceEnter(_D("SetLicenseType"), tlInternal);
	m_pThis->LicenseSetType(nType);
	TraceLeave(_D("SetLicenseType"), tlInternal);
	return;
}

DWORD ReadLicense()
{
	DWORD dwRetVal;

	TraceEnter(_D("ReadLicense"), tlInternal);
	dwRetVal = m_pThis->LicenseCodeRead();
	TraceLeave(_D("ReadLicense"), dwRetVal, tlInternal);
	return dwRetVal;
}

DWORD WriteLicense()
{
	DWORD dwRetVal;

	TraceEnter(_D("WriteLicense"), tlInternal);
	dwRetVal = m_pThis->LicenseCodeWrite();
	TraceLeave(_D("WriteLicense"), dwRetVal, tlInternal);
	return dwRetVal;
}

int CalcDays(const int nTime)
{
	int nRetVal;

	TraceEnter(_D("CalcDays"), tlInternal);
	nRetVal = m_pThis->LicenseCalcDays(nTime);
	TraceLeave(_D("CalcDays"), nRetVal, tlInternal);
	return nRetVal;
}

USHORT CalcExpiryDay(const int nRemaining)
{
	USHORT usRetVal;

	TraceEnter(_D("CalcExpiryDay"), tlInternal);
	usRetVal = m_pThis->LicenseCalcExpiryDay(nRemaining);
	TraceLeave(_D("CalcExpiryDay"), (DWORD)usRetVal, tlInternal);
	return usRetVal;
}

int CalcRemainingTime(const UCHAR uchType, const int nUsedTime)
{
	int nRetVal;

	TraceEnter(_D("GetRemainingTime"), tlInternal);
	nRetVal = m_pThis->LicenseCalcRemainingTime(uchType);
	TraceLeave(_D("GetRemainingTime"), nRetVal, tlInternal);
	return nRetVal;
}

int CalcElapsedTime(const UCHAR uchType, const int nRemaining)
{
	int nRetVal;

	TraceEnter(_D("CalcElapsedTime"), tlInternal);
	nRetVal = m_pThis->LicenseCalcElapsedTime(uchType, nRemaining);
	TraceLeave(_D("CalcElapsedTime"), nRetVal, tlInternal);
	return nRetVal;
}

DWORD GetRandom(short sIndex, short sLower, short sUpper)
{
	DWORD dwRetVal;

	TraceEnter(_D("GetRandom"), tlInternal);
	dwRetVal = m_pThis->LicenseGetRandom(sIndex, sLower, sUpper);
	TraceLeave(_D("GetRandom"), dwRetVal, tlInternal);
	return dwRetVal;
}

DWORD CheckLicense(const int nDisk, LPWIVLIC lpLicense)
{
	DWORD dwRetVal;

	TraceEnter(_D("CheckLicense"), tlInternal);
	TraceInternal(_D("CheckLicense: Calling m_pThis->LicenseSourcesCheck (<%s>)"), BtoS((LPBYTE)lpLicense, sizeof(WIVLIC)));
	dwRetVal = m_pThis->LicenseSourcesCheck(nDisk, lpLicense);
	TraceLeave(_D("CheckLicense"), dwRetVal, tlInternal);
	return dwRetVal;
}
 
LPVOID UpdateSources(TODAYLISTITEM *pTodayListItem, LPDWORD lpdwParam)
{
	LPVOID lpvRetVal;
	TraceEnter(_D("UpdateSources"), tlInternal);
	lpvRetVal = m_pThis->LicenseElapsedUpdate(pTodayListItem, lpdwParam);
	TraceLeave(_D("UpdateSources"), (DWORD)lpvRetVal, tlInternal);
	return lpvRetVal;
}

LPCTSTR GetIMEI()
{
	LPCTSTR	lpRetVal;
	TraceEnter(_D("GetIMEI"), tlInternal);
	lpRetVal = m_pThis->LicenseGetIMEI();
	TraceLeave(_D("GetIMEI"), (DWORD)lpRetVal, tlInternal);
	return lpRetVal;
}

BOOL GetSerialNumber(LPTSTR lpszSerialNumber)
{
	BOOL blRetVal;
	TraceEnter(_D("GetSerialNumber"), tlInternal);
	blRetVal = m_pThis->LicenseGetSerialNumber(lpszSerialNumber);
	TraceLeave(_D("GetSerialNumber"), (DWORD)blRetVal, tlInternal);
	return blRetVal;
}

void EncodeParams(DWORD &dwParams, DWORD &dwConfig)
{
	TraceEnter(_D("EncodeParams"), tlInternal);
	m_pThis->EncodeRILParams(dwParams, dwConfig);
	TraceLeave(_D("EncodeParams"), tlInternal);
	return;
}

void DecodeParams(DWORD &dwParams, int &nType, int &nDays, DWORD &dwConfig)
{
	TraceEnter(_D("DecodeParams"), tlInternal);
	m_pThis->DecodeRILParams(dwParams, nType, nDays, dwConfig);
	TraceLeave(_D("DecodeParams"), tlInternal);
	return;
}

BOOL Crypt(LPCTSTR lpszWith, const LPBYTE lpbIn, LPBYTE lpbOut, LPDWORD lpdwSize, bool bWay)
{
	BOOL blRetVal;

	TraceEnter(_D("Crypt"), tlInternal);
	blRetVal = m_pThis->DataCrypt(lpszWith, lpbIn, lpbOut, lpdwSize, bWay);
	TraceLeave(_D("Crypt"), blRetVal, tlInternal);
	return blRetVal;
}

void EnablePowerNotifications()
{
	LPFNFUNCCRMQ	lpfnCRMQ;
	LPFNFUNCRPN		lpfnRPN;
	LPFNFUNCCT		lpfnCT;
	MSGQUEUEOPTIONS msgqOptions = {0};

	TraceEnter(_D("EnablePowerNotifications"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("EnablePowerNotifications: License data is NULL"));
		TraceLeave(_D("EnablePowerNotifications"), tlInternal);
		return;
	}

	lpfnCRMQ = (LPFNFUNCCRMQ)m_pThis->GetFunctionAddress(FNCRMQ);
	if (lpfnCRMQ == NULL)
	{
		TraceError(_D("EnablePowerNotifications: Could not get address of CreateMsgQueue function"));
		TraceLeave(_D("EnablePowerNotifications"), tlInternal);
		return;
	}

	lpfnRPN = (LPFNFUNCRPN)m_pThis->GetFunctionAddress(FNRPN);
	if (lpfnRPN == NULL)
	{
		TraceError(_D("EnablePowerNotifications: Could not get address of RequestPowerNotifications function"));
		TraceLeave(_D("EnablePowerNotifications"), tlInternal);
		return;
	}

	lpfnCT = (LPFNFUNCCT)m_pThis->GetFunctionAddress(FNCT);
	if (lpfnCT == NULL)
	{
		TraceError(_D("EnablePowerNotifications: Could not get address of CreateThread function"));
		TraceLeave(_D("EnablePowerNotifications"), tlInternal);
		return;
	}

	m_pLicenseData->blKeepGoing = true;
	
	m_pLicenseData->nBufferSize = (MAX_PATH+sizeof(POWER_BROADCAST))*MAX_MESSAGES;
	m_pLicenseData->lpbBuffer = new BYTE[m_pLicenseData->nBufferSize];

    msgqOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    msgqOptions.dwFlags = 0;
    msgqOptions.dwMaxMessages = MAX_MESSAGES;
    msgqOptions.cbMaxMessage = sizeof(POWER_BROADCAST) + MAX_PATH;
    msgqOptions.bReadAccess = TRUE;

	m_pLicenseData->htMQueue = lpfnCRMQ(g_szProductShortName, &msgqOptions);
	m_pLicenseData->hnwrNoti = lpfnRPN(m_pLicenseData->htMQueue, PBT_RESUME | PBT_TRANSITION);
	m_pLicenseData->htPowerThread = lpfnCT(NULL, 0, PowerNotificationsThread, 0, NULL, &m_pLicenseData->dwThreadID);
	TraceInternal(_D("EnablePowerNotifications: htMQueue = 0x%08X, hnwrNoti = 0x%08X, htPowerThread = 0x%08X, dwThreadID = 0x%08X"),
					m_pLicenseData->htMQueue, m_pLicenseData->hnwrNoti, m_pLicenseData->htPowerThread, m_pLicenseData->dwThreadID);

	TraceLeave(_D("EnablePowerNotifications"), tlInternal);
	return;
}

void DisablePowerNotifications()
{
	LPFNFUNCSPN		lpfnSPN;
	LPFNFUNCCLMQ	lpfnCLMQ;
	LPFNFUNCTT		lpfnTT;

	TraceEnter(_D("DisablePowerNotifications"), tlInternal);

	if (!m_pLicenseData)
	{
		TraceError(_D("DisablePowerNotifications: License data is NULL"));
		TraceLeave(_D("DisablePowerNotifications"), tlInternal);
		return;
	}

	m_pLicenseData->blKeepGoing = false;

	lpfnSPN = (LPFNFUNCSPN)m_pThis->GetFunctionAddress(FNSPN);
	if (lpfnSPN == NULL)
	{
		TraceError(_D("DisablePowerNotifications: Could not get address of StopPowerNotifications function"));
		TraceLeave(_D("DisablePowerNotifications"), tlInternal);
		return;
	}

	lpfnCLMQ = (LPFNFUNCCLMQ)m_pThis->GetFunctionAddress(FNCLMQ);
	if (lpfnCLMQ == NULL)
	{
		TraceError(_D("DisablePowerNotifications: Could not get address of CloseMsgQueue function"));
		TraceLeave(_D("DisablePowerNotifications"), tlInternal);
		return;
	}

	lpfnTT = (LPFNFUNCTT)m_pThis->GetFunctionAddress(FNTT);
	if (lpfnTT == NULL)
	{
		TraceError(_D("DisablePowerNotifications: Could not get address of TerminateThread function"));
		TraceLeave(_D("DisablePowerNotifications"), tlInternal);
		return;
	}

	TraceInternal(_D("DisablePowerNotifications: htMQueue = 0x%08X, hnwrNoti = 0x%08X, htPowerThread = 0x%08X, dwThreadID = 0x%08X"),
		m_pLicenseData->htMQueue, m_pLicenseData->hnwrNoti, m_pLicenseData->htPowerThread, m_pLicenseData->dwThreadID);

	if (m_pLicenseData->htPowerThread)
	{
		if (!IsSecondEdition())
		{
			DWORD	dwStatus = 0;
			HANDLE	hT = m_pLicenseData->htPowerThread;

			if (!lpfnTT(hT, dwStatus))
			{
				DWORD dwError = GetLastError();
				TraceError(_D("DisablePowerNotifications: TerminateThread failed, error = %d (0x%08X)"), dwError, dwError);
			}
		}

		CloseHandle(m_pLicenseData->htPowerThread);
		m_pLicenseData->htPowerThread = NULL;
	}

	if (m_pLicenseData->lpbBuffer)
	{
		delete m_pLicenseData->lpbBuffer;
		m_pLicenseData->lpbBuffer = NULL;
	}

	if (m_pLicenseData->hnwrNoti)
	{
		lpfnSPN(m_pLicenseData->hnwrNoti);
		m_pLicenseData->hnwrNoti = NULL;
	}
	
	if (m_pLicenseData->htMQueue)
	{
		lpfnCLMQ(m_pLicenseData->htMQueue);
		m_pLicenseData->htMQueue = NULL;
	}
	
	TraceLeave(_D("DisablePowerNotifications"), tlInternal);
	return;
}

DWORD WINAPI PowerNotificationsThread(LPVOID lpParms)
{
	LPFNFUNCWSO		lpfnWSO;
	LPFNFUNCRMQ		lpfnRMQ;
	LPFNFUNCGST		lpfnGST;
	LPFNFUNCSTTFT	lpfnSTTFT;
	LPFNFUNCET		lpfnET;
	DWORD			dwRead;
	DWORD			dwFlags;
	PPOWER_BROADCAST pPwrInfo;
	SYSTEMTIME		stNow;
	static __int64	i64Suspend = 0;
	static __int64	i64Resume = 0;


	if (!m_pLicenseData)
	{
		TraceError(_D("PowerNotificationsThread: License data is NULL"));
		return -1;
	}

	lpfnWSO = (LPFNFUNCWSO)m_pThis->GetFunctionAddress(FNWSO);
	if (lpfnWSO == NULL)
	{
		TraceInternal(_D("PowerNotificationsThread: Could not get address of WaitForSingleObject function"));
		return -1;
	}

	lpfnRMQ = (LPFNFUNCRMQ)m_pThis->GetFunctionAddress(FNRMQ);
	if (lpfnRMQ == NULL)
	{
		TraceInternal(_D("PowerNotificationsThread: Could not get address of ReadMsgQueue function"));
		return -1;
	}

	lpfnGST = (LPFNFUNCGST)m_pThis->GetFunctionAddress(FNGST);
	if (lpfnGST == NULL)
	{
		TraceInternal(_D("PowerNotificationsThread: Could not get address of GetSystemTime function"));
		return -1;
	}

	lpfnSTTFT = (LPFNFUNCSTTFT)m_pThis->GetFunctionAddress(FNSTTFT);
	if (lpfnSTTFT == NULL)
	{
		TraceInternal(_D("PowerNotificationsThread: Could not get address of SystemTimeToFileTime function"));
		return -1;
	}

	lpfnET = (LPFNFUNCET)m_pThis->GetFunctionAddress(FNET);
	if (lpfnET == NULL)
	{
		TraceInternal(_D("PowerNotificationsThread: Could not get address of ExitThread function"));
		return -1;
	}

	while (TRUE)//m_pLicenseData->blKeepGoing)
	{
		if (m_pLicenseData == NULL) lpfnET(0);

        memset(m_pLicenseData->lpbBuffer, 0, m_pLicenseData->nBufferSize);

		lpfnWSO(m_pLicenseData->htMQueue, INFINITE);

		if (m_pLicenseData->htMQueue == NULL) lpfnET(0);
		if (m_pLicenseData->lpbBuffer == NULL) lpfnET(0);

		if (lpfnRMQ(m_pLicenseData->htMQueue, m_pLicenseData->lpbBuffer, m_pLicenseData->nBufferSize, &dwRead, MAX_MESSAGES, &dwFlags))
		{
			if (dwRead >= sizeof(POWER_BROADCAST))
			{
				pPwrInfo = (POWER_BROADCAST*)m_pLicenseData->lpbBuffer;

				if (pPwrInfo->Message & PBT_TRANSITION)
				{
					if ( (pPwrInfo->Flags & POWER_STATE_SUSPEND) || (pPwrInfo->Flags & POWER_STATE_CRITICAL) )
					{
						TraceInternal(WIV_EMPTY_STRING);
						TraceInternal(_D("PowerNotificationsThread: pPwrInfo->Message    = PBT_TRANSITION"));
						lpfnGST(&stNow);
						lpfnSTTFT(&stNow, (FILETIME *)&i64Suspend);
						TraceInternal(_D("PowerNotificationsThread: PBT_TRANSITION, i64Suspend = 0x%016I64X"), i64Suspend);
					}
				}
				if (pPwrInfo->Message & PBT_RESUME)
				{
					TraceInternal(WIV_EMPTY_STRING);
					TraceInternal(_D("PowerNotificationsThread: pPwrInfo->Message    = PBT_RESUME"));
					lpfnGST(&stNow);
					lpfnSTTFT(&stNow, (FILETIME *)&i64Resume);
					TraceInternal(_D("PowerNotificationsThread: PBT_RESUME, i64Suspend = 0x%016I64X"), i64Suspend);
					TraceInternal(_D("PowerNotificationsThread: PBT_RESUME, i64Resume  = 0x%016I64X"), i64Resume);
					m_pLicenseData->nSuspendTicks = (int)((i64Resume-i64Suspend)/10000000)*WIV_TICKS_PER_SECOND;
					TraceInternal(_D("PowerNotificationsThread: PBT_RESUME, m_pLicenseData->nSuspendTicks = %d"), m_pLicenseData->nSuspendTicks);
				}
			}
		}
	} 

//	CloseHandle(m_pLicenseData->htPowerThread);
//	m_pLicenseData->htPowerThread = NULL;
//	return 0;
	lpfnET(0);
}

int LicenseRegisterNotification(LPFNNOTIFY lpfnNotify)
{
	TraceEnter(_D("LicenseRegisterNotification"), tlInternal);

	for (int i = 0; i < ARRAYSIZE(m_fnClients); i++)
	{
		if (m_fnClients[i] == NULL)
		{
			m_fnClients[i] = lpfnNotify;
			m_nClientCount += 1;
			break;
		}
	}

	TraceInternal(_D("LicenseRegisterNotification: m_nClientCount = %d,  client = 0x%08X"), m_nClientCount, m_fnClients[m_nClientCount - 1]);
	
	TraceLeave(_D("LicenseRegisterNotification"), m_nClientCount, tlInternal);
	return m_nClientCount;
}

int LicenseDeregisterNotification(LPFNNOTIFY lpfnNotify)
{
	TraceEnter(_D("LicenseDeregisterNotification"), tlInternal);

	TraceInternal(_D("LicenseDeregisterNotification, lpfnNotify = 0x%08X"), lpfnNotify);

	for (int i = 0; i < ARRAYSIZE(m_fnClients); i++)
	{
		if (m_fnClients[i] == lpfnNotify)
		{
			m_fnClients[i] = NULL;
			m_nClientCount -= 1;
			break;
		}
	}

	TraceInternal(_D("LicenseDeregisterNotification: m_nClientCount = %d"), m_nClientCount);
	
	TraceLeave(_D("LicenseDeregisterNotification"), m_nClientCount, tlInternal);
	return m_nClientCount;
}


} // namespace WiV
