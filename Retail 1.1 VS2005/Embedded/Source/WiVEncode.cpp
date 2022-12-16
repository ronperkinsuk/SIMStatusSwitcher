////////////////////////////////////////////////////////////////////////
//                                                                    //
// WiVEncode.cpp: Implementation of WiViT Global Data Encryption.     //
//                                                                    //
// Create a 'Special' build configuration based on the Debug one.     //
// Exlude this file from build for any other configurations except    //
// Special.  Add preprocessor definition "WIV_ENCRYPT_BLD" in the     //
// Special build settings.                                            //
//                                                                    //
// Add the following function definition to one of your project's     //
// header files:                                                      //
//                                                                    //
//    #ifdef WIV_ENCRYPT_BLD                                          //
//    bool WiVGenEncryptedData();                                     //
//    #endif                                                          //
//                                                                    //
// To generate the encrypted global data code, add the following      //
// lines somewhere in your project:                                   //
//                                                                    //
//    #ifdef WIV_ENCRYPT_BLD                                          //
//        WiVGenEncryptedData();                                      //
//    #endif                                                          //
//                                                                    //
// This will produce a Trace file on the device which should be       //
// copied to the PC. Note that if there is a lot of global data, the  //
// device may seem to not respond for up to 15-30 seconds. This is    //
// quite normal.                                                      //
//                                                                    //
// Copy the code from the indicated area in the generate file to an   //
// appropriate place within your project source.                      //
// File .                                                             //
//                                                                    //
// Remember to change the project build settings back from Special to //
// Release or Debug to build the Application.                         //
//                                                                    //
////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <wincrypt.h>
#include <todaycmn.h>

extern struct SSS_GLOBALS *SSSGlobals;

#include "WiVDefs.h"
#include "WiVUtils.h"
#include "WiVLicense.h"
#include "WiVTrace.h"

namespace WiV
{

int	WiVInitMaps();
int	WiVInitStrings();
int	WiVInitDWords();
int	WiVInitOther();

#pragma pack(1)
struct SSS_Special
{
//	DWORD	dwMapSize;
//	BYTE	bMapData[128*sizeof(char)];
	DWORD	dwNameSize;
	BYTE	bNameData[16*sizeof(TCHAR)];
	DWORD	dwRandSize;
	BYTE	bRandData[16*sizeof(DWORD)];
	DWORD	dwNumStrings;
	DWORD	dwStringsSize;
	BYTE	bStringData[2048];
	DWORD	dwNumDWords;
	DWORD	dwDWordsSize;
	BYTE	bDWordData[2048];
	DWORD	dwNumOther;
	DWORD	dwOtherSize;
	BYTE	bOtherData[1024];
} static *pm_SSS_Special;
#pragma pack()



bool WiVGenEncryptedData()
{
	TraceEnter(_D("WiVEncode::WiVGenEncryptedData"), tlInternal);

	int		nMapsSize;
	int		nStringsSize;
	int		nDWordsSize;
	int		nOtherSize;
	int		nOpenBufferIndex;
	int		nEncryptedBufferIndex;
	int		nDefsPerLine;
	int		nDefNum;
	int		nTotalDefs;

	TCHAR	szPad[2];
	LPBYTE	pbOpenBuf = NULL;
	LPBYTE	pbEncryptedBuf = NULL;
	DWORD	dwOpenSize;
	DWORD	dwEncryptedSize;

	TraceInternal(_D("WiVGenEncryptedData: Setting up buffer"));

	// Allocate zero initialised structure memory
	TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
				  _D("Allocating %d bytes of memory for structure"), sizeof(SSS_Special));

	pm_SSS_Special = (struct SSS_Special *)LocalAlloc (LPTR, sizeof(SSS_Special));
	if (!pm_SSS_Special)
	{
		TraceError(_D("WiVEncode::WiVGenEncryptedData: ")
						_D("Could not allocate memory for structure"));
		TraceLeave(_D("WiVEncode::WiVGenEncryptedData"), (DWORD)false, tlInternal);
		return false;
	}

	TraceCodeStart(_D("====== Generated Code Start ===================="));

//===========================================
// Start of Code Generation
//===========================================

	TraceCode(_D("\r\nCopy and paste code below between similarly marked ")
					_D("separators in SSSGlobals.h\r\n"));

	TraceCode(_D("\r\n//------------- begin paste special (SSSGlobals.h) ")
						_D("---------------\r\n"));

	// Clear buffer first
//	memset(pm_SSS_Special, 0, sizeof(SSS_Special));

////////////////////////////
// Initialize data map
////////////////////////////
	
	nMapsSize = WiVInitMaps();

////////////////////////////
// Initialize strings
////////////////////////////
	
	nStringsSize = WiVInitStrings();

////////////////////////////
// Initialize DWORDS
////////////////////////////
	
	nDWordsSize = WiVInitDWords();

////////////////////////////
// Initialize Other Values
////////////////////////////
	
	nOtherSize = WiVInitOther();

	// Calculate buffer size required
//	dwOpenSize		= pm_SSS_Special->dwMapSize
//					+ pm_SSS_Special->dwNameSize
	dwOpenSize		= pm_SSS_Special->dwNameSize
					+ pm_SSS_Special->dwRandSize
					+ pm_SSS_Special->dwStringsSize
					+ pm_SSS_Special->dwDWordsSize
					+ pm_SSS_Special->dwOtherSize
					+ (8 * sizeof(DWORD));

	dwEncryptedSize = dwOpenSize + 64;

	// Free any existing file buffer.
	if (pbOpenBuf)
	{
		TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
						_D("Freeing existing open memory buffer"));
		LocalFree (pbOpenBuf);
	}

	if (pbEncryptedBuf)
	{
		TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
						_D("Freeing existing encrypted memory buffer"));
		LocalFree (pbEncryptedBuf);
	}

	// Allocate zero initialised buffers.
	TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
					_D("Allocating %d bytes of memory for open buffer"), dwOpenSize);
	pbOpenBuf = (PBYTE)LocalAlloc (LPTR, dwOpenSize);
	if (!pbOpenBuf)
	{
		TraceError(_D("WiVEncode::WiVGenEncryptedData: ")
						_D("Could not allocate memory for open buffer"));

		// Free the structure buffer
		if (pm_SSS_Special)
		{
			TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
							_D("Freeing existing structure buffer"));
			LocalFree (pm_SSS_Special);
		}

		TraceLeave(_D("WiVEncode::WiVGenEncryptedData"), (DWORD)false, tlInternal);
		return false;
	}

	TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
				_D("Allocating %d bytes of zero initialised memory for encrypted buffer"), dwEncryptedSize);
	pbEncryptedBuf = (PBYTE)LocalAlloc (LPTR, dwEncryptedSize);
	if (!pbEncryptedBuf)
	{
		TraceError(_D("WiVEncode::WiVGenEncryptedData: ")
					_D("Could not allocate memory for encrypted buffer"));

		// Free the structure buffer
		if (pm_SSS_Special)
		{
			TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
					_D("Freeing existing structure buffer"));
			LocalFree (pm_SSS_Special);
		}

		// Free the open buffer
		if (pbOpenBuf)
		{
			TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
					_D("Freeing existing open memory buffer"));
			LocalFree (pbOpenBuf);
		}

		TraceLeave(_D("WiVEncode::WiVGenEncryptedData"), (DWORD)false, tlInternal);
		return false;
	}

	// Initialize the open buffer.
