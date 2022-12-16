//////////////////////////////////////////////////////////////////////
//
// SSSLang.h: Language translation.
//
//////////////////////////////////////////////////////////////////////
#ifndef INC_SSS_LANG_H
#define INC_SSS_LANG_H

//-----------------
// Displayed text
//-----------------
 
// Miscellaneous common strings
#define SSS_TEXT_OK							((LPCTSTR)LangLoadString(m_hmInstance, IDS_OK))
#define SSS_TEXT_CANCEL						((LPCTSTR)LangLoadString(m_hmInstance, IDS_CANCEL))
#define SSS_TEXT_ACTIVE_SIM					((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTIVE_SIM))
#define SSS_TEXT_TELEPHONE_NO				((LPCTSTR)LangLoadString(m_hmInstance, IDS_TELEPHONE_NO))
#define	SSS_TEXT_OPTIONS_VIEWORCHANGE		((LPCTSTR)LangLoadString(m_hmInstance, IDS_OPTIONS_VIEWORCHANGE))
#define SSS_TEXT_MESSAGE_CHANGES_MADE		((LPCTSTR)LangLoadString(m_hmInstance, IDS_MESSAGE_CHANGES_MADE))
#define SSS_TEXT_MESSAGE_SURE_TO_CANCEL		((LPCTSTR)LangLoadString(m_hmInstance, IDS_MESSAGE_SURE_TO_CANCEL))

// Error messages
#define SSS_TEXT_ERROR_PIN_MISMATCH			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ERROR_PIN_MISMATCH))
#define SSS_TEXT_ERROR_INCORRECT_PIN		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ERROR_INCORRECT_PIN))
#define SSS_TEXT_ERROR_UNAVAILABLE			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ERROR_UNAVAILABLE))
#define SSS_TEXT_ERROR_PEGHELP				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ERROR_PEGHELP))
#define SSS_TEXT_ERROR_MESSAGEBOX			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ERROR_MESSAGEBOX))
#define SSS_TEXT_ERROR_NOT_ENOUGH_MEMORY	((LPCTSTR)LangLoadString(m_hmInstance, IDS_ERROR_NOT_ENOUGH_MEMORY))

// Today Popup menu items
#define SSS_TEXT_MENU_OPTIONS				((LPCTSTR)LangLoadString(m_hmInstance, IDS_MENU_OPTIONS))
#define SSS_TEXT_MENU_REFRESH				((LPCTSTR)LangLoadString(m_hmInstance, IDS_MENU_REFRESH))
#define SSS_TEXT_MENU_SWITCH_SIM			((LPCTSTR)LangLoadString(m_hmInstance, IDS_MENU_SWITCH_SIM))
#define SSS_TEXT_MENU_TURN_PHONE_ON			((LPCTSTR)LangLoadString(m_hmInstance, IDS_MENU_TURN_PHONE_ON))
#define SSS_TEXT_MENU_TURN_PHONE_OFF		((LPCTSTR)LangLoadString(m_hmInstance, IDS_MENU_TURN_PHONE_OFF))
#define SSS_TEXT_MENU_PHONE_SETTINGS		((LPCTSTR)LangLoadString(m_hmInstance, IDS_MENU_PHONE_SETTINGS))
#define SSS_TEXT_MENU_ABOUT					((LPCTSTR)LangLoadString(m_hmInstance, IDS_MENU_ABOUT))

// License status
#define SSS_TEXT_TODAY_LICENSE_EXPIRED		((LPCTSTR)LangLoadString(m_hmInstance, IDS_TODAY_LICENSE_EXPIRED))
#define SSS_TEXT_TODAY_LICENSE_TAP_FOR_INFO	((LPCTSTR)LangLoadString(m_hmInstance, IDS_TODAY_LICENSE_TAP_FOR_INFO))
#define SSS_TEXT_TODAY_LICENSE_PLEASE_REG	((LPCTSTR)LangLoadString(m_hmInstance, IDS_TODAY_LICENSE_PLEASE_REGISTER))
#define SSS_TEXT_TODAY_LICENSE_FUNCTION_LTD ((LPCTSTR)LangLoadString(m_hmInstance, IDS_TODAY_LICENSE_FUNCTION_LIMITED))
#define SSS_TEXT_TODAY_LICENSE_INVALID		((LPCTSTR)LangLoadString(m_hmInstance, IDS_TODAY_LICENSE_INVALID))

