//////////////////////////////////////////////////////////////////////
//
// WiVTrace.cpp: Implementation of the CWiVTrace class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIV_DEBUG

#include "SSSCommon.h"

#include "WiVTrace.h"
#include "WiVReg.h"
#include "WiVUtils.h"

extern SSS_GLOBALS *SSSGlobals;

// Helper instance
static	CWiVTrace	WiVTrace;

// Data members
static	HANDLE		m_hfFile;
static	TCHAR		m_szTraceFilePrefix[MAX_PATH + 1] = WIV_EMPTY_STRING;
static	SYSTEMTIME	m_stStartTime;
static	TCHAR		m_szTraceFilePath[MAX_PATH + 1] = WIV_EMPTY_STRING;
static	TCHAR		m_szTimeStr[WIV_MAX_STRING + 1];
static	int			m_nCurrentTabWidth = 0;

static	long		m_lThreadId;

static	TraceLevel	m_tlTraceLevel;
static	bool		m_blActive = false;
static	bool		m_blCommit = false;
static	int			m_nTabWidth = WIV_DEFAULT_TAB_WIDTH;
static	int			m_nLineWidth = WIV_DEFAULT_LINE_WIDTH;
static	DWORD		m_dwFilter;
static	bool		m_blInternal = false;

static	int			m_nCodePos = -1;

// Constructor and Destructor
CWiVTrace::CWiVTrace()
{
	m_hfFile = NULL;
	m_tlTraceLevel = tlNone;
	m_lThreadId = 0;
	m_dwFilter = 0;
}

CWiVTrace::~CWiVTrace()
{
	CloseTraceFile();
}

// Close the current trace file
void CWiVTrace::CloseTraceFile()
{
	if(m_hfFile) ::CloseHandle(m_hfFile);
	m_hfFile = NULL;
}

// Open trace file
HANDLE CWiVTrace::OpenTraceFile()
{
	SYSTEMTIME	stLocalTime;

	// Get time and construct filename
	::GetLocalTime(&stLocalTime);

	if (_tcslen(m_szTraceFilePrefix) == 0)
	{
		_tcsncpy(m_szTraceFilePrefix, _D("Trace"), MAX_PATH);
	}

	// Construct the trace file path
	_snwprintf(m_szTraceFilePath, MAX_PATH, _D("%s_%04d%02d%02d_%02d%02d%02d_%X.txt"),	m_szTraceFilePrefix,
		stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
		::GetCurrentProcessId());

	// Open/Create the trace file
	m_hfFile = CreateFile
	(
		m_szTraceFilePath,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		NULL
	);


	// If successful, set file pointer to end of file
	// and save the start time
	if(m_hfFile)
	{
		SetFilePointer(m_hfFile, 0, NULL, FILE_END); 

		m_stStartTime = stLocalTime;
	}

	if (m_nCurrentTabWidth == 0) m_nCurrentTabWidth = 1;

	// Return the file handle
	return m_hfFile;
}

// Lock the trace file for exclusive access
void CWiVTrace::Lock()
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
void CWiVTrace::Unlock()
{
	// Only the thread that set the lock can release it
	::InterlockedCompareExchange(&m_lThreadId, 0, ::GetCurrentThreadId());
}

// Set the current trace level
void CWiVTrace::SetLevel(const TraceLevel nLevel)
{
	m_tlTraceLevel = (nLevel > tlNone ? nLevel : tlNone);
}

// Set the line width
void CWiVTrace::SetLineWidth(const int nLineWidth)
{
	m_nLineWidth = (nLineWidth <= WIV_MIN_LINE_WIDTH ? WIV_DEFAULT_LINE_WIDTH : nLineWidth > WIV_MAX_LINE_WIDTH ? WIV_MAX_LINE_WIDTH : nLineWidth);
}

// Set the tab width
void CWiVTrace::SetTabWidth(const int nTabWidth)
{
	m_nTabWidth = (nTabWidth <= WIV_MIN_TAB_WIDTH ? WIV_DEFAULT_TAB_WIDTH : nTabWidth > WIV_MAX_TAB_WIDTH ? WIV_MAX_TAB_WIDTH : nTabWidth);
}

// Set the commit flag
void CWiVTrace::SetCommit(const bool blCommit)
{
	m_blCommit = blCommit;
}

// Set the filter flags
void CWiVTrace::SetFilter(const DWORD dwFilter)
{
	m_dwFilter = dwFilter;
}