//	memset(pbOpenBuf, 0, dwOpenSize);

	// Transfer data to buffer
	nOpenBufferIndex = 0;

	// License Data Map
//	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwMapSize, sizeof(DWORD));
//	nOpenBufferIndex += sizeof(DWORD);

//	if (pm_SSS_Special->dwMapSize > 0)
//	{
//		memcpy(&pbOpenBuf[nOpenBufferIndex],
//				pm_SSS_Special->bMapData,
//				pm_SSS_Special->dwMapSize);

//		nOpenBufferIndex += pm_SSS_Special->dwMapSize;
//	}

	// Name Data Map
	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwNameSize, sizeof(DWORD));
	nOpenBufferIndex += sizeof(DWORD);

	if (pm_SSS_Special->dwNameSize > 0)
	{
		memcpy(&pbOpenBuf[nOpenBufferIndex],
				pm_SSS_Special->bNameData,
				pm_SSS_Special->dwNameSize);

		nOpenBufferIndex += pm_SSS_Special->dwNameSize;
	}

	// Rand Data Map
	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwRandSize, sizeof(DWORD));
	nOpenBufferIndex += sizeof(DWORD);

	if (pm_SSS_Special->dwRandSize > 0)
	{
		memcpy(&pbOpenBuf[nOpenBufferIndex],
				pm_SSS_Special->bRandData,
				pm_SSS_Special->dwRandSize);

		nOpenBufferIndex += pm_SSS_Special->dwRandSize;
	}

	// Strings
	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwNumStrings, sizeof(DWORD));
	nOpenBufferIndex += sizeof(DWORD);

	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwStringsSize, sizeof(DWORD));
	nOpenBufferIndex += sizeof(DWORD);

	if (pm_SSS_Special->dwStringsSize > 0)
	{
		memcpy(&pbOpenBuf[nOpenBufferIndex],
				pm_SSS_Special->bStringData,
				pm_SSS_Special->dwStringsSize);

		nOpenBufferIndex += pm_SSS_Special->dwStringsSize;
	}

	// DWORDS
	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwNumDWords, sizeof(DWORD));
	nOpenBufferIndex += sizeof(DWORD);

	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwDWordsSize, sizeof(DWORD));
	nOpenBufferIndex += sizeof(DWORD);

	if (pm_SSS_Special->dwDWordsSize > 0)
	{
		memcpy(&pbOpenBuf[nOpenBufferIndex],
				pm_SSS_Special->bDWordData,
				pm_SSS_Special->dwDWordsSize);

		nOpenBufferIndex += pm_SSS_Special->dwDWordsSize;
	}

	// Other Values
	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwNumOther, sizeof(DWORD));
	nOpenBufferIndex += sizeof(DWORD);

	memcpy(&pbOpenBuf[nOpenBufferIndex], &pm_SSS_Special->dwOtherSize, sizeof(DWORD));
	nOpenBufferIndex += sizeof(DWORD);

	if (pm_SSS_Special->dwOtherSize > 0)
	{
		memcpy(&pbOpenBuf[nOpenBufferIndex],
				pm_SSS_Special->bOtherData,
				pm_SSS_Special->dwOtherSize);

		nOpenBufferIndex += pm_SSS_Special->dwOtherSize;
	}

//=============================
// Encryption
//=============================

	// Initialize the encrypted buffer.
//	memset(pbEncryptedBuf, 0, dwEncryptedSize);

	DWORD dwSize = dwOpenSize;

	BOOL bl = Crypt(NULL, pbOpenBuf, pbEncryptedBuf, &dwSize, true);

	dwEncryptedSize = dwSize;
	
	TraceInternal(_D("WiVGenEncryptedData: GPL returned %08X"), bl);

	
	TraceCode(_D("\r\n#define SSS_NUM_STRINGS	%d"), pm_SSS_Special->dwNumStrings);
	TraceCode(_D("\r\n#define SSS_STRINGS_SIZE	%d\r\n"), nStringsSize);
	TraceCode(_D("\r\n#define SSS_NUM_DWORDS	%d"), pm_SSS_Special->dwNumDWords);
	TraceCode(_D("\r\n#define SSS_DWORDS_SIZE	%d\r\n"), nDWordsSize);
	TraceCode(_D("\r\n#define SSS_NUM_OTHER		%d"), pm_SSS_Special->dwNumOther);
	TraceCode(_D("\r\n#define SSS_OTHER_SIZE	%d\r\n"), nOtherSize);

	TraceCode(_D("\r\n#pragma pack(1)"));
	TraceCode(_D("\r\nstruct SSS_GLOBALS\r\n{"));

//	TraceCode(_D("\r\n\tDWORD\tdwMapSize;"));
//	if (pm_SSS_Special->dwMapSize > 0)
//	{
//		TraceCode(_D("\r\n\tBYTE\tbMapData[SSS_MAP_SIZE];")); 
//	}

	TraceCode(_D("\r\n\tDWORD\tdwNameSize;"));
	if (pm_SSS_Special->dwNameSize > 0)
	{
		TraceCode(_D("\r\n\tBYTE\tbNameData[SSS_NAME_SIZE];")); 
	}

	TraceCode(_D("\r\n\tDWORD\tdwRandSize;"));
	if (pm_SSS_Special->dwRandSize > 0)
	{
		TraceCode(_D("\r\n\tBYTE\tbRandData[SSS_RAND_SIZE];")); 
	}

	TraceCode(_D("\r\n\tDWORD\tdwNumStrings;"));
	TraceCode(_D("\r\n\tDWORD\tdwStringsSize;"));
	if (pm_SSS_Special->dwStringsSize > 0)
	{
		TraceCode(_D("\r\n\tBYTE\tbStringData[SSS_STRINGS_SIZE];"));
	}

	TraceCode(_D("\r\n\tDWORD\tdwNumDWords;"));
	TraceCode(_D("\r\n\tDWORD\tdwDWordsSize;"));
	if (pm_SSS_Special->dwDWordsSize > 0)
	{
		TraceCode(_D("\r\n\tBYTE\tbDWordData[SSS_DWORDS_SIZE];"));
	}

	TraceCode(_D("\r\n\tDWORD\tdwNumOther;"));
	TraceCode(_D("\r\n\tDWORD\tdwOtherSize;"));
	if (pm_SSS_Special->dwOtherSize > 0)
	{
		TraceCode(_D("\r\n\tBYTE\tbOtherData[SSS_OTHER_SIZE];"));
	}

	TraceCode(_D("\r\n};"));
	TraceCode(_D("\r\n#pragma pack()\r\n"));
	TraceCode(_D("\r\nstatic SSS_GLOBALS SSSGlobalData;"));
	TraceCode(_D("\r\nstatic SSS_GLOBALS *SSSGlobals = &SSSGlobalData;\r\n"));

	// Strings
	nDefNum = 1;
	_tcscpy(szPad, _D(""));

	TraceCode(_D("\r\n#define SSS_INBUF_SIZE	%d\r\n"), dwEncryptedSize);
	TraceCode(_D("\r\n#define SSS_OUTBUF_SIZE	%d\r\n"), dwOpenSize);

	TraceCode(_D("\r\n#define rnd%02d "), nDefNum);

	for (nEncryptedBufferIndex = 0, nDefsPerLine = 0;
		 nEncryptedBufferIndex < (int)dwEncryptedSize;
		 nEncryptedBufferIndex++, nDefsPerLine++)
	{
		if (nDefsPerLine >= 16)
		{
			TraceCode(_D("\r\n#define rnd%02d "), ++nDefNum);
			_tcscpy(szPad, _D(""));
			nDefsPerLine = 0;
		}

		TraceCode(_D("%s0x%02X"), szPad, pbEncryptedBuf[nEncryptedBufferIndex]);
		_tcscpy(szPad, _D(","));
	}

	nTotalDefs = nDefNum;
	
	TraceCode(_D("\r\n\r\nconst\tBYTE\tm_biBuf[SSS_INBUF_SIZE] = {"));

	for (nDefNum = 1; nDefNum < nTotalDefs; nDefNum++)
	{
		TraceCode(_D("rnd%02d,"), nDefNum);
	}

	TraceCode(_D("rnd%02d};"), nTotalDefs);

	TraceCode(_D("\r\nstatic  BYTE	m_boBuf[SSS_OUTBUF_SIZE];\r\n"));