// Phone status
#define SSS_TEXT_PHONE_NUMBER_NOT_SHOWN		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_NUMBER_NOT_SHOWN))
#define SSS_TEXT_PHONE_STATUS_READY			((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_READY))
#define SSS_TEXT_PHONE_STATUS_READY_SL		WIV_EMPTY_STRING
#define SSS_TEXT_PHONE_STATUS_LABEL			((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_LABEL))
#define SSS_TEXT_PHONE_STATUS_INITIALIZING	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_INITIALIZING))
#define SSS_TEXT_PHONE_STATUS_REFRESHING	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_REFRESHING))
#define SSS_TEXT_PHONE_STATUS_REGISTERING	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_REGISTERING))
#define SSS_TEXT_PHONE_STATUS_WAIT_SIGNAL	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_WAIT_SIGNAL))
#define SSS_TEXT_PHONE_STATUS_WAIT_PIN		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_WAIT_PIN))
#define SSS_TEXT_PHONE_TAP_TO_ENTER_PIN		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_TAP_TO_ENTER_PIN))
#define SSS_TEXT_PHONE_STATUS_ENTERING_PIN	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_ENTERING_PIN))
#define SSS_TEXT_PHONE_STATUS_TURNING_ON	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_TURNING_ON))
#define SSS_TEXT_PHONE_STATUS_TURNING_OFF	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_TURNING_OFF))
#define SSS_TEXT_PHONE_STATUS_SWITCHING_SIM	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_SWITCHING_SIM))
#define SSS_TEXT_PHONE_STATUS_PIN_ACCEPTED	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_PIN_ACCEPTED))
#define SSS_TEXT_PHONE_STATUS_PIN_ERROR		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_PIN_ERROR))
#define SSS_TEXT_PHONE_STATUS_OFF			((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_OFF))
#define SSS_TEXT_PHONE_STATUS_BLOCKED		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_BLOCKED))
#define SSS_TEXT_PHONE_STATUS_LOCKED		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_LOCKED))
#define SSS_TEXT_PHONE_STATUS_INVALID		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_INVALID))
#define SSS_TEXT_PHONE_STATUS_PREV_PIN_ERR	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_STATUS_PREV_PIN_ERR))
#define SSS_TEXT_PHONE_PROVIDER_LABEL		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_PROVIDER_LABEL))
#define SSS_TEXT_PHONE_PROVIDER_LABEL_SL	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_PROVIDER_LABEL_SL))
#define SSS_TEXT_PHONE_PROVIDER_LOOKING		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_PROVIDER_LOOKING))
#define SSS_TEXT_PHONE_PROVIDER_NOT_APPLY	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_PROVIDER_NOT_APPLY))
#define SSS_TEXT_PHONE_REGISTERED_HOME		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_REGISTERED_HOME))
#define SSS_TEXT_PHONE_REGISTERED_ROAMING	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_REGISTERED_ROAMING))
#define SSS_TEXT_PHONE_REGISTERED_UNKNOWN	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_REGISTERED_UNKNOWN))
#define SSS_TEXT_PHONE_NUMBER_UNAVAILABLE	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONE_NUMBER_UNAVAILABLE))

// Display dialog
#define SSS_TEXT_DISPLAY_TITLE				((LPCTSTR)LangLoadString(m_hmInstance, IDS_DISPLAY_TITLE))
#define SSS_TEXT_DISPLAY_SHOW_PHONE_NUMBER	((LPCTSTR)LangLoadString(m_hmInstance, IDS_DISPLAY_SHOW_PHONE_NUMBER))
#define SSS_TEXT_DISPLAY_SHOW_TSP			((LPCTSTR)LangLoadString(m_hmInstance, IDS_DISPLAY_SHOW_TSP))
#define SSS_TEXT_DISPLAY_SINGLE_LINE		((LPCTSTR)LangLoadString(m_hmInstance, IDS_DISPLAY_SINGLE_LINE))

// Appearance dialog
#define SSS_TEXT_APPEARANCE_TITLE			((LPCTSTR)LangLoadString(m_hmInstance, IDS_APPEARANCE_TITLE))
#define SSS_TEXT_APPEARANCE_LINE_1_BOLD		((LPCTSTR)LangLoadString(m_hmInstance, IDS_APPEARANCE_LINE_1_BOLD))
#define SSS_TEXT_APPEARANCE_LINE_2_BOLD		((LPCTSTR)LangLoadString(m_hmInstance, IDS_APPEARANCE_LINE_2_BOLD))
#define SSS_TEXT_APPEARANCE_ICON_SETS		((LPCTSTR)LangLoadString(m_hmInstance, IDS_APPEARANCE_ICON_SETS))

// Icon set names
#define SSS_TEXT_ICON_SET_STANDARD_PHONE	((LPCTSTR)LangLoadString(m_hmInstance, IDS_ICON_SET_STANDARD_PHONE))
#define SSS_TEXT_ICON_SET_IN_OUT_BUTTON		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ICON_SET_IN_OUT_BUTTON))
#define SSS_TEXT_ICON_SET_MOBILE_PHONE		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ICON_SET_MOBILE_PHONE))
#define SSS_TEXT_ICON_SET_PDA				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ICON_SET_PDA))
#define SSS_TEXT_ICON_SET_TRAFFIC			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ICON_SET_TRAFFIC))
#define SSS_TEXT_ICON_SET_TUBE				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ICON_SET_TUBE))