// Set the trace file name prefix
void CWiVTrace::SetFilePrefix(LPCTSTR strFilePrefix)
{
	// Close existing trace file first
	CloseTraceFile();

	// Copy the file name prefix
	_tcsncpy(m_szTraceFilePrefix, strFilePrefix, MAX_PATH);
}

LPCTSTR CWiVTrace::GetIndent()
{
	static TCHAR	szTab[WIV_MAX_LINE_WIDTH + 1];

	_zclr(szTab);

	if (m_nCurrentTabWidth > 0)
	{
		for (int i = 0; i < m_nCurrentTabWidth; i++)
			_tcscat(szTab, WIV_SPACE_STRING);
	}
	return szTab;
}

LPCTSTR CWiVTrace::TraceNewLine()
{    
	int				nLength;
	static TCHAR	szReturn[WIV_MAX_LINE_WIDTH +1];

	nLength = m_nCurrentTabWidth + _tcslen(m_szTimeStr)+11;
	_tcsncpy(szReturn, _D("\r\n"), WIV_MAX_LINE_WIDTH);

	for (int i = 0; i < nLength; i++)
		_tcscat(szReturn, WIV_SPACE_STRING);

	return szReturn;
}


void CWiVTrace::TraceOut(const TraceLevel nLevel, LPCTSTR strFormat, va_list vaArgs)
{
	bool	blManualOpen = false;

	// If current trace level is tlNone (off), return immediately
	if(m_tlTraceLevel == tlNone) return;

	// If the specified trace level is greater than the current trace level and is not equal to tlAlways,
	// or the specified trace level is equal to tlInternal and the internal flag is not set, return immediately.
	if (((nLevel != tlAlways) &&	(nLevel != tlInternal) && (nLevel != tlCode) && (nLevel > m_tlTraceLevel)) || ((nLevel == tlInternal) && !(m_blInternal))) return;

	WiVTrace.Lock();

	try
	{
		// Get trace file handle	
		HANDLE hFile = m_hfFile;

		// Get local time
		SYSTEMTIME stLocalTime;
		::GetLocalTime(&stLocalTime);

		// Open the trace file if not already open
		if(hFile==NULL)
		{
			hFile = WiVTrace.OpenTraceFile();
			blManualOpen = true;
		}

		// If date has changed, close the old
		// trace file and open a new one
		else if (stLocalTime.wYear!=m_stStartTime.wYear||
				stLocalTime.wMonth!=m_stStartTime.wMonth||
				stLocalTime.wDay!=m_stStartTime.wDay)
		{
			WiVTrace.CloseTraceFile();
			hFile = WiVTrace.OpenTraceFile();
		}

		// Write the trace message
		if(hFile)
		{
			const	int nMaxSize = 8192;
			TCHAR	pBuffer[nMaxSize + 51] = WIV_EMPTY_STRING;
			char	szBuffer[nMaxSize + 51]= "";
			int		nPos = 0;

			if (nLevel != tlCode)
			{
				GetTimeFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, &stLocalTime, NULL, m_szTimeStr, WIV_MAX_STRING);

				// Construct time stamp and thread ID
				nPos = _snwprintf(pBuffer, nMaxSize + 50, _D("%s_%X:%s"), m_szTimeStr, m_lThreadId, WiVTrace.GetIndent());
			}

			// Add the trace message
			nPos += _vsntprintf(&pBuffer[nPos],nMaxSize-nPos,strFormat,vaArgs);

			if (nLevel != tlCode)
			{
				// Add new line
				nPos += _snwprintf(&pBuffer[nPos], nMaxSize + 50, _D("\r\n"));
			}
			else
			{
				m_nCodePos += nPos;
			}

			// Convert to ascii
			WiV::UtoA(pBuffer, szBuffer, _tcslen(pBuffer));

			// Write to file
			DWORD dwBytes;
			::WriteFile(hFile,szBuffer,strlen(szBuffer),&dwBytes,NULL);
//			::WriteFile(hFile,pBuffer,nPos*2,&dwBytes,NULL);

			// If commit flag is set, ensure data is on disk
			if (m_blCommit)
				::FlushFileBuffers(hFile);

			if (blManualOpen)
				WiVTrace.CloseTraceFile();
		}
	}
	catch(...)
	{
		// TODO: MessageBox(NULL,_D("Exception"),_D("SSS"),MB_OKCANCEL | MB_ICONINFORMATION);
	}

	WiVTrace.Unlock();
}

