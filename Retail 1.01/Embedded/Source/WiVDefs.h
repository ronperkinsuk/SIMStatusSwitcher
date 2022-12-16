//////////////////////////////////////////////////////////////////////
//
// WiVDefs.h: WiV Definitions.
//
//////////////////////////////////////////////////////////////////////
#ifndef INC_WIV_DEFS_H
#define INC_WIV_DEFS_H

namespace WiV
{

//=====================
// Function Macros
//=====================

//---------------------------------------------
// ARRAYSIZE(array):
//
// Returns number of elements in an array
// when passed the array name. NB. Will NOT
// work with pointers to arrays, for example
// when passed as arguments into a function.
//---------------------------------------------
#define ARRAYSIZE(_array) (sizeof(_array) / sizeof((_array)[0]))   

//-----------------------------
// _zclr(_string):
//
// Zero-fill a string
//-----------------------------
#define _zclr(_string) (_tcsncpy((_string), WIV_EMPTY_STRING, ARRAYSIZE((_string))))

//-----------------------------
// _zncpy(dest,src,len):
//
// Zero-fill then copy string
//-----------------------------
#define _zcpy(_dest,_src) (_zclr((_dest)));(_tcsncpy((_dest), (_src), (ARRAYSIZE((_dest)) - 1)))

//-----------------------------
// MAX(num1,num2):
//
// Returns max of two numbers
//-----------------------------
//inline int MAX(register const int num1, register const int num2){return (num1 > num2 ? num1 : num2);}
#define MAX(_num1,_num2)   ((_num1) > (_num2) ? (_num1) : (_num2))

//-----------------------------------------
// _X(rect), _Y(rect), _W(rect), _H(rect):
//
// Easier reference to RECT structure
//-----------------------------------------
//inline long _X(RECT rect){return rect.left;}
//inline long _R(RECT rect){return rect.right;}
//inline long _Y(RECT rect){return rect.top;}
//inline long _B(RECT rect){return rect.bottom;}
//inline long _W(RECT rect){return rect.right - rect.left;}
//inline long _H(RECT rect){return rect.bottom - rect.top;}
#define _X(_rect)	((_rect).left)
#define _R(_rect)	((_rect).right)
#define _Y(_rect)	((_rect).top)
#define _B(_rect)	((_rect).bottom)
#define _W(_rect)	((_rect).right - (_rect).left)
#define _H(_rect)	((_rect).bottom - (_rect).top)
	
#define SWAPBYTES(_word)	(MAKEWORD(HIBYTE(_word), LOBYTE(_word)))

#define MAX_MESSAGES						5


#define	WM_WIV_REFRESH						WM_APP + 5

//=====================
// Font styles
//=====================
#define WIV_FONT_NONE						0x00000000
#define WIV_FONT_UNDERLINE					0x00000001
#define WIV_FONT_ITALIC						0x00000002
#define WIV_FONT_STRIKEOUT					0x00000004

//=====================
// Font sizes
//=====================
#define WIV_FONT_NORMAL						0x00000010
#define WIV_FONT_SMALL						0x00000020
#define WIV_FONT_MEDIUM						0x00000040
#define WIV_FONT_LARGE						0x00000080

//=======================
// Special string values
//=======================
#define WIV_EMPTY_STRING					_T("")
#define WIV_SPACE_STRING					_T(" ")
#define WIV_CHEVRON_RIGHT					_T(">")
#define WIV_CHEVRON_LEFT					_T("<")
#define WIV_NULL_CHAR						_T('\0')
#define WIV_COPYRIGHT_SIGN					g_szFormatCopyright

#define WIV_MAX_PATH						MAX_PATH
#define WIV_MAX_BUFFER						4096
#define WIV_MAX_STRING						256
#define WIV_MAX_BINARY						100
#define WIV_MAX_DWORD						16
#define WIV_MAX_NAME						32
#define	WIV_MAX_LICENSE_SECTION				5

#define WIV_MAX_LIC_STRING_LENGTH			20	// For entered/viewable license code
#define WIV_MAX_LICENSES					7	// 1 trial + 2 full + 2 beta + 2 special
#define WIV_MAX_ENCRYPTED_LICENSE_LENGTH	24	// Encrypted license data length
#define WIV_MAX_ENCRYPTED_IMEI_LENGTH		8	// 8 bytes = 16 digits
#define WIV_MAX_LICENSE_LENGTH				WIV_MAX_ENCRYPTED_LICENSE_LENGTH + WIV_MAX_ENCRYPTED_IMEI_LENGTH	// License + IMEI

#define WIV_LICTYPE_ERROR					((char)-2)
#define WIV_LICTYPE_NONE					(char)(WIV_LICTYPE_ERROR + 1)
#define WIV_LICTYPE_FULL					(char)(WIV_LICTYPE_NONE + 1)
#define WIV_LICTYPE_TRIAL					(char)(WIV_LICTYPE_FULL + 1)
#define WIV_LICTYPE_BETA					(char)(WIV_LICTYPE_TRIAL + 1)
#define WIV_LICTYPE_SPECIAL					(char)(WIV_LICTYPE_BETA + 1)

#define WIV_DAYS_REMAINING_GREEN			3
#define WIV_DAYS_REMAINING_AMBER			0

#define WIV_LICENSE_CONDITION_GREEN			0x00a1
#define WIV_LICENSE_CONDITION_AMBER			0x2892
#define WIV_LICENSE_CONDITION_RED			0x12c4
#define WIV_LICENSE_CONDITION_WHITE			0x1780

//=====================
// Options dialog stuff
//=====================
#define WIV_OPTIONS_TITLE_HEIGHT			23
#define WIV_OPTIONS_TITLE_BAR_HEIGHT		(WIV_OPTIONS_TITLE_HEIGHT + 3)

//======================
// Customisation file
//======================
#define	WIV_CUSTOM_NEW_LINE					"\r\n"
#define	WIV_CUSTOM_DELIM					_T("::") 
#define	WIV_CUSTOM_COMMENT					';'

//======================
// Language Translation
//======================
#define	WIV_REG_LANGUAGE_KEY			g_szRegOptionsKey
#define	WIV_REG_DEFAULT_LANGUAGE_ID		g_szRegLangDefaultLanguageID
#define WIV_REG_DEFAULT_LANGUAGE_NAME	g_szRegLangDefaultLanguageName
#define	WIV_MAX_LANGUAGE_ID				8

#define	WIV_CUSTOM_LOWEST_KEY_INDEX		0
#define	WIV_CUSTOM_HIGHEST_KEY_INDEX	1
#define	WIV_CUSTOM_MAX_KEY_SIZE			32
#define	WIV_CUSTOM_MAX_TABLE_ENTRIES	200

// Special key values
#define WIV_LANG_KEY_UNKNOWN			0
#define WIV_LANG_KEY_EMPTY				-1
#define WIV_LANG_KEY_COMMENT			-2
#define WIV_LANG_KEY_HEADER				-3
#define WIV_LANG_KEY_VERSION			-4
#define WIV_LANG_KEY_PRODUCT			-5
#define WIV_LANG_KEY_LANGUAGE_ID		-6
#define WIV_LANG_KEY_LANGUAGE_NAME		-7
#define WIV_LANG_KEY_TABLE				-8
#define WIV_LANG_KEY_TABLE_BEGIN		-9
#define WIV_LANG_KEY_TABLE_END			-10

// Identifier strings
#define WIV_LANG_TEXT_HEADER_ID			_T("LANGFILE")
#define WIV_LANG_TEXT_VERSION			_T("Version")
#define WIV_LANG_TEXT_PRODUCT			_T("PRODUCT")
#define WIV_LANG_TEXT_LANGUAGE_ID		_T("LANGUAGEID")
#define WIV_LANG_TEXT_LANGUAGE_NAME		_T("LANGUAGENAME")
#define WIV_LANG_TEXT_TABLE				_T("TABLE")
#define WIV_LANG_TEXT_TABLE_BEGIN		_T("BEGIN")
#define WIV_LANG_TEXT_TABLE_END			_T("END")

typedef struct  {
	TCHAR	ID[WIV_MAX_LANGUAGE_ID + 1];
	TCHAR	Name[WIV_MAX_NAME + 1];
} WIVLANG, *PWIVLANG;

} // namespace WiV

#endif // INC_WIV_DEFS_H
