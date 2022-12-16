//////////////////////////////////////////////////////////////////////
//
// WiVTrace.h: Interface for the CWiVTrace class.
//
//////////////////////////////////////////////////////////////////////
#ifndef INC_WIV_TRACE_H
#define INC_WIV_TRACE_H

#include <windows.h>

#define		_D(x) __TEXT(x)
#define		_DB( x ) _T( #x )

#define		TracePushLevel(_tl) {TraceLevel tlOld = TraceGetLevel(); TraceSetLevel((_tl));
#define		TracePopLevel() TraceSetLevel(tlOld);}

//-----------------
// Help stuff
//-----------------
#define	SSS_HELP_TAG_DEBUG_SETTINGS				_D("debug_settings")

//-----------------
// Displayed Text
//-----------------
#define SSS_TEXT_DEBUG_TITLE					_D("Debug")
#define SSS_TEXT_NO_TRACE_FILES					_D("<No Trace Files>")
#define SSS_TEXT_INVALID_PATH					_D("<Invalid Path>")
#define SSS_TEXT_CANT_OPEN_FILE					_D("Couldn't open file")
#define SSS_TEXT_TRACE_FILE_CONTENTS			_D("Trace File Contents:")

#define SSS_TEXT_DEBUG_CHECK_TRACE_ACTIVE		_D("Trace Active")

#define SSS_TEXT_DEBUG_BUTTON_ELLIPSES			_D("...")
#define SSS_TEXT_DEBUG_STATIC_TRACE_PATH		_D("Trace Path")
#define SSS_TEXT_DEBUG_STATIC_TRACE_FILES		_D("Current Trace Files")
#define SSS_TEXT_DEBUG_STATIC_TRACE_LEVEL		_D("Trace Level")

#define SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_NONE	_D("None - No trace")
#define SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_FATAL	_D("Fatal - Program abort")
#define SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_ERROR	_D("Error - Unrecoverable problems")
#define SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_WARNING	_D("Warning - Recoverable problems")
#define SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_INFO	_D("Info - Informational, for interest")
#define SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_DEBUG	_D("Debug - Complete execution path")
#define SSS_TEXT_DEBUG_LIST_TRACE_LEVEL_DETAIL	_D("Detail - All trace")

//-----------------
// Default values
//-----------------
#define SSS_DEFAULT_TRACE_ACTIVE			false
#define SSS_DEFAULT_TRACE_PATH				WiV::GetInstallPath()
#define SSS_DEFAULT_TRACE_LEVEL				tlNone

//-----------------
// Trace stuff
//-----------------
#define	SSS_MAX_AVAILABLE_TRACE_LEVELS		8
#define	SSS_MAX_TRACE_LEVEL_NAME_LENGTH		40

#define SSS_TRACE_PREFIX					_D("\\SSSTrace")
#define SSS_TEXT_FILTER_ALL_FILES			((LPCTSTR)LangLoadString(m_hmInstance, IDS_FILTER_ALL_FILES))

//-----------------------
// Property sheet stuff 
//-----------------------
#define	SSS_MAX_PROP_SHEET_PAGES			7

//-----------------
// Registry stuff
//-----------------

// User configurable through options
#define	WIV_REG_DEBUG_KEY			_D("Internal")
#define WIV_REG_TRACE_ACTIVE		_D("TraceActive")
#define WIV_REG_TRACE_PATH			_D("TracePath")
#define WIV_REG_TRACE_LEVEL			_D("TraceLevel")

// Registry configured only
#define WIV_REG_TRACE_FILTER		_D("TraceFilter")
#define WIV_REG_TRACE_COMMIT		_D("TraceCommit")
#define WIV_REG_TRACE_TABWIDTH		_D("TraceTabWidth")
#define WIV_REG_TRACE_LINEWIDTH		_D("TraceLineWidth")
#define WIV_REG_TRACE_INTERNAL		_D("TraceInternal")

#define WIV_MIN_TAB_WIDTH			2
#define WIV_MAX_TAB_WIDTH			8
#define WIV_DEFAULT_TAB_WIDTH		3

#define	WIV_MIN_LINE_WIDTH			80
#define	WIV_MAX_LINE_WIDTH			WIV_MAX_STRING
#define WIV_DEFAULT_LINE_WIDTH		120

// Trace output filter values
#define WIV_TRACE_FILTER_TODAY		0x00000001
#define WIV_TRACE_FILTER_OPTIONS	0x00000002
#define WIV_TRACE_FILTER_PHONE		0x00000004
#define WIV_TRACE_FILTER_RIL		0x00000008
#define WIV_TRACE_FILTER_INTERNAL	0x00008000
#define WIV_TRACE_FILTER_ALL		0x0000FFFF