// Actions dialog
#define SSS_TEXT_ACTIONS_TITLE				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTIONS_TITLE))
#define SSS_TEXT_ACTIONS_TAP				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTIONS_TAP))
#define SSS_TEXT_ACTIONS_TAH				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTIONS_TAH))
#define SSS_TEXT_ACTIONS_TODAY_ICON_TAP		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTIONS_TODAY_ICON_TAP))
#define SSS_TEXT_ACTIONS_TODAY_ICON_TAH		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTIONS_TODAY_ICON_TAH))
#define SSS_TEXT_ACTIONS_BUTTON				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTIONS_BUTTON))

// Actions
#define SSS_TEXT_ACTION_REFRESH				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTION_REFRESH))
#define SSS_TEXT_ACTION_SHOW_POPUP			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTION_SHOW_POPUP))
#define SSS_TEXT_ACTION_OPTIONS				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTION_OPTIONS))
#define SSS_TEXT_ACTION_SWITCH_SIM			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTION_SWITCH_SIM))
#define SSS_TEXT_ACTION_TOGGLE_RADIO		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTION_TOGGLE_RADIO))
#define SSS_TEXT_ACTION_PHONE_SETTINGS		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ACTION_PHONE_SETTINGS))

// Security dialog
#define SSS_TEXT_SECURITY_TITLE				((LPCTSTR)LangLoadString(m_hmInstance, IDS_SECURITY_TITLE))
#define SSS_TEXT_SECURITY_STATIC_DEFAULT	((LPCTSTR)LangLoadString(m_hmInstance, IDS_SECURITY_STATIC_DEFAULT))
#define SSS_TEXT_SECURITY_MAKE_DEFAULT		((LPCTSTR)LangLoadString(m_hmInstance, IDS_SECURITY_MAKE_DEFAULT))
#define SSS_TEXT_SECURITY_STATIC_AUTO_PIN	((LPCTSTR)LangLoadString(m_hmInstance, IDS_SECURITY_STATIC_AUTO_PIN))
#define SSS_TEXT_SECURITY_AUTO_PIN_INIT		((LPCTSTR)LangLoadString(m_hmInstance, IDS_SECURITY_AUTO_PIN_INIT))
#define SSS_TEXT_SECURITY_AUTO_PIN_RADIOON	((LPCTSTR)LangLoadString(m_hmInstance, IDS_SECURITY_AUTO_PIN_RADIOON))
#define SSS_TEXT_SECURITY_CREATE_PIN		((LPCTSTR)LangLoadString(m_hmInstance, IDS_SECURITY_CREATE_PIN))
#define SSS_TEXT_SECURITY_CHANGE_PIN		((LPCTSTR)LangLoadString(m_hmInstance, IDS_SECURITY_CHANGE_PIN))

// PIN entry dialog
#define SSS_TEXT_PINENTRY_TITLE				((LPCTSTR)LangLoadString(m_hmInstance, IDS_PINENTRY_TITLE))
#define SSS_TEXT_PINENTRY_SPECIFY_PIN		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PINENTRY_SPECIFY_PIN))
#define SSS_TEXT_PINENTRY_STATIC_ENTER_PIN	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PINENTRY_STATIC_ENTER_PIN))
#define SSS_TEXT_PINENTRY_STATIC_CURRENT	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PINENTRY_STATIC_CURRENT))
#define SSS_TEXT_PINENTRY_STATIC_NEW		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PINENTRY_STATIC_NEW))
#define SSS_TEXT_PINENTRY_STATIC_CONFIRM	((LPCTSTR)LangLoadString(m_hmInstance, IDS_PINENTRY_STATIC_CONFIRM))

// Language dialog
#define SSS_TEXT_LANGUAGE_TITLE				((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_TITLE))
#define SSS_TEXT_LANGUAGE_AVAILABLE			((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_AVAILABLE))
#define SSS_TEXT_LANGUAGE_DEFAULT			((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_DEFAULT))
#define SSS_TEXT_LANGUAGE_CURRENT			((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_CURRENT))
#define SSS_TEXT_LANGUAGE_LOAD				((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_LOAD))
#define SSS_TEXT_LANGUAGE_SET_DEFAULT		((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_SET_DEFAULT))
#define SSS_TEXT_LANGUAGE_ID				((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_ID))
#define SSS_TEXT_LANGUAGE_NAME				((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_NAME))
#define SSS_TEXT_LANGUAGE_DEFAULT_NOT_FOUND	((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_DEFAULT_NOT_FOUND))
#define SSS_TEXT_LANGUAGE_DEFAULT_NOT_DEFINED ((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_DEFAULT_NOT_DEFINED))
#define SSS_TEXT_LANGUAGE_CURRENT_BUILT_IN	((LPCTSTR)LangLoadString(m_hmInstance, IDS_LANGUAGE_CURRENT_BUILT_IN))