//===========================================
// End of Code Generation
//===========================================

	TraceCode(_D("\r\n\r\n//------------- end paste special (SSSGlobals.h) ")
					_D("---------------\r\n"));

	TraceCodeEnd(_D("====== Generated Code End ======================"));

//===========================================
// Tidy up memory
//===========================================

	// Free the structure buffer
	if (pm_SSS_Special)
	{
		TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
					_D("Freeing existing structure buffer"));
		LocalFree (pm_SSS_Special);
	}

	// Free the open buffer
	if (pbOpenBuf)
	{
		TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
					_D("Freeing existing open memory buffer"));
		LocalFree (pbOpenBuf);
	}

	// Free the encrypted buffer
	if (pbEncryptedBuf)
	{
		TraceInternal(_D("WiVEncode::WiVGenEncryptedData: ")
					_D("Freeing encrypted memory buffer"));
		LocalFree (pbEncryptedBuf);
	}

	TraceLeave(_D("WiVEncode::WiVGenEncryptedData"), (DWORD)true, tlInternal);

	return true;
}

int WiVInitMaps()
{
/*
//=========================
// License Data Map
//=========================
	char	achMapData[128]		= {	'Q','A','Z','W','S','X','E','D',
									'C','R','F','V','T','G','B','Y',
									'H','N','U','J','M','I','K','O',
									'L','P','P','O','I','U','Y','T',
									'R','E','W','Q','L','K','J','H',
									'G','F','D','S','A','M','N','B',
									'V','C','X','Z','M','N','B','V',
									'C','X','Z','L','K','J','H','G',
									'F','D','S','A','P','O','I','U',
									'Y','T','R','E','W','Q','Q','W',
									'E','R','T','Y','U','I','O','P',
									'A','S','D','F','G','H','J','K',
									'L','Z','X','C','X','Z','M','N',
									'B','V','C','X','Z','L','M','I',
									'K','O','L','P','P','O','I','U',
									'R','M','B','F','I','K','J','C'};
*/
//=========================
// PIN Registry Name Table
//=========================
	TCHAR	aszNameData[16]		= {	_T('J'), _T('B'), _T('F'), _T('P'),
									_T('M'), _T('Q'), _T('Z'), _T('H'),
									_T('K'), _T('I'), _T('L'), _T('O'),
									_T('A'), _T('C'), _T('W'), _T('G')};


//=========================
// Randoms Look-up Table
//=========================
	DWORD	adwRandData[16]		= {	0x703A59B5, 0xB196499E, 0x42B78C76, 0xD333EDF2,
									0x3443F8ED,	0x5536BB2E, 0x165C9C52, 0x079F7F46,
									0xC81D3E83, 0x996A1E77,	0x6AE23BF0, 0xAB93B2D1,
									0xEC8EBC2D, 0x8D86BE34, 0xFE2A92E9, 0x2F0E473A};
/*

[HKEY_CLASSES_ROOT\CLSID\{28D3086B-6F51-4ca6-B555-5D98A2234BA8}]

{703A59B5-926C-4ff3-8954-C3194C083522} 0 - Trial for any device
{B196499E-8DA2-40de-8C25-32B114B9FC29} 1 - Full
{42B78C76-BC99-4e46-BAB5-7A77840837B2} 2 - Full
{D333EDF2-EE44-428f-A266-8BBC31F8DBCB} 3 - Beta
{3443F8ED-F74D-4d5b-8797-83AF2FB582D1} 4 - Beta
{5536BB2E-49E6-4045-9E7D-026B79B0AA6A} 5 - Special
{165C9C52-2E9B-4bd3-9353-29B06E4BACB8} 6 - Special
{079F7F46-090D-40fc-BCDE-2CB15AE7273D} 7
{C81D3E83-A0DA-4df3-8E8D-012438E31BFA} 8
{996A1E77-508B-4642-AD23-58FE99D2C37D} 9
{6AE23BF0-D012-4a83-9873-FD4E2F139362} A
{AB93B2D1-EB77-491d-87D5-D0F242B9005E} B
{EC8EBC2D-C02C-4aad-A6D0-02CDC038F1FC} C
{8D86BE34-8637-49e5-BA83-8AE7EE483652} D
{FE2A92E9-6C83-44a4-BBA2-32FC4B8920D1} E
{2F0E473A-7AD2-48f5-AF9A-FFF0F067BD64} F
*/

//	int	nMapSize = sizeof(achMapData);
	int	nNameSize = sizeof(aszNameData);
	int	nRandSize = sizeof(adwRandData);

/*
	for (int x = 1; x <= ARRAYSIZE(adwRandData); x++)
	{
		TraceCode(_D("\r\n"));
		TraceCode(_D("\r\n#define SSS_RAND_%02d          %08X"), x, adwRandData[x - 1]);
		TraceCode(_D("\r\n#define SSS_FLAGS_INDEX%02d    %d"), x, 
			(adwRandData[x - 1] & WIV_FLAGS_INDEX_MASK) >> WIV_FLAGS_INDEX_SHIFT);
		TraceCode(_D("\r\n#define SSS_FLAGS_LICTYPE%02d  %d"), x, 
			(adwRandData[x - 1] & WIV_FLAGS_LICTYPE_MASK) >> WIV_FLAGS_LICTYPE_SHIFT);
		TraceCode(_D("\r\n#define SSS_FLAGS_DAYS%02d     %d"), x, 
			(adwRandData[x - 1] & WIV_FLAGS_DAYS_MASK) >> WIV_FLAGS_DAYS_SHIFT);
		TraceCode(_D("\r\n#define SSS_FLAGS_INSTALL%02d  %d"), x, 
			(adwRandData[x - 1] & WIV_FLAGS_INSTALL_MASK) >> WIV_FLAGS_INSTALL_SHIFT);
	}

	TraceCode(_D("\r\n"));
	for (x = 1; x <= ARRAYSIZE(adwRandData); x++)
	{
		TraceCode(_D("\r\n#define SSS_FLAGS_INDEX%02d    %d"), x,
			(adwRandData[x - 1] & WIV_FLAGS_INDEX_MASK) >> WIV_FLAGS_INDEX_SHIFT);
	}

	TraceCode(_D("\r\n"));
	for (x = 1; x <= ARRAYSIZE(adwRandData); x++)
	{
		TraceCode(_D("\r\n#define SSS_FLAGS_LICTYPE%02d  %d"), x,
			(adwRandData[x - 1] & WIV_FLAGS_LICTYPE_MASK) >> WIV_FLAGS_LICTYPE_SHIFT);
	}

	TraceCode(_D("\r\n"));
	for (x = 1; x <= ARRAYSIZE(adwRandData); x++)
	{
		TraceCode(_D("\r\n#define SSS_FLAGS_DAYS%02d     %d"), x,
			(adwRandData[x - 1] & WIV_FLAGS_DAYS_MASK) >> WIV_FLAGS_DAYS_SHIFT);
	}

	TraceCode(_D("\r\n"));
	for (x = 1; x <= ARRAYSIZE(adwRandData); x++)
	{
		TraceCode(_D("\r\n#define SSS_FLAGS_INSTALL%02d  %d"), x, 
			(adwRandData[x - 1] & WIV_FLAGS_INSTALL_MASK) >> WIV_FLAGS_INSTALL_SHIFT);
	}

	TraceCode(_D("\r\n"));
*/

	// Specify number of data map bytes
//	pm_SSS_Special->dwMapSize = nMapSize;

	// Store data map bytes
//	memcpy(pm_SSS_Special->bMapData, achMapData, nMapSize);

	// Specify number of name table bytes
	pm_SSS_Special->dwNameSize = nNameSize;

	// Store name table bytes
	memcpy(pm_SSS_Special->bNameData, aszNameData, nNameSize);

	// Specify number of random table bytes
	pm_SSS_Special->dwRandSize = nRandSize;

	// Store random table bytes
	memcpy(pm_SSS_Special->bRandData, adwRandData, nRandSize);

//	TraceCode(_D("\r\n#define SSS_MAP_SIZE %d"), nMapSize);
//	TraceCode(_D("\r\n#define SSS_MAP_ENTRIES %d\r\n"), nMapSize/sizeof(char));
	
	TraceCode(_D("\r\n#define SSS_NAME_SIZE %d"), nNameSize);
	TraceCode(_D("\r\n#define SSS_NAME_ENTRIES %d\r\n"), nNameSize/sizeof(TCHAR));

	TraceCode(_D("\r\n#define SSS_RAND_SIZE %d"), nRandSize);
	TraceCode(_D("\r\n#define SSS_RAND_ENTRIES %d\r\n"), nRandSize/sizeof(DWORD));
	
//	return nMapSize + nNameSize + nRandSize;
	return nNameSize + nRandSize;
}