LPCTSTR CWiVTrace::TraceBlock(LPCTSTR szIntro, LPCTSTR szMessage, int nLength)
{
    static TCHAR	szMsgOut[WIV_MAX_STRING + 1];

    TCHAR	szMsg[WIV_MAX_LINE_WIDTH + 1];
    TCHAR	szMsgBit[WIV_MAX_LINE_WIDTH + 1];
	TCHAR	szTemp[WIV_MAX_LINE_WIDTH + 1];

    DWORD	dwLen;
    DWORD	dwLenMsg;
    int		nLineCount;
    
    if ((nLength <= 0) || (nLength > WIV_MAX_LINE_WIDTH))
	{
        dwLen = WIV_MAX_LINE_WIDTH;
	}
    else
	{
        dwLen = nLength;
	}
    
    _tcsncpy(szMsg, szMessage, WIV_MAX_LINE_WIDTH);
    _tcsncpy(szMsgOut, szIntro, WIV_MAX_LINE_WIDTH);
    
    dwLenMsg = _tcslen(szMsg);
    
    nLineCount = 1;

    while (dwLenMsg > dwLen)
	{
        if (dwLenMsg <= dwLen)
		{
            _tcsncpy(szMsgBit, szMsg, WIV_MAX_LINE_WIDTH);
		}
        else
		{
            _tcsncpy(szMsgBit, szMsg, dwLen);
            _tcsncpy(szMsg, &szMsg[dwLenMsg - dwLen], WIV_MAX_LINE_WIDTH);
		}
        
        if (nLineCount > 1)
		{
			_tcsncpy(szTemp, TraceNewLine(), WIV_MAX_LINE_WIDTH);
			for (int i = 0; i < (int)_tcslen(szIntro); i++)
				_tcsncat(szTemp, WIV_SPACE_STRING, WIV_MAX_LINE_WIDTH - _tcslen(szTemp));

			_tcsncat(szTemp, szMsgBit, WIV_MAX_LINE_WIDTH - _tcslen(szTemp));
		}
        
        nLineCount = nLineCount + 1;
        _tcsncat(szMsgOut, szTemp, WIV_MAX_STRING - _tcslen(szMsgOut));
        dwLenMsg = _tcslen(szMsg);
	}
    
    if (dwLenMsg > 0)
	{

        _tcsncpy(szMsgBit, szMsg, WIV_MAX_LINE_WIDTH);

        if (nLineCount > 1)
		{
			_tcsncpy(szTemp, TraceNewLine(), WIV_MAX_LINE_WIDTH);
			for (int i = 0; i < (int)_tcslen(szIntro); i++)
				_tcsncat(szTemp, WIV_SPACE_STRING, WIV_MAX_LINE_WIDTH - _tcslen(szTemp));

			_tcsncat(szTemp, szMsgBit, WIV_MAX_LINE_WIDTH - _tcslen(szTemp));
		}
        
        _tcsncat(szMsgOut, szTemp, WIV_MAX_STRING - _tcslen(szMsgOut));
	}
    
    return szMsgOut;
}

bool TraceGetActive()
{
	DWORD	dwResult;
	HKEY	hKey;
	bool	bData;

	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);

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
		return TRUE;
	}

	bData = RegGetValue(hKey, WIV_REG_TRACE_ACTIVE, false);

	RegKeyClose(hKey);

	return bData;
}

bool TraceGetInternal()
{
	DWORD	dwResult;
	HKEY	hKey;
	bool	bData;

	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);

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
		return TRUE;
	}

	bData = RegGetValue(hKey, WIV_REG_TRACE_INTERNAL, false);

	RegKeyClose(hKey);

	return bData;
}

bool TraceGetCommit()
{
	DWORD	dwResult;
	HKEY	hKey;
	bool	bData;

	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);

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
		return TRUE;
	}

	bData = RegGetValue(hKey, WIV_REG_TRACE_COMMIT, false);

	RegKeyClose(hKey);

	return bData;
}

TraceLevel TraceGetLevel()
{
	DWORD dwResult;
	HKEY  hKey;

	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);

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
		return tlError;
	}

	dwResult = RegGetValue(hKey, WIV_REG_TRACE_LEVEL, (DWORD)tlNone);

	RegKeyClose(hKey);

	return (TraceLevel)dwResult;
}

