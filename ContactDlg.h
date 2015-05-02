#pragma once
#include "abookbtn.h"

class CContactDlg : public CXUIDialog
{
	CXUIFreeDraw*	m_ctlPhoto;
	CAddressBook*	m_btn;
	IRecordParent*	m_recParent;

	HICON			m_icon;
public:
	CRecord		m_rec;
	std::vector<std::wstring> m_categories;

	CContactDlg(CXUIEngine* engine, CAddressBook* btn, IRecordParent* recParent);
	~CContactDlg(void);

	virtual void OnInitDialog();
	virtual BOOL OnEndDialog(UINT code);

	XUI_BEGIN_BIND_MAP
		XUI_BIND_ITEM	(L"photoPreview",	L"freedraw",	m_ctlPhoto)
	XUI_END_BIND_MAP

	XUI_BEGIN_EVENT_MAP
		XUI_HANDLE_CLICKED		(L"btnCategory",	OnSelectCategory)
		XUI_HANDLE_FREEDRAW		(L"photoPreview",	OnDrawPhoto)
		XUI_HANDLE_FSSELECTED	(L"browseFile",		OnPhotoSelected)
		XUI_HANDLE_CLICKED		(L"btnDeletePhoto",	OnDeletePhoto)
	XUI_END_EVENT_MAP

private:
	int		phoneTypeToInt(LPCWSTR val);
	LPCWSTR phoneTypeToStr(int val);
	void	loadPhoto();

	BOOL	OnSelectCategory();
	BOOL	OnDrawPhoto(HDC hdc, LPRECT rcDraw);
	BOOL	OnPhotoSelected(LPWSTR fileName);
	BOOL	OnDeletePhoto();
};