int WiVInitStrings()
{

	int		nLen;
	int		nStringsByteIndex;
	DWORD	dwStringCount;

//=====================
// Identification
//=====================
	char	szCompanyName[]					= "WiViT";
	char	szCompanyURL[]					= "http://www.wivit.com";
	char	szCompanySupport[]				= "support@wivit.com";
	char	szCompanyIdentity[]				= "The WiViT Team";
	char	szProductName[]					= "SIM Status Switcher";
	char	szProductVersion[]				= "1.00";
	char	szProductType[]					= "Retail";
	char	szProductBuild[]				= "0705";
	char	szProductShortName[]			= "WiVSSS";
	char	szVersionLabel[]				= "Version";
	char	szBuildLabel[]					= "Build";
	char	szCopyrightLabel[]				= "Copyright";
	char	szBetaLabel[]					= "Beta";
	char	szRetailLabel[]					= "Retail";
	char	szTrialLabel[]					= "Trial";
	char	szSpecialLabel[]				= "Special";
	char	szLicenseLabel[]				= "License";
	char	szRegisteredLabel[]				= "Registered";
	char	szReservedLabel[]				= "All rights reserved";
	char	szDevelopedByLabel[]			= "Designed and Developed by";
	char	szNoLicenseLabel[]				= "Missing License";
	char	szInvalidLabel[]				= "Invalid License";
	char	szExpiredLabel[]				= "Expired";
	char	szCustomFileHeader[]			= "WiViT Customisation File";
	char	szSSSTodayClass[]				= "SSSTodayClass";
	char	szSSSTodayWnd[]					= "SSSTodayWnd";

//=====================
// Registry stuff
//=====================
	char	szRegInstallPath[]				= "InstallPath";
	char	szTodayItemsKey[]				= "Microsoft\\Today\\Items";
	char	szTodayItemFlagsKey[]			= "Flags";
	char	szSoftwareKey[]					= "Software";
	char	szSpecialKey[]					= "{28D3086B-6F51-4ca6-B555-5D98A2234BA8}";
	char	szClsIDKey[]					= "CLSID";
	char	szTypeLibKey[]					= "TypeLib";
	char	szDefaultKey[]					= "";
	char	szSecondKey[]					= "2.0";
	char	szRegLicenseType[]				= "LicenseType";
	char	szRegOptionsKey[]				= "Options";
	char	szRegShowPhoneNumber[]			= "ShowNumber";
	char	szRegShowTSP[]					= "ShowTSP";
	char	szRegSingleLineDisplay[]		= "SingleLineDisplay";
	char	szRegLine1BoldFont[]			= "Line1BoldFont";
	char	szRegLine2BoldFont[]			= "Line2BoldFont";
	char	szRegTodayIconSet[]				= "IconSet";
	char	szRegTapAction[]				= "TapAction";
	char	szRegTAHAction[]				= "TAHAction";
	char	szRegTodayIconTapAction[]		= "TodayIconTapAction";
	char	szRegTodayIconTAHAction[]		= "TodayIconTAHAction";
	char	szRegOptionsHidePersonal[]		= "HidePersonalInfo";
	char	szRegLangDefaultLanguageID[]	= "DefaultLanguageID";
	char	szRegLangDefaultLanguageName[]	= "DefaultLanguageName";

//=====================
// Help stuff
//=====================
	char	szHelpTagAboutSSS[]				= "about_sss";
	char	szHelpTagActionsSettings[]		= "actions_settings";
	char	szHelpTagAppearanceSettings[]	= "appearance_settings";
	char	szHelpTagDisplaySettings[]		= "display_settings";
	char	szHelpTagLanguageSettings[]		= "language_settings";
	char	szHelpTagSecuritySettings[]		= "security_settings";
	char	szHelpTagAboutSettings[]		= "about_settings";
	char	szHelpTagInformationDialog[]	= "information_dialog";
	char	szHelpTagRegistrationDialog[]	= "registration_dialog";

//=====================
// Format Strings
//=====================
	char	szFormatSlashes[]				= "\\/";
	char	szFormatSlash[]					= "/";
	char	szFormat2005[]					= "2005";
	char	szFormatBackSlash[]				= "\\";
	char	szFormatCopyright[]				= "©";
	char	szFormatRegValue[]				= "{%04X%04X-%4.4s-%4.4s-%4.4s-%12.12s}";
	char	szFormatSerialNumber[]			= "%02X%04X%02X%06X%01X";

//=====================
// Miscellaneous stuff
//=====================
	char	szDiskLabel[]					= "DSK";
	char	szMSDefaultOEM[]				= "MSWIN4.1";
	char	szConfigMgrDir[]				= "ConfigMgr";
	char	szConfigMgrFile[]				= "Windows_NETCFKeys_System.Default.blb";
	char	szSMSReceiver[]					= "SMSReceiver_dll.dll";
	char	szEmptyReg[]					= "{00000000-0000-0000-0000-00000000000}";
	char	szFakePhoneNumber[]				= "07712345678";
	char	szFakeIMEI[]					= "35193800001234501";
	char	szFakeUserID[]					= "234106901234567";
	char	szFakeICCID[]					= "44110001234567891";
	char	szWIVEnd[]						= "WIV_END";
	char	szICCIDPrefix[]					= "89";

	char	szLinkText[]					= "file:ctlpnl cplmain";
	char	szCtlPnl[]						= "ctlpnl.exe";
	char	szCplMain[]						= "cplmain.cpl";
	char	szMailTo[]						= "mailto:";
	char	szOpenDoc[]						= "-opendoc ";
	char	szOpen[]						= "open";
	char	szPWordExe[]					= "pword.exe";
	char	szHelpPrefix[]					= "file:";
	char	szHelpFile[]					= "WiVSSSHelp";
	char	szPegHelpExe[]					= "peghelp.exe";


//===========================================
// Copy the data and generate code as we go
//===========================================
	
	TraceCode(_D("\r\n"));

	dwStringCount = 0;
	nStringsByteIndex = 0;

//=====================
// Identification
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Identification")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = strlen(szCompanyName);
	pm_SSS_Special->bStringData[nStringsByteIndex++]	= nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szCompanyName, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szCompanyName (WiV::GlobalGetString(%d)) // (\"%S\")"),
							dwStringCount, szCompanyName);

	nLen = strlen(szCompanyURL);
	pm_SSS_Special->bStringData[nStringsByteIndex++]	= nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szCompanyURL, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szCompanyURL (WiV::GlobalGetString(%d)) // (\"%S\")"),
							dwStringCount, szCompanyURL);

	nLen = strlen(szCompanySupport);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szCompanySupport, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szCompanySupport (WiV::GlobalGetString(%d)) // (\"%S\")"),
						dwStringCount, szCompanySupport);

	nLen = strlen(szCompanyIdentity);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szCompanyIdentity, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szCompanyIdentity (WiV::GlobalGetString(%d)) // (\"%S\")"),
						dwStringCount, szCompanyIdentity);

	nLen = strlen(szProductName);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szProductName, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szProductName (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szProductName);

	nLen = strlen(szProductVersion);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szProductVersion, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szProductVersion (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szProductVersion);

	nLen = strlen(szProductType);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szProductType, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szProductType (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szProductType);

	nLen = strlen(szProductBuild);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szProductBuild, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szProductBuild (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szProductBuild);

	nLen = strlen(szProductShortName);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szProductShortName, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szProductShortName (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szProductShortName);

	nLen = strlen(szVersionLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szVersionLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szVersionLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szVersionLabel);

	nLen = strlen(szBuildLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szBuildLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szBuildLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szBuildLabel);

	nLen = strlen(szCopyrightLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szCopyrightLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szCopyrightLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szCopyrightLabel);

	nLen = strlen(szBetaLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szBetaLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szBetaLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szBetaLabel);

	nLen = strlen(szRetailLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRetailLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRetailLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRetailLabel);

	nLen = strlen(szTrialLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szTrialLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szTrialLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szTrialLabel);

	nLen = strlen(szSpecialLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szSpecialLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szSpecialLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szSpecialLabel);

	nLen = strlen(szLicenseLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szLicenseLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szLicenseLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szLicenseLabel);

	nLen = strlen(szRegisteredLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegisteredLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegisteredLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegisteredLabel);

	nLen = strlen(szReservedLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szReservedLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szReservedLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szReservedLabel);

	nLen = strlen(szDevelopedByLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szDevelopedByLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szDevelopedByLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szDevelopedByLabel);

	nLen = strlen(szNoLicenseLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szNoLicenseLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szNoLicenseLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szNoLicenseLabel);

	nLen = strlen(szInvalidLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szInvalidLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szInvalidLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szInvalidLabel);

	nLen = strlen(szExpiredLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szExpiredLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szExpiredLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szExpiredLabel);

	nLen = strlen(szCustomFileHeader);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szCustomFileHeader, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szCustomFileHeader (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szCustomFileHeader);

	nLen = strlen(szSSSTodayClass);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szSSSTodayClass, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szSSSTodayClass (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szSSSTodayClass);

	nLen = strlen(szSSSTodayWnd);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szSSSTodayWnd, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szSSSTodayWnd (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szSSSTodayWnd);

