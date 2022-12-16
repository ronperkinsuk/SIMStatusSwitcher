//////////////////////////////////////////////////////////////////////
//
// WiVLang.h: Language translation.
//
//////////////////////////////////////////////////////////////////////
#ifndef INC_WIV_LANG_H
#define INC_WIV_LANG_H

using namespace WiV;

class CWiVLang
{

// Friend functions
friend LPCTSTR	LangLoadString(HMODULE hmInstance, const UINT uiStringID);
friend bool		LangLoadTable(LPCTSTR lpszProduct, LPCTSTR lpszLanguage);
friend bool		LangGetTableStatus();
friend int		LangGetLanguagesList(PWIVLANG *lppsLanguages);
friend PWIVLANG	LangGetDefaultLanguage();
friend void		LangSetDefaultLanguage(WIVLANG sLanguage);
friend PWIVLANG	LangGetCurrentLanguage();
friend void		LangSetCurrentLanguage(WIVLANG sLanguage);
friend DWORD	LangReadDefaultLanguage();
friend DWORD	LangWriteDefaultLanguage();

#ifdef WIV_DEBUG
#endif

private:

WIVLANG		m_asLanguages[100];
WIVLANG		m_sDefaultLanguage;
WIVLANG		m_sCurrentLanguage;
int			m_nNumLanguages;

class WIVTRANSTRINGS 
{

public:

	int		nLowIndex;
	int		nHighIndex;
	int		nMaxEntries;
	TCHAR	aszStrings[WIV_CUSTOM_MAX_TABLE_ENTRIES + 1][WIV_MAX_STRING + 1];

	WIVTRANSTRINGS::WIVTRANSTRINGS()
	{
//		TraceInfo(_D("WIVTRANSTRINGS::Constructor"));
		nMaxEntries = WIV_CUSTOM_MAX_TABLE_ENTRIES - 1;
		nLowIndex = nMaxEntries - 1;
		nHighIndex = 0;
	}

	WIVTRANSTRINGS::~WIVTRANSTRINGS()
	{
//		TraceInfo(_D("WIVTRANSTRINGS::Destructor"));
//		TraceInfo(_D("WIVTRANSTRINGS::Destructor: nMaxEntries = %d"), nMaxEntries);
//		TraceInfo(_D("WIVTRANSTRINGS::Destructor: nLowIndex = %d"), nLowIndex);
//		TraceInfo(_D("WIVTRANSTRINGS::Destructor: nHighIndex = %d"), nHighIndex);
	}

}; // class WIVTRANSTRINGS
 
WIVTRANSTRINGS *m_pTranStrings;

// Private functions
void	Lock();
void	Unlock();
HANDLE	OpenFile();
void	CloseFile();
long	LoadFile();
int		GetLine(LPTSTR lpszLine);
int		ParseLine(LPCTSTR lpszLine, LPTSTR lpszValue);
bool	ValidateFile(LPCTSTR lpszFileID, LPCTSTR lpszVersion);
bool	LoadTable(LPCTSTR lpszProduct, LPCTSTR lpszLanguage);
LPCTSTR GetEntry(const int nKey);

#ifdef WIV_DEBUG
#endif

public:

// Constructor/Destructor
CWiVLang();
~CWiVLang();

}; // class CWiVLang

// Global non-member functions
//bool	LangLoadTable(LPCTSTR lpszProduct, LPCTSTR lpszLanguage);
//LPCTSTR	LangLoadString(HMODULE hmInstance, const UINT uiStringID);

#ifdef WIV_DEBUG
#endif

#endif // INC_WIV_LANG_H