DWORD TraceGetFilter()
{
	DWORD dwResult;
	HKEY  hKey;

	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);

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
		return tlError;
	}

	dwResult = RegGetValue(hKey, WIV_REG_TRACE_FILTER, (DWORD)0);

	RegKeyClose(hKey);

	return dwResult;
}

DWORD TraceGetLineWidth()
{
	DWORD dwResult;
	HKEY  hKey;

	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);

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
		return tlError;
	}

	dwResult = RegGetValue(hKey, WIV_REG_TRACE_LINEWIDTH, (DWORD)WIV_DEFAULT_LINE_WIDTH);

	RegKeyClose(hKey);

	return dwResult;
}

DWORD TraceGetTabWidth()
{
	DWORD dwResult;
	HKEY  hKey;

	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);

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
		return tlError;
	}

	dwResult = RegGetValue(hKey, WIV_REG_TRACE_TABWIDTH, (DWORD)WIV_DEFAULT_TAB_WIDTH);

	RegKeyClose(hKey);

	return dwResult;
}

DWORD TraceGetPath(LPTSTR lpszTracePath)
{
	DWORD dwResult;
	HKEY  hKey;

	dwResult = RegKeyOpen(WIV_REG_DEBUG_KEY, &hKey);

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
		return -1;
	}

	TCHAR	szPath[MAX_PATH + 1] = WIV_EMPTY_STRING;
	_snwprintf(szPath, MAX_PATH, _D("%s\\Trace"), SSS_DEFAULT_TRACE_PATH);

	dwResult = RegGetValue(hKey, WIV_REG_TRACE_PATH, lpszTracePath, WIV_MAX_PATH, szPath);

	RegKeyClose(hKey);

	return dwResult;
}

void TraceFileSetPrefix(LPCTSTR strFilePrefix)
{
	WiVTrace.Lock();
	WiVTrace.SetFilePrefix(strFilePrefix);
	WiVTrace.Unlock();
}

void TraceSetLevel(const TraceLevel nLevel)
{
	WiVTrace.Lock();
	WiVTrace.SetLevel(nLevel);
	WiVTrace.Unlock();
}

void TraceSetLineWidth(const int nLineWidth)
{
	WiVTrace.Lock();
	WiVTrace.SetLineWidth(nLineWidth);
	WiVTrace.Unlock();
}

void TraceSetTabWidth(const int nTabWidth)
{
	WiVTrace.Lock();
	WiVTrace.SetTabWidth(nTabWidth);
	WiVTrace.Unlock();
}

void TraceSetCommit(const bool blCommit)
{
	WiVTrace.Lock();
	WiVTrace.SetCommit(blCommit);
	WiVTrace.Unlock();
}

void TraceSetFilter(const DWORD dwFilter)
{
	WiVTrace.Lock();
	WiVTrace.SetFilter(dwFilter);
	WiVTrace.Unlock();
}

void TraceStart()
{
	SYSTEMTIME	stLocalTime;
	TCHAR		szDateStr[WIV_MAX_STRING + 1];
	TCHAR		szTimeStr[WIV_MAX_STRING + 1];

	// Set up the trace file (directory and name prefix)
	TCHAR		szTracePath[WIV_MAX_PATH + 1];

	memset(szTracePath, 0, sizeof(szTracePath));

	TraceGetPath(szTracePath);
	_tcsncat(szTracePath, SSS_TRACE_PREFIX, WIV_MAX_PATH - _tcslen(szTracePath));

	TraceFileSetPrefix(szTracePath);

	// Get registry settings
	m_blActive		= TraceGetActive();
	m_tlTraceLevel	= m_blActive ? TraceGetLevel() : tlNone;
	m_blCommit		= TraceGetCommit();
	m_nTabWidth		= TraceGetTabWidth();
	m_nLineWidth	= TraceGetLineWidth();
	m_dwFilter		= TraceGetFilter();
	m_blInternal	= TraceGetInternal();

	TraceSetLevel(m_tlTraceLevel);
	TraceSetLineWidth(m_nLineWidth);
	TraceSetTabWidth(m_nTabWidth);
	TraceSetCommit(m_blCommit);
	TraceSetFilter(m_dwFilter);

	WiVTrace.OpenTraceFile();

	::GetLocalTime(&stLocalTime);

	// Write the first trace message
	::GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocalTime, NULL, szDateStr, WIV_MAX_STRING);
	::GetTimeFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, &stLocalTime, NULL, szTimeStr, WIV_MAX_STRING);

	TraceAlways(_D("Trace level %d%s Started on %s at %s"), m_tlTraceLevel, (m_blInternal ? _D(" (Plus Internal)") : _D("")), szDateStr, szTimeStr);
	TraceAlways(_D("Commit = %d, Line Width = %d, Tab Width = %d, Filters = %08X"), m_blCommit, m_nLineWidth, m_nTabWidth, m_dwFilter);
	TraceAlways(WIV_EMPTY_STRING);
}