//=====================
// Registry stuff
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Registry stuff")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = strlen(szRegInstallPath);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegInstallPath, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegInstallPath (WiV::GlobalGetString(%d)) // (\"%S\")"), 
		dwStringCount, szRegInstallPath);
	
	nLen = strlen(szTodayItemFlagsKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szTodayItemFlagsKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szTodayItemFlagsKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szTodayItemFlagsKey);

	nLen = strlen(szTodayItemsKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szTodayItemsKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szTodayItemsKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szTodayItemsKey);

	nLen = strlen(szSoftwareKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szSoftwareKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szSoftwareKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szSoftwareKey);

	nLen = strlen(szSpecialKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szSpecialKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szSpecialKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szSpecialKey);

	nLen = strlen(szClsIDKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szClsIDKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szClsIDKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szClsIDKey);

	nLen = strlen(szTypeLibKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szTypeLibKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szTypeLibKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szTypeLibKey);

	nLen = strlen(szDefaultKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szDefaultKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szDefaultKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szDefaultKey);

	nLen = strlen(szSecondKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szSecondKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szSecondKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szSecondKey);

	nLen = strlen(szRegLicenseType);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegLicenseType, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegLicenseType (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegLicenseType);

	nLen = strlen(szRegOptionsKey);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegOptionsKey, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegOptionsKey (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegOptionsKey);

	nLen = strlen(szRegShowPhoneNumber);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegShowPhoneNumber, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegShowPhoneNumber (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegShowPhoneNumber);

	nLen = strlen(szRegShowTSP);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegShowTSP, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegShowTSP (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegShowTSP);

	nLen = strlen(szRegSingleLineDisplay);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegSingleLineDisplay, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegSingleLineDisplay (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegSingleLineDisplay);

	nLen = strlen(szRegLine1BoldFont);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegLine1BoldFont, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegLine1BoldFont (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegLine1BoldFont);

	nLen = strlen(szRegLine2BoldFont);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegLine2BoldFont, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegLine2BoldFont (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegLine2BoldFont);

	nLen = strlen(szRegTodayIconSet);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegTodayIconSet, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegTodayIconSet (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegTodayIconSet);

	nLen = strlen(szRegTapAction);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegTapAction, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegTapAction (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegTapAction);

	nLen = strlen(szRegTAHAction);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegTAHAction, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegTAHAction (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegTAHAction);

	nLen = strlen(szRegTodayIconTapAction);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegTodayIconTapAction, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegTodayIconTapAction (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegTodayIconTapAction);

	nLen = strlen(szRegTodayIconTAHAction);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegTodayIconTAHAction, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegTodayIconTAHAction (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegTodayIconTAHAction);

	nLen = strlen(szRegOptionsHidePersonal);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegOptionsHidePersonal, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegOptionsHidePersonal (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegOptionsHidePersonal);

	nLen = strlen(szRegLangDefaultLanguageID);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegLangDefaultLanguageID, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegLangDefaultLanguageID (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegLangDefaultLanguageID);

	nLen = strlen(szRegLangDefaultLanguageName);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szRegLangDefaultLanguageName, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szRegLangDefaultLanguageName (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szRegLangDefaultLanguageName);

