#pragma once

#include "abookbtn.h"

class CEditProgDlg : public CXUIDialog
{
	CXUIComboBox*	m_ctlFields;
public:
	CProgram	m_prog;

	CEditProgDlg(CXUIEngine* engine);
	virtual ~CEditProgDlg(void);

	XUI_BEGIN_BIND_MAP
		XUI_BIND_ITEM(L"field",	L"combobox",	m_ctlFields)
	XUI_END_BIND_MAP

	virtual void OnInitDialog();
	virtual BOOL OnEndDialog(UINT code);
};
