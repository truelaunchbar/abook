#pragma		 once

#include "TabSwitcher.h"

struct TAB_DATA
{
	UINT			tabID;
	UINT			caption;
	UINT			label;
	UINT			imgID;
	CTrueDialog*	dlg;
};

#define BEGIN_TABS(tabCount, defTab)	TAB_DATA m_tabs[tabCount];\
	void OnInitDialog()\
	{\
		UINT actTab = defTab;
#define DEF_TAB(tabIndex, tabClass, id, tabCaption, tabLabel, tabImage)		m_tabs[tabIndex].caption = tabCaption;\
																				m_tabs[tabIndex].label = tabLabel;\
																				m_tabs[tabIndex].tabID = id;\
																				m_tabs[tabIndex].imgID = tabImage;\
																				m_tabs[tabIndex].dlg = new tabClass(this);
#define END_TABS 	TSWITEM tswItem;\
					int tabCount = sizeof(m_tabs) / sizeof(m_tabs[0]);\
					tswItem.hInst	= g_hInst;\
					for(int i=0; i < tabCount; i++)\
					{\
						TCHAR label[255];\
						LoadString(m_hResInst, m_tabs[i].label, label, 255);\
						tswItem.imgID	= m_tabs[i].imgID;\
						tswItem.label	= label;\
						tswItem.lParam	= m_tabs[i].tabID;\
						tswItem.state	= TSW_STATE_ENABLED;\
						if(actTab == m_tabs[i].tabID)\
						{\
							tswItem.state |= TSW_STATE_SELECTED;\
						}\
						SendDlgItemMessage(m_hWnd, IDC_TABS, TSWM_INSERTITEM, 0, (LPARAM) &tswItem);\
					}\
					OnTabSwitch(TAB_OPTIONS);\
	}\
	void OnTabSwitch(int tabID)\
	{\
		int tabCount = sizeof(m_tabs) / sizeof(m_tabs[0]);\
		RECT rcPage;\
		GetWindowRect(GetDlgItem(m_hWnd, IDC_RECT), &rcPage);\
		MapWindowPoints(NULL, m_hWnd, (LPPOINT) &rcPage, 2);\
		for(int i=0; i < tabCount; i++)\
		{\
			if(m_tabs[i].tabID == tabID)\
			{\
				if(m_tabs[i].dlg)\
				{\
					TCHAR caption[255];\
					LoadString(m_hResInst, m_tabs[i].caption, caption, 255);\
					SetDlgItemText(m_hWnd, IDC_PAGELABEL, caption);\
					if(!m_tabs[i].dlg->m_hWnd)\
					{\
						m_tabs[i].dlg->Create(m_hWnd, m_hResInst);\
						SetWindowPos(m_tabs[i].dlg->m_hWnd, NULL, rcPage.left, rcPage.top, rcPage.right - rcPage.left, rcPage.bottom - rcPage.top, SWP_NOZORDER);\
					}\
					ShowWindow(m_tabs[i].dlg->m_hWnd, SW_SHOW);\
				}\
				break;\
			}\
		}\
		for(int i=0; i < tabCount; i++)\
		{\
			if(m_tabs[i].tabID != tabID)\
			{\
				if(m_tabs[i].dlg && m_tabs[i].dlg->m_hWnd)\
				{\
					ShowWindow(m_tabs[i].dlg->m_hWnd, SW_HIDE);\
				}\
			}\
		}\
	}\
	void OnCommand(DWORD itemID)\
	{\
		switch(LOWORD(itemID))\
		{\
		case IDC_TABS:\
			if(HIWORD(itemID) == TSWN_CHANGED)\
			{\
				UINT tab = SendDlgItemMessage(m_hWnd, IDC_TABS, TSWM_GETSELECTED, 0, 0);\
				OnTabSwitch(tab);\
			}\
			break;\
		}\
	}\
	BOOL OnEndDialog(void)\
	{\
		int tabCount = sizeof(m_tabs) / sizeof(m_tabs[0]);\
		for(int i=0; i < tabCount; i++)\
		{\
			if(m_tabs[i].dlg && m_tabs[i].dlg->m_hWnd && !m_tabs[i].dlg->OnEndDialog())\
			{\
				return FALSE;\
			}\
		}\
		return TRUE;\
	}