//=====================
// Help stuff
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Help stuff")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = strlen(szHelpTagAboutSSS);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagAboutSSS, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagAboutSSS (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagAboutSSS);

	nLen = strlen(szHelpTagActionsSettings);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagActionsSettings, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagActionsSettings (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagActionsSettings);

	nLen = strlen(szHelpTagAppearanceSettings);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagAppearanceSettings, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagAppearanceSettings (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagAppearanceSettings);

	nLen = strlen(szHelpTagDisplaySettings);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagDisplaySettings, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagDisplaySettings (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagDisplaySettings);

	nLen = strlen(szHelpTagLanguageSettings);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagLanguageSettings, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagLanguageSettings (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagLanguageSettings);

	nLen = strlen(szHelpTagSecuritySettings);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagSecuritySettings, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagSecuritySettings (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagSecuritySettings);

	nLen = strlen(szHelpTagAboutSettings);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagAboutSettings, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagAboutSettings (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagAboutSettings);

	nLen = strlen(szHelpTagInformationDialog);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagInformationDialog, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagInformationDialog (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagInformationDialog);

	nLen = strlen(szHelpTagRegistrationDialog);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpTagRegistrationDialog, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpTagRegistrationDialog (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpTagRegistrationDialog);

//=====================
// Format Strings
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Format Strings")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = strlen(szFormatSlashes);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFormatSlashes, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFormatSlashes (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFormatSlashes);

	nLen = strlen(szFormatSlash);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFormatSlash, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFormatSlash (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFormatSlash);

	nLen = strlen(szFormat2005);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFormat2005, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFormat2005 (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFormat2005);

	nLen = strlen(szFormatBackSlash);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFormatBackSlash, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFormatBackSlash (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFormatBackSlash);

	nLen = strlen(szFormatCopyright);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFormatCopyright, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFormatCopyright (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFormatCopyright);

	nLen = strlen(szFormatRegValue);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFormatRegValue, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFormatRegValue (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFormatRegValue);

	nLen = strlen(szFormatSerialNumber);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFormatSerialNumber, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFormatSerialNumber (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFormatSerialNumber);

//=====================
// Miscellaneous stuff
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Miscellaneous stuff")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = strlen(szDiskLabel);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szDiskLabel, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szDiskLabel (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szDiskLabel);

	nLen = strlen(szMSDefaultOEM);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szMSDefaultOEM, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szMSDefaultOEM (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szMSDefaultOEM);

	nLen = strlen(szConfigMgrDir);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szConfigMgrDir, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szConfigMgrDir (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szConfigMgrDir);

	nLen = strlen(szConfigMgrFile);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szConfigMgrFile, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szConfigMgrFile (WiV::GlobalGetString(%d)) // (\"%S\")"), 
		dwStringCount, szConfigMgrFile);
	
	nLen = strlen(szSMSReceiver);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szSMSReceiver, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szSMSReceiver (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szSMSReceiver);

	nLen = strlen(szEmptyReg);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szEmptyReg, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szEmptyReg (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szEmptyReg);

	nLen = strlen(szFakePhoneNumber);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFakePhoneNumber, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFakePhoneNumber (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFakePhoneNumber);

	nLen = strlen(szFakeIMEI);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFakeIMEI, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFakeIMEI (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFakeIMEI);
	
	nLen = strlen(szFakeUserID);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFakeUserID, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFakeUserID (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFakeUserID);
	
	nLen = strlen(szFakeICCID);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szFakeICCID, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szFakeICCID (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szFakeICCID);

	nLen = strlen(szWIVEnd);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szWIVEnd, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szWIVEnd (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szWIVEnd);

	nLen = strlen(szICCIDPrefix);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szICCIDPrefix, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szICCIDPrefix (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szICCIDPrefix);

	nLen = strlen(szLinkText);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szLinkText, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szLinkText (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szLinkText);

	nLen = strlen(szCtlPnl);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szCtlPnl, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szCtlPnl (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szCtlPnl);

	nLen = strlen(szCplMain);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szCplMain, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szCplMain (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szCplMain);

	nLen = strlen(szMailTo);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szMailTo, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szMailTo (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szMailTo);

	nLen = strlen(szOpenDoc);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szOpenDoc, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szOpenDoc (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szOpenDoc);

	nLen = strlen(szOpen);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szOpen, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szOpen (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szOpen);

	nLen = strlen(szPWordExe);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szPWordExe, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szPWordExe (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szPWordExe);

	nLen = strlen(szHelpPrefix);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpPrefix, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpPrefix (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szHelpPrefix);

	nLen = strlen(szHelpFile);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szHelpFile, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szHelpFile (WiV::GlobalGetString(%d)) // (\"%S\")"), 
		dwStringCount, szHelpFile);
	
	nLen = strlen(szPegHelpExe);
	pm_SSS_Special->bStringData[nStringsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bStringData[nStringsByteIndex], szPegHelpExe, nLen);
	nStringsByteIndex += nLen;
	dwStringCount += 1;
	TraceCode(_D("\r\n#define g_szPegHelpExe (WiV::GlobalGetString(%d)) // (\"%S\")"), 
						dwStringCount, szPegHelpExe);


	// Specify number of strings
	pm_SSS_Special->dwNumStrings = dwStringCount;

	// Specify total size of strings
	pm_SSS_Special->dwStringsSize = nStringsByteIndex;

	return nStringsByteIndex;
}