void TraceStop()
{
	SYSTEMTIME	stLocalTime;
	TCHAR		szDateStr[WIV_MAX_STRING + 1];
	TCHAR		szTimeStr[WIV_MAX_STRING + 1];

	::GetLocalTime(&stLocalTime);

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocalTime, NULL, szDateStr, WIV_MAX_STRING); 
	GetTimeFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, &stLocalTime, NULL, szTimeStr, WIV_MAX_STRING);

	// Write the last trace message
	TraceAlways(WIV_EMPTY_STRING);
	TraceAlways(_D("Commit = %d, Line Width = %d, Tab Width = %d, Filters = %08X"), m_blCommit, m_nLineWidth, m_nTabWidth, m_dwFilter);
	TraceAlways(_D("Trace level %d%s Stopped on %s at %s"), m_tlTraceLevel, (m_blInternal ? _D(" (Plus Internal)") : _D("")), szDateStr, szTimeStr);
	
	if (m_hfFile)
		WiVTrace.CloseTraceFile();

	return;
}

void TraceEnter(LPCTSTR strFunction, const TraceLevel tlLevel)
{
    TCHAR	szChevrons[WIV_MAX_TAB_WIDTH + 1] = WIV_EMPTY_STRING;
    TCHAR	szText[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
    
	if ((tlLevel == tlInternal) && (!m_blInternal))
	{
		return;
	}

	for (int i = 0; i < m_nTabWidth - 1; i++)
		_tcsncat(szChevrons, WIV_CHEVRON_RIGHT, WIV_MAX_TAB_WIDTH - _tcslen(szChevrons));

	if (tlLevel == tlInternal)
		TraceInternal(_D("%s Entering %s"), szChevrons, strFunction);
	else
		TraceDebug(_D("%s Entering %s"), szChevrons, strFunction);
    
    m_nCurrentTabWidth = m_nCurrentTabWidth + m_nTabWidth;
    if (m_nCurrentTabWidth >= WIV_MAX_LINE_WIDTH) m_nCurrentTabWidth = WIV_MAX_LINE_WIDTH;
	return;
}

void TraceLeave(LPCTSTR strFunction, LPCTSTR szRetVal, const TraceLevel tlLevel)
{
    TCHAR	szChevrons[WIV_MAX_TAB_WIDTH + 1] = WIV_EMPTY_STRING;
    TCHAR	szText[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
    
	if ((tlLevel == tlInternal) && (!m_blInternal))
	{
		return;
	}

	for (int i = 0; i < m_nTabWidth - 1; i++)
		_tcsncat(szChevrons, WIV_CHEVRON_LEFT, WIV_MAX_TAB_WIDTH - _tcslen(szChevrons));

    m_nCurrentTabWidth = m_nCurrentTabWidth - m_nTabWidth;
    
    if (m_nCurrentTabWidth <= 0) m_nCurrentTabWidth = 1;
    
	if (tlLevel == tlInternal)
		TraceInternal(_D("%s Leaving %s, returning <%s>"), szChevrons, strFunction, szRetVal);
	else
		TraceDebug(_D("%s Leaving %s, returning <%s>"), szChevrons, strFunction, szRetVal);
}

void TraceLeave(LPCTSTR strFunction, const DWORD dwRetVal, const TraceLevel tlLevel)
{
    TCHAR	szChevrons[WIV_MAX_TAB_WIDTH + 1] = WIV_EMPTY_STRING;
    TCHAR	szText[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
    
	if ((tlLevel == tlInternal) && (!m_blInternal))
	{
		return;
	}

	for (int i = 0; i < m_nTabWidth - 1; i++)
		_tcsncat(szChevrons, WIV_CHEVRON_LEFT, WIV_MAX_TAB_WIDTH - _tcslen(szChevrons));

    m_nCurrentTabWidth = m_nCurrentTabWidth - m_nTabWidth;
    
    if (m_nCurrentTabWidth <= 0) m_nCurrentTabWidth = 1;
    
	if (tlLevel == tlInternal)
		TraceInternal(_D("%s Leaving %s, returning <%08X>"), szChevrons, strFunction, dwRetVal);
	else
		TraceDebug(_D("%s Leaving %s, returning <%08X>"), szChevrons, strFunction, dwRetVal);
}

void TraceLeave(LPCTSTR strFunction, const TraceLevel tlLevel)
{
    TCHAR	szChevrons[WIV_MAX_TAB_WIDTH + 1] = WIV_EMPTY_STRING;
    TCHAR	szText[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;
    
	if ((tlLevel == tlInternal) && (!m_blInternal))
	{
		return;
	}

	for (int i = 0; i < m_nTabWidth - 1; i++)
		_tcsncat(szChevrons, WIV_CHEVRON_LEFT, WIV_MAX_TAB_WIDTH - _tcslen(szChevrons));

    m_nCurrentTabWidth = m_nCurrentTabWidth - m_nTabWidth;
    
    if (m_nCurrentTabWidth <= 0) m_nCurrentTabWidth = 1;
    
	if (tlLevel == tlInternal)
		TraceInternal(_D("%s Leaving %s"), szChevrons, strFunction);
	else
		TraceDebug(_D("%s Leaving %s"), szChevrons, strFunction);
}

void TraceFatal(LPCTSTR strFormat, ...)
{
	TCHAR	szFormat[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	_snwprintf(szFormat, WIV_MAX_STRING, _D("FATAL::%s"), strFormat);

	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlFatal, szFormat, args);
	va_end(args);
}

void TraceError(LPCTSTR strFormat, ...)
{
	TCHAR	szFormat[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	_snwprintf(szFormat, WIV_MAX_STRING, _D("ERROR::%s"), strFormat);

	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlError, szFormat, args);
	va_end(args);
}

void TraceWarning(LPCTSTR strFormat, ...)
{
	TCHAR	szFormat[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	_snwprintf(szFormat, WIV_MAX_STRING, _D("Warning::%s"), strFormat);

	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlWarning, szFormat, args);
	va_end(args);
}

void TraceInfo(LPCTSTR strFormat, ...)
{
	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlInfo, strFormat, args);
	va_end(args);
}

void TraceDetail(LPCTSTR strFormat, ...)
{
	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlDetail, strFormat, args);
	va_end(args);
}

void TraceAlways(LPCTSTR strFormat, ...)
{
	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlAlways, strFormat, args);
	va_end(args);
}

void TraceInternal(LPCTSTR strFormat, ...)
{
	TCHAR	szFormat[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	_snwprintf(szFormat, WIV_MAX_STRING, _D("Internal::%s"), strFormat);

	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlInternal, szFormat, args);
	va_end(args);
}

void TraceDebug(LPCTSTR strFormat, ...)
{
	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlDebug, strFormat, args);
	va_end(args);
}

