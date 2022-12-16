//======================================================================
//
// WiVLang.cpp: Implementation of the CWiVLang class.
//
//======================================================================
#include <windows.h>

#include "SSSCommon.h"
#include "WiVUtils.h"
#include "WiVLang.h"
#include "WiVReg.h"

//======================================================================
//Member variables
//======================================================================

extern SSS_GLOBALS *SSSGlobals;

// Helper instance
static	CWiVLang	WiVLang;

static	HANDLE		m_hfFile = NULL;
static	TCHAR		m_szLangFilePath[MAX_PATH + 1] = WIV_EMPTY_STRING;
static	PBYTE		m_pbBuff = 0; 
static	long		m_lThreadId = 0L;
static	DWORD		m_dwFileSize = 0;
static	DWORD		m_dwBufferPointer = 0;
static	DWORD		m_dwLanguagePointer = 0;
static	bool		m_blStringTableLoaded = false;

// Constructor
CWiVLang::CWiVLang()
{
	m_nNumLanguages = 0;
}

// Destructor
CWiVLang::~CWiVLang()
{
	// Close any opened language file.
	if (m_hfFile != INVALID_HANDLE_VALUE && m_hfFile != NULL)
	{
		::CloseHandle (m_hfFile);
		m_hfFile = NULL;
	}

	// Free any file buffer.
	if (m_pbBuff)
	{
		::LocalFree (m_pbBuff);
		m_pbBuff = NULL;
	}

	if (m_pTranStrings)
	{
		delete m_pTranStrings;
		m_pTranStrings = NULL;
	}

	return;
}

// Lock the Lang file for exclusive access
void CWiVLang::Lock()
{
	long nThreadId = ::GetCurrentThreadId();

	while(m_lThreadId!=nThreadId)
	{
		// Wait until successfully locked
		::InterlockedCompareExchange(&m_lThreadId, nThreadId, 0);

		if(m_lThreadId==nThreadId) break;

		::Sleep(25);
	}
}

// Release lock
void CWiVLang::Unlock()
{
	// Only the thread that set the lock can release it
	::InterlockedCompareExchange(&m_lThreadId, 0, ::GetCurrentThreadId());
}