int WiVInitDWords()
{

	int		nLen;
	int		nDWordsByteIndex;
	DWORD	dwDWordCount;

//=====================
// Colour References
//=====================
	COLORREF	crGreenOK					= RGB(48, 167, 47);
	COLORREF	crGreen						= RGB(46, 180, 87);
	COLORREF	crBlueTitle					= RGB(0, 0, 156);
	COLORREF	crBlueLink					= RGB(0, 0, 220);
	COLORREF	crBlueLight					= RGB(51, 102, 204);
	COLORREF	crBlueDark					= RGB(0, 57, 182);
	COLORREF	crBlueInfo					= RGB(0, 0, 255);
	COLORREF	crRedError					= RGB(196, 18, 0);
	COLORREF	crAmberWarning				= RGB(255, 135, 0);

//=====================
// Today item stuff
//=====================
	DWORD	dwTodayActionRefresh			= 0;
	DWORD	dwTodayActionShowPopup			= 1;
	DWORD	dwTodayActionOptions			= 2;
	DWORD	dwTodayActionSwitchSIM			= 3;
	DWORD	dwTodayActionToggleRadio		= 4;
	DWORD	dwTodayActionPhoneSettings		= 5;

	DWORD	dwGetUserOptions				= 1;
	DWORD	dwGetSecurityOptions			= 2;
	DWORD	dwGetDebugOptions				= 4;
	DWORD	dwGetAllOptions					= 7;
	DWORD	dwGetForce						= 8;

	DWORD	dwIconTypeNeutral				= 0;
	DWORD	dwIconTypeOn					= 1;
	DWORD	dwIconTypeOff					= 2;
	DWORD	dwIconTypeFull					= 3;

	DWORD	dwTodayIconSetStandardPhone		= 0;
	DWORD	dwTodayIconSetInOutButton		= 1;
	DWORD	dwTodayIconSetMobilePhone		= 2;
	DWORD	dwTodayIconSetPDA				= 3;
	DWORD	dwTodayIconSetTraffic			= 4;
	DWORD	dwTodayIconSetWiViT				= 5;

	DWORD	dwTodaySingleLineHeight			= 18;
	DWORD	dwTodayDoubleLineHeight			= 32;

//=====================
// Phone Stuff
//=====================
	DWORD	dwPhoneAccessOK					= 0x00000000;
	DWORD	dwPhoneAccessError				= 0x80000000;

	DWORD	dwPhoneStateUnknown				= 0x00000000;
	DWORD	dwPhoneStateOn					= 0x00000001;
	DWORD	dwPhoneStateOff					= 0x00000002;

	DWORD	dwServiceProvider				= 0x00006f46;
	DWORD	dwIMSINumber					= 0x00006f07;
	DWORD	dwMSISDNNumber					= 0x00006f40;
	DWORD	dwICCIDNumber					= 0x00002fe2;

//=====================
// Security Stuff
//=====================
	DWORD	dwSecuritySequenceNoAuto		= 0x00000000;
	DWORD	dwSecuritySequenceNoPIN			= 0x00000001;
	DWORD	dwSecuritySequencePIN			= 0x00000002;

	DWORD	dwSecuritySequenceStepStart		= 0x00000000;

	DWORD	dwSecuritySequenceActionNoPIN	= 0x00000000;
	DWORD	dwSecuritySequenceActionCreate	= 0x00000001;
	DWORD	dwSecuritySequenceActionChange	= 0x00000002;
	DWORD	dwSecuritySequenceActionCurrent	= 0x00000003;
	DWORD	dwSecuritySequenceActionNew		= 0x00000004;
	DWORD	dwSecuritySequenceActionEnter	= 0x00000005;
	DWORD	dwSecuritySequenceActionConfirm	= 0x00000006;
	DWORD	dwSecuritySequenceActionNone	= 0x00000009;

//=====================
// Notifications
//=====================
	DWORD	dwNotifyNone					= 0x00000000;
	DWORD	dwNotifyICCID					= 0x00000001;
	DWORD	dwNotifyIMSI					= 0x00000002;
	DWORD	dwNotifyRadioPresence			= 0x00000004;
	DWORD	dwNotifyRadioState				= 0x00000008;
	DWORD	dwNotifySIMLock					= 0x00000010;
	DWORD	dwNotifyRegistration			= 0x00000020;
	DWORD	dwNotifyNumber					= 0x00000040;
	DWORD	dwNotifyOperator				= 0x00000080;
	DWORD	dwNotifyEquipmentInfo			= 0x00000100;
	DWORD	dwNotifyPhonebook				= 0x00000200;
	DWORD	dwNotifyHandle					= 0x00000400;
	DWORD	dwNotifyLicense					= 0x00000800;
	DWORD	dwNotifySerialNumber			= 0x00001000;
	DWORD	dwNotifySIMError				= 0x01000000;
	DWORD	dwNotifySIMUnlockError			= 0x02000000;
	DWORD	dwNotifyAPIError				= 0x04000000;

#ifdef SSS_V2_IMP
	DWORD	dwNotifySignalQuality			= 0x00000800;
#endif // #ifdef SSS_V2_IMP

//==========================================
// Copy the data and generate code as we go
//==========================================
	
	TraceCode(_D("\r\n"));

	dwDWordCount = 0;
	nDWordsByteIndex = 0;

//=====================
// Colour values
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Colour values")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;	
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crGreenOK, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crGreenOK (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crGreenOK);

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;			
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crGreen, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crGreen (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crGreen);

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crBlueTitle, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crBlueTitle (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crBlueTitle);

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crBlueLink, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crBlueLink (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crBlueLink);

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crBlueLight, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crBlueLight (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crBlueLight);

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crBlueDark, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crBlueDark (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crBlueDark);

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crBlueInfo, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crBlueInfo (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crBlueInfo);

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crRedError, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crRedError (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crRedError);

	nLen = sizeof(COLORREF);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &crAmberWarning, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_crAmberWarning (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, crAmberWarning);

//=====================
// Today item stuff
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Today item stuff")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayActionRefresh, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayActionRefresh (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayActionRefresh);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayActionShowPopup, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayActionShowPopup (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayActionShowPopup);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayActionOptions, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayActionOptions (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayActionOptions);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayActionSwitchSIM, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayActionSwitchSIM (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayActionSwitchSIM);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayActionToggleRadio, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayActionToggleRadio (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayActionToggleRadio);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayActionPhoneSettings, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayActionPhoneSettings (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayActionPhoneSettings);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwGetUserOptions, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwGetUserOptions (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwGetUserOptions);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwGetSecurityOptions, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwGetSecurityOptions (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwGetSecurityOptions);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwGetDebugOptions, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwGetDebugOptions (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwGetDebugOptions);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwGetAllOptions, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwGetAllOptions (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwGetAllOptions);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwGetForce, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwGetForce (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwGetForce);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwIconTypeNeutral, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwIconTypeNeutral (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwIconTypeNeutral);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwIconTypeOn, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwIconTypeOn (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwIconTypeOn);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwIconTypeOff, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwIconTypeOff (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwIconTypeOff);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwIconTypeFull, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwIconTypeFull (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwIconTypeFull);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayIconSetStandardPhone, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayIconSetStandardPhone (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayIconSetStandardPhone);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayIconSetInOutButton, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayIconSetInOutButton (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayIconSetInOutButton);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayIconSetMobilePhone, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayIconSetMobilePhone (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayIconSetMobilePhone);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayIconSetPDA, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayIconSetPDA (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayIconSetPDA);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayIconSetTraffic, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayIconSetTraffic (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayIconSetTraffic);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayIconSetWiViT, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayIconSetWiViT (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayIconSetWiViT);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodaySingleLineHeight, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodaySingleLineHeight (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodaySingleLineHeight);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwTodayDoubleLineHeight, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwTodayDoubleLineHeight (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwTodayDoubleLineHeight);

//=====================
// Phone Stuff
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Phone Stuff")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwPhoneAccessOK, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwPhoneAccessOK (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwPhoneAccessOK);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwPhoneAccessError, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwPhoneAccessError (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwPhoneAccessError);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwPhoneStateUnknown, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwPhoneStateUnknown (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwPhoneStateUnknown);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwPhoneStateOn, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwPhoneStateOn (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwPhoneStateOn);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwPhoneStateOff, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwPhoneStateOff (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwPhoneStateOff);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwServiceProvider, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwServiceProvider (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwServiceProvider);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwIMSINumber, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwIMSINumber (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwIMSINumber);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwMSISDNNumber, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwMSISDNNumber (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwMSISDNNumber);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwICCIDNumber, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwICCIDNumber (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwICCIDNumber);


//=====================
// Security Stuff
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Security Stuff")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceNoAuto, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceNoAuto (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceNoAuto);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceNoPIN, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceNoPIN (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceNoPIN);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequencePIN, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequencePIN (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequencePIN);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceStepStart, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceStepStart (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceStepStart);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceActionNoPIN, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceActionNoPIN (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceActionNoPIN);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceActionCreate, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceActionCreate (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceActionCreate);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceActionChange, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceActionChange (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceActionChange);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceActionCurrent, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceActionCurrent (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceActionCurrent);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceActionNew, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceActionNew (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceActionNew);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceActionEnter, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceActionEnter (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceActionEnter);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceActionConfirm, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceActionConfirm (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceActionConfirm);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwSecuritySequenceActionNone, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwSecuritySequenceActionNone (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwSecuritySequenceActionNone);


//=====================
// Notifications
//=====================
	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Notifications")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyNone, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyNone (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyNone);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyICCID, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyICCID (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyICCID);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyIMSI, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyIMSI (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyIMSI);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyRadioPresence, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyRadioPresence (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyRadioPresence);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyRadioState, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyRadioState (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyRadioState);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifySIMLock, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifySIMLock (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifySIMLock);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyRegistration, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyRegistration (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyRegistration);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyNumber, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyNumber (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyNumber);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyOperator, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyOperator (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyOperator);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyEquipmentInfo, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyEquipmentInfo (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyEquipmentInfo);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyPhonebook, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyPhonebook (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyPhonebook);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyHandle, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyHandle (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyHandle);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyLicense, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyLicense (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyLicense);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifySerialNumber, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifySerialNumber (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifySerialNumber);

#ifdef SSS_V2_IMP
	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifySignalQuality, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifySignalQuality (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifySignalQuality);
#endif //#ifdef SSS_V2_IMP

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifySIMError, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifySIMError (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifySIMError);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifySIMUnlockError, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifySIMUnlockError (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifySIMUnlockError);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bDWordData[nDWordsByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bDWordData[nDWordsByteIndex], &dwNotifyAPIError, nLen);
	nDWordsByteIndex += nLen;
	dwDWordCount += 1;
	TraceCode(_D("\r\n#define g_dwNotifyAPIError (WiV::GlobalGetDword(%d)) // (0x%08X)"), dwDWordCount, dwNotifyAPIError);


	// Specify number of DWORDs
	pm_SSS_Special->dwNumDWords = dwDWordCount;

	// Specify total size of DWORDs
	pm_SSS_Special->dwDWordsSize = nDWordsByteIndex;

	return nDWordsByteIndex;
}