void TraceWrite(const TraceLevel nLevel, LPCTSTR strFormat, ...)
{
	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(nLevel, strFormat, args);
	va_end(args);
}

int TraceMessage(LPCTSTR strFormat, ...)
{
	TCHAR szText[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	va_list args;
	va_start(args, strFormat);
	_vsntprintf(szText, WIV_MAX_STRING, strFormat, args);
	va_end(args);

	return MessageBox(NULL, szText, _D("WiVSSS"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);

}

void TraceCodeStart(LPCTSTR strFormat, ...)
{
	TCHAR	szFormat[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	_snwprintf(szFormat, WIV_MAX_STRING, _D("\r\n\r\n%s\r\n"), strFormat);

	va_list args;
	va_start(args, strFormat);
	m_nCodePos = 0;
	WiVTrace.TraceOut(tlCode, szFormat, args);
	va_end(args);
}

void TraceCodeEnd(LPCTSTR strFormat, ...)
{
	TCHAR	szFormat[WIV_MAX_STRING + 1] = WIV_EMPTY_STRING;

	_snwprintf(szFormat, WIV_MAX_STRING, _D("\r\n%s\r\n\r\n"), strFormat);

	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlCode, szFormat, args);
	va_end(args);
	m_nCodePos = -1;
}

void TraceCode(LPCTSTR strFormat, ...)
{
	va_list args;
	va_start(args, strFormat);
	WiVTrace.TraceOut(tlCode, strFormat, args);
	va_end(args);
}

#endif // #ifdef WIV_DEBUG
