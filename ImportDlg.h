#pragma once
#include "abookbtn.h"
#include "xuicustomlist.h"

#define IMPORT_ACT_NONE		0
#define IMPORT_ACT_IMPORT	1
#define IMPORT_ACT_UPDATE	2

class CImportDlg : public CXUIDialog
{
	std::vector<std::wstring>	m_files;
	CAddressBook*				m_btn;
	CXUICustomList*				m_ctlList;
	int							m_itemHeight;
	int							m_textHeight;
	HFONT						m_fntBold;
	HFONT						m_fntNormal;
	HFONT						m_fntCaption;
	HFONT						m_fntLinks;
	int							m_labelsWidth;
	int							m_captionHeight;
	int							m_linksWidth;
	int							m_linksHeight;
	HICON						m_hIcon;
public:
	std::vector<CRecord*>		m_records;
	std::vector<UINT>			m_actions;

	CImportDlg(CXUIEngine* engine, CAddressBook* btn);
	virtual ~CImportDlg(void);

	XUI_BEGIN_BIND_MAP
		XUI_BIND_ITEM(L"list",	L"customlist",	m_ctlList)
	XUI_END_BIND_MAP

	XUI_BEGIN_EVENT_MAP
		XUI_HANDLE_CUSTLST_DRAWITEM		(L"list",	OnDrawRecord)
		XUI_HANDLE_CUSTLST_ITEMCLICK	(L"list",	OnItemClick)
		XUI_HANDLE_CUSTLST_ITEMDBLCLICK	(L"list",	OnItemDblClick)
	XUI_END_EVENT_MAP

	virtual void OnInitDialog();
	virtual BOOL OnEndDialog(UINT code);

	void clear();
	void addFile(LPCWSTR fileName);

private:
	void import();
	void import(LPCWSTR fileName);
	void updateImportBtn();
	void fillList();
	void calcItemHeight();

	BOOL OnDrawRecord(HDC hdc, CUSTOM_LIST_DRAWITEM* di);
	BOOL OnItemClick(HWND hWnd, CUSTOM_LIST_CLICK* clc);
	BOOL OnItemDblClick(HWND hWnd, CUSTOM_LIST_CLICK* clc);
};