// About dialog
#define SSS_TEXT_ABOUT_TITLE				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_TITLE))
#define SSS_TEXT_ABOUT_FILE_ERROR			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_FILE_ERROR))
#define SSS_TEXT_ABOUT_SUPPORT				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_SUPPORT))
#define	SSS_TEXT_ABOUT_LICENCE_TYPE			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_LICENCE_TYPE))
#define SSS_TEXT_ABOUT_TRIAL_DAY_REMAINING	((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_TRIAL_DAY_REMAINING))
#define SSS_TEXT_ABOUT_TRIAL_DAYS_REMAINING	((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_TRIAL_DAYS_REMAINING))
#define SSS_TEXT_ABOUT_TRIAL_EXPIRED		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_TRIAL_EXPIRED))
#define SSS_TEXT_ABOUT_SIM_INFORMATION		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_SIM_INFORMATION))
#define SSS_TEXT_ABOUT_REGISTRATION			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_REGISTRATION))
#define SSS_TEXT_ABOUT_PHONEBOOK			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_PHONEBOOK))
#define SSS_TEXT_ABOUT_LOCATION				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_LOCATION))
#define SSS_TEXT_ABOUT_TOTAL_ENTRIES		((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_TOTAL_ENTRIES))
#define SSS_TEXT_ABOUT_USED_ENTRIES			((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_USED_ENTRIES))
#define SSS_TEXT_ABOUT_PENDING_VERIFICATION	((LPCTSTR)LangLoadString(m_hmInstance, IDS_ABOUT_PENDING_VERIFICATION))

// Registration dialog
#define SSS_TEXT_REGISTRATION_INFORMATION	((LPCTSTR)LangLoadString(m_hmInstance, IDS_REGISTRATION_INFORMATION))
#define SSS_TEXT_REGISTRATION_CAREFUL		((LPCTSTR)LangLoadString(m_hmInstance, IDS_REGISTRATION_CAREFUL))
#define SSS_TEXT_REGISTRATION_OK_TO_RETURN	((LPCTSTR)LangLoadString(m_hmInstance, IDS_REGISTRATION_OK_TO_RETURN))
#define SSS_TEXT_REGISTRATION_TAKE_EFFECT	((LPCTSTR)LangLoadString(m_hmInstance, IDS_REGISTRATION_TAKE_EFFECT))
#define SSS_TEXT_REGISTRATION_TODAY_PLUGIN	((LPCTSTR)LangLoadString(m_hmInstance, IDS_REGISTRATION_TODAY_PLUGIN))

// Information dialog
#define SSS_TEXT_DEVICE_INFORMATION			((LPCTSTR)LangLoadString(m_hmInstance, IDS_DEVICE_INFORMATION))
#define SSS_TEXT_SIM_INFORMATION			((LPCTSTR)LangLoadString(m_hmInstance, IDS_SIM_INFORMATION))
#define SSS_TEXT_MANUFACTURER				((LPCTSTR)LangLoadString(m_hmInstance, IDS_MANUFACTURER))
#define SSS_TEXT_MODEL						((LPCTSTR)LangLoadString(m_hmInstance, IDS_MODEL))
#define SSS_TEXT_REVISION					((LPCTSTR)LangLoadString(m_hmInstance, IDS_REVISION))
#define SSS_TEXT_IMEI_NUMBER				((LPCTSTR)LangLoadString(m_hmInstance, IDS_IMEI_NUMBER))
#define SSS_TEXT_IMSI_NUMBER				((LPCTSTR)LangLoadString(m_hmInstance, IDS_IMSI_NUMBER))
#define SSS_TEXT_ICCID_NUMBER				((LPCTSTR)LangLoadString(m_hmInstance, IDS_ICCID_NUMBER))

// Phonebook locations
#define SSS_TEXT_PHONEBOOK_SIM				((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONEBOOK_SIM))
#define SSS_TEXT_PHONEBOOK_OWN				((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONEBOOK_OWN))
#define SSS_TEXT_PHONEBOOK_RECENT			((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONEBOOK_RECENT))
#define SSS_TEXT_PHONEBOOK_FIXED			((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONEBOOK_FIXED))
#define SSS_TEXT_PHONEBOOK_EMERGENCY		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONEBOOK_EMERGENCY))
#define SSS_TEXT_PHONEBOOK_UNKNOWN			((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONEBOOK_UNKNOWN))
#define SSS_TEXT_PHONEBOOK_UNAVAILABLE		((LPCTSTR)LangLoadString(m_hmInstance, IDS_PHONEBOOK_UNAVAILABLE))

#endif // INC_SSS_LANG_H