// Open the language file
HANDLE CWiVLang::OpenFile()
{
	TraceEnter(_D("CWiVLang::OpenFile"), tlInternal);

	// Construct the language file path
	_snwprintf (m_szLangFilePath, MAX_PATH, _T("%s%s%s"), GetInstallPath(), g_szProductShortName, _T(".lang"));

	TraceInternal(_D("CWiVLang::OpenFile: File = %s"), m_szLangFilePath);

	// Open the Lang file
	m_hfFile = ::CreateFile
	(
		m_szLangFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (m_hfFile == INVALID_HANDLE_VALUE || m_hfFile == NULL)
	{
//		MessageBox (NULL, SSS_TEXT_CANT_OPEN_FILE, SSS_TEXT_ERROR_MESSAGEBOX, MB_OK);
		TraceWarning(_D("CWiVLang::OpenFile: Cannot open file"));
		TraceLeave(_D("CWiVLang::OpenFile"), (DWORD)NULL, tlInternal);
		return NULL;
	}

	// Get the size of the file
	m_dwFileSize = (int)::GetFileSize (m_hfFile, NULL);

	TraceInternal(_D("CWiVLang::OpenFile: File size = %d"), m_dwFileSize);

	// See if file > 2Gig
	if (m_dwFileSize < 0)
	{
		TraceError(_D("CWiVLang::OpenFile: Unmanageable file size"));
		TraceLeave(_D("CWiVLang::OpenFile"), (DWORD)NULL, tlInternal);
		return NULL;
	}

	// Free any existing file buffer.
	if (m_pbBuff)
	{
		TraceInternal(_D("CWiVLang::OpenFile: Freeing existing memory buffer"));
		::LocalFree (m_pbBuff);
	}

	// Allocate a zero initialised buffer.
	TraceInternal(_D("CWiVLang::OpenFile: Allocating %d zero initialised bytes of memory"), m_dwFileSize + 2);
	m_pbBuff = (PBYTE)::LocalAlloc (LPTR, m_dwFileSize + 2);
	if (!m_pbBuff)
	{
//		MessageBox (NULL, SSS_TEXT_ERROR_NOT_ENOUGH_MEMORY, SSS_TEXT_ERROR_MESSAGEBOX, MB_OK);
		TraceError(_D("CWiVLang::OpenFile: Could not allocate memory"));
		TraceLeave(_D("CWiVLang::OpenFile"), (DWORD)NULL, tlInternal);
		return NULL;
	}

	// Initialize the buffer.
//	memset(m_pbBuff, 0, m_dwFileSize + 2);

	// Return the file handle
	TraceLeave(_D("CWiVLang::OpenFile"), (DWORD)m_hfFile, tlInternal);
	return m_hfFile;
}

// Close the current language file
void CWiVLang::CloseFile()
{
	TraceEnter(_D("CWiVLang::CloseFile"), tlInternal);

	// Close any opened language file.
	if (m_hfFile != INVALID_HANDLE_VALUE && m_hfFile != NULL)
	{
		TraceInternal(_D("CWiVLang::CloseFile: Closing file"));
		::CloseHandle (m_hfFile);
		m_hfFile = NULL;
	}

	// Free any file buffer.
	if (m_pbBuff)
	{
		TraceInternal(_D("CWiVLang::CloseFile: Freeing memory"));
		::LocalFree (m_pbBuff);
		m_pbBuff = NULL;
	}

	TraceLeave(_D("CWiVLang::CloseFile"), tlInternal);
	return;
}

// Attempt to read the language file into a buffer. 
long CWiVLang::LoadFile()
{
	DWORD	dwBytesRead;
	DWORD	dwFileSize = m_dwFileSize;

	TraceEnter(_D("CWiVLang::LoadFile"), tlInternal);

	// Reset file pointer
	m_dwBufferPointer = 0;

	// Check file handle
	if (m_hfFile == INVALID_HANDLE_VALUE || m_hfFile == NULL)
	{
		TraceError(_D("CWiVLang::LoadFile: File handle is null or invalid"));
		TraceLeave(_D("CWiVLang::LoadFile"), (DWORD)-1, tlInternal);
		return -1;
	}

	// Check file name specified
	if (_tcslen(m_szLangFilePath) == 0)
	{
		TraceError(_D("CWiVLang::LoadFile: File name is blank"));
		TraceLeave(_D("CWiVLang::LoadFile"), (DWORD)-1, tlInternal);
		return -1;
	}

	// Check file size
	if (dwFileSize == 0)
	{
		TraceError(_D("CWiVLang::LoadFile: File size is zero"));
		TraceLeave(_D("CWiVLang::LoadFile"), (DWORD)-1, tlInternal);
		return -1;
	}

	TraceInternal(_D("CWiVLang::LoadFile: Reading file"));
	if (!::ReadFile (m_hfFile, m_pbBuff, m_dwFileSize, &dwBytesRead, NULL))
	{
		TraceError(_D("CWiVLang::LoadFile: ReadFile failed"));
		TraceLeave(_D("CWiVLang::LoadFile"), (DWORD)-1, tlInternal);
		return -1;
	}

	// Check for end of file. 
	if (dwBytesRead == 0) 
	{ 
		// We're at the end of the file 
		TraceInternal(_D("CWiVLang::LoadFile: End of file"));
		TraceLeave(_D("CWiVLang::LoadFile"), (DWORD)0, tlInternal);
		return 0;
	} 

	while (dwFileSize > 0)
	{
		dwFileSize -= dwBytesRead;
		if (dwFileSize < 0) dwFileSize = 0;

		TraceInternal(_D("CWiVLang::LoadFile: Read %d bytes from file, FileSize now %d"), dwBytesRead, dwFileSize);
// TODO: What to do if not all of file read first time
		TraceInternal(_D("CWiVLang::LoadFile: Reading file"));
		if (!::ReadFile (m_hfFile, m_pbBuff, m_dwFileSize, &dwBytesRead, NULL)) break;
	}

	TraceLeave(_D("CWiVLang::LoadFile"), (DWORD)0, tlInternal);
	return 0;
}

bool CWiVLang::ValidateFile(LPCTSTR lpszFileID, LPCTSTR lpszVersion)
{
	TCHAR szLine[WIV_MAX_STRING + 1];

	bool blResult = false;

	TraceEnter(_D("CWiVLang::ValidateFile"), tlInternal);
	TraceInternal(_D("CWiVLang::ValidateFile: File ID to check = <%s>, Version to check = <%s>"), lpszFileID, lpszVersion);
	
	// While there a lines left to check, look for header line
	while(m_dwBufferPointer < m_dwFileSize)
	{
		_zclr(szLine);
		if (WiVLang.GetLine(szLine) == WIV_LANG_KEY_HEADER)
		{
			TraceInternal(_D("CWiVLang::ValidateFile: Header line found with value <%s>"), szLine);
			if (_tcscmp(szLine, lpszFileID) == 0)
			{
				TraceInternal(_D("CWiVLang::ValidateFile: Header line confirmed OK"));
				while(m_dwBufferPointer < m_dwFileSize)
				{
					_zclr(szLine);
					if (WiVLang.GetLine(szLine) == WIV_LANG_KEY_VERSION)
					{
						// Version line found
						TraceInternal(_D("CWiVLang::ValidateFile: Version line found with value <%s>"), szLine);
						if (_tcscmp(szLine, lpszVersion) == 0)
						{
							TraceInternal(_D("CWiVLang::ValidateFile: Version confirmed OK"));
							blResult = true;
						}
						else
						{
							TraceInternal(_D("CWiVLang::ValidateFile: Invalid Version"));
						}

						break;
					}
				}
			}
			else
			{
				TraceInternal(_D("CWiVLang::ValidateFile: Invalid Header line"));
			}

			break;
		}
	}

	TraceLeave(_D("CWiVLang::ValidateFile"), (DWORD)blResult, tlInternal);

	return blResult;
}

bool CWiVLang::LoadTable(LPCTSTR lpszProduct, LPCTSTR lpszLanguage)
{
	TCHAR	szLine[WIV_MAX_STRING + 1];
	bool	blResult = false;
	int		nKey = 0;

	TraceEnter(_D("CWiVLang::LoadTable"), tlInternal);
	TraceInternal(_D("CWiVLang::LoadTable: Product = <%s>, Language = <%s>"), lpszProduct, lpszLanguage);

	if (m_pTranStrings)
	{
		delete m_pTranStrings;
	}

	m_pTranStrings = NULL;

	m_dwLanguagePointer = 0;
	WiVLang.m_nNumLanguages = 0;

	// While there are lines left to check, look for correct product
	while(m_dwBufferPointer < m_dwFileSize)
	{
		_zclr(szLine);
		if (WiVLang.GetLine(szLine) == WIV_LANG_KEY_PRODUCT)
		{
			TraceInternal(_D("CWiVLang::LoadTable: Product line found with value <%s>"), szLine);
			if (_tcscmp(szLine, lpszProduct) == 0)
			{
				TraceInternal(_D("CWiVLang::LoadTable: Product line confirmed OK"));
				while(m_dwBufferPointer < m_dwFileSize)
				{
					_zclr(szLine);
					if (WiVLang.GetLine(szLine) == WIV_LANG_KEY_LANGUAGE_ID)
					{
						// Language ID line found
						TraceInternal(_D("CWiVLang::LoadTable: Language ID line found with value <%s>"), szLine);

						_tcsncpy(WiVLang.m_asLanguages[WiVLang.m_nNumLanguages].ID, szLine, WIV_MAX_LANGUAGE_ID);

						if (WiVLang.GetLine(szLine) == WIV_LANG_KEY_LANGUAGE_NAME)
						{
							TraceInternal(_D("CWiVLang::LoadTable: Language Name line found with value <%s>"), szLine);
							_tcsncpy(WiVLang.m_asLanguages[WiVLang.m_nNumLanguages].Name, szLine, WIV_MAX_NAME);
						}

						WiVLang.m_nNumLanguages++;

						if (_tcscmp(WiVLang.m_asLanguages[WiVLang.m_nNumLanguages - 1].ID, lpszLanguage) == 0)
						{
							TraceInternal(_D("CWiVLang::LoadTable: Language confirmed OK"));
							
							// Store pointer to required language for use later
							m_dwLanguagePointer = m_dwBufferPointer;

							TraceInternal(_D("CWiVLang::LoadTable: Required language pointer = %d"), m_dwLanguagePointer);
						}
						else
						{
							TraceInternal(_D("CWiVLang::LoadTable: Not a language line, so continue"));
						}

						break;
					}
				}
				if (!blResult) continue;
			}
			else
			{
				TraceInternal(_D("CWiVLang::LoadTable: Not the product we want, so continue"));
				continue;
			}

			break;
		}
	}

	TraceInternal(_D("CWiVLang::LoadTable: %d Languages found"), WiVLang.m_nNumLanguages);

	if (m_dwLanguagePointer != 0)
	{
		// Reset pointer to required language
		m_dwBufferPointer = m_dwLanguagePointer;

		// Now find the TABLE:START line
		while(m_dwBufferPointer < m_dwFileSize)
		{
			_zclr(szLine);
			if (WiVLang.GetLine(szLine) == WIV_LANG_KEY_TABLE)
			{
				// Table line found
				TraceInternal(_D("CWiVLang::LoadTable: table line found with value <%s>"), szLine);
				if (_tcscmp(szLine, WIV_LANG_TEXT_TABLE_BEGIN) == 0)
				{
					TraceInternal(_D("CWiVLang::LoadTable: Table start confirmed OK"));
					blResult = true;
					break;
				}
				else
				{
					TraceInternal(_D("CWiVLang::LoadTable: Not a table start line, so continue"));
					continue;
				}
			}
		}
	}

	if (blResult)
	{
		// Found the correct table, so load it into the array

		// While there are lines left to check, look for TABLE:END line
		while(m_dwBufferPointer < m_dwFileSize)
		{
			nKey = WiVLang.GetLine(szLine);
			TraceInternal(_D("CWiVLang::LoadTable: WiVLang.GetLine returned <%08X>"), nKey);

			if (nKey > 0)
			{
				TraceInternal(_D("CWiVLang::LoadTable: Translation line found with index = %d and value = <%s>"), nKey, szLine);
				if ((nKey < 0) || (nKey > WIV_CUSTOM_MAX_TABLE_ENTRIES))
				{
					TraceInternal(_D("CWiVLang::LoadTable: Translation line not stored, index = %d out of bounds"), nKey);
				}
				else
				{
					if (m_pTranStrings == NULL)
					{
						TraceInternal(_D("CWiVLang::LoadTable: m_pTranStrings is NULL, so creating storage for strings"));
						m_pTranStrings = new WIVTRANSTRINGS;
						if (m_pTranStrings == NULL)
						{
							TraceError(_D("CWiVLang::LoadTable: Could not allocate storage for m_pTranStrings"));
							break;
						}
						m_pTranStrings->nMaxEntries = WIV_CUSTOM_MAX_TABLE_ENTRIES - 1;
						m_pTranStrings->nLowIndex = m_pTranStrings->nMaxEntries - 1;
						m_pTranStrings->nHighIndex = 0;
					}

					if (nKey < m_pTranStrings->nLowIndex) m_pTranStrings->nLowIndex = nKey;
					if (nKey > m_pTranStrings->nHighIndex) m_pTranStrings->nHighIndex = nKey;
					_tcsncpy(m_pTranStrings->aszStrings[nKey], szLine, WIV_MAX_STRING);
					TraceInternal(_D("CWiVLang::LoadTable: Translation line stored at index %d"), nKey);
				}
			}
			else
			{
				if (nKey == WIV_LANG_KEY_TABLE)
				{
					TraceInternal(_D("CWiVLang::LoadTable: TABLE line found with value <%s>"), szLine);
					if (_tcscmp(szLine, WIV_LANG_TEXT_TABLE_END) == 0)
					{
						TraceInternal(_D("CWiVLang::LoadTable: End of table found, so finish loading"));
						break;
					}
				}
				else if (nKey == WIV_LANG_KEY_EMPTY)
				{
					TraceInternal(_D("CWiVLang::LoadTable: Empty line found so ignoring"));
				}
				else if (nKey == WIV_LANG_KEY_COMMENT)
				{
					TraceInternal(_D("CWiVLang::LoadTable: Comment line found so ignoring"));
				}
				else
				{
					TraceInternal(_D("CWiVLang::LoadTable: Unknown line type found so ignoring, index = %d and value = <%s>"), nKey, szLine);
				}
			}
		}

		if (m_pTranStrings)
		{
			// Report high and low index values
			TraceInternal(_D("CWiVLang::LoadTable: Product and language table loaded successfully, low index = %d, high index = %d"), m_pTranStrings->nLowIndex, m_pTranStrings->nHighIndex);
		}
		else
		{
			TraceError(_D("CWiVLang::LoadTable: m_pTranStrings is NULL, not enough memory to allocate storage"));
			blResult = false;
		}
	}
	else
	{
		TraceWarning(_D("CWiVLang::LoadTable: Language <%s> for product <%s> not found"), lpszLanguage, lpszProduct);
	}
	
	m_blStringTableLoaded = blResult;
	TraceLeave(_D("CWiVLang::LoadTable"), (DWORD)blResult, tlInternal);

	return blResult;
}

int CWiVLang::GetLine(LPTSTR lpszLine)
{
	int		nKey = 0;
	DWORD	dwLineLength = 0;
	TCHAR	szLine[WIV_MAX_STRING + 1];
	char	*pszCRLF;
	TCHAR	szValue[WIV_MAX_STRING + 1];

	TraceEnter(_D("CWiVLang::GetLine"), tlInternal);

	// Check file handle
	if (m_hfFile == INVALID_HANDLE_VALUE || m_hfFile == NULL)
	{
		TraceError(_D("CWiVLang::GetLine: File handle is null or invalid"));
		TraceLeave(_D("CWiVLang::GetLine"), (DWORD)WIV_LANG_KEY_UNKNOWN, tlInternal);
		return WIV_LANG_KEY_UNKNOWN;
	}

	TraceInternal(_D("CWiVLang::GetLine: Buffer pointer = %d"), m_dwBufferPointer);

	// Get the first line
	pszCRLF = strstr( (PSZ)&m_pbBuff[m_dwBufferPointer], WIV_CUSTOM_NEW_LINE );

	if (pszCRLF == NULL)
	{
		TraceInternal(_D("CWiVLang::GetLine: No end of line found, use end of file instead"));
		dwLineLength = m_dwFileSize - m_dwBufferPointer;
	}
	else
	{
		TraceInternal(_D("CWiVLang::GetLine: End of line found at <%08p>"), pszCRLF);
		dwLineLength = pszCRLF - (PSZ)&m_pbBuff[m_dwBufferPointer];
	}

	TraceInternal(_D("CWiVLang::GetLine: Line Length = %d"), dwLineLength);

	_zclr(szLine);
	_zclr(szValue);

	if (dwLineLength > 0)
	{
		if (m_pbBuff[m_dwBufferPointer] == WIV_CUSTOM_COMMENT)
		{
			TraceInternal(_D("CWiVLang::GetLine: Comment line"));
			nKey =  WIV_LANG_KEY_COMMENT;
		}
		else
		{
			// Convert to a Unicode String
			TraceInternal(_D("CWiVLang::GetLine: Calling MultiByteToWideChar to convert line to unicode"));
			MultiByteToWideChar(CP_ACP, 0, (PSZ)&m_pbBuff[m_dwBufferPointer], dwLineLength, szLine, WIV_MAX_STRING);
			TraceInternal(_D("CWiVLang::GetLine: Line converted"));

			TraceInternal(_D("CWiVLang::GetLine: Calling ParseLine for line <%s>"), szLine);
			nKey = ParseLine(szLine, szValue);
			TraceInternal(_D("CWiVLang::GetLine: Back from ParseLine, index = %d, line = <%s>, value = <%s>"), nKey, szLine, szValue);

			_tcsncpy(lpszLine, szValue, WIV_MAX_STRING);
		}
	}
	else
	{
		TraceInternal(_D("CWiVLang::GetLine: Empty line"));
		nKey = WIV_LANG_KEY_EMPTY;
	}

	// Update buffer pointer
	m_dwBufferPointer += (dwLineLength + 2);
	TraceInternal(_D("CWiVLang::GetLine: Buffer pointer now = %d"), m_dwBufferPointer);

	TraceLeave(_D("CWiVLang::GetLine"), (DWORD)nKey, tlInternal);
	return nKey;
}

int CWiVLang::ParseLine(LPCTSTR lpszLine, LPTSTR lpszValue)
{
	int		nKey = 0;
	DWORD	dwLinePointer = 0;
	DWORD	dwValueLength = 0;
	DWORD	dwLineLength = _tcslen(lpszLine);
	TCHAR	szLine[WIV_MAX_STRING + 1];
	TCHAR	szKey[WIV_CUSTOM_MAX_KEY_SIZE + 1];
	TCHAR	*pszDelim;

	TraceEnter(_D("CWiVLang::ParseLine"), tlInternal);

	TraceInternal(_D("CWiVLang::ParseLine: Line length = %d, line = <%s>"), dwLineLength, lpszLine);


	if (dwLineLength == 0)
	{
		TraceInternal(_D("CWiVLang::ParseLine: Empty line"));
		TraceLeave(_D("CWiVLang::ParseLine"), WIV_LANG_KEY_EMPTY, tlInternal);
		return WIV_LANG_KEY_EMPTY;
	}

	_zcpy(szLine, lpszLine);

	// Find the first delimiter
	pszDelim = _tcsstr( &szLine[dwLinePointer], WIV_CUSTOM_DELIM);

	// If no delimiter
	if (pszDelim == NULL)
	{
		TraceInternal(_D("CWiVLang::ParseLine: No delimiter"));
		TraceLeave(_D("CWiVLang::ParseLine"), (DWORD)WIV_LANG_KEY_EMPTY, tlInternal);
		return WIV_LANG_KEY_EMPTY;
	}

	TraceInternal(_D("CWiVLang::ParseLine: Delimiter found at <%08p>"), pszDelim);

	dwValueLength = pszDelim - &szLine[dwLinePointer];

	TraceInternal(_D("CWiVLang::ParseLine: Value Length = %d"), dwValueLength);

	_zclr(szKey);
	_tcsncpy(szKey, &szLine[dwLinePointer], min(dwValueLength, WIV_CUSTOM_MAX_KEY_SIZE));
	TraceInternal(_D("Key = <%s>"), szKey);

	if (_tcsicmp(szKey, g_szCustomFileHeader) == 0)
	{
		TraceInternal(_D("Key is the header"));
		nKey = WIV_LANG_KEY_HEADER;

	}else if (_tcsicmp(szKey, WIV_LANG_TEXT_VERSION) == 0)
	{
		TraceInternal(_D("Key is the version"));
		nKey = WIV_LANG_KEY_VERSION;

	}else if (_tcsicmp(szKey, WIV_LANG_TEXT_PRODUCT) == 0)
	{
		TraceInternal(_D("Key is the product"));
		nKey = WIV_LANG_KEY_PRODUCT;

	}else if (_tcsicmp(szKey, WIV_LANG_TEXT_LANGUAGE_ID) == 0)
	{
		TraceInternal(_D("Key is the language ID"));
		nKey = WIV_LANG_KEY_LANGUAGE_ID;

	}else if (_tcsicmp(szKey, WIV_LANG_TEXT_LANGUAGE_NAME) == 0)
	{
		TraceInternal(_D("Key is the language name"));
		nKey = WIV_LANG_KEY_LANGUAGE_NAME;

	}else if (_tcsicmp(szKey, WIV_LANG_TEXT_TABLE) == 0)
	{
		TraceInternal(_D("Key is the table"));
		nKey = WIV_LANG_KEY_TABLE;
	}
	else
	{
		nKey = _wtoi(szKey);
		TraceInternal(_D("Key number is %d"), nKey);
	}

	// Update line pointer
	dwLinePointer += (dwValueLength + 2);

	TraceInternal(_D("CWiVLang::ParseLine: Line pointer now = %d"), dwLinePointer);

	// Get next value
	TraceInternal(_D("CWiVLang::ParseLine, Value length = %d"), dwLineLength - dwLinePointer);
	_tcsncpy(lpszValue, &szLine[dwLinePointer], dwLineLength - dwLinePointer );
	TraceInternal(_D("CWiVLang::ParseLine, Value <%s>"), lpszValue);

	TraceLeave(_D("CWiVLang::ParseLine"), (DWORD)nKey, tlInternal);
	return nKey;
}

LPCTSTR CWiVLang::GetEntry(const int nKey)
{
	LPCTSTR	pszEntry;
//	int		nLowIndex;
//	int		nHighIndex;

	TraceEnter(_D("CWiVLang::GetEntry"), tlInternal);
	TraceInternal(_D("CWiVLang::GetEntry: Key = %d"), nKey);

	if (!m_blStringTableLoaded)
	{
		TraceInternal(_D("CWiVLang::GetEntry: String table not loaded"));
		TraceLeave(_D("CWiVLang::GetEntry"), (LPCTSTR)NULL, tlInternal);
		return NULL;
	}

//	nLowIndex = _wtoi(m_szTranStrings[WIV_CUSTOM_LOWEST_KEY_INDEX].szString);
//	nHighIndex = _wtoi(m_szTranStrings[WIV_CUSTOM_HIGHEST_KEY_INDEX].szString);

//	if ((nKey < nLowIndex) || (nKey > nHighIndex))
	if ((nKey < m_pTranStrings->nLowIndex) || (nKey > m_pTranStrings->nHighIndex))
	{
		TraceInternal(_D("CWiVLang::GetEntry, Index %d out of bounds"), nKey);
		TraceLeave(_D("CWiVLang::GetEntry"), (LPCTSTR)NULL, tlInternal);
		return NULL;
	}

	// Get the string
	pszEntry = m_pTranStrings->aszStrings[nKey];
//	pszEntry = m_szTranStrings[nKey].szString;

	TraceLeave(_D("CWiVLang::GetEntry"), (LPCTSTR)pszEntry, tlInternal);

	return pszEntry;
}

bool LangLoadTable(LPCTSTR lpszProduct, LPCTSTR lpszLanguage)
{
	bool	blResult = false;

	TraceEnter(_D("LangLoadTable"), tlInternal);

	TraceInternal(_D("LangLoadTable: Locking"));
	WiVLang.Lock();
	TraceInternal(_D("LangLoadTable: Locked"));

	TraceInternal(_D("LangLoadTable: Opening file"));
	if (WiVLang.OpenFile() != NULL)
	{
		TraceInternal(_D("LangLoadTable: File opened"));
		TraceInternal(_D("LangLoadTable: Loading file"));
		if (WiVLang.LoadFile() >= 0)
		{
			TraceInternal(_D("LangLoadTable: File loaded"));

			TraceInternal(_D("LangLoadTable: Validating file"));
			if (WiVLang.ValidateFile(WIV_LANG_TEXT_HEADER_ID, _T("1.0")))
			{
				TraceInternal(_D("LangLoadTable: File Validated"));
				TraceInternal(_D("LangLoadTable: Loading translation table"));
				if (WiVLang.LoadTable(lpszProduct, lpszLanguage))
				{
					TraceInternal(_D("LangLoadTable: Translation table loaded"));
					blResult = true;
				}
			}
		}

		TraceInternal(_D("LangLoadTable: Closing file"));
		WiVLang.CloseFile();
		TraceInternal(_D("LangLoadTable: File closed"));
	}

	TraceInternal(_D("LangLoadTable: Unlocking"));
	WiVLang.Unlock();
	TraceInternal(_D("LangLoadTable: Unlocked"));

	TraceLeave(_D("LangLoadTable"), (DWORD)blResult, tlInternal);

	return blResult;
}

int LangGetLanguagesList(PWIVLANG *lppsLanguages)
{
	int	nNumLanguages = WiVLang.m_nNumLanguages;

	*lppsLanguages = WiVLang.m_asLanguages;
	return nNumLanguages;
} 

bool LangGetTableStatus()
{
	return m_blStringTableLoaded;
} 

PWIVLANG LangGetCurrentLanguage()
{
	PWIVLANG	psRetVal = NULL;

	TraceEnter(_D("LangGetCurrentLanguage"), tlInternal);

	psRetVal =  &WiVLang.m_sCurrentLanguage;
	TraceLeave(_D("LangGetCurrentLanguage"), (DWORD)psRetVal, tlInternal);
	return psRetVal;
}

void LangSetCurrentLanguage(WIVLANG sLanguage)
{
	TraceEnter(_D("LangSetCurrentLanguage"), tlInternal);

	_tcsncpy(WiVLang.m_sCurrentLanguage.ID, sLanguage.ID, WIV_MAX_LANGUAGE_ID);
	_tcsncpy(WiVLang.m_sCurrentLanguage.Name, sLanguage.Name, WIV_MAX_NAME);

	TraceLeave(_D("LangSetCurrentLanguage"), tlInternal);
	return;
}

PWIVLANG LangGetDefaultLanguage()
{
	PWIVLANG	psRetVal = NULL;

	TraceEnter(_D("LangGetDefaultLanguage"), tlInternal);

	LangReadDefaultLanguage();

	if (_tcslen(WiVLang.m_sCurrentLanguage.ID) == 0)
	{
		_tcsncpy(WiVLang.m_sCurrentLanguage.ID, WiVLang.m_sDefaultLanguage.ID, WIV_MAX_LANGUAGE_ID);
		_tcsncpy(WiVLang.m_sCurrentLanguage.Name, WiVLang.m_sDefaultLanguage.Name, WIV_MAX_NAME);
	}

	psRetVal =  &WiVLang.m_sDefaultLanguage;
	TraceLeave(_D("LangGetDefaultLanguage"), (DWORD)psRetVal, tlInternal);
	return psRetVal;
}

void LangSetDefaultLanguage(WIVLANG sLanguage)
{
	TraceEnter(_D("LangSetDefaultLanguage"), tlInternal);

	_tcsncpy(WiVLang.m_sDefaultLanguage.ID, sLanguage.ID, WIV_MAX_LANGUAGE_ID);
	_tcsncpy(WiVLang.m_sDefaultLanguage.Name, sLanguage.Name, WIV_MAX_NAME);

	LangWriteDefaultLanguage();

	TraceLeave(_D("LangSetDefaultLanguage"), tlInternal);
	return;
}

DWORD LangReadDefaultLanguage()
{
	DWORD dwResult;
	HKEY  hKey;
	TCHAR	szLanguageID[WIV_MAX_LANGUAGE_ID + 1] = WIV_EMPTY_STRING;
	TCHAR	szLanguageName[WIV_MAX_NAME + 1] = WIV_EMPTY_STRING;

	TraceEnter(_D("LangReadDefaultLanguage"), tlInternal);
	dwResult = RegKeyOpen(WIV_REG_LANGUAGE_KEY, &hKey);

	if (dwResult == REG_CREATED_NEW_KEY)
	{
		dwResult = ERROR_SUCCESS;
	}
	else
	{
		dwResult = ERROR_SUCCESS;
	}

	if (dwResult != ERROR_SUCCESS)
	{
		RegKeyClose(hKey);
		TraceLeave(_D("LangReadDefaultLanguage"), -1, tlInternal);
		return -1;
	}

	TraceInternal(_D("LangReadDefaultLanguage: Calling RegGetValue for %s"), WIV_REG_DEFAULT_LANGUAGE_ID);
	dwResult = RegGetValue(hKey, WIV_REG_DEFAULT_LANGUAGE_ID, szLanguageID, WIV_MAX_LANGUAGE_ID, WIV_EMPTY_STRING);
	TraceInternal(_D("LangReadDefaultLanguage: Back from RegGetValue, Result = %d, Language ID = <%s>"), dwResult, szLanguageID);

	TraceInternal(_D("LangReadDefaultLanguage: Calling RegGetValue for %s"), WIV_REG_DEFAULT_LANGUAGE_NAME);
	dwResult = RegGetValue(hKey, WIV_REG_DEFAULT_LANGUAGE_NAME, szLanguageName, WIV_MAX_NAME, WIV_EMPTY_STRING);
	TraceInternal(_D("LangReadDefaultLanguage: Back from RegGetValue, Result = %d, Language Name = <%s>"), dwResult, szLanguageName);

	RegKeyClose(hKey);

	_tcsncpy(WiVLang.m_sDefaultLanguage.ID, szLanguageID, WIV_MAX_LANGUAGE_ID);
	_tcsncpy(WiVLang.m_sDefaultLanguage.Name, szLanguageName, WIV_MAX_NAME);

	TraceLeave(_D("LangReadDefaultLanguage"), dwResult, tlInternal);
	return dwResult;
}

DWORD LangWriteDefaultLanguage()
{
	DWORD dwResult;
	HKEY  hKey;
	TCHAR	szLanguageID[WIV_MAX_LANGUAGE_ID + 1] = WIV_EMPTY_STRING;
	TCHAR	szLanguageName[WIV_MAX_NAME + 1] = WIV_EMPTY_STRING;

	TraceEnter(_D("LangReadDefaultLanguage"), tlInternal);
	dwResult = RegKeyOpen(WIV_REG_LANGUAGE_KEY, &hKey);

	if (dwResult == REG_CREATED_NEW_KEY)
	{
		dwResult = ERROR_SUCCESS;
	}
	else
	{
		dwResult = ERROR_SUCCESS;
	}

	if (dwResult != ERROR_SUCCESS)
	{
		RegKeyClose(hKey);
		TraceLeave(_D("LangWriteDefaultLanguage"), -1, tlInternal);
		return -1;
	}

	_tcsncpy(szLanguageID, WiVLang.m_sDefaultLanguage.ID, WIV_MAX_LANGUAGE_ID);
	_tcsncpy(szLanguageName, WiVLang.m_sDefaultLanguage.Name, WIV_MAX_NAME);

	TraceInternal(_D("LangWriteDefaultLanguage: Calling RegSetValue for %s"), WIV_REG_DEFAULT_LANGUAGE_ID);
	dwResult = RegSetValue(hKey, WIV_REG_DEFAULT_LANGUAGE_ID, szLanguageID);
	TraceInternal(_D("LangWriteDefaultLanguage: Back from RegSetValue, Result = %d"), dwResult);

	TraceInternal(_D("LangWriteDefaultLanguage: Calling RegSetValue for %s"), WIV_REG_DEFAULT_LANGUAGE_NAME);
	dwResult = RegSetValue(hKey, WIV_REG_DEFAULT_LANGUAGE_NAME, szLanguageName);
	TraceInternal(_D("LangWriteDefaultLanguage: Back from RegSetValue, Result = %d"), dwResult);

	RegKeyClose(hKey);

	TraceLeave(_D("LangWriteDefaultLanguage"), dwResult, tlInternal);
	return dwResult;
}

LPCTSTR	LangLoadString(HMODULE hmInstance, const UINT uiStringID)
{
	LPCTSTR	pszString;

	TraceEnter(_D("LangLoadString"), tlInternal);
	TraceInternal(_D("LangLoadString: String ID = %d"), uiStringID);

	if (m_blStringTableLoaded)
	{
		pszString = WiVLang.GetEntry(uiStringID);

		if (pszString == NULL)
		{
			TraceWarning(_D("LangLoadString: String ID <%d> not found in language table, so load it from resources"), uiStringID);
			pszString = (LPCTSTR)LoadString(hmInstance, uiStringID, NULL, 0);
		}
	}
	else
	{
		TraceInternal(_D("LangLoadString: String table not loaded, so load string from resources"));
		pszString = (LPCTSTR)LoadString(hmInstance, uiStringID, NULL, 0);
	}

	TraceLeave(_D("LangLoadString"), (DWORD)pszString, tlInternal);
	return pszString;
}