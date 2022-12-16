//////////////////////////////////////////////////////////////////////
//
// SIMStatusswitcher.h: Definitions for the SIM Status Switcher
//
//////////////////////////////////////////////////////////////////////
/********************************************************************
	created:	2005/04/17
	created:	17:4:2005   0:58
	filename: 	D:\Development\Projects\SIM Status Switcher\Embedded\SIMStatusSwitcher.h
	file path:	D:\Development\Projects\SIM Status Switcher\Embedded
	file base:	SIMStatusSwitcher
	file ext:	h
	author:		The WiViT Team
	
	purpose:	Definitions for the SIM Status Switcher
*********************************************************************/
#ifndef	INC_SSS_SIMSTATUSSWITCHER_H
#define INC_SSS_SIMSTATUSSWITCHER_H

HWND APIENTRY		InitializeCustomItem(TODAYLISTITEM *ptli, HWND hWndParent);
LRESULT CALLBACK	WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI		CustomItemOptionsDlgProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

#endif // INC_SSS_SIMSTATUSSWITCHER_H