//-------------------------------------------------------------------------------------------
// The various predefined tracing levels (and their intended uses).
//
//   tlNone		- No trace output, even if trace is active
//   tlFatal	- Errors at this level will cause the program to abort.
//   tlError	- For errors that can't be recovered from
//   tlWarning	- For warnings, bad things but recoverable
//   tlDebug	- Program execution, function entry/exit tracing messages
//   tlInfo		- Informational, data values, etc. - just for interest
//   tlDetail	- Detailed output, procedural narrative, etc. - all trace is output
//   tlInternal	- Internal use only - traces the trace functions themselves 
//   tlAlways   - Always trace irrespective of current level, unless trace is inactive
//   tlCode		- Special Level to write data verbatim
//-------------------------------------------------------------------------------------------
enum TraceLevel
{
    tlNone,
    tlFatal,
    tlError,
    tlWarning,
    tlInfo,
    tlDebug,
    tlDetail,
    tlInternal,
    tlAlways,
	tlCode
};

class CWiVTrace
{

private:

	// Friend functions
	friend void TraceStart();
	friend void TraceStop();
	friend void TraceFileSetPrefix(LPCTSTR strFilePrefix);
	friend void TraceSetLevel(const TraceLevel nLevel);
	friend void TraceSetTabWidth(const int nTabWidth);
	friend void TraceSetLineWidth(const int nLineWidth);
	friend void TraceSetFilter(const DWORD dwFilter);
	friend void TraceSetCommit(const bool blCommit);
	friend void TraceWrite(const TraceLevel nLevel, LPCTSTR strFormat, ...);

	friend void TraceFatal(LPCTSTR strFormat, ...);
	friend void TraceError(LPCTSTR strFormat, ...);
	friend void TraceWarning(LPCTSTR strFormat, ...);
	friend void TraceInfo(LPCTSTR strFormat, ...);
	friend void TraceDetail(LPCTSTR strFormat, ...);
	friend void TraceAlways(LPCTSTR strFormat, ...);
	friend void TraceInternal(LPCTSTR strFormat, ...);
	friend void TraceDebug(LPCTSTR strFormat, ...);
	friend int	TraceMessage(LPCTSTR strFormat, ...);
	friend void	TraceCodeStart(LPCTSTR strFormat, ...);
	friend void	TraceCodeEnd(LPCTSTR strFormat, ...);
	friend void	TraceCode(LPCTSTR strFormat, ...);

	void		CloseTraceFile();
	HANDLE		OpenTraceFile();
	void		Lock();
	void		Unlock();
	void		SetLevel(const TraceLevel nLevel);
	void		SetTabWidth(const int nTabWidth);
	void		SetLineWidth(const int nLineWidth);
	void		SetFilter(const DWORD dwFilter);
	void		SetCommit(const bool blCommit);
	void		SetFilePrefix(LPCTSTR strFilePrefix);
	LPCTSTR		GetIndent();
	LPCTSTR		TraceNewLine();
	LPCTSTR		TraceBlock(LPCTSTR szIntro, LPCTSTR szMessage, int nLength);
	void		TraceOut(const TraceLevel nLevel, LPCTSTR strFormat, va_list vaArgs);

public:

	CWiVTrace();
	~CWiVTrace();

}; // class CWiVTrace

// Global non-member functions
//void		TraceStart();
//void		TraceStop();
TraceLevel	TraceGetLevel();
//void		TraceFileSetPrefix(LPCTSTR strFilePrefix);
//void		TraceSetLevel(const TraceLevel nLevel);
//void		TraceSetTabWidth(const int nTabWidth);
//void		TraceSetLineWidth(const int nLineWidth);
//void		TraceSetFilter(const DWORD dwFilter);
//void		TraceSetCommit(const bool blCommit);
//void		TraceWrite(const TraceLevel nLevel, LPCTSTR strFormat, ...);
//void		TraceFatal(LPCTSTR strFormat, ...);
//void		TraceError(LPCTSTR strFormat, ...);
//void		TraceWarning(LPCTSTR strFormat, ...);
//void		TraceInfo(LPCTSTR strFormat, ...);
//void		TraceDetail(LPCTSTR strFormat, ...);
//void		TraceAlways(LPCTSTR strFormat, ...);
//void		TraceInternal(LPCTSTR strFormat, ...);
//void		TraceDebug(LPCTSTR strFormat, ...);
void		TraceEnter(LPCTSTR strFunction, const TraceLevel tlLevel = tlNone);
void		TraceLeave(LPCTSTR strFunction, const TraceLevel tlLevel = tlNone);
void		TraceLeave(LPCTSTR strFunction, const DWORD dwRetVal, const TraceLevel tlLevel = tlNone);
void		TraceLeave(LPCTSTR strFunction, LPCTSTR szRetVal, const TraceLevel tlLevel = tlNone);
//int			TraceMessage(LPCTSTR strFormat, ...);
//void		TraceCodeStart(LPCTSTR strFormat, ...);
//void		TraceCodeEnd(LPCTSTR strFormat, ...);
//void		TraceCode(LPCTSTR strFormat, ...);

#endif // INC_WIV_TRACE_H