int WiVInitOther()
{
	int		nLen;
	int		nOtherByteIndex;
	DWORD	dwOtherCount;

	DWORD	dwParams	= 0x86518D15;
	DWORD	dwCRC		= 0xC8425739;

//==========================================
// Copy the data and generate code as we go
//==========================================
	
//	dwParams = CalcCRC32((LPBYTE)&GetExp, sizeof(SSS_GLOBALS), 0);
//	dwCRC = CalcCRC32((LPBYTE)&MakeLic, sizeof(SSS_GLOBALS), 0);

	TraceCode(_D("\r\n"));

	dwOtherCount = 0;
	nOtherByteIndex = 0;

	TraceCode(_D("\r\n\r\n//=====================")); 
	TraceCode(_D("\r\n// Other")); 
	TraceCode(_D("\r\n//=====================")); 

	nLen = sizeof(DWORD);
	pm_SSS_Special->bOtherData[nOtherByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bOtherData[nOtherByteIndex], &dwParams, nLen);
	nOtherByteIndex += nLen;
	dwOtherCount += 1;
	TraceCode(_D("\r\n#define g_dwParams (WiV::GlobalGetOther(%d)) // (0x%08X)"), dwOtherCount, dwParams);

	nLen = sizeof(DWORD);
	pm_SSS_Special->bOtherData[nOtherByteIndex++] = nLen;
	memcpy(&pm_SSS_Special->bOtherData[nOtherByteIndex], &dwCRC, nLen);
	nOtherByteIndex += nLen;
	dwOtherCount += 1;
	TraceCode(_D("\r\n#define g_dwCRC (WiV::GlobalGetOther(%d)) // (0x%08X)"), dwOtherCount, dwCRC);

	TraceCode(_D("\r\n"));

	// Specify number of other values
	pm_SSS_Special->dwNumOther = dwOtherCount;

	// Specify total size of other values
	pm_SSS_Special->dwOtherSize = nOtherByteIndex;

	return nOtherByteIndex;
}

} // namespace WiV

