#include "globals.h"
#include ".\alphagroupbtn.h"
#include "abookbtn.h"
#include "RecordBtn.h"
#include "resource.h"
#include "xuistrings.h"

CAlphaGroupBtn::CAlphaGroupBtn(CAddressBook* parent, TCHAR alpha)
{
	m_isChanged = FALSE;
	m_alpha = alpha;
	parent->GetAlphaGroupText(alpha, m_Name, 80);
	m_parent = parent;
	m_changesHandle = NULL;
}

CAlphaGroupBtn::~CAlphaGroupBtn(void)
{
}

void CAlphaGroupBtn::GetChildUID(LPWSTR uid)
{
	lstrcpy(uid, TEXT("letter: "));
	uid[7] = m_alpha;
}

BOOL CAlphaGroupBtn::isMenu(void)
{
	return TRUE;
}

HANDLE CAlphaGroupBtn::GetChangesHandle() 
{ 
	if(!m_changesHandle)
	{
		m_changesHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	return m_changesHandle; 
}

void CAlphaGroupBtn::PrepareChanges(HANDLE hChanges)
{
	ResetEvent(hChanges);
}

void CAlphaGroupBtn::CloseChangesHandle(HANDLE hChanges)
{
	CloseHandle(hChanges);
	m_changesHandle = NULL;
}

UINT CAlphaGroupBtn::GetMenuFlags(void)
{
	return MENU_F_NORENAME | MENU_F_VARHEIGHT;
}

struct AB_ALPHA_ENUM_DATA
{
	std::vector<UINT>	recs;
	int					lastID;
};

BOOL CAlphaGroupBtn::FirstChildData(CHILDS_DATA* data)
{
	AB_ALPHA_ENUM_DATA* enumData = new AB_ALPHA_ENUM_DATA;
	enumData->lastID = 0;
	enumData->recs = m_parent->GetRecordsByFirstLetter(m_alpha);
	if(!enumData->recs[0])
	{
		delete enumData;
		return FALSE;
	}
	data->childData = (LPVOID) enumData->recs[enumData->lastID];
	data->enumData = (LPVOID) enumData;
	if(data->Name)
	{
		wsprintf(data->Name, TEXT("rec:%d"), enumData->recs[enumData->lastID]);
	}
	return TRUE;
}

BOOL CAlphaGroupBtn::NextChildData(CHILDS_DATA* data)
{
	AB_ALPHA_ENUM_DATA* enumData = (AB_ALPHA_ENUM_DATA*) data->enumData;
	enumData->lastID++;
	if (enumData->lastID >= (int) enumData->recs.size())
	{
		delete enumData;
		return FALSE;
	}
	data->childData = (LPVOID) enumData->recs[enumData->lastID];
	data->enumData = (LPVOID) enumData;
	if(data->Name)
	{
		wsprintf(data->Name, TEXT("rec:%d"), enumData->recs[enumData->lastID]);
	}
	return TRUE;
}

BOOL CAlphaGroupBtn::FreeChildData(CHILDS_DATA* data)
{
	return TRUE;
}

BOOL CAlphaGroupBtn::CreateChild(CREATE_CHILD_DATA* childData)
{
	UINT id = (UINT) childData->childData;
	CRecordBtn* btn = new CRecordBtn(m_parent, id, this);
	childData->child = btn;
	return TRUE;
}


UINT CAlphaGroupBtn::ReadChanges(void)
{
	if(m_parent->GetGroupType() != GROUP_TYPE_ALPHA)
	{
		return 2;
	}
	UINT res = 0;
	std::vector<UINT> recs = m_parent->GetRecordsByFirstLetter(m_alpha);
	if(recs.empty())
	{
		res = 2;
	} else if(m_isChanged)
	{
		res = 1;
	}
	return res;
}

BOOL CAlphaGroupBtn::isDeletedRecord(UINT recID)
{
	CRecord rec;
	if(!m_parent->GetRecord(recID, rec))
	{
		return TRUE;
	}
	std::wstring val = rec.GetFieldValue(L"displayname");
	while(!val.empty() && !IsCharAlphaNumeric(val[0])) val.erase(0);
	if(val.empty())
	{
		val = L"?";
	}
	if(IsCharNumeric(val[0]))
	{
		val = L"1";
	}
	WCHAR str[2];
	str[0] = m_alpha;
	str[1] = 0;
	if(StrCmpNI(str, val.c_str(), 1))
	{
		return TRUE;
	}
	return FALSE;
}

void CAlphaGroupBtn::NotfyDelete(void)
{
	if(ReadChanges() == 2)
	{
		m_parent->NotfyDelete();
	} else
	{
		if(m_changesHandle)
		{
			SetEvent(m_changesHandle);
		}
	}
}

void CAlphaGroupBtn::OnDraw(HDC hDC, LPRECT rcItem)
{
	POINT ptIcon;
	m_container->GetIconPoint(&ptIcon);
	int szIcon = m_container->GetIconSize();

	HFONT font = ::CreateFont(	szIcon-2,				// height of font
								0,						// average character width
								0,						// angle of escapement
								0,						// base-line orientation angle
								700,					// font weight
								FALSE,					// italic attribute option
								FALSE,					// underline attribute option
								FALSE,					// strikeout attribute option
								DEFAULT_CHARSET,        // character set identifier
								OUT_DEFAULT_PRECIS,		// output precision
								CLIP_DEFAULT_PRECIS,	// clipping precision
								5,						// output quality
								FF_DONTCARE,			// pitch and family
								TEXT("Verdana"));       // typeface name
	RECT rcDraw;
	rcDraw.left = rcItem->left + ptIcon.x;
	rcDraw.top = rcItem->top + ptIcon.y;
	rcDraw.right = rcDraw.left + szIcon;
	rcDraw.bottom = rcDraw.top + szIcon;
	WCHAR txt[2];
	if(m_alpha == TEXT('1'))
	{
		txt[0] = TEXT('1');
	} else
	{
		txt[0] = m_alpha;
	}
	txt[1] = 0;
	CharUpper(txt);

	m_container->drawText(hDC, txt, -1, &rcDraw, DT_SINGLELINE | DT_CENTER | DT_VCENTER, 
		DTF_DRAWGLOW | DTF_GLOWCOLOR | DTF_COLOR | DTF_GLOWSIZE, font, RGB(255, 255, 255), 0, RGB(0, 0, 0), 2);

	DeleteObject(font);
}

void CAlphaGroupBtn::NotifyChange(void)
{
	WCHAR newName[80];
	newName[0] = 0;
	m_parent->GetAlphaGroupText(m_alpha, newName, 80);
	if(StrCmp(newName, m_Name))
	{
		lstrcpy(m_Name, newName);
		m_isChanged = TRUE;
		if(m_changesHandle)
		{
			SetEvent(m_changesHandle);
		}
	}
}

BOOL CAlphaGroupBtn::SupportSortMenu(void)
{
	return TRUE;
}

void CAlphaGroupBtn::CompareButtons(COMPARE_BUTTONS_DATA* cpBtns)
{
	if(cpBtns->btn1->isMenu() && !cpBtns->btn2->isMenu())
	{
		cpBtns->result = -1;
	} else if(!cpBtns->btn1->isMenu() && cpBtns->btn2->isMenu())
	{
		cpBtns->result = 1;
	} else	if(cpBtns->fnc == AMID_NAME)
	{
		TCHAR txt1[500];
		TCHAR txt2[500];
		cpBtns->btn1->GetButtonText(txt1);
		cpBtns->btn2->GetButtonText(txt2);
		cpBtns->result = StrCmpI(txt1, txt2);
	} else
	{
		cpBtns->result = 0;
	}
}

BOOL CAlphaGroupBtn::QueryArrangeMenu(UINT idx, ARRANGE_MENU_DATA* qamData)
{
	switch(idx)
	{
	case AMID_NAME-1:
		qamData->sortID = AMID_NAME;
		StringCchCopy(qamData->name, 100, m_parent->m_xui.getStringDef(XUI_SORT_NAME));
		return TRUE;
	}
	return FALSE;
}

BOOL CAlphaGroupBtn::SupportGlassMenu()
{
	return TRUE;
}

BOOL CAlphaGroupBtn::applyMargins()
{
	return FALSE;
}
