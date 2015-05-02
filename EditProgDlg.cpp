#include "globals.h"
#include "EditProgDlg.h"
#include "xuistrings.h"

CEditProgDlg::CEditProgDlg(CXUIEngine* engine) : CXUIDialog(L"res:prog.xml", engine)
{
}

CEditProgDlg::~CEditProgDlg(void)
{
}

void CEditProgDlg::OnInitDialog()
{
	CXUIDialog::OnInitDialog();

	BOOL found = FALSE;

	m_ctlFields->addItem(0, m_engine->getStringDef(XUI_CUSTOMFIELD));
	m_ctlFields->value_INT(0);

	for(int i=0; g_defText[i].fldName; i++)
	{
		if(g_defText[i].suppotsCustomCmd)
		{
			m_ctlFields->addItem(i + 1, m_engine->getStringDef(g_defText[i].strID, g_defText[i].attr, g_defText[i].defText));
			if(g_defText[i].fldName == m_prog.getField())
			{
				m_ctlFields->value_INT(i + 1);
				found = TRUE;
			}
		}
	}

	if(!found)
	{
		find(L"custom")->value_STR(m_prog.getField().c_str());
	}
	
	find(L"cmdline")->value_STR(m_prog.getCmdLine().c_str());
}

BOOL CEditProgDlg::OnEndDialog( UINT code )
{
	if(code == IDOK)
	{
		m_prog.setCmdLine(find(L"cmdline")->value_STRdef(L""));
		int idx = m_ctlFields->value_INT();
		if(!idx)
		{
			m_prog.setField(find(L"custom")->value_STRdef(L""));
		} else
		{
			m_prog.setField(g_defText[idx - 1].fldName);
		}
	}
	return TRUE;
}